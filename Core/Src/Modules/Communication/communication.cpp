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
#include "taskmanager.hpp"
#include <string.h>
#include <string>

#define MAX_BUF_LEN 32

/* Memory buffer for wifi bytes in transit */
static uint8_t wifi_message_buff[MAX_BUF_LEN];
static uint8_t wifi_message_len;

/* Memory buffer for serial bytes in transit */
static uint8_t serial_message_buff[MAX_BUF_LEN];
static uint8_t serial_message_len;

/* Memory buffer for storing wifi commands in transit */
static char wifi_command_buff[MAX_BUF_LEN];
static uint8_t wifi_command_counter;

/* Memory buffer for storing serial commands in transit */
static char serial_command_buff[MAX_BUF_LEN];
static uint8_t serial_command_counter;


/* Get Number of Arguments
 *
 * Calculates the number of spaces present in a string
 * (i.e. the number of arguments)
 */
int get_number_of_arguments(char* str, int strlen)
{
	int count = 1;

	for(int i = 0; i < strlen; i++)
	{
		if(str[i]==' ')
		{
			count++;
		}
	}

	return count;
}


void handle_wifi_command() {
	std::string cmd = "";

	for (int i = 0; i < wifi_command_counter; i++) {
		cmd += wifi_command_buff[wifi_command_counter];
	}

	ROVER_PRINTLN("[WiFi] %s", cmd.c_str());

	wifi_command_counter = 0;
}


void handle_serial_command() {
	// If the command is <3 characters it cannot be an actual command
	if (serial_command_counter < 3) {
		// Reset the command counter index for a new command
		serial_command_counter = 0;

		// Send nothing
		const char *argv[] = {};
		console_command(0, argv);
		return;
	}




	// Split the command into arguments
	int argc = get_number_of_arguments(serial_command_buff, serial_command_counter);
	std::string args[argc];

	// Initialise each string as empty
	for (int i = 0; i < argc; i++) {
		args[i] = "";
	}

	int word_counter = 0;
	//unsigned int max_len = 0;

	// For each character in the serial command
	for (int i = 0; i < serial_command_counter; i++) {
		// If this char is space
		if (serial_command_buff[i] == ' ') {
			// Save this arg and ignore this character
			word_counter ++;

			continue;
		}


		// If not alphanumeric, skip
		if (!isalpha(serial_command_buff[i]) && !isdigit(serial_command_buff[i])) {
			continue;
		}

		//
		args[word_counter] += serial_command_buff[i];
	}

	const char *argv[argc];

	for (int i = 0; i < argc; i++) {
		argv[i] = args[i].c_str();
	}


	// Send the command to the taskmanager console
	console_command(argc, argv);


	// Reset the command counter index for a new command
	serial_command_counter = 0;
}



static void run() {
	// If atleast one byte of data was received from the wifi module
	if (WiFi_read(&wifi_message_len)) {

		// For each byte of data received
		for (int i = 0; i < wifi_message_len; i++) {
			// If the byte is an eol
			if (wifi_message_buff[i] == '\n') {
				// The command is completed. Now handle the command
				handle_wifi_command();
			}

			// Transfer the byte of data to the command buffer
			wifi_command_buff[wifi_command_counter] = (char) wifi_message_buff[i];

			// Increment the wifi command counter
			wifi_command_counter ++;
		}

	}

	// If atleast one byte of data was received from the wifi module
	if (Serial_read(&serial_message_len)) {

		// For each byte of data received
		for (int i = 0; i < serial_message_len; i++) {
			// If the byte is an eol
			if (serial_message_buff[i] == '\n') {
				// The command is completed. Now handle the command
				handle_serial_command();
			}

			// Transfer the byte of data to the command buffer
			serial_command_buff[serial_command_counter] = (char) serial_message_buff[i];

			// Increment the serial counter
			serial_command_counter ++;
		}

	}

	// Cyclical time step
	osDelay(500);
}





void StartCommunication(void *argument) {
	// Initialise comms
	init_wifi_comm(wifi_message_buff);
	init_serial_comm(serial_message_buff);

	// Initialise the command counters
	wifi_command_counter = 0;
	serial_command_counter = 0;

	for (;;)
	{
		run();
	}

	osThreadTerminate(NULL);
}
