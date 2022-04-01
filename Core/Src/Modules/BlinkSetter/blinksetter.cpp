#include "blinksetter.hpp"
#include "eORB.hpp"
#include "stm32f4xx_hal.h"

static struct blink_t msg;

static void run() {
	msg.timestamp = HAL_GetTick();
	msg.frequency = 50;
	publish(TOPIC_BLINK, &msg);
	osDelay(1000);

	msg.timestamp = HAL_GetTick();
	msg.frequency = 300;
	publish(TOPIC_BLINK, &msg);
	osDelay(1000);
}


void StartBlinkSetter(void *argument) {
	for (;;)
	{
		run();
	}

	osThreadTerminate(NULL);
}
