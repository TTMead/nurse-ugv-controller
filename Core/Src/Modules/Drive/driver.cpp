/******** nurse controller **********
 *
 * @file driver.cpp
 * @desc Controls the driving of the motor
 *
 * @author Evan Devoy
 *
 ************************************/


#include "driver.hpp"
#include "stm32f4xx_hal.h"
#include "eORB.hpp"
#include "serial.h"
#include "motor_driver.h"
#include "string.h"



		/* **** Settings **** */

/* Speed Settings */
#define SPEED_LOW 0.1
#define SPEED_MED 1
#define SPEED_HIGH 1.5
float SPEED;

/* Control Settings */
#define Kp 0.7 //Straight: 0.18
#define Ki 0
#define Kd 3.5 //Straight: 2

/* IR Settings */
#define ON_WHITE_TRACK
#define BLACKTHRESHOLD 2000
#define WHITETHRESHOLD 100

/* Algorithm Settings (Don't comment the one to use) */
// #define PID_V2
#define PID_V1
// #define BANG_BANG

/* PWM Settings */
#define MOTOR_MAX_PWM 1000


		/* **** Mappings **** */

/* GPIO mappings */
#define LEFTMOTORIN1 GPIO_PIN_6
#define LEFTMOTORIN2 GPIO_PIN_7
#define RIGHTMOTORIN3 GPIO_PIN_8
#define RIGHTMOTORIN4 GPIO_PIN_9
#define ON GPIO_PIN_SET
#define OFF GPIO_PIN_RESET

/* Direction mappings */
#define FORWARD 1
#define BACKWARD 0
#define STOP 2



		/* **** Local variables **** */

int sensor_sub; // Subscription to the sensor topic
sensor_values_t sensor; // Memory location of received sensor values
int motorSpeed[2] = {0,0}; // The speeds of the left (0) and right (1) motors
int print_counter;

PID_t pidLine;
float previousPosition;
uint64_t previousTime;
float previousError;

bool isDriving;


		/* **** Motor Directional Functions **** */

void leftMotorGPIO(int direction) {
	
	if (direction == FORWARD)
	{
		HAL_GPIO_WritePin(GPIOC, LEFTMOTORIN1, ON);
		HAL_GPIO_WritePin(GPIOC, LEFTMOTORIN2, OFF);
	}
	else if (direction == BACKWARD)
	{
		HAL_GPIO_WritePin(GPIOC, LEFTMOTORIN1, OFF);
		HAL_GPIO_WritePin(GPIOC, LEFTMOTORIN2, ON);
	}
	else 
	{
		HAL_GPIO_WritePin(GPIOC, LEFTMOTORIN1, OFF);
		HAL_GPIO_WritePin(GPIOC, LEFTMOTORIN2, OFF);
	}
	
}

void rightMotorGPIO(int direction) {
	
	if (direction == FORWARD)
	{
		HAL_GPIO_WritePin(GPIOC, RIGHTMOTORIN3, OFF);
		HAL_GPIO_WritePin(GPIOC, RIGHTMOTORIN4, ON);
	}
	else if (direction == BACKWARD)
	{
		HAL_GPIO_WritePin(GPIOC, RIGHTMOTORIN3, ON);
		HAL_GPIO_WritePin(GPIOC, RIGHTMOTORIN4, OFF);
	}
	else 
	{
		HAL_GPIO_WritePin(GPIOC, RIGHTMOTORIN3, OFF);
		HAL_GPIO_WritePin(GPIOC, RIGHTMOTORIN4, OFF);
	}
	
}



/*
 * Calculates the position of the rover given a set of sensor values
 */
float linePosition(sensor_values_t x){
	float sum = x.s0 + x.s1 + x.s2 + x.s3 + x.s4 + x.s5 + x.s6 + x.s7;
	return 1000*(1.0*x.s0 + 2.0*x.s1 + 3.0*x.s2 + 4.0*x.s3 + 5.0*x.s4 + 6.0*x.s5 + 7.0*x.s6 + 8.0*x.s7)/(sum);
}


/*
 * Cyclic executive
 */
