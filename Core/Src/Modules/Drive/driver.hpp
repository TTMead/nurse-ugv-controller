/******** nurse controller **********
 *
 * @file driver.hpp
 * @desc Controls the driving of the motor
 *
 * @author Evan Devoy
 *
 ************************************/

#include "cmsis_os.h"
#include "pid.h"

const osThreadAttr_t Driver_attributes = {
  .name = "Drive Control",
  .stack_size = 256*8,
  .priority = (osPriority_t) osPriorityHigh,
};

void StartDriver(void *argument);
int driver_main(int argc, const char *argv[]);
