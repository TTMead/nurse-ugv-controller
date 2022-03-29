/******** nurse controller **********
 *
 * @file blinky.hpp
 * @desc Handles the blinking of the status LED
 *
 * @author Timothy Mead
 *
 ************************************/


#include "cmsis_os.h"

const osThreadAttr_t Blinky_attributes = {
  .name = "Blinky",
  .stack_size = 256,
  .priority = (osPriority_t) osPriorityNormal,
};

void StartBlinky(void *argument);

