/*!
 *
 *                          ULIPE RTOS VERSION 4
 *
 *
 *  \file OsDriverUart.h
 *
 *  \brief generic UART device driver
 *
 *  In this file the is contained all the definitions about inter cpu messaging
 *
 *  Author: FSN
 *
 */

#ifndef OS_DRIVER_UART_H_
#define OS_DRIVER_UART_H_


/* uart generic driver structure  */
typedef struct {
	OsStatus_t (*uLipeUartConfig)(Device_t *this, uint32_t baud, uint32_t configMask);
	OsStatus_t (*uLipeUartSendByte)(Device_t *this, char c, uint16_t timeout );
	OsStatus_t (*uLipeUartSendStream)(Device_t *this, void *data, uint32_t size, uint16_t timeout);
	OsStatus_t (*uLipeUartReadByte) (Device_t *this, char *c, uint16_t timeout);
	OsStatus_t (*uLipeUartReadStream)(Device_t *this, void *data, uint32_t expected_size, uint32_t *actual_size, uint16_t timeout);
}UartDeviceApi_t;


/*
 * UART configuration options
 */
#define UART_8_BITS			0x00
#define UART_9_BITS 		0x01
#define UART_PARITY_ODD		(0x00 << 2)
#define UART_PARITY_EVEN	(0x01 << 2)
#define UART_NO_PARIY		(0x02 << 2)





#endif
