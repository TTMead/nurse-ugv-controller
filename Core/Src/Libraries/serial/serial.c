#include "serial.h"
#include "cmsis_os.h"
#include <stdarg.h>
#include <stdio.h>


#define TX_TIMEOUT 100
#define MAX_STRING_LENGTH 150

#define MAX_BYTES_IN_RECEIVE_BUFFER 50

uint8_t* wifi_buff_pointer;
uint32_t wifi_counter;

uint8_t* nav_buff_pointer;
uint32_t nav_counter;


/* Buffer to store messages mid transfer */
static char str[MAX_STRING_LENGTH];


void ROVER_PRINT (const char *p_string, ...)
{
	va_list arg;
	int formatted_length;



	va_start (arg, p_string);

	// Format the string
	formatted_length = vsprintf(str, p_string, arg);

	// Check if exceeds max length;
	if (formatted_length > MAX_STRING_LENGTH) {
		ROVER_PRINT("\n\rERROR: MAX STRING LENGTH EXCEEDED. INCREASE MAX STRING LENGTH OR BREAK UP PRINT STATEMENTS\n\r");
		return;
	}

	// Send the string across UART channel
	HAL_UART_Transmit(serial_uart_handle, (uint8_t*) str, formatted_length, TX_TIMEOUT);


	va_end (arg);

	return;
}


void ROVER_PRINTLN(const char *p_string, ...)
{
	va_list arg;
	int formatted_length;



	va_start (arg, p_string);

	// Format the string
	formatted_length = vsprintf(str, p_string, arg);

	// Add newline and cr
	str[formatted_length] = '\n';
	str[formatted_length+1] = '\r';


	// Check if exceeds max length;
	if (formatted_length+2 > MAX_STRING_LENGTH) {
		ROVER_PRINT("\n\rERROR: MAX STRING LENGTH EXCEEDED. INCREASE MAX STRING LENGTH OR BREAK UP PRINT STATEMENTS\n\r");
		return;
	}



	// Send the string across UART channel
	HAL_UART_Transmit(serial_uart_handle, (uint8_t*) str, formatted_length+2, TX_TIMEOUT);

	va_end (arg);

	return;
}


/* HAL_UART_RxCpltCallback
 *
 * This interrupt is called by the HAL middleware whenever a uart message is received on a non-blocking comms port (i.e. wifi or nav)
 */
void HAL_UART_RxCpltCallback(UART_HandleTypeDef *huart)
{
	// Pointers to variables that depend on which uart is being called
	uint32_t* counter_ptr;
	UART_HandleTypeDef* uart_handle;
	uint8_t* serial_buff;

	// Set the pointer variables for wifi or nav
	if (huart == wifi_module_uart_handle) {
		counter_ptr = &wifi_counter;
		uart_handle = wifi_module_uart_handle;
		serial_buff = wifi_buff_pointer;
	} else if (huart == wifi_module_uart_handle) {
		counter_ptr = &nav_counter;
		uart_handle = navigator_uart_handle;
		serial_buff = nav_buff_pointer;
	}

	// If we have the room, increment the counter. Else, ignore the incomming byte
	if (*counter_ptr < MAX_BYTES_IN_RECEIVE_BUFFER) {
		*counter_ptr = *counter_ptr + 1;
	}

	// Reset poll interrupt for this uart
	HAL_UART_Receive_IT(uart_handle, &serial_buff[*counter_ptr], 1);
}





void init_wifi_comm(uint8_t* message_buf) {
	// Save the message buffer for wifi
	wifi_buff_pointer = message_buf;
	wifi_counter = 0;

	// Start polling for wifi serial data
	HAL_UART_Receive_IT (wifi_module_uart_handle, &wifi_buff_pointer[wifi_counter], 1);
}

void init_nav_comm(uint8_t* message_buf) {
	// Save the message buffer for navigation
	nav_buff_pointer = message_buf;
	nav_counter = 0;

	// Start polling for navigator serial data
	HAL_UART_Receive_IT (navigator_uart_handle, &nav_buff_pointer[nav_counter], 1);
}



uint8_t WiFi_read(uint8_t *message_len) {
	// Return negative if no characters available
	if (wifi_counter == 0){
		return 0;
	}

	// Return the length of the message as the counter size
	*message_len = wifi_counter;

	// Reset the counter as the messages have been read
	wifi_counter = 0;

	return 1;
}


uint8_t Nav_read(uint8_t *message_len) {
	// Return negative if no characters available
	if (nav_counter == 0){
		return 0;
	}

	// Return the length of the message as the counter size
	*message_len = nav_counter;

	// Reset the counter as the messages have been read
	nav_counter = 0;

	return 1;
}


uint8_t NAV_send(uint8_t *message_buf, uint8_t message_len) {
	return HAL_UART_Transmit(navigator_uart_handle, message_buf, message_len, TX_TIMEOUT);
}


