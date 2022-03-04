#include "serial.hpp"
#include <stdarg.h>
#include <stdio.h>


#define TX_TIMEOUT 100
#define MAX_STRING_LENGTH 150



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


uint8_t NAV_send(uint8_t *message_buf, uint8_t message_len) {
	return HAL_UART_Transmit(navigator_uart_handle, message_buf, message_len, TX_TIMEOUT);
}



/* Reads a byte
 *
 * @returns 0 if byte was found
 * 			1 if byte was not found
 */
uint8_t NAV_read_byte(uint8_t *byte_buf) {
	uint8_t success = HAL_UART_Receive(navigator_uart_handle, byte_buf, 1, 0);

	return (success == HAL_TIMEOUT || success == HAL_ERROR || success == HAL_BUSY);
}



uint8_t NAV_read(uint8_t *message_buf, uint8_t *message_len) {
	if (NAV_read_byte(&message_buf[0]) == 0) {
		uint8_t counter = 1;

		while(true) {
			if (NAV_read_byte(&message_buf[counter]) == 0) {
				counter ++;
			} else {
				return 0;
			}
		}
	}

	return 1;
}