static void run() {

	if (!isDriving) {
		HAL_Delay(15);
		return;
	}

	// Receive sensor data
	if (check(sensor_sub))
	{
		copy(sensor_sub, &sensor);
	}

	#ifdef PID_V2
		// https://www.robotshop.com/community/blog/show/pid-tutorials-for-line-following

		// Current position of the robot
		float position = sensor;

		// This is our goal (Corresponds to perfect placement) - may be a different value
		int setpoint = 2500;

		// Calculate the error in position (Aim to make this zero - then the robot will follow the line smoothly)
		// A value towards 0 indicates the robot is to far to the left, a value towards 5000 indicates the robot is to far to the right
		int error = setpoint - position; // FOR BLACK ON WHITE: position - setpoint

		// Use PID to calculate motorspeed
		int motorSpeed = Kp * error + Kd * (error - lastError);
		lastError = error;

		// Any PWM motor value 0-255 should work
		int leftMotorSpeed = LEFTBASESPEED - motorSpeed; // FOR BLACK ON WHITE: LEFTBASESPEED + motorSpeed
		int rightMotorSpeed = RIGHTBASESPEED + motorSpeed; // FOR BLACK ON WHITE: RIGHTBASESPEED - motorSpeed

		// Clamp the motor speed values to ensure they are operating within the correct range
		if (leftMotorSpeed > LEFTMAXSPEED) leftMotorSpeed = LEFTMAXSPEED;
		if (rightMotorSpeed > RIGHTMAXSPEED) rightMotorSpeed = RIGHTMAXSPEED;
		if (leftMotorSpeed < 0) leftMotorSpeed = 0;
		if (rightMotorSpeed < 0) rightMotorSpeed = 0;

	#endif

	#ifdef PID_V1

		// Get change in time since last loop call
		float dt = HAL_GetTick() - previousTime;

		// Get current pos and derivative
		float position = linePosition(sensor);
		// float positionDot = (position - previousPosition);

		// Setpoint value
		float setPoint = 4500;

		// Calculate desired yaw effort
#ifdef ON_WHITE_TRACK
		int yawEffort = (int) (-Kp*(position - setPoint) + (-Kd*((position - setPoint) - previousError)));
#else
		int yawEffort = (int) (Kp*(position - setPoint) + (Kd*((setPoint - position) - previousError)));
#endif
		//int yawEffort = (int) pid_calculate(&pidLine, setPoint, position, positionDot, dt/1000.0);

		// Update previous position and previous time
		previousPosition = position;
		previousTime = previousTime + dt;
		previousError = (position - setPoint);

		// Set motor efforts and clamp
		motorSpeed[0] = (int) (SPEED*clamp((500.0 + yawEffort), 0.0, 1000.0));
		motorSpeed[1] = (int) (SPEED*clamp((500.0 - yawEffort), 0.0, 1000.0));

		print_counter = print_counter + 1;
		if (print_counter > 10) {
			print_counter = 0;
			ROVER_PRINTLN("[Driver] Position %d, Yaw Effort %d, Left Motor %d, Right Motor %d", (int)position, (int)yawEffort, (int)motorSpeed[0], (int)motorSpeed[1]);
		}

		// Send motor speeds to PWM
		set_left_motor_speed(motorSpeed[0]);
	    set_right_motor_speed(motorSpeed[1]);

	#endif

	#ifdef BANG_BANG
		// Bang-Bang Straight Path
		if (sensor.s4 < WHITETHRESHOLD)
		{
			// Turn Left
			motorSpeed[0] = 40;
			motorSpeed[1] = 60;
		}
		else if (sensor.s3 < WHITETHRESHOLD)
		{
			// Turn Right
			motorSpeed[0] = 60;
			motorSpeed[1] = 40;
		}
		else
		{
			// Go Straight
			motorSpeed[0] = 50;
			motorSpeed[1] = 50;
		}

		// Print motor speed
		//ROVER_PRINTLN("[Driver] Left Motor: %d, Right Motor: %d", motorSpeed[0], motorSpeed[1]);

		// Print sensor/direction outputs
		//if (1) {
		//	char dir; if (sensor.s4 < THRESHOLD) {dir = 'L';} else if (sensor.s3 < THRESHOLD) {dir = 'R';} else {dir = 'S';}
		//	ROVER_PRINTLN("[Driver] %d %d %d %d %d %d %d %d %c", (sensor.s0 < THRESHOLD), (sensor.s1 < THRESHOLD), (sensor.s2 < THRESHOLD), (sensor.s3 < THRESHOLD), (sensor.s4 < THRESHOLD), (sensor.s5 < THRESHOLD), (sensor.s6 < THRESHOLD), (sensor.s7 < THRESHOLD), dir);
		//}
	#endif

	// Controls the frequency of the cyclic executive
	HAL_Delay(15);
}


/*
 * Initialisation function
 */
void StartDriver(void *argument) {
	// Sensor data subscription
	sensor_sub = subscribe(TOPIC_SENSORS);

	SPEED = SPEED_LOW;

	// Initialise hardwares
	leftMotorGPIO(FORWARD);
	rightMotorGPIO(FORWARD);
	initialise_motor_pwm();

	pid_init(&pidLine);
	pid_set_parameters(&pidLine, Kp, Ki, Kd, 1000, -(MOTOR_MAX_PWM/2), (MOTOR_MAX_PWM));

	previousPosition = 4.5;
	previousTime = HAL_GetTick();

	//int lastError = 0;
	previousError = 0;

	print_counter = 0;

	isDriving = false;

	for (;;)
	{
		run();
	}

	osThreadTerminate(NULL);
}


/*
 * Console Interface
 */
int driver_main(int argc, const char *argv[]) {

	if (argc < 2) {
		ROVER_PRINTLN("[Driver] Please enter a command: Stop, Start");
		return 1;
	}

	if (!strcmp(argv[1], "stop")) {
		isDriving = false;
		set_left_motor_speed(0);
		set_right_motor_speed(0);
		HAL_Delay(50);
		set_left_motor_speed(0);
		set_right_motor_speed(0);
		HAL_Delay(50);
		set_left_motor_speed(0);
		set_right_motor_speed(0);
		return 0;
	}

	if (argc < 3) {
		ROVER_PRINTLN("[Driver] Please enter a command an option: low, med, high");
		return 1;
	}

	if (!strcmp(argv[1], "start")) {
		isDriving = true;
		if (!strcmp(argv[2], "low")) {
					SPEED = SPEED_LOW;
					return 0;
		}
		if (!strcmp(argv[2], "med")) {
			SPEED = SPEED_MED;
			return 0;
		}
		if (!strcmp(argv[2], "high")) {
			SPEED = SPEED_HIGH;
			return 0;
		}
		isDriving = false;
		return 1;
	}







	return 0;
}
