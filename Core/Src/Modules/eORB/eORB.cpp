/******** nurse controller **********
 *
 * @file eORB.cpp
 * @desc embedded Object Request Broker. Facilitates pub/sub communication between modules.
 *
 *
 * @author Timothy Mead
 *
 ************************************/


#include "eORB.hpp"
#include "serial.h"
#include <string.h>




static int subscriberCount; /* This variable keeps track of the most recent subscriber ID (also an indicator of the current number of subscribers) */
/* Bins to store each fifo data stream */
static osMessageQId bin_handles[number_of_bins];
static int bin_topics[number_of_bins];

static osMessageQId topic_listener_bin_handles[NUM_TOPICS];


/*
 * get_size_of_topic
 *
 * Returns the msg packet size for a given topic
 *
 * @param none
 * @return int the sizeof the struct corresponding to the given topic
 */
static int get_size_of_topic(int topic) {
    int size = 0;
    switch (topic) {
        case TOPIC_HEARTBEAT:
            size = sizeof(heartbeat_t);
            break;
        case TOPIC_BLINK:
            size = sizeof(blink_t);
            break;
        case TOPIC_WAYPOINT:
			size = sizeof(waypoint_t);
			break;
        case TOPIC_SYS_COMMAND:
			size = sizeof(system_command_t);
			break;
        case TOPIC_SENSORS:
			size = sizeof(sensor_values_t);
			break;
        case TOPIC_OBSTACLE:
			size = sizeof(obstacle_t);
			break;
    }



    return size;
}


void StarteORB(void* ignore)
{
	// Start with 0 subscriptions
    subscriberCount = 0;

    // Initialise the topic listener bins
    for (int i = 0; i < NUM_TOPICS; i++) {
    	topic_listener_bin_handles[i] = osMessageQueueNew(1, get_size_of_topic(i), NULL);
    }

    for(;;)
    {
    	run();
    }

    osThreadTerminate(NULL);
}


static void run()
{
	osDelay(1000);
}





int subscribe(int topic) {
    bin_topics[subscriberCount] = topic;

    bin_handles[subscriberCount] = osMessageQueueNew(msg_queue_len, get_size_of_topic(topic), NULL);


    subscriberCount += 1;


    return subscriberCount-1;
}


void publish(int topic, void* data) {

    // For each subscriber queue
    for (int i = 0; i < subscriberCount; i++) {

        // If this subscriber queue matches this topic
        if (bin_topics[i] == topic) {
            // Add the data to queue
        	osMessageQueuePut(bin_handles[i], data, 0U, 0U);
        }
    }

    // Update the data in listener bin
    osMessageQueueReset(topic_listener_bin_handles[topic]);
    osMessageQueuePut(topic_listener_bin_handles[topic], data, 0U, 0U);
}



int check(int subscriberID) {
	// Check the number if there is atleast one new message in this subscription
	return osMessageQueueGetCount(bin_handles[subscriberID]) > 0;
}



int copy(int subscriberID, void* data) {
	// Copy the message of this subscriber into the data pointer
    return osMessageQueueGet(bin_handles[subscriberID], data, NULL, 0);
}


int eORB_main(int argc, const char *argv[]) {
	if (argc < 1) {
		ROVER_PRINTLN("[eORB] Please provide a command: listen");
		return 1;
	}

	if (!strcmp(argv[0], "listen")) {
		if (!strcmp(argv[0], "heartbeat")) {
			if (osMessageQueueGetCount(topic_listener_bin_handles[0]) > 0) {
				heartbeat_t heartbeat_msg;
				osMessageQueueGet(topic_listener_bin_handles[0], &heartbeat_msg, NULL, 0);

				ROVER_PRINTLN("Published %f milliseconds ago.", (float) (HAL_GetTick() - heartbeat_msg.timestamp));
			} else {
				ROVER_PRINTLN("Topic never published");

			}

			return 0;
		}

		if (!strcmp(argv[0], "blink")) {
			if (osMessageQueueGetCount(topic_listener_bin_handles[1]) > 0) {
				blink_t blink_msg;
				osMessageQueueGet(topic_listener_bin_handles[1], &blink_msg, NULL, 0);

				ROVER_PRINTLN("Published %f milliseconds ago.", (float) (HAL_GetTick() - blink_msg.timestamp));
				ROVER_PRINTLN("Freq; %d", (int) (blink_msg.frequency));
			} else {
				ROVER_PRINTLN("Topic never published");
			}
			return 0;
		}

		ROVER_PRINTLN("Unknown topic");
		return 2;
	}

	ROVER_PRINTLN("Unknown command");
	return 1;
}




