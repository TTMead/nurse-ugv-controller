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
#include "msg_peripheralstate.h"
#include "driver.hpp"
#include "stm32f4xx_hal.h"
#include "eORB.hpp"
#include "serial.h"
#include "motor_driver.h"
#include "string.h"


#define MIN(x, y) (((x) > (y)) ? (x) : (y))


		/* **** Settings **** */

/* PID Tuning Mode */
// #define PID_TUNING_MODE

/* Speed Settings */
#define SPEED_LOW 0.4
#define SPEED_MED 0.6
#define SPEED_HIGH 0.8
float SPEED = 0.35;

/* Control Settings */

// OLD
#define Kp 0.7 //Straight: 0.18
#define Ki 0
#define Kd 3.5 //Straight: 2

// NEW
//#define Kp 1.6
//#define 0
//#define Kd 7

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
#define TURN_PWM 225
float TURN_DELAY_RATIO = 6.2;


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
int obstacle_sub;

sensor_values_t sensor_msg;
waypoint_t waypoint_msg;
system_command_t sys_msg;
obstacle_t obstacle_msg;

uint8_t previous_waypoint;
uint8_t current_waypoint;

drive_cmd drive_commands[MAX_TURN_COUNT];
int drive_command_counter;
int intersection_refractory_counter;
int intersection_refractory_time = 150;

int print_counter;
int off_track_counter;
bool debug;

/* Control variables */
PID_t pidLine;
float previousPosition;
uint64_t previousTime;
float previousError;
int motorSpeed[2] = {0,0}; // The speeds of the left (0) and right (1) motors

/* State variables */
bool isDriving; // Do we have a waypoint to go to?
bool armed; // Are we armed?
bool ESTOP_TRIGGERED; // Is the ESTOP enabled?
bool obstacleDetected; // Is an obstacle in front of the rover?

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
	if (intersection_refractory_counter > 0) {
		intersection_refractory_counter --;
		return false;
	}

	int sum = x.s0 + x.s1 + x.s2 + x.s3 + x.s4 + x.s5 + x.s6 + x.s7;
	int avg = sum/8;
	// ROVER_PRINTLN("[Driver] S1: %d, S2: %d, S3 %d, S4: %d, S5: %d, S6 %d, S7 %d, S8 %d", x.s0, x.s1, x.s2, x.s3, x.s4, x.s5, x.s6, x.s7);

	if (avg < 500)
	{
		ROVER_PRINTLN("[Driver] Intersection Reached");

		intersection_refractory_counter = intersection_refractory_time;
		return true;
	}
	else
	{
		return false;
	}
}

/*
 * Calculates the position of the rover given a set of sensor values
 */
bool off_track(sensor_values_t x){

	int sum = x.s0 + x.s1 + x.s2 + x.s3 + x.s4 + x.s5 + x.s6 + x.s7;
	int avg = sum/8;

	// ROVER_PRINTLN("[Driver] S1: %d, S2: %d, S3 %d, S4: %d, S5: %d, S6 %d, S7 %d, S8 %d", x.s0, x.s1, x.s2, x.s3, x.s4, x.s5, x.s6, x.s7);



	if (avg > 2000)
	{
		off_track_counter ++;

		if (off_track_counter > 230) {
			ROVER_PRINTLN("[Driver] Off Road E-STOP");
			return true;
		}
	}
	else
	{
		off_track_counter = 0;
	}

	return false;
}

void turnLeft() {
	// turns 90 degrees left at motor speed 300 for a delay of 500
	leftMotorGPIO(BACKWARD);
	rightMotorGPIO(FORWARD);

	set_left_motor_speed(TURN_PWM);
	set_right_motor_speed(TURN_PWM);

	osDelay((int) ((float)TURN_PWM * (float)TURN_DELAY_RATIO));
}

void turnRight() {
	// turns 90 degrees right at motor speed 300 for a delay of 500
	leftMotorGPIO(FORWARD);
	rightMotorGPIO(BACKWARD);


	set_left_motor_speed(TURN_PWM);
	set_right_motor_speed(TURN_PWM);

	osDelay((int) ((float)TURN_PWM * (float)TURN_DELAY_RATIO));
}

