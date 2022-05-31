#include <cstring>

#define MAX_NUMBER_WAYPOINTS 5
#define MAX_TURN_COUNT 5 /* Max number of turns between any two waypoints */


/* A possible action the drive system can take */
enum drive_cmd {
	LEFT,
	RIGHT,
	STRAIGHT,
	STOP_TRIP
};


/* Map Travel Matrix of Drive Commands
 *
 *    A B C D E
 * A
 * B
 * C
 * D
 * E
 */

const drive_cmd transition_table[5][5][MAX_TURN_COUNT] = {
	//{{STOP}, 					{RIGHT, STOP}, 		{RIGHT, LEFT, RIGHT, STOP}, 		{STRAIGHT, STOP}, 		{RIGHT, STOP}},
	//{{RIGHT, STOP}, 			{STOP}, 			{RIGHT, LEFT, RIGHT, STOP}, 		{STRAIGHT, STOP}, 		{RIGHT, STOP}},
	//{{STRAIGHT, STOP}, 		{RIGHT, STOP}, 		{STOP}, 							{STRAIGHT, STOP}, 		{RIGHT, STOP}},
	//{{LEFT, STOP}, 			{RIGHT, STOP}, 		{RIGHT, LEFT, RIGHT, STOP}, 		{STOP}, 				{RIGHT, STOP}},
	//{{RIGHT, RIGHT, STOP}, 	{RIGHT, STOP}, 		{RIGHT, LEFT, RIGHT, STOP}, 		{STRAIGHT, STOP}, 		{STOP}}

	{{STOP_TRIP}, 					{RIGHT, STOP_TRIP}, 		{LEFT, STOP_TRIP}, 			{LEFT, STRAIGHT, STOP_TRIP}, 	{RIGHT, STRAIGHT, STOP_TRIP}},
	{{LEFT, STOP_TRIP}, 			{STOP_TRIP}, 				{STRAIGHT, STOP_TRIP}, 		{LEFT, RIGHT, STOP_TRIP}, 		{RIGHT, RIGHT, STOP_TRIP}},
	{{RIGHT, STOP_TRIP}, 			{STRAIGHT, STOP_TRIP}, 		{STOP_TRIP}, 				{LEFT, LEFT, STOP_TRIP}, 		{RIGHT, LEFT, STOP_TRIP}},
	{{STRAIGHT, RIGHT, STOP_TRIP}, 	{LEFT, RIGHT, STOP_TRIP}, 	{RIGHT, RIGHT, STOP_TRIP}, 	{STOP_TRIP}, 					{STRAIGHT, STOP_TRIP}},
	{{STRAIGHT, LEFT, STOP_TRIP}, 	{LEFT, LEFT, STOP_TRIP}, 	{RIGHT, LEFT, STOP_TRIP}, 	{STRAIGHT, STOP_TRIP}, 			{STOP_TRIP}}
};


/* get directions
 *
 * Returns the required driving directions when given a start and exit waypoint
 *
 * @param a - The starting waypoint
 * @param b - The ending waypoint
 * @param buffer - a buffer to store the drive command
 */
int get_directions(int a, int b, drive_cmd *buffer) {
	std::memcpy(buffer, transition_table[a][b], MAX_TURN_COUNT*sizeof(drive_cmd));

	return 0;
}
