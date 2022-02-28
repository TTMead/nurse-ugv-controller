#include "cmsis_os.h"
#include "topic.h"

#define number_of_bins 50 /* The number of xQueue bins to store data for */
#define msg_queue_len 5 /* The max number of messages an individual stream can hold */


const osThreadAttr_t eORB_attributes = {
  .name = "eORB",
  .stack_size = 256*32,
  .priority = (osPriority_t) osPriorityHigh,
};


/*
 * task_eORB
 *
 * Entry point for the eORB task
 *
 * @param none
 * @return none
 */
void StarteORB(void* ignore);

/*
 * run
 *
 * Main run loop for the eORB task
 *
 * @param none
 * @return none
 */
static void run();




/*
 * subscribe
 *
 * Subscribes to a topic by creating a new xQueueStream
 *
 * @param int The topic to subscribe to
 * @return int The subscriber ID of the newly created stream
 */
int subscribe(int topic);

/*
 * publish
 *
 * Publishes data to all streams of a given topic
 *
 * @param int The topic to publish to to
 * @param void* A null pointer to the data to publish
 *
 * @return int The flag of the operation
 */
void publish(int topic, void* data);

/*
 * check
 *
 * Checks if data is available for a given subscriberID stream
 *
 * @param int The topic to check
 *
 * @return int 1 for data available, else 0
 */
int check(int subscriberID);


int copy(int subscriberID, void* data);
