#include "cmsis_os.h"

const osThreadAttr_t BlinkSetter_attributes = {
  .name = "Blink Setter",
  .stack_size = 256,
  .priority = (osPriority_t) osPriorityNormal,
};

void StartBlinkSetter(void *argument);

