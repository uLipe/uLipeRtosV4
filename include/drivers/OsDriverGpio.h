/*!
 *
 *                          ULIPE RTOS VERSION 4
 *
 *
 *  \file OsDriverGpio.h
 *
 *  \brief generic GPIO device driver
 *
 *
 *  Author: FSN
 *
 */

#ifndef __OS_DRIVER_GPIO_H_
#define __OS_DRIVER_GPIO_H_
#if OS_USE_GPIO_DRIVERS > 0

/*
 * Gpio callback type
 */
typedef void (*uLipeGpioCallBack_t) (Device_t *dev);

/*
 *  GPIO Device API structure
 */
typedef struct {
	OsStatus_t (*uLipeGpioConfigureRow)(Device_t *this, uint8_t bitOffset ,uint8_t len, uint32_t acceptMask,uint32_t configMask );
	OsStatus_t (*uLipeGpioConfigureSingle)(Device_t *this, uint8_t bitPos, uint32_t configMask );
	OsStatus_t (*uLipeGpioRegisterIntCallback)(Device_t * this, uint8_t bit, uLipeGpioCallBack_t cb);
	OsStatus_t (*uLipeGpioWriteRow)(Device_t *this,uint8_t bitOffset ,uint8_t len, uint32_t acceptMask, uint32_t value);
	OsStatus_t (*uLipeGpioWriteSingle)(Device_t *this, uint8_t bitPos, uint8_t value );
	uint32_t   (*uLipeGpioReadRow)(Device_t *this,uint8_t bitOffset ,uint8_t len, uint32_t acceptMask, OsStatus_t *err);
	bool 	   (*uLipeGpioReadSingle) (Device_t *this, uint8_t bitPos, OsStatus_t *err );
}GpioDeviceApi_t;

/*
 *  Gpio configuration masks
 */

#define GPIO_DIR_IN				0x00
#define GPIO_DIR_OUT			0x01
#define GPIO_PUSH_PULL			(0x00 << 2)
#define GPIO_OPEN_DRAIN			(0x01 << 2)
#define GPIO_INT_DISABLED		(0x00 << 4)
#define GPIO_INT_ENABLED		(0x01 << 4)
#define GPIO_INT_RISING_EDGE	(0x00 << 6)
#define GPIO_INT_FALLING_EDGE	(0x01 << 6)
#define GPIO_USE_HIGH_SPEED		(0x00 << 8)
#define GPIO_USE_MID_SPEED		(0x01 << 8)
#define GPIO_USE_LOW_SPEED		(0x02 << 8)


/*!
 * 	uLipeDriverGpioConfigRow()
 *
 *  \brief configure the row of gpio with the configuration passed in config mask
 *
 */
static __inline OsStatus_t uLipeDriverGpioConfigRow(Device_t *dev, uint8_t bitOffset ,uint8_t len,
				uint32_t acceptMask,uint32_t configMask )
{
	GpioDeviceApi_t *api = (GpioDeviceApi_t *)dev->deviceApi;
	OsStatus_t ret;

	if(api) {
		ret = api->uLipeGpioConfigureRow(dev, bitOffset, len, acceptMask, configMask);
	} else {
		ret = kInvalidParam;
	}
	return(ret);
}

/*!
 * 	uLipeDriverGpioConfigSingle()
 *
 *  \brief configure a single bit of gpio with the configuration passed in config mask
 *
 */
static __inline OsStatus_t uLipeDriverGpioConfigSingle(Device_t *dev, uint8_t bitPos, uint32_t configMask )
{
	GpioDeviceApi_t *api = (GpioDeviceApi_t *)dev->deviceApi;
	OsStatus_t ret;

	if(api) {
		ret = api->uLipeGpioConfigureSingle(dev, bitPos, configMask);
	} else {
		ret = kInvalidParam;
	}
	return(ret);
}

/*!
 * 	uLipeDriverGpioRegCallback()
 *
 *  \brief Registers a callback triggered bit edge sensed by gpio bit
 *
 */
static __inline OsStatus_t uLipeDriverGpioRegCallback(Device_t * dev, uint8_t bit, uLipeGpioCallBack_t cb)
{
	GpioDeviceApi_t *api = (GpioDeviceApi_t *)dev->deviceApi;
	OsStatus_t ret;

	if(api) {
		ret = api->uLipeGpioRegisterIntCallback(dev, bit, cb);
	} else {
		ret = kInvalidParam;
	}
	return(ret);
}


/*!
 * 	uLipeDriverGpioWriteRow()
 *
 *  \brief writes a bit row on the device gpio output register
 *
 */
static __inline OsStatus_t uLipeDriverGpioWriteRow(Device_t *dev,uint8_t bitOffset ,uint8_t len,
			uint32_t acceptMask, uint32_t value)
{
	GpioDeviceApi_t *api = (GpioDeviceApi_t *)dev->deviceApi;
	OsStatus_t ret;

	if(api) {
		ret = api->uLipeGpioWriteRow(dev, bitOffset, len, acceptMask, value);
	} else {
		ret = kInvalidParam;
	}
	return(ret);
}

/*!
 * 	uLipeDriverGpioWriteSingle()
 *
 *  \brief writes a single bit on the device gpio output register
 *
 */
static __inline OsStatus_t uLipeDriverGpioWriteSingle(Device_t *dev, uint8_t bitPos, uint8_t value )
{
	GpioDeviceApi_t *api = (GpioDeviceApi_t *)dev->deviceApi;
	OsStatus_t ret;

	if(api) {
		ret = api->uLipeGpioWriteSingle(dev, bitPos, value);
	} else {
		ret = kInvalidParam;
	}

	return(ret);
}

/*!
 * 	uLipeDriverGpioReadRow()
 *
 *  \brief reads a bit row from the device's gpio input register
 *
 */
static __inline uint32_t uLipeDriverGpioReadRow(Device_t *dev,uint8_t bitOffset ,uint8_t len,
			uint32_t acceptMask, OsStatus_t *err)
{
	GpioDeviceApi_t *api = (GpioDeviceApi_t *)dev->deviceApi;
	uint32_t ret;

	if(api) {
		ret = api->uLipeGpioReadRow(dev, bitOffset, len, acceptMask, err);
	} else {
		ret = 0xFFFFFFFF;
		if(err != NULL) {
			*err = kInvalidParam;
		}
	}
	return(ret);
}


/*!
 * 	uLipeDriverGpioReadSingle()
 *
 *  \brief reads a bit from the device's gpio input register
 *
 */
static __inline bool uLipeDriverGpioReadSingle(Device_t *dev, uint8_t bitPos, OsStatus_t *err )
{
	GpioDeviceApi_t *api = (GpioDeviceApi_t *)dev->deviceApi;
	bool ret;

	if(api) {
		ret = api->uLipeGpioReadSingle(dev, bitPos, err);
	} else {
		ret = false;
		if(err != NULL) {
			*err = kInvalidParam;
		}
	}
	return(ret);
}

#endif
#endif
