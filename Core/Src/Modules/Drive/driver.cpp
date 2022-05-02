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


// Cyclic executive
static void run() {
	// ToDo

	// HELLO
	ROVER_PRINTLN("Hello World!");

	// Controls the frequency of the cyclic executive
	HAL_Delay(15);
}



void StartDriver(void *argument) {
	// Initialisation Code
	//blink_sub = subscribe(TOPIC_SENSOR);

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
