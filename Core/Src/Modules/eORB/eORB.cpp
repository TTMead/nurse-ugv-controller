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



static int subscriberCount; /* This variable keeps track of the most recent subscriber ID (also an indicator of the current number of subscribers) */
/* Bins to store each fifo data stream */
static osMessageQId bin_handles[number_of_bins];
static int bin_topics[number_of_bins];




void StarteORB(void* ignore)
{
    subscriberCount = 0;

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
    }


    return size;
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
}



int check(int subscriberID) {
	// Check the number if there is atleast one new message in this subscription
	return osMessageQueueGetCount(bin_handles[subscriberID]) > 0;
}



int copy(int subscriberID, void* data) {
	// Copy the message of this subscriber into the data pointer
    return osMessageQueueGet(bin_handles[subscriberID], data, NULL, 0);
}




