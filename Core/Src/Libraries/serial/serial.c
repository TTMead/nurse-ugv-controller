#include "serial.h"
#include "cmsis_os.h"
#include <stdarg.h>
#include <stdio.h>
#include <stdbool.h>

#define TX_TIMEOUT 100
#define MAX_STRING_LENGTH 150

#define MAX_BYTES_IN_RECEIVE_BUFFER 50

uint8_t* serial_buff_pointer;
uint32_t serial_counter;
bool serial_read;

uint8_t* wifi_buff_pointer;
uint32_t wifi_counter;
bool wifi_read;

uint8_t* nav_buff_pointer;
uint32_t nav_counter;
bool nav_read;

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



// Pointers to variables that depend on which uart is being called
uint32_t* counter_ptr;
UART_HandleTypeDef* uart_handle;
uint8_t* memory_buf;
bool* read;

/* HAL_UART_RxCpltCallback
 *
 * This interrupt is called by the HAL middleware whenever a uart message is received on a non-blocking comms port (i.e. wifi or nav)
 */
void HAL_UART_RxCpltCallback(UART_HandleTypeDef *huart)
{
	// Set the appropriate pointers
	if (huart == serial_uart_handle) {
		counter_ptr = &serial_counter;
		uart_handle = serial_uart_handle;
		memory_buf = serial_buff_pointer;
		read = &serial_read;
	} else if (huart == wifi_module_uart_handle) {
		counter_ptr = &wifi_counter;
		uart_handle = wifi_module_uart_handle;
		memory_buf = wifi_buff_pointer;
		read = &wifi_read;
	} else if (huart == navigator_uart_handle) {
		counter_ptr = &nav_counter;
		uart_handle = navigator_uart_handle;
		memory_buf = nav_buff_pointer;
		read = &nav_read;
	}

	// If this buffer has been read
	if (*read) {
		// Move the last chunk of data to the front and reset the count pointer to position 1
		memory_buf[0] = memory_buf[*counter_ptr];
		*counter_ptr = 1;

		// Mark it as unread
		*read = false;
	}
	else {
		// If we have the room, increment the counter. Else, ignore the incoming byte
		if (*counter_ptr < MAX_BYTES_IN_RECEIVE_BUFFER) {
			*counter_ptr = *counter_ptr + 1;
		}
	}

	// Reset poll interrupt for this uart at the next pointer
	HAL_UART_Receive_IT(uart_handle, &memory_buf[*counter_ptr], 1);
}



void init_serial_comm(uint8_t* message_buf) {
	// Save the message buffer for wifi
	serial_buff_pointer = message_buf;
	serial_counter = 0;

	serial_read = false;

	// Start polling for wifi serial data
	HAL_UART_Receive_IT (serial_uart_handle, &serial_buff_pointer[serial_counter], 1);
}

void init_wifi_comm(uint8_t* message_buf) {
	// Save the message buffer for wifi
	wifi_buff_pointer = message_buf;
	wifi_counter = 0;

	wifi_read = false;

	// Start polling for wifi serial data
	HAL_UART_Receive_IT (wifi_module_uart_handle, &wifi_buff_pointer[wifi_counter], 1);
}

void init_nav_comm(uint8_t* message_buf) {
	// Save the message buffer for navigation
	nav_buff_pointer = message_buf;
	nav_counter = 0;

	nav_read = false;

	// Start polling for navigator serial data
	HAL_UART_Receive_IT (navigator_uart_handle, &nav_buff_pointer[nav_counter], 1);
}




uint8_t Serial_read(uint8_t *message_len) {
	// Return negative if no characters available or there is no new unread data
	if (serial_counter == 0 || serial_read == true){
		return 0;
	}

	// Return the length of the message as the counter size
	*message_len = serial_counter;

	// Indicate that this buffer has been read.
	serial_read = true;

	return 1;
}



uint8_t WiFi_read(uint8_t *message_len) {
	// Return negative if no characters available
	if (wifi_counter == 0 || wifi_read == true){
		return 0;
	}

	// Return the length of the message as the counter size
	*message_len = wifi_counter;

	// Indicate that this buffer has been read.
	wifi_read = true;

	return 1;
}


uint8_t Nav_read(uint8_t *message_len) {
	// Return negative if no characters available
	if (nav_counter == 0 || nav_read == true){
		return 0;
	}

	// Return the length of the message as the counter size
	*message_len = nav_counter;

	// Indicate that this buffer has been read.
	serial_read = true;

	return 1;
}


uint8_t NAV_send(uint8_t *message_buf, uint8_t message_len) {
	return HAL_UART_Transmit(navigator_uart_handle, message_buf, message_len, TX_TIMEOUT);
}


