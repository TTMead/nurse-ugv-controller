/******** nurse controller **********
 *
 * @file taskmanager.hpp
 * @desc This module manages the memory and initialisation of the RTOS tasks
 *
 * @author Timothy Mead
 *
 ************************************/


#include "ugv_cmsis_interface.h" /* Some functions exported to API */
#include "cmsis_os.h"

// Uncomment this term to display memory usage of tasks to console
// #define DISPLAY_MEMORY_USAGE

extern osThreadId_t task_handles[NUMBER_OF_TASKS];
void StartTaskManager(void *argument);

/* Sends a command to the task manager console */
int console_command(int argc, const char *argv[]);
