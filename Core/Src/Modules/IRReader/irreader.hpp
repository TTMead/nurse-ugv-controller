/******** nurse controller **********
 *
 * @file irreader.hpp
 * @desc Reads the IR sensors and publishes to the sensor topic.
 *
 * @author Timothy Mead
 *
 ************************************/

#include "cmsis_os.h"
#include "stm32f4xx_hal.h"
#include "serial.h"
#include "string.h"

extern ADC_HandleTypeDef *sensor_adc_handle;

const osThreadAttr_t IRReader_attributes = {
  .name = "IR Reader",
  .stack_size = 256*2,
  .priority = (osPriority_t) osPriorityHigh,
};

void StartIRReader(void *argument);
int irreader_main(int argc, const char *argv[]);
