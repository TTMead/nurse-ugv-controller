/******** nurse controller **********
 *
 * @file driver.cpp
 * @desc Controls the driving of the motor
 *
 * @author Evan Devoy
 * @author Timothy Mead
 *
 ************************************/

#include <navigation.hpp>
#include "driver.hpp"
#include "stm32f4xx_hal.h"
#include "eORB.hpp"
#include "serial.h"
#include "motor_driver.h"
#include "string.h"



		/* **** Settings **** */

/* Speed Settings */
#define SPEED_LOW 0.4
#define SPEED_MED 0.6
#define SPEED_HIGH 0.8
float SPEED = SPEED_LOW;

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

/* Generic */
int sensor_sub;
int waypoint_sub;
int system_sub;

sensor_values_t sensor_msg;
waypoint_t waypoint_msg;
system_command_t sys_msg;

uint8_t previous_waypoint;
uint8_t current_waypoint;

drive_cmd drive_commands[MAX_TURN_COUNT];
int drive_command_counter;

int print_counter;
bool debug;

/* Control variables */
PID_t pidLine;
float previousPosition;
uint64_t previousTime;
float previousError;
int motorSpeed[2] = {0,0}; // The speeds of the left (0) and right (1) motors

/* State variables */
bool isDriving; // Controls whether the drive train is on
bool armed;
bool ESTOP_TRIGGERED;

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

		/* **** END Motor Directional Functions **** */




/*
 * Calculates the position of the rover given a set of sensor values
 */
float linePosition(sensor_values_t x){
	float sum = x.s0 + x.s1 + x.s2 + x.s3 + x.s4 + x.s5 + x.s6 + x.s7;
	return 1000*(1.0*x.s0 + 2.0*x.s1 + 3.0*x.s2 + 4.0*x.s3 + 5.0*x.s4 + 6.0*x.s5 + 7.0*x.s6 + 8.0*x.s7)/(sum);
}

/*
 * Calculates the position of the rover given a set of sensor values
 */
bool on_intersection(sensor_values_t x){
	int sum = x.s0 + x.s1 + x.s2 + x.s3 + x.s4 + x.s5 + x.s6 + x.s7;
	int avg = sum/8;
	ROVER_PRINTLN("[irreader] S1: %d, S2: %d, S3 %d, S4: %d, S5: %d, S6 %d, S7 %d, S8 %d", x.s0, x.s1, x.s2, x.s3, x.s4, x.s5, x.s6, x.s7);

	if (avg < 500)
	{
		return true;
	}
	else
	{
		return false;
	}
}

void runExtraInch() {
	// Give the robot a little nudge
	leftMotorGPIO(FORWARD);
	rightMotorGPIO(FORWARD);

	set_left_motor_speed(300);
	set_right_motor_speed(300);

	osDelay(500);
}

void turnLeft() {
	// turns 90 degrees left at motor speed 300 for a delay of 500
	leftMotorGPIO(BACKWARD);
	rightMotorGPIO(FORWARD);

	set_left_motor_speed(300);
	set_right_motor_speed(300);

	osDelay(500);
}

void turnRight() {
	// turns 90 degrees right at motor speed 300 for a delay of 500
	leftMotorGPIO(FORWARD);
	rightMotorGPIO(BACKWARD);


	set_left_motor_speed(300);
	set_right_motor_speed(300);

	osDelay(500);
}

void reverseTurn() {
	// turns 180 degrees right at motor speed 300 for a delay of 1000
	leftMotorGPIO(FORWARD);
	rightMotorGPIO(BACKWARD);

	set_left_motor_speed(300);
	set_right_motor_speed(300);

	// Delay one second
	osDelay(1000);
}


