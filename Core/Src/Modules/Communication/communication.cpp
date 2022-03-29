/******** nurse controller **********
 *
 * @file communication.cpp
 * @desc Receives UART information from the ESP WiFi module and handles it appropriately.
 *
 * @author Timothy Mead
 *
 ************************************/


#include "serial.h"
#include "stm32f4xx_hal.h"
#include "communication.hpp"

static uint8_t message_buf[32];
static uint8_t message_len;

static void run() {
	// If atleast one byte of data was received from the wifi module
	if (WiFi_read(&message_len)) {

		// Print the data to serial
		for (int i = 0; i < message_len; i++) {
			ROVER_PRINT("%c", message_buf[i]);
		}


	}

	// Cyclical time step
	osDelay(500);
}



void StartCommunication(void *argument) {
	// Initialise comms with WiFi module
	init_wifi_comm(message_buf);

	for (;;)
	{
		run();
	}

	osThreadTerminate(NULL);
}