void reverseTurn() {
	// turns 180 degrees right at motor speed 300 for a delay of 1000
	leftMotorGPIO(FORWARD);
	rightMotorGPIO(BACKWARD);

	ROVER_PRINTLN("REVERSE_TURN");

	set_left_motor_speed(TURN_PWM);
	set_right_motor_speed(TURN_PWM);

	osDelay((int) ((float)TURN_PWM * (2.0*TURN_DELAY_RATIO)));
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
	motorSpeed[0] = (int) (SPEED*clamp((625.0 + yawEffort), 250, 1000.0));
	motorSpeed[1] = (int) (SPEED*clamp((625.0 - yawEffort), 250, 1000.0));

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


void trigger_emergency_stop() {
	ESTOP_TRIGGERED = true;
	isDriving = false;

	stop_motors();
}

void Drive() {
	// Receive sensor data
	if (check(sensor_sub))
	{
		copy(sensor_sub, &sensor_msg);
	}

	// Emergency Stop if off the track
	if (off_track(sensor_msg)) {
		trigger_emergency_stop();
	}

	// Check if on intersection
	if (on_intersection(sensor_msg)) {
		ROVER_PRINTLN("[Driver] Drive Command: %d", drive_commands[drive_command_counter]);

		switch (drive_commands[drive_command_counter])
		{
		case STRAIGHT:
			// Do nothing
			break;
		case LEFT:
			osDelay(100);
			turnLeft();
			break;
		case RIGHT:
			osDelay(100);
			turnRight();
			break;
		case STOP_TRIP:
			// Stopping trip and ready to receive next waypoint
			stop_motors();
			isDriving = false;

			// Send the peripheral controller an update packet
			uint8_t message_buf[10];
			pack_peripheralstate_msg(message_buf, 0, 1, 0);
			NAV_send(message_buf, MSG_SIZE_PERIPHERALSTATE);

			break;
		}

		drive_command_counter ++;
	}





	FollowLine();
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

	// Receive System Command info
	if (check(system_sub))
	{
		copy(system_sub, &sys_msg);

		ROVER_PRINTLN("[Driver] MSG: %d, %d, %d, %d, %d, %d, %d", sys_msg.arm, sys_msg.disarm, sys_msg.estop, sys_msg.peripheral_items_collected, sys_msg.serving_completed, sys_msg.waypoint_reached);

		// Immediately trigger if an estop
		if (sys_msg.estop) {
			trigger_emergency_stop();
			return;
		}

		// If items collected, send rover back to waypoint.
		if (sys_msg.peripheral_items_collected) {
			waypoint_msg.timestamp = HAL_GetTick();
			waypoint_msg.waypoint_num = 1;
			publish(TOPIC_WAYPOINT, &waypoint_msg);
		}

	}

	// Check for obstacle info
	if (check(obstacle_sub)) {
		copy(obstacle_sub, &obstacle_msg);

		obstacleDetected = obstacle_msg.obstacle_detected;
	}


#ifdef PID_TUNING_MODE

	// Receive sensor data
	if (check(sensor_sub))
	{
		copy(sensor_sub, &sensor_msg);
	}

	if (!armed) {
		stop_motors();
		if (sys_msg.arm) {
			armed = true;
			ROVER_PRINTLN("[Driver] Armed");
		}

	} else {
		FollowLine();
		if (sys_msg.disarm) {
			armed = false;
			ROVER_PRINTLN("[Driver] Disarmed");
		}
	}


#else

	if (!armed) {
		if (sys_msg.arm) {
			armed = true;
			ROVER_PRINTLN("[Driver] Armed");
		}

		stop_motors();
	} else {
		if (sys_msg.disarm) {
			armed = false;
			ROVER_PRINTLN("[Driver] Disarmed");
		}

		if (isDriving) {

			if (obstacleDetected) {
				// Ensure motors are stopped
				stop_motors();
			} else {
				// Use for debugging to find the delay time to turn
				//osDelay(500);
				//isDriving = false;

				// Only drive if no obstacles, and there is a waypoint available
				Drive();
			}

		} else {
			// Ensure motors are stopped
			stop_motors();

			// Check for new waypoint data
			if (check(waypoint_sub))
			{
				// Copy waypoint message
				copy(waypoint_sub, &waypoint_msg);

				// Check if this is a reset message
				if (waypoint_msg.waypoint_num == 9) {
					previous_waypoint = 0;
					current_waypoint = 0;

					isDriving = false;
					ROVER_PRINTLN("[Driver] Reset to waypoint A");
					stop_motors();

					osDelay(15);
					return;
				}


				previous_waypoint = current_waypoint;
				current_waypoint = waypoint_msg.waypoint_num - 1; // This module starts counting waypoints from 0 while the serial protocol starts from 1

				// Convert waypoint into drive directions
				get_directions(previous_waypoint, current_waypoint, drive_commands);

				// Start driving
				isDriving = true;
				drive_command_counter = 0;
				intersection_refractory_counter = intersection_refractory_time;

				// Inform serial
				ROVER_PRINTLN("[Driver] Waypoint: %c", waypoint_id_to_char(current_waypoint));

				// Send the peripheral controller an update packet
				uint8_t message_buf[10];
				pack_peripheralstate_msg(message_buf, 1, 0, 0);
				NAV_send(message_buf, MSG_SIZE_PERIPHERALSTATE);


				// Turn before starting
				reverseTurn();
			}
		}
	}

#endif

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
	obstacle_sub = subscribe(TOPIC_OBSTACLE);

	SPEED = SPEED_LOW;

	// Initialise hardware interfaces
	leftMotorGPIO(FORWARD);
	rightMotorGPIO(FORWARD);
	initialise_motor_pwm();

	pid_init(&pidLine);
	pid_set_parameters(&pidLine, Kp, Ki, Kd, 1000, -(MOTOR_MAX_PWM/2), (MOTOR_MAX_PWM));

	previousPosition = 4.5;
	previousTime = HAL_GetTick();


	previousError = 0;
	print_counter = 0;
	off_track_counter = 0;
	debug = false;
	armed = false;

	isDriving = false;
	ESTOP_TRIGGERED = false;
	obstacleDetected = false;

	previous_waypoint = 0;
	current_waypoint = 0;

	intersection_refractory_counter = 0;

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
