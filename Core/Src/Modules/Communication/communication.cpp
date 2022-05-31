/******** nurse controller **********
 *
 * @file communication.cpp
 * @desc Receives UART information from the ESP WiFi module and handles it appropriately.
 *
 * @author Timothy Mead
 *
 ************************************/


#include "serial.h"
#include "msg_heartbeat.h"
#include "msg_arm.h"
#include "msg_waypoint.h"
#include "msg_peripheralstate.h"
#include "msg_estop.h"
#include "msg_obstacle.h"
#include "stm32f4xx_hal.h"
#include "communication.hpp"
#include "taskmanager.hpp"
#include "eORB.hpp"
#include <string.h>
#include <string>

#define MAX_BUF_LEN 32

// Debug parameters
bool debug_wifi;

/* Memory buffer for wifi bytes in transit */
static uint8_t wifi_message_buff[MAX_BUF_LEN];
static uint8_t wifi_message_len;

/* Memory buffer for serial bytes in transit */
static uint8_t serial_message_buff[MAX_BUF_LEN];
static uint8_t serial_message_len;

/* Memory buffer for storing wifi commands in transit */
static uint8_t wifi_command_buff[MAX_BUF_LEN];
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

	// Initialise command read buff
	std::string cmd = "";

	// Copy command into new buff
	for (int i = 0; i < wifi_command_counter; i++) {
		// Skip newline characters
		if (wifi_command_buff[i] == 10) {
			continue;
		}

		// Add this char into buff
		cmd += wifi_command_buff[i];
	}

	// Unpack the message
	uint8_t id;
	uint8_t payload_length;
	uint8_t payload[6];

	// If the message fails the unpacking
	if (!unpack_message(&id, &payload_length, payload, (uint8_t*) cmd.c_str()) == 0) {
		// Print warning
		ROVER_PRINTLN("[Communicator] A WiFi message failed checksum!");
	} else {
			// Handle the command
		if (id == MSG_ID_HEARTBEAT) {
			if (debug_wifi) { ROVER_PRINTLN("[WiFi] Heartbeat from Component %d received!", payload[0]); };
		}
		if (id == MSG_ID_ARM) {
			// Send the arm system command
			system_command_t command;
			command.timestamp = HAL_GetTick();
			unpack_arm_payload(payload, &command.arm, &command.disarm);
			publish(TOPIC_SYS_COMMAND, &command);
		}
		if (id == MSG_ID_ESTOP) {
			// Send the ESTOP system command
			system_command_t command;
			command.timestamp = HAL_GetTick();
			command.estop = 1;
			publish(TOPIC_SYS_COMMAND, &command);
		}
		if (id == MSG_ID_WAYPOINT) {
			// Send the waypoint command
			waypoint_t waypoint_msg;
			waypoint_msg.timestamp = HAL_GetTick();
			unpack_waypoint_payload(payload, &waypoint_msg.waypoint_num);
			publish(TOPIC_WAYPOINT, &waypoint_msg);
		}
		if (id == MSG_ID_PERIPHERALSTATE) {
			// Send the peripheral command
			system_command_t command;
			command.timestamp = HAL_GetTick();
			bool temp;
			unpack_peripheralstate_payload(payload, &temp, &temp, &command.peripheral_items_collected);
			publish(TOPIC_SYS_COMMAND, &command);
		}
		if (id == MSG_ID_OBSTACLE) {
			// Send the peripheral command
			obstacle_t obstacle_msg;
			obstacle_msg.timestamp = HAL_GetTick();
			unpack_obstacle_payload(payload, &obstacle_msg.obstacle_detected);
			publish(TOPIC_OBSTACLE, &obstacle_msg);
		}
	}


	// Reset the wifi command buff
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
			// If the byte is a NULL byte
			if (wifi_message_buff[i] == (uint8_t) '\n') {
				// The command is completed. Now handle the command
				handle_wifi_command();
			}

			// Transfer the byte of data to the command buffer
			wifi_command_buff[wifi_command_counter] = wifi_message_buff[i];

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
	// Initialise settings
	debug_wifi = false;

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


int communication_main(int argc, const char *argv[]) {
	// Check if the command has enough arguments
	if (argc < 2) {
		ROVER_PRINT("[Communicator] Please enter a command, ");
		ROVER_PRINTLN("    - debugwifi");
		return 1;
	}

	// Handle the command
	if (!strcmp(argv[1], "debugwifi")) {
		// Toggle the wifi printing
		debug_wifi = !debug_wifi;

		// Print outcome
		const char *text;
		if (debug_wifi) { text = "on "; } else { text = "off"; }
		ROVER_PRINTLN("[Communicator] Debug WiFi %s!", text);
	}

	// Command wasn't recognised
	ROVER_PRINT("[Communicator] Unrecognised command, ");
	ROVER_PRINTLN("    - debugwifi");
	return 2;
}



