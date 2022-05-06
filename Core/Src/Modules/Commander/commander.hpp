/******** nurse controller **********
 *
 * @file commander.hpp
 * @desc Navigator and state machine, commands other modules to act.
 *
 *
 * @author Timothy Mead
 *
 ************************************/


#include "cmsis_os.h"

const osThreadAttr_t Commander_attributes = {
  .name = "Commander",
  .stack_size = 256*8,
  .priority = (osPriority_t) osPriorityHigh,
};


void StartCommander(void *argument);


int commander_main(int argc, const char *argv[]);

