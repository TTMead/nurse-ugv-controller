#include "blinksetter.hpp"
#include "eORB.hpp"

blink_t msg;

static void run() {

	msg.timestamp = 1;
	msg.frequency = 50;
	publish(TOPIC_BLINK, &msg);
	osDelay(1000);

	msg.timestamp = 1;
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