void FollowLine() {
	leftMotorGPIO(FORWARD);
	rightMotorGPIO(FORWARD);

	// Get change in time since last loop call
	float dt = HAL_GetTick() - previousTime;

	// Get current pos and derivative
	float position = linePosition(sensor_msg);

	// Setpoint value
	float setPoint = 4500;

	// Calculate desired yaw effort
#ifdef ON_WHITE_TRACK
	int yawEffort = (int) (-Kp*(position - setPoint) + (-Kd*((position - setPoint) - previousError)));
#else
	int yawEffort = (int) (Kp*(position - setPoint) + (Kd*((setPoint - position) - previousError)));
#endif

	// Update previous position and previous time
	previousPosition = position;
	previousTime = previousTime + dt;
	previousError = (position - setPoint);

	// Set motor efforts and clamp
	motorSpeed[0] = (int) (SPEED*clamp((500.0 + yawEffort), 0.0, 1000.0));
	motorSpeed[1] = (int) (SPEED*clamp((500.0 - yawEffort), 0.0, 1000.0));

	print_counter = print_counter + 1;
	if (print_counter > 20) {
		print_counter = 0;
		//ROVER_PRINTLN("[Driver] Control Rate %d Hz, Position %d, Yaw Effort %d, Left Motor %d, Right Motor %d", (int)(1000.0/dt), (int)position, (int)yawEffort, (int)motorSpeed[0], (int)motorSpeed[1]);
	}

	// Send motor speeds to PWM
	set_left_motor_speed(motorSpeed[0]);
	set_right_motor_speed(motorSpeed[1]);
}

void stop_motors() {
	leftMotorGPIO(STOP);
	rightMotorGPIO(STOP);

	set_left_motor_speed(0);
	set_right_motor_speed(0);
}

void Drive() {
	// Receive sensor data
	if (check(sensor_sub))
	{
		copy(sensor_sub, &sensor_msg);
	}

	// Check if on intersection
	if (on_intersection(sensor_msg)) {
		drive_command_counter ++;

		ROVER_PRINTLN("[Driver] PATH: %s", drive_commands[drive_command_counter]);
		switch (drive_commands[drive_command_counter])
		{
		case STRAIGHT:
			runExtraInch();
			break;
		case LEFT:
			turnLeft();
			break;
		case RIGHT:
			turnRight();
			break;
		case STOP_TRIP:
			stop_motors();
			break;
		}

	}

	FollowLine();
	//turnLeft();
}


void trigger_emergency_stop() {
	ESTOP_TRIGGERED = true;
	isDriving = false;

	stop_motors();
}



/*
 * Cyclic executive
 */
static void run() {
	// If in an ESTOP, dont run anything
	if (ESTOP_TRIGGERED) {
		// Ensure motors are not moving
		set_left_motor_speed(0);
		set_right_motor_speed(0);
		osDelay(15);
		return;
	}

	// Receive waypoint data
	if (check(waypoint_sub))
	{
		// Copy waypoint message
		copy(waypoint_sub, &waypoint_msg);
		previous_waypoint = current_waypoint;
		current_waypoint = waypoint_msg.waypoint_num;

		// Convert waypoint into drive directions
		get_directions(previous_waypoint, current_waypoint, drive_commands);

		// Start driving
		isDriving = true;
		drive_command_counter = 0;
	}

	// Receive System Command info
	if (check(system_sub))
	{
		copy(system_sub, &sys_msg);

		// Immediately trigger if an estop
		if (sys_msg.estop) {
			trigger_emergency_stop();
			return;
		}

	}


	if (!armed) {
		if (sys_msg.arm) {
			armed = true;
		}

		stop_motors();
	} else {
		if (sys_msg.disarm) {
			armed = false;
		}

		if (isDriving) {
			Drive();
			// Use for debugging to find the delay time to turn
			//osDelay(500);
			//isDriving = false;
		} else {
			stop_motors();
		}
	}

	if (debug) {
		print_counter = print_counter + 1;
		if (print_counter > 20) {
			print_counter = 0;
			ROVER_PRINTLN("[Driver] Arm %s", armed ? "true" : "false");
			ROVER_PRINTLN("[Driver] Driving %s", isDriving ?  "true" : "false");
		}
	}


	// Controls the frequency of the cyclic executive
	osDelay(15);
}




/*
 * Initialisation function
 */
void StartDriver(void *argument) {
	// Sensor data subscription
	sensor_sub = subscribe(TOPIC_SENSORS);
	waypoint_sub = subscribe(TOPIC_WAYPOINT);
	system_sub = subscribe(TOPIC_SYS_COMMAND);

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
	debug = false;
	armed = false;

	isDriving = false;
	ESTOP_TRIGGERED = false;

	previous_waypoint = 0;
	current_waypoint = 0;

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
		ROVER_PRINTLN("[Driver] Please enter a command: Stop, Start, FPS");
		return 1;
	}

	if (!strcmp(argv[1], "stop")) {
		isDriving = false;
		return 0;
	}

	if (!strcmp(argv[1], "debug")) {
		ROVER_PRINTLN("[Driver] Debug Toggled");
		debug = !debug;
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
