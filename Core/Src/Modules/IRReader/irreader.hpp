#include "cmsis_os.h"
#include "stm32f4xx_hal.h"
#include "serial.h"

extern ADC_HandleTypeDef *sensor_adc_handle;

const osThreadAttr_t IRReader_attributes = {
  .name = "IR Reader",
  .stack_size = 256,
  .priority = (osPriority_t) osPriorityHigh,
};

void StartIRReader(void *argument);

