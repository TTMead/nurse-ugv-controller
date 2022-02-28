#include "blinky.hpp"
#include "stm32f4xx_hal.h"
#include "eORB.hpp"

int blink_sub;
int freq;

static void run() {
	// If blink sub has new data
	if (check(blink_sub)) {
		// Store the new freq
		copy(blink_sub, &freq);
	}

	// Toggle the pin at the given frequency
	HAL_GPIO_TogglePin (GPIOA, GPIO_PIN_5);
	osDelay(freq);
}


void StartBlinky(void *argument) {
	blink_sub = subscribe(TOPIC_BLINK);
	freq = 1000;

	for (;;)
	{
		run();
	}

	osThreadTerminate(NULL);
}







