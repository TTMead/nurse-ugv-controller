#include "cmsis_os.h"

const osThreadAttr_t Communication_attributes = {
  .name = "Communication",
  .stack_size = 256*48,
  .priority = (osPriority_t) osPriorityHigh,
};

void StartCommunication(void *argument);

void init_wifi_comm();
