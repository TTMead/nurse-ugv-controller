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
uint16_t sensor[8];
uint8_t motorSpeed[2] = {0,0}


// Motor Functions
void leftMotorGPIO(int direction) {
	
	if (direction == FORWARD)
	{
		HAL_GPIO_WritePin(GPIOC, LEFTMOTORIN1, ON)
		HAL_GPIO_WritePin(GPIOC, LEFTMOTORIN2, OFF)
	}
	else if (direction == BACKWARD)
	{
		HAL_GPIO_WritePin(GPIOC, LEFTMOTORIN1, OFF)
		HAL_GPIO_WritePin(GPIOC, LEFTMOTORIN2, ON)
	}
	else 
	{
		HAL_GPIO_WritePin(GPIOC, LEFTMOTORIN1, OFF)
		HAL_GPIO_WritePin(GPIOC, LEFTMOTORIN2, OFF)
	}
	
}

void rightMotorGPIO(bool direction) {
	
	if (direction == FORWARD)
	{
		HAL_GPIO_WritePin(GPIOC, RIGHTMOTORIN3, ON)
		HAL_GPIO_WritePin(GPIOC, RIGHTMOTORIN4, OFF)
	}
	else if (direction == BACKWARD)
	{
		HAL_GPIO_WritePin(GPIOC, RIGHTMOTORIN3, OFF)
		HAL_GPIO_WritePin(GPIOC, RIGHTMOTORIN4, ON)
	}
	else 
	{
		HAL_GPIO_WritePin(GPIOC, RIGHTMOTORIN3, OFF)
		HAL_GPIO_WritePin(GPIOC, RIGHTMOTORIN4, OFF)
	}
	
}

// Cyclic executive
static void run() {
	// ToDo

	// Receive sensor data
	if (check(sensor_sub)
	{
		copy(sensor_sub, sensor);
	}

	// Bang-Bang Straight Path
	if (sensor[4] < THRESHOLD)
	{
		motorSpeed[0] = 40;
		motorSpeed[1] = 60;
	}
	else if (sensor[5] < THRESHOLD)
		motorSpeed[0] = 60;
		motorSpeed[1] = 40;
	}
	else
	{
		motorSpeed[0] = 50;
		motorSpeed[1] = 50;
	}

	// Print motor speed
	ROVER_PRINTLN('Left Motor: %d, Right Motor: %d', motorSpeed[0], motorSpeed[1])

	// Controls the frequency of the cyclic executive
	HAL_Delay(15);
}



void StartDriver(void *argument) {
	// Initialisation Code
	sensor_sub = subscribe(TOPIC_SENSOR);
	leftMotorGPIO(FORWARD);
	rightMotorGPIO(FORWARD);

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
