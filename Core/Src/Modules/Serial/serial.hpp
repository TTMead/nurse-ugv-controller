#include <stdint.h>
#include "../modules.h"

extern UART_HandleTypeDef *serial_uart_handle;
extern UART_HandleTypeDef *navigator_uart_handle;


/* ROVER PRINT
 *
 * Formats a string with the given arguments across the serial UART line.
 *
 */
void ROVER_PRINT (const char *p_string, ...);


/* ROVER PRINTLN
 *
 * Formats a string with the given arguments across the serial UART line.
 * Sends a linebreak and carriage return afterwards.
 */
void ROVER_PRINTLN (const char *p_string, ...);


/* NAV Send
 *
 * Sends a message to the Navigator component (Raspberry pi)
 *
 * @param message_buf - pointer to the message buffer to send
 * @param message_len - length of the message in bytes
 *
 * @returns 0 if successfully sent
 * 			1 if unsuccessful
 */
uint8_t NAV_send(uint8_t *message_buf, uint8_t message_len);


/* NAV Read
 *
 * Reads a serial message to the Navigator component (Raspberry pi)
 *
 * @param message_buf - pointer to the buffer to store the message
 * @param message_len - length of the message in bytes
 *
 * @returns 0 if a message was found
 * 			1 if unsuccessful or no message found
 */
uint8_t NAV_read(uint8_t *message_buf, uint8_t *message_len);

