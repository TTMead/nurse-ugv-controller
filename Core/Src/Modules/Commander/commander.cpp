/******** nurse controller **********
 *
 * @file commander.cpp
 * @desc Navigator and state machine, commands other modules to act.
 *
 *
 * @author Timothy Mead
 *
 ************************************/


#include "commander.hpp"
#include "eORB.hpp"
#include "stm32f4xx_hal.h"
#include "serial.h"
#include "string.h"
#include "driver.hpp"

#define MAX_NUMBER_WAYPOINTS 5



/* Map Travel Matrix of Drive Commands
 *
 *    A B C D E
 * A
 * B
 * C
 * D
 * E
 */

const drive_cmd transition_table[5][5][MAX_TURN_COUNT] = {
	{{STOP}, 				{RIGHT, STOP}, 		{RIGHT, LEFT, RIGHT, STOP}, 		{STRAIGHT, STOP}, 		{RIGHT, STOP}},
	{{RIGHT, STOP}, 		{STOP}, 			{RIGHT, LEFT, RIGHT, STOP}, 		{STRAIGHT, STOP}, 		{RIGHT, STOP}},
	{{STRAIGHT, STOP}, 		{RIGHT, STOP}, 		{STOP}, 							{STRAIGHT, STOP}, 		{RIGHT, STOP}},
	{{LEFT, STOP}, 			{RIGHT, STOP}, 		{RIGHT, LEFT, RIGHT, STOP}, 		{STOP}, 				{RIGHT, STOP}},
	{{RIGHT, RIGHT, STOP}, 	{RIGHT, STOP}, 		{RIGHT, LEFT, RIGHT, STOP}, 		{STRAIGHT, STOP}, 		{STOP}}
};

enum STATE {
	DISARMED,
	IDLE,
	DRIVING,
	SERVING,
	ESTOP
} state;

const char *state_names[] = {"Disarmed","Idle","Driving","Serving", "E-STOP"};

waypoint previous_waypoint;
waypoint current_waypoint;

static waypoint_t incomming_waypoint;
static system_command_t incomming_command;

static bool print_state;


/* Subscription handles */
int waypoint_sub;
int sys_cmd_sub;


/* get directions
 *
 * Returns the required driving directions when given a start and exit waypoint
 *
 * @param a - The starting waypoint
 * @param b - The ending waypoint
 * @param buffer - a buffer to store the drive command
 */
int get_directions(waypoint a, waypoint b, drive_cmd *buffer) {
	memcpy(buffer, transition_table[a][b], MAX_TURN_COUNT*sizeof(drive_cmd));

	return 0;
}


static void run() {
	// Check incomming system commands
	if (check(sys_cmd_sub)) {
		// Grab the new message
		copy(sys_cmd_sub, &incomming_command);
	}

	if (incomming_command.estop) {
		state = ESTOP;
	}

		/****** State machine ******/
	if (state == DISARMED) {
		// If told to arm, switch to idle state
		if (incomming_command.arm) {
			state = IDLE;

			// Start drive train
			const char * argv[] = {"driver", "start", "low"};
			driver_main(3, argv);
		}
	} else if (state == IDLE) {
		// Check for a new waypoint message
		if (check(waypoint_sub)) {
			// Store the new waypoint
			copy(waypoint_sub, &incomming_waypoint);

			// Switch to driving
			state = DRIVING;

			// Send a message to drive train
			drive_command_t drive_msg;
			drive_msg.timestamp = HAL_GetTick();
			get_directions(current_waypoint, previous_waypoint, drive_msg.commands);
			publish(TOPIC_DRIVE_COMMAND, &drive_msg);
		}

		// Check for disarm
		if (incomming_command.disarm) {
			state = DISARMED;

			// Stop driver
			const char * argv[] = {"driver", "stop"};
			driver_main(2, argv);
		}

	} else if (state == DRIVING) {
		// Check if the drive has been finished.
		if (incomming_command.waypoint_reached) {
			// Switch to serve mode
			state = SERVING;

			// Inform the pi
			// ToDo
		}

	} else if (state == SERVING) {
		// Check if the serving has been finished (received from the pi)
		if (incomming_command.serving_completed) {
			// Switch back to idling
			state = IDLE;
		}

	}
		/****** End of State machine ******/

	// Print the state if activated
	if (print_state) {
		ROVER_PRINTLN("[Commander] Commander State: %s", state_names[(int)state]);
	}

	// Enforce control loop
	osDelay(100);
}


void StartCommander(void *argument) {
	// Subscribe to topics
	waypoint_sub = subscribe(TOPIC_WAYPOINT);
	sys_cmd_sub = subscribe(TOPIC_SYS_COMMAND);

	print_state = false;
	state = DISARMED;

	for (;;)
	{
		run();
	}

	osThreadTerminate(NULL);
}


int commander_main(int argc, const char *argv[]) {
	// Check if the command has enough arguments
	if (argc < 2) {
		ROVER_PRINTLN("[Commander] Please enter a command: printstate");
		return 1;
	}


	// Handle the command
	if (!strcmp(argv[1], "printstate")) {
		// Toggle printstate
		print_state = !print_state;

		// Inform via serial
		const char *text;
		if (print_state) { text = "on "; } else { text = "off"; }
		ROVER_PRINTLN("[Commander] Printing states, %s", text);
		return 0;
	}


	// Command wasn't recognised
	ROVER_PRINTLN("[Commander] Unrecognised command");
	return 2;
}
