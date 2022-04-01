/******** nurse controller **********
 *
 * @file taskmanager.hpp
 * @desc This module manages the memory and initialisation of the RTOS tasks
 *
 * @author Timothy Mead
 *
 ************************************/


// External libraries
#include "stdlib.h"
#include <string.h>

// Internal headers
#include "serial.h"
#include "blinky.hpp"
#include "blinksetter.hpp"
#include "taskmanager.hpp"
#include "eORB.hpp"
#include "communication.hpp"




osThreadId_t task_handles[NUMBER_OF_TASKS];
static uint16_t task_stack_size[NUMBER_OF_TASKS];
static uint8_t task_counter = 1;

static bool display_memory_usage;

static void run()
{
	if (display_memory_usage) {
		ROVER_PRINTLN("");
		ROVER_PRINTLN("================");
		ROVER_PRINTLN("Memory Usage");
		ROVER_PRINTLN("================");
		for (int i = 0; i < NUMBER_OF_TASKS; i++) {
			ROVER_PRINT(osThreadGetName(task_handles[i]));
			ROVER_PRINTLN(" heap: %u/%u bytes in use", task_stack_size[i] - osThreadGetStackSpace(task_handles[i]), task_stack_size[i]);
		}
		ROVER_PRINTLN("");
	}

	osDelay(1000);
}


static void InitialiseTask(osThreadFunc_t start_function, const osThreadAttr_t *thread_attributes)
{
	task_handles[task_counter] = osThreadNew(start_function, NULL, thread_attributes);
	task_stack_size[task_counter] = thread_attributes->stack_size;

	task_counter ++;
}


void StartTaskManager(void *argument)
{
	ROVER_PRINTLN("STARTING TASK MANAGER");

	// Add the task manager stack size
	task_stack_size[0] = TASK_MANAGER_STACK_SIZE;

	// Default settings
	display_memory_usage = false;

	// Initialise other tasks
	InitialiseTask(StarteORB, &eORB_attributes);
	InitialiseTask(StartCommunication, &Communication_attributes);
	InitialiseTask(StartBlinkSetter, &BlinkSetter_attributes);
	InitialiseTask(StartBlinky, &Blinky_attributes);

	for (;;)
	{
		run();
	}

	osThreadTerminate(NULL);
}





int console_command(int argc, const char *argv[]) {
	// Check if the command has any arguments
	if (argc < 1) {
		ROVER_PRINTLN("[Task Manager] No command provided");
		return 1;
	}

	//	Print the received command
	ROVER_PRINT(">");
	for (int i = 0; i < argc; i++) {
		ROVER_PRINT("%s", argv[0]);
	}
	ROVER_PRINTLN("");


	// Handle the command
	if (!strcmp(argv[0], "memory")) {
		// Toggle memory display
		display_memory_usage = !display_memory_usage;
		ROVER_PRINTLN("[Task Manager] Memory display toggled!");
		return 0;

	} else if (!strcmp(argv[0], "eORB")) {
		eORB_main(argc, argv);
		return 0;
	}

	// Command wasn't recognised
	ROVER_PRINTLN("[Task Manager] Unrecognised command");
	return 2;
}



