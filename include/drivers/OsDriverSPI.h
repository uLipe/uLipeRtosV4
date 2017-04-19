/*!
 *
 *                          ULIPE RTOS VERSION 4
 *
 *
 *  \file OsDriverSPI.h
 *
 *  \brief generic SPI device driver
 *
 *
 *  Author: FSN
 *
 */

#ifndef __OS_DRIVER_SPI_H_
#define __OS_DRIVER_SPI_H_

#if OS_USE_SPI_DRIVERS > 0

/*
 * SPI driver API structure
 */
typedef struct {
	OsStatus_t (*SpiDriverConfig)(Device_t *dev, uint32_t expectedSpeed, uint32_t *actualSpeed, uint32_t configMask);
	OsStatus_t (*SpiDriverEnable)(Device_t *dev);
	OsStatus_t (*SpiDriverDisable)(Device_t *dev);
	OsStatus_t (*SpiDriverWriteByte)(Device_t *dev, uint8_t c, uint16_t timeout);
	OsStatus_t (*SpiDriverWriteStream)(Device_t *dev, void *data, uint32_t len, uint16_t timeout);
	OsStatus_t (*SpiDriverReadByte)(Device_t *dev, uint8_t *c, uint16_t timeout);
	OsStatus_t (*SpiDriverReadStream)(Device_t *dev, void *data, uint32_t expectedLen, uint32_t *actualLen, uint16_t timeout);
}SpiDriverApi_t;



/*
 * SPI device configuration mask:
 */
#define SPI_DEVICE_MASTER	0x00
#define SPI_DEVICE_SLAVE	0x01
#define SPI_DEVICE_MODE_0	(0x00 << 2)
#define SPI_DEVICE_MODE_1	(0x01 << 2)
#define SPI_DEVICE_MODE_2	(0x02 << 2)
#define SPI_DEVICE_MODE_3   (0x03 << 2)
#define SPI_DEVICE_HW_CS	(0x00 << 4)
#define SPI_DEVICE_SW_CS	(0x01 << 4)


/*!
 *  uLipeDriverSpiConfig()
 *
 *  \brief configures the spi device driver
 */
static inline OsStatus_t uLipeDriverSpiConfig(Device_t *dev, uint32_t expectedSpeed, uint32_t *actualSpeed, uint32_t configMask)
{
	OsStatus_t ret;
	SpiDriverApi_t *api = (SpiDriverApi_t *)dev->deviceApi;

	if(api) {
		ret = api->SpiDriverConfig(dev,expectedSpeed,actualSpeed,configMask);
	} else {
		ret = kInvalidParam;
	}

	return(ret);
}

/*!
 *  uLipeDriverSpiEnable()
 *
 *  \brief Enables  the spi device driver
 */
static inline OsStatus_t uLipeDriverSpiEnable(Device_t *dev)
{
	OsStatus_t ret;
	SpiDriverApi_t *api = (SpiDriverApi_t *)dev->deviceApi;

	if(api) {
		ret = api->SpiDriverEnable(dev);
	} else {
		ret = kInvalidParam;
	}

	return(ret);
}


/*!
 *  uLipeDriverSpiDisable()
 *
 *  \brief Disables the spi device driver
 */
static inline OsStatus_t uLipeDriverSpiDisable(Device_t *dev)
{
	OsStatus_t ret;
	SpiDriverApi_t *api = (SpiDriverApi_t *)dev->deviceApi;

	if(api) {
		ret = api->SpiDriverDisable(dev);
	} else {
		ret = kInvalidParam;
	}

	return(ret);
}

/*!
 *  uLipeDriverSpiWriteByte()
 *
 *  \brief writes a byte on data register of the spi device driver
 */
static inline OsStatus_t uLipeDriverSpiWriteByte(Device_t *dev, uint8_t c, uint16_t timeout)
{
	OsStatus_t ret;
	SpiDriverApi_t *api = (SpiDriverApi_t *)dev->deviceApi;

	if(api) {
		ret = api->SpiDriverWriteByte(dev,c,timeout);
	} else {
		ret = kInvalidParam;
	}

	return(ret);
}


/*!
 *  uLipeDriverSpiWriteStream()
 *
 *  \brief writes a stream of bytes on data register of the spi device driver
 */
static inline OsStatus_t uLipeDriverSpiWriteStream(Device_t *dev, void *data, uint32_t len, uint16_t timeout)
{
	OsStatus_t ret;
	SpiDriverApi_t *api = (SpiDriverApi_t *)dev->deviceApi;

	if(api) {
		ret = api->SpiDriverWriteStream(dev,data,len,timeout);
	} else {
		ret = kInvalidParam;
	}

	return(ret);
}

/*!
 *  uLipeDriverSpiReadByte()
 *
 *  \brief reads a single byte from data register of the spi device driver
 */
static inline OsStatus_t uLipeDriverSpiReadByte(Device_t *dev, uint8_t *c, uint16_t timeout)
{
	OsStatus_t ret;
	SpiDriverApi_t *api = (SpiDriverApi_t *)dev->deviceApi;

	if(api) {
		ret = api->SpiDriverReadByte(dev,c,timeout);
	} else {
		ret = kInvalidParam;
	}

	return(ret);
}


/*!
 *  uLipeDriverSpiReadStream()
 *
 *  \brief reads a stream of bytes from data register of the spi device driver
 */
static inline OsStatus_t uLipeDriverSpiReadStream(Device_t *dev, void *data, uint32_t expectedLen, uint32_t *actualLen, uint16_t timeout)
{
	OsStatus_t ret;
	SpiDriverApi_t *api = (SpiDriverApi_t *)dev->deviceApi;

	if(api) {
		ret = api->SpiDriverReadStream(dev,data,expectedLen,actualLen,timeout);
	} else {
		ret = kInvalidParam;
	}

	return(ret);
}

#endif
#endif
