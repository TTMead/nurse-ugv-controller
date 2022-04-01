/*********************************************************************
 * serial.h
 *
 * This library serves as an interface between the stm32f411re
 * UART HAL and the nurse UGV code base. This library will
 * have to be updated for other HAL's if a separate microcontroller
 * is used.
 *********************************************************************/
#include <stdint.h>
#include <stdarg.h>
#include <stdio.h>
#include "ugv_cmsis_interface.h"


#define TX_TIMEOUT 100
#define MAX_STRING_LENGTH 150


/* Pointer to the debug UART handle */
extern UART_HandleTypeDef *serial_uart_handle;

/* Pointer to the navigator UART handle */
extern UART_HandleTypeDef *navigator_uart_handle;

/* Pointer to the navigator UART handle */
extern UART_HandleTypeDef *wifi_module_uart_handle;

#ifdef __cplusplus
extern "C" {
#endif


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
void ROVER_PRINTLN(const char *p_string, ...);


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
 * @param message_len - pointer to store the length of the message in bytes
 * @returns 1 if a message was found
 * 			0 if unsuccessful or no message found
 */
uint8_t NAV_read(uint8_t *message_len);


/* Initialise Navigator Communication
 *
 * Begins interrupt polling for data on the Navigator UART channel.
 *
 * @param message_buf - the message buffer used to contain any data received.
 */
void init_nav_comm(uint8_t* message_buf);


/* WiFi Read
 *
 * Reads a serial message from the WiFi Module
 *
 * @param message_len - pointer to store the length of the message in bytes
 * @returns 1 if a message was found
 * 			0 if unsuccessful or no message found
 */
uint8_t WiFi_read(uint8_t *message_len);


/* Initialise Wifi Communication
 *
 * Begins interrupt polling for data on the WiFi UART channel.
 *
 * @param message_buf - the message buffer used to contain any data received.
 */
void init_wifi_comm(uint8_t* message_buf);


/* Serial Read
 *
 * Reads a serial message from the serial debug port
 *
 * @param message_len - pointer to store the length of the message in bytes
 * @returns 1 if a message was found
 * 			0 if unsuccessful or no message found
 */
uint8_t Serial_read(uint8_t *message_len);


/* Initialise Serial Communication
 *
 * Begins interrupt polling for data on the Serial debug UART channel.
 *
 * @param message_buf - the message buffer used to contain any data received.
 */
void init_serial_comm(uint8_t* message_buf);


#ifdef __cplusplus
}
#endif
