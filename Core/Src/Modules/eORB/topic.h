#include <stdint.h>

#ifndef eORB_topics
#define eORB_topics

#define NUM_TOPICS 2

#define TOPIC_HEARTBEAT 0
#define TOPIC_BLINK 1


struct heartbeat_t {
	uint64_t timestamp;
};

struct blink_t {
	uint64_t timestamp;
    uint32_t frequency;
};





#endif
