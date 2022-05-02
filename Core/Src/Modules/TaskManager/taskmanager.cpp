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
#include "commander.hpp"
#include "irreader.hpp"
#include "driver.hpp"



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
	ROVER_PRINTLN("[Task Manager] Rover booting up.");

	// Add the task manager stack size
	task_stack_size[0] = TASK_MANAGER_STACK_SIZE;

	// Default settings
	display_memory_usage = false;

	// Initialise other tasks
	InitialiseTask(StarteORB, &eORB_attributes);
	InitialiseTask(StartCommunication, &Communication_attributes);
	InitialiseTask(StartBlinkSetter, &BlinkSetter_attributes);
	InitialiseTask(StartBlinky, &Blinky_attributes);
	InitialiseTask(StartCommander, &Commander_attributes);
	InitialiseTask(StartIRReader, &IRReader_attributes);
	InitialiseTask(StartDriver, &Driver_attributes);

	for (;;)
	{
		run();
	}

	osThreadTerminate(NULL);
}



void print_commands() {
	ROVER_PRINTLN("available applications:");
	ROVER_PRINTLN("    - taskmanager");
	ROVER_PRINTLN("    - eORB");
	ROVER_PRINTLN("    - communicator");
	ROVER_PRINTLN("    - commander");
	ROVER_PRINTLN("    - irreader");
	ROVER_PRINTLN("    - driver");
}


int console_command(int argc, const char *argv[]) {
	// Command indicator
	ROVER_PRINT(">");

	// Check if the command has any arguments
	if (argc < 1) {
		ROVER_PRINTLN("");
		ROVER_PRINT("[Task Manager] No application provided, ");
		print_commands();
		return 1;
	}

	//	Print the received command
	for (int i = 0; i < argc; i++) {
		ROVER_PRINT("%s ", argv[i]);
	}
	ROVER_PRINTLN("");


	// Handle the application
	if (!strcmp(argv[0], "taskmanager")) {
		// Check for number of arguments
		if (argc < 2) {
			ROVER_PRINTLN("[Task Manager] Please provide an command: memory");
			return 1;
		}

		// Task manager memory command
		if (!strcmp(argv[1], "memory")) {
			// Toggle memory display
			display_memory_usage = !display_memory_usage;
			ROVER_PRINTLN("[Task Manager] Memory display toggled!");
			return 0;
		}

	} else if (!strcmp(argv[0], "eORB")) {
		eORB_main(argc, argv);
		return 0;
	} else if (!strcmp(argv[0], "communicator")) {
		communication_main(argc, argv);
		return 0;
	} else if (!strcmp(argv[0], "commander")) {
		commander_main(argc, argv);
		return 0;
	} else if (!strcmp(argv[0], "irreader")) {
		irreader_main(argc, argv);
		return 0;
	} else if (!strcmp(argv[0], "driver")) {
		driver_main(argc, argv);
		return 0;
	}

	// Command wasn't recognised
	ROVER_PRINT("[Task Manager] Unrecognised app, ");
	print_commands();
	return 2;
}



