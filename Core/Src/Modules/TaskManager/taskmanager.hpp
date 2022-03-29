/******** nurse controller **********
 *
 * @file taskmanager.hpp
 * @desc This module manages
 *
 * @author Timothy Mead
 *
 ************************************/


#include "ugv_cmsis_interface.h" /* Some functions exported to API */
#include "cmsis_os.h"

// #define DISPLAY_MEMORY_USAGE

extern osThreadId_t task_handles[NUMBER_OF_TASKS];
void StartTaskManager(void *argument);

