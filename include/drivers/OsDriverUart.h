/*!
 *
 *                          ULIPE RTOS VERSION 4
 *
 *
 *  \file OsDriverUart.h
 *
 *  \brief generic UART device driver
 *
 *
 *  Author: FSN
 *
 */

#ifndef OS_DRIVER_UART_H_
#define OS_DRIVER_UART_H_
#if OS_USE_UART_DRIVERS > 0



/* uart generic driver structure  */
typedef struct {
	OsStatus_t (*uLipeUartConfig)(Device_t *this, uint32_t baud, uint32_t configMask);
	OsStatus_t (*uLipeUartSendByte)(Device_t *this, uint8_t c, uint16_t timeout );
	OsStatus_t (*uLipeUartSendStream)(Device_t *this, void *data, uint32_t size, uint16_t timeout);
	OsStatus_t (*uLipeUartReadByte) (Device_t *this, uint8_t *c, uint16_t timeout);
	OsStatus_t (*uLipeUartReadStream)(Device_t *this, void *data, uint32_t expected_size, uint32_t *actual_size, uint16_t timeout);
	OsStatus_t (*uLipeUartEnable)(Device_t *this);
	OsStatus_t (*uLipeUartDisable)(Device_t *this);
}UartDeviceApi_t;


/*
 * UART configuration options
 */
#define UART_8_BITS				0x00
#define UART_9_BITS 			0x01
#define UART_NO_PARIY			(0x00 << 2)
#define UART_PARITY_EVEN		(0x01 << 2)
#define UART_PARITY_ODD			(0x02 << 2)
#define START_BIT_LEN_1			(0x00 << 4)
#define START_BIT_LEN_2			(0x01 << 4)
#define STOP_BIT_LEN_1			(0x00 << 6)
#define STOP_BIT_LEN_2			(0x01 << 6)
#define HARDWARE_FLOW_CTL_EN	(0x01 << 8)


/*!
 * 	uLipeDriverUartInit()
 *
 * 	\brief Inits the uart device driver with baud and user config bitmask
 */
static __inline OsStatus_t uLipeDriverUartInit(Device_t * dev, uint32_t baud_rate, uint32_t configMask)
{
	UartDeviceApi_t *api = (UartDeviceApi_t *)dev->deviceApi;
	OsStatus_t ret;

	if(api) {
		ret = api->uLipeUartConfig(dev, baud_rate, configMask);
	} else {
		ret = kInvalidParam;
	}

	return(ret);
}



/*!
 * 	uLipeDriverUartSendByte()
 *
 * 	\brief Sends a single byte through device uart
 */
static __inline OsStatus_t uLipeDriverUartSendByte(Device_t * dev, uint8_t c, uint16_t timeout)
{
	UartDeviceApi_t *api = (UartDeviceApi_t *)dev->deviceApi;
	OsStatus_t ret;

	if(api) {
		ret = api->uLipeUartSendByte(dev, c, timeout);
	} else {
		ret = kInvalidParam;
	}

	return(ret);
}

/*!
 * 	uLipeDriverUartSendStream()
 *
 * 	\brief Sends a stream of bytes through uart device
 */
static __inline OsStatus_t uLipeDriverUartSendStream(Device_t *dev, void *data, uint32_t size, uint16_t timeout)
{
	UartDeviceApi_t *api = (UartDeviceApi_t *)dev->deviceApi;
	OsStatus_t ret;

	if(api) {
		ret = api->uLipeUartSendStream(dev, data,size, timeout);
	} else {
		ret = kInvalidParam;
	}

	return(ret);
}

/*!
 * 	uLipeUartReadByte()
 *
 * 	\brief Reads a byte that comes from uart device
 */
static __inline OsStatus_t uLipeDriverUartReadByte(Device_t *dev, uint8_t *c, uint16_t timeout)
{
	UartDeviceApi_t *api = (UartDeviceApi_t *)dev->deviceApi;
	OsStatus_t ret;

	if(api) {
		ret = api->uLipeUartReadByte(dev,c,timeout);
	} else {
		ret = kInvalidParam;
	}

	return(ret);
}

/*!
 * 	uLipeDriverUartReadStream()
 *
 * 	\brief Reads a byte stream which comes from uart device
 */
static __inline OsStatus_t uLipeDriverUartReadStream(Device_t *dev, void *data, uint32_t expected_size, uint32_t *actual_size, uint16_t timeout)
{
	UartDeviceApi_t *api = (UartDeviceApi_t *)dev->deviceApi;
	OsStatus_t ret;

	if(api) {
		ret = api->uLipeUartReadStream(dev, data, expected_size, actual_size, timeout);
	} else {
		ret = kInvalidParam;
	}

	return(ret);
}

/*!
 * 	uLipeDriverUartEnable()
 *
 * 	\briefs enable a previous configured uart
 */
static __inline OsStatus_t uLipeDriverUartEnable(Device_t *dev)
{
	UartDeviceApi_t *api = (UartDeviceApi_t *)dev->deviceApi;
	OsStatus_t ret;

	if(api) {
		ret = api->uLipeUartEnable(dev);
	} else {
		ret = kInvalidParam;
	}

	return(ret);
}


/*!
 * 	uLipeDriverUartDisable()
 *
 * 	\briefs powers off a uart device keep its configuration
 */
static __inline OsStatus_t uLipeDriverUartDisable(Device_t *dev)
{
	UartDeviceApi_t *api = (UartDeviceApi_t *)dev->deviceApi;
	OsStatus_t ret;

	if(api) {
		ret = api->uLipeUartDisable(dev);
	} else {
		ret = kInvalidParam;
	}

	return(ret);
}

#endif
#endif
