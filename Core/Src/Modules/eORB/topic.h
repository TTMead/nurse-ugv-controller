#include <stdint.h>

#ifndef eORB_topics
#define eORB_topics

#define NUM_TOPICS 6

#define TOPIC_HEARTBEAT 0
#define TOPIC_BLINK 1
#define TOPIC_WAYPOINT 2
#define TOPIC_SYS_COMMAND 3
#define TOPIC_SENSORS 5
#define TOPIC_OBSTACLE 6

struct heartbeat_t {
	uint64_t timestamp;
};

struct blink_t {
	uint64_t timestamp;
    uint32_t frequency;
};

struct waypoint_t {
	uint64_t timestamp;
	uint8_t waypoint_num;
};


struct system_command_t {
	uint64_t timestamp;
	bool arm;
	bool disarm;
	bool estop;
	bool waypoint_reached;
	bool serving_completed;
	bool peripheral_items_collected;
};

struct obstacle_t {
	uint64_t timestamp;
	bool obstacle_detected;
};




struct sensor_values_t {
	uint64_t timestamp;
	uint16_t s0;
	uint16_t s1;
	uint16_t s2;
	uint16_t s3;
	uint16_t s4;
	uint16_t s5;
	uint16_t s6;
	uint16_t s7;
};





#endif
