/******** nurse controller **********
 *
 * @file communication.hpp
 * @desc Receives UART information from the ESP WiFi module and handles it appropriately.
 *
 * @author Timothy Mead
 *
 ************************************/

#include "cmsis_os.h"

const osThreadAttr_t Communication_attributes = {
  .name = "Communication",
  .stack_size = 256*48,
  .priority = (osPriority_t) osPriorityHigh4,
};

void StartCommunication(void *argument);

void init_wifi_comm();

// Console interface
int communication_main(int argc, const char *argv[]);
