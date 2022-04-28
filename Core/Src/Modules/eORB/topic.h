#include <stdint.h>

#ifndef eORB_topics
#define eORB_topics

#define NUM_TOPICS 5

#define TOPIC_HEARTBEAT 0
#define TOPIC_BLINK 1
#define TOPIC_WAYPOINT 2
#define TOPIC_SYS_COMMAND 3
#define TOPIC_DRIVE_COMMAND 4
#define TOPIC_SENSORS 5

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
};


/* Max number of turns between any two waypoints */
#define MAX_TURN_COUNT 5

/* The five waypoints of the map */
enum waypoint {
	A,
	B,
	C,
	D,
	E
};

/* A possible action the drive system can take */
enum drive_cmd {
	LEFT,
	RIGHT,
	STRAIGHT,
	STOP
};

struct drive_command_t {
	uint64_t timestamp;
	drive_cmd commands[MAX_TURN_COUNT];
	waypoint end_waypoint;
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
