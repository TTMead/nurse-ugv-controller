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

#define LEFTMOTORIN1 GPIO_PIN_6
#define LEFTMOTORIN2 GPIO_PIN_7
#define RIGHTMOTORIN3 GPIO_PIN_8
#define RIGHTMOTORIN4 GPIO_PIN_9
#define ON GPIO_PIN_SET
#define OFF GPIO_PIN_RESET
#define FORWARD 1
#define BACKWARD 0
#define THRESHOLD 2000
#define FULLSPEED 100
#define STOP 0

int sensor_sub;
sensor_values_t sensor;
uint8_t motorSpeed[2] = {0,0};

// Comment this to use bang-bang
#define USING_PID
#define Kp 0
#define Ki 0
#define Kd 0

PID_t pidLine;
float previousPosition;
uint64_t previousTime;

// Motor Functions
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
		HAL_GPIO_WritePin(GPIOC, RIGHTMOTORIN3, ON);
		HAL_GPIO_WritePin(GPIOC, RIGHTMOTORIN4, OFF);
	}
	else if (direction == BACKWARD)
	{
		HAL_GPIO_WritePin(GPIOC, RIGHTMOTORIN3, OFF);
		HAL_GPIO_WritePin(GPIOC, RIGHTMOTORIN4, ON);
	}
	else 
	{
		HAL_GPIO_WritePin(GPIOC, RIGHTMOTORIN3, OFF);
		HAL_GPIO_WritePin(GPIOC, RIGHTMOTORIN4, OFF);
	}
	
}

// Position
float linePosition(sensor_values_t x){
	return (1*x.s0 + 2*x.s1 + 3*x.s2 + 4*x.s3 + 5*x.s4 + 6*x.s5 + 7*x.s6 + 8*x.s7)/8
}


// Cyclic executive
static void run() {

	// Receive sensor data
		if (check(sensor_sub))
		{
			copy(sensor_sub, &sensor);
		}


	#ifdef USING_PID

		uint16_t dt = HAL_GetTick() - previousTime;
		float position = linePosition(sensor);
		float positionDot = (position - previousPosition);
		float setPoint = 4.5;
		float yawEffort = pid_calculate(&pidLine, setPoint, position, positionDot, dt);

		previousPosition = position;
		previousTime = previousTime + dt;

		motorSpeed[0] = 50 + yawEffort;
		motorSpeed[1] = 50 - yawEffort;

	#else
		// Bang-Bang Straight Path
		if (sensor.s4 < THRESHOLD)
		{
			// Turn Left
			motorSpeed[0] = 40;
			motorSpeed[1] = 60;
		}
		else if (sensor.s3 < THRESHOLD)
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
		if (1) {
			char dir; if (sensor.s4 < THRESHOLD) {dir = 'L';} else if (sensor.s3 < THRESHOLD) {dir = 'R';} else {dir = 'S';}
			ROVER_PRINTLN("[Driver] %d %d %d %d %d %d %d %d %c", (sensor.s0 < THRESHOLD), (sensor.s1 < THRESHOLD), (sensor.s2 < THRESHOLD), (sensor.s3 < THRESHOLD), (sensor.s4 < THRESHOLD), (sensor.s5 < THRESHOLD), (sensor.s6 < THRESHOLD), (sensor.s7 < THRESHOLD), dir);
		}
	#endif

	// Controls the frequency of the cyclic executive
	HAL_Delay(15);
}



void StartDriver(void *argument) {
	// Initialisation Code
	sensor_sub = subscribe(TOPIC_SENSORS);
	leftMotorGPIO(FORWARD);
	rightMotorGPIO(FORWARD);

	pid_init(&pidLine);
	pid_set_parameters(&pidLine, Kp, Ki, Kd, 100, -50, 50);

	previousPosition = 4.5;
	previousTime = HAL_GetTick();

	for (;;)
	{
		run();
	}

	osThreadTerminate(NULL);
}


// Console Interface
int driver_main(int argc, const char *argv[]) {
	return 0;
}
