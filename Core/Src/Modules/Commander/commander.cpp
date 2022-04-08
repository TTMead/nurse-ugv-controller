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
	SERVING
} state;


waypoint current_waypoint;
waypoint waypoint_queue[MAX_NUMBER_WAYPOINTS];
uint8_t waypoints_in_queue;

static waypoint_t incomming_waypoint;
static system_command_t incomming_command;

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
	// Check for a new waypoint message
	if (check(waypoint_sub)) {
		// Store the new waypoint
		copy(waypoint_sub, &incomming_waypoint);

		// Add it to the top of queue
		if (waypoints_in_queue < MAX_NUMBER_WAYPOINTS) {
			waypoint_queue[waypoints_in_queue] = (waypoint) incomming_waypoint.waypoint_num;
			waypoints_in_queue ++;
		}
	}

	// State machine
	if (state == DISARMED) {
		// Check incomming system commands
		if (check(sys_cmd_sub)) {
			// Grab the new message
			copy(sys_cmd_sub, &incomming_command);

			// If told to arm, switch to idle state
			if (incomming_command.arm) {
				state = IDLE;
			}
		}
	} else if (state == IDLE) {
		// Check if any waypoints in the queue, if so start driving to them
		if (waypoints_in_queue > 0) {
			state = DRIVING;

			// Send a message to the drive train to the waypoint at the bottom of the queue
			drive_command_t drive_msg;
			drive_msg.timestamp = HAL_GetTick();
			get_directions(current_waypoint, waypoint_queue[0], drive_msg.commands);
			drive_msg.end_waypoint = waypoint_queue[0];

			publish(TOPIC_DRIVE_COMMAND, &drive_msg);

			// Shift waypoint queue down
			waypoints_in_queue --;
			for (int i = 0; i < waypoints_in_queue; i++) {
				waypoint_queue[i] = waypoint_queue[i+1];
			}
		}

	} else if (state == DRIVING) {
		// Check if the drive has been finished.
		if (incomming_command.waypoint_reached) {
			// Switch to serve mode
			state = SERVING;
		}
	} else if (state == SERVING) {
		// Check if the serving has been finished.
		if (incomming_command.serving_completed) {
			// Switch back to idling
			state = IDLE;
		}
	}

	// Enforce control loop
	osDelay(100);
}


void StartCommander(void *argument) {
	// Subscribe to topics
	waypoint_sub = subscribe(TOPIC_WAYPOINT);
	sys_cmd_sub = subscribe(TOPIC_SYS_COMMAND);

	waypoints_in_queue = 0;

	for (;;)
	{
		run();
	}

	osThreadTerminate(NULL);
}


int commander_main(int argc, const char *argv[]) {
	// Check if the command has enough arguments
	if (argc < 1) {
		ROVER_PRINT("[Commander] Module has no commands");
		return 1;
	}

	return 0;
}
