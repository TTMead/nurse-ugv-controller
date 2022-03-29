// API that exposes the rover codebase

#ifndef ROVER_INTERFACE
#define ROVER_INTERFACE

#include "stm32f4xx_hal.h"
#include "cmsis_os.h"


#define NUMBER_OF_TASKS 5
#define TASK_MANAGER_STACK_SIZE 128*4


#ifdef __cplusplus
extern "C" {
#endif

// Functions that need to be exported to the main code
extern UART_HandleTypeDef *serial_uart_handle;
extern UART_HandleTypeDef *navigator_uart_handle;
extern UART_HandleTypeDef *wifi_module_uart_handle;
extern osThreadId_t task_handles[NUMBER_OF_TASKS];
extern void StartTaskManager(void *argument);

#ifdef __cplusplus
}
#endif


#endif


