#include <stdint.h>

#ifndef eORB_topics
#define eORB_topics

#define TOPIC_HEARTBEAT 0
#define TOPIC_BLINK 1

class topic {
public:
	uint64_t timestamp;
};

class heartbeat_t : public topic {};

class blink_t : public topic {
public:
    uint32_t frequency;
};





#endif
