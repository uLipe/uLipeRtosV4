/*!
 *
 *                          ULIPE RTOS VERSION 4
 *
 *
 *  \file OsDriverI2C.h
 *
 *  \brief generic I2C device driver
 *
 *
 *  Author: FSN
 *
 */

#ifndef __OS_DRIVER_I2C_H_
#define __OS_DRIVER_I2C_H_

#if OS_USE_I2C_DRIVERS > 0

/*
 * I2C device API structure
 */
typedef struct {
	OsStatus_t (*I2cDriverConfig)(Device_t *dev, uint32_t expectedSpeed, uint32_t *actualSpeed, uint32_t configMask);
	OsStatus_t (*I2cDriverEnable)(Device_t *dev);
	OsStatus_t (*I2cDriverDisable)(Device_t *dev);
	OsStatus_t (*I2cSetSlaveAddr)(Device_t *dev, uint16_t slaveAddr);
	OsStatus_t (*I2cDriverWriteByte)(Device_t *dev, uint16_t slaveAddr, uint16_t addr, uint8_t c, uint16_t timeout);
	OsStatus_t (*I2cDriverWriteStream)(Device_t *dev,uint16_t slaveAddr, uint16_t base_addr, void *data, uint32_t len, uint16_t timeout);
	OsStatus_t (*I2cDriverReadByte)(Device_t *dev, uint16_t slaveAddr, uint16_t addr, uint8_t *c, uint16_t timeout);
	OsStatus_t (*I2cDriverReadStream)(Device_t *dev,uint16_t slaveAddr, uint16_t base_addr, void *data, uint32_t expectedLen, uint32_t *actualLen, uint16_t timeout);
}I2cDriverApi_t;


/*
 * I2c device configuration masks
 */
#define I2C_DEVICE_MASTER 		0x00
#define I2C_DEVICE_SLAVE  		0x01
#define I2C_7BIT_ADDR			(0x00 << 2)
#define I2C_10BIT_ADDR			(0x01 << 2)


/*!
 *  uLipeDriverI2cConfig()
 *
 *  \brief configures the I2c device driver
 */
static __inline OsStatus_t uLipeDriverI2cConfig(Device_t *dev, uint32_t expectedSpeed, uint32_t *actualSpeed, uint32_t configMask)
{
	OsStatus_t ret;
	I2cDriverApi_t *api = (I2cDriverApi_t *)dev->deviceApi;

	if(api) {
		ret = api->I2cDriverConfig(dev,expectedSpeed,actualSpeed,configMask);
	} else {
		ret = kInvalidParam;
	}

	return(ret);
}

/*!
 *  uLipeDriverI2cEnable()
 *
 *  \brief Enables  the I2c device driver
 */
static __inline OsStatus_t uLipeDriverI2cEnable(Device_t *dev)
{
	OsStatus_t ret;
	I2cDriverApi_t *api = (I2cDriverApi_t *)dev->deviceApi;

	if(api) {
		ret = api->I2cDriverEnable(dev);
	} else {
		ret = kInvalidParam;
	}

	return(ret);
}



/*!
 *  uLipeDriverI2cDisable()
 *
 *  \brief Disables the I2c device driver
 */
static __inline OsStatus_t uLipeDriverI2cDisable(Device_t *dev)
{
	OsStatus_t ret;
	I2cDriverApi_t *api = (I2cDriverApi_t *)dev->deviceApi;

	if(api) {
		ret = api->I2cDriverDisable(dev);
	} else {
		ret = kInvalidParam;
	}

	return(ret);
}

/*!
 *  uLipeDriverI2cSetSlaveAddr()
 *
 *  \brief Set the slave address of the I2c device driver
 */
static __inline OsStatus_t uLipeDriverI2cSetSlaveAddr(Device_t *dev, uint16_t slaveAddr)
{
	OsStatus_t ret;
	I2cDriverApi_t *api = (I2cDriverApi_t *)dev->deviceApi;

	if(api) {
		ret = api->I2cSetSlaveAddr(dev, slaveAddr);
	} else {
		ret = kInvalidParam;
	}

	return(ret);
}


/*!
 *  uLipeDriverI2cWriteByte()
 *
 *  \brief writes a byte on data register of the I2c device driver
 */
static __inline OsStatus_t uLipeDriverI2cWriteByte(Device_t *dev, uint16_t slaveAddr, uint16_t addr,
			uint8_t c, uint16_t timeout)
{
	OsStatus_t ret;
	I2cDriverApi_t *api = (I2cDriverApi_t *)dev->deviceApi;

	if(api) {
		ret = api->I2cDriverWriteByte(dev,slaveAddr,addr,c,timeout);
	} else {
		ret = kInvalidParam;
	}

	return(ret);
}


/*!
 *  uLipeDriverI2cWriteStream()
 *
 *  \brief writes a stream of bytes on data register of the I2c device driver
 */
static __inline OsStatus_t uLipeDriverI2cWriteStream(Device_t *dev,uint16_t slaveAddr, uint16_t base_addr,
			void *data, uint32_t len, uint16_t timeout)
{
	OsStatus_t ret;
	I2cDriverApi_t *api = (I2cDriverApi_t *)dev->deviceApi;

	if(api) {
		ret = api->I2cDriverWriteStream(dev,slaveAddr,base_addr,data,len,timeout);
	} else {
		ret = kInvalidParam;
	}

	return(ret);
}

/*!
 *  uLipeDriverI2cReadByte()
 *
 *  \brief reads a single byte from data register of the I2c device driver
 */
static __inline OsStatus_t uLipeDriverI2cReadByte(Device_t *dev, uint16_t slaveAddr, uint16_t addr,
			uint8_t *c, uint16_t timeout)
{
	OsStatus_t ret;
	I2cDriverApi_t *api = (I2cDriverApi_t *)dev->deviceApi;

	if(api) {
		ret = api->I2cDriverReadByte(dev,slaveAddr,addr,c,timeout);
	} else {
		ret = kInvalidParam;
	}

	return(ret);
}


/*!
 *  uLipeDriverI2cReadStream()
 *
 *  \brief reads a stream of bytes from data register of the I2c device driver
 */
static __inline OsStatus_t uLipeDriverI2cReadStream(Device_t *dev,uint16_t slaveAddr, uint16_t base_addr, void *data,
			uint32_t expectedLen, uint32_t *actualLen, uint16_t timeout)
{
	OsStatus_t ret;
	I2cDriverApi_t *api = (I2cDriverApi_t *)dev->deviceApi;

	if(api) {
		ret = api->I2cDriverReadStream(dev,slaveAddr,base_addr,data,expectedLen,actualLen,timeout);
	} else {
		ret = kInvalidParam;
	}

	return(ret);
}

#endif
#endif
