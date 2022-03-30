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

static uint8_t command_buf[32];
static uint8_t command_counter;

static void run() {
	// If atleast one byte of data was received from the wifi module
	if (WiFi_read(&message_len)) {

		// For each byte of data received
		for (int i = 0; i < message_len; i++) {
			// If the byte is an eol
			if (message_buf[i]) {
				// The command is completed. Now handle the command
				handle_command();
			}

			// Transfer the byte of data to the command buffer
			command_buf[command_counter] = message_buf[i];
		}

	}

	// Cyclical time step
	osDelay(500);
}

void handle_command() {

	command_counter = 0;
}



void StartCommunication(void *argument) {
	// Initialise comms with WiFi module
	init_wifi_comm(message_buf);

	// Initialise the command
	command_counter = 0;

	for (;;)
	{
		run();
	}

	osThreadTerminate(NULL);
}
