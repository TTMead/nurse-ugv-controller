#include <stdint.h>
#include "../modules.h"

extern UART_HandleTypeDef *serial_uart_handle;


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
