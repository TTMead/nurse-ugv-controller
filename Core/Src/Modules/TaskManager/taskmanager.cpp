// External libraries
#include "stdlib.h"


// Internal headers
#include "serial.hpp"
#include "blinky.hpp"
#include "blinksetter.hpp"
#include "taskmanager.hpp"
#include "eORB.hpp"


osThreadId_t task_handles[NUMBER_OF_TASKS];
static uint16_t task_stack_size[NUMBER_OF_TASKS];
static uint8_t task_counter = 1;


static void run()
{
	#ifdef DISPLAY_MEMORY_USAGE
	ROVER_PRINTLN("================");
	ROVER_PRINTLN("Memory Usage");
	ROVER_PRINTLN("================");
	for (int i = 0; i < NUMBER_OF_TASKS; i++) {
		ROVER_PRINT(osThreadGetName(task_handles[i]));
		ROVER_PRINTLN(" heap: %u/%u bytes in use", task_stack_size[i] - osThreadGetStackSpace(task_handles[0]), task_stack_size[i]);
	}
	ROVER_PRINTLN("");
	#endif

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
	// Add the task manager stack size
	task_stack_size[0] = TASK_MANAGER_STACK_SIZE;

	// Initialise other tasks
	InitialiseTask(StarteORB, &eORB_attributes);
	InitialiseTask(StartBlinkSetter, &BlinkSetter_attributes);
	InitialiseTask(StartBlinky, &Blinky_attributes);

	for (;;)
	{
		run();
	}

	osThreadTerminate(NULL);
}


