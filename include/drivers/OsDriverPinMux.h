/*!
 *
 *                          ULIPE RTOS VERSION 4
 *
 *
 *  \file OsDriverPinMux.h
 *
 *  \brief generic pinmux device driver
 *
 *
 *  Author: FSN
 *
 */

#ifndef __OS_DRIVER_PINMUX_H_
#define __OS_DRIVER_PINMUX_H_

#if OS_USE_PINMUX_DRIVERS > 0

/*
 *  Platform pinmux device API structure
 */
typedef struct {
	OsStatus_t(*PinMuxDriverSetToAnalog)(Device_t * dev, uint8_t bitPos);
	OsStatus_t(*PinMuxDriverSetToGpio) (Device_t * dev, uint8_t bitPos);
	OsStatus_t (*PinMuxDriverSetToAlternate)(Device_t *dev, uint8_t bitPos, uint32_t alternateFun);
	OsStatus_t (*PinMuxDriverSetRowToAnalog)(Device_t * dev, uint8_t bitOffset, uint8_t len, uint32_t acceptMask);
	OsStatus_t (*PinMuxDriverSetRowToGpio)(Device_t * dev, uint8_t bitOffset, uint8_t len, uint32_t acceptMask);
	OsStatus_t (*PinMuxDriverSetRowToAlternate)(Device_t * dev, uint8_t bitOffset, uint8_t len, uint32_t acceptMask, uint32_t alternateFun);

}PinMuxDriverApi_t;


/*
 * Alternate function enumeration
 */
typedef enum {
	kAltFun0 ,
	kAltFun1 ,
	kAltFun2 ,
	kAltFun3 ,
	kAltFun4 ,
	kAltFun5 ,
	kAltFun6 ,
	kAltFun7 ,
	kAltFun8 ,
	kAltFun9 ,
	kAltFun10 ,
	kAltFun11 ,
	kAltFun12 ,
	kAltFun13 ,
	kAltFun14 ,
	kAltFun15 ,
}Alternate_t;



/*!
 * 	uLipeDriverPinMuxSetToAnalog()
 *
 * 	\brief sets a specified gpio bit to analog if supported
 *
 */
static inline OsStatus_t uLipeDriverPinMuxSetToAnalog(Device_t * dev, uint8_t bitPos)
{
	OsStatus_t ret;
	PinMuxDriverApi_t * api = (PinMuxDriverApi_t *)dev->deviceApi;

	if(api) {
		ret = api->PinMuxDriverSetToAnalog(dev, bitPos);
	} else {
		ret = kInvalidParam;
	}
	return(ret);
}


/*!
 * 	uLipeDriverPinMuxSetToGpio()
 *
 * 	\brief sets a specified gpio bit to digital I/O if supported
 *
 */
static inline OsStatus_t uLipeDriverPinMuxSetToGpio(Device_t * dev, uint8_t bitPos)
{
	OsStatus_t ret;
	PinMuxDriverApi_t * api = (PinMuxDriverApi_t *)dev->deviceApi;

	if(api) {
		ret = api->PinMuxDriverSetToGpio(dev, bitPos);
	} else {
		ret = kInvalidParam;
	}
	return(ret);
}

/*!
 * 	uLipeDriverPinMuxSetToAlt()
 *
 * 	\brief sets a specified gpio bit to one of supported alternate functions
 *
 */
static inline OsStatus_t uLipeDriverPinMuxSetToAlt(Device_t *dev, uint8_t bitPos, Alternate_t alternateFun)
{
	OsStatus_t ret;
	PinMuxDriverApi_t * api = (PinMuxDriverApi_t *)dev->deviceApi;

	if(api) {
		ret = api->PinMuxDriverSetToAlternate(dev, bitPos, (uint32_t)alternateFun);
	} else {
		ret = kInvalidParam;
	}
	return(ret);
}

/*!
 * 	uLipeDriverPinMuxSetRowToAnalog()
 *
 * 	\brief sets a specified gpio bit row to analog function
 *
 */
static inline OsStatus_t uLipeDriverPinMuxSetRowToAnalog(Device_t * dev, uint8_t bitOffset, uint8_t len, uint32_t acceptMask)
{
	OsStatus_t ret;
	PinMuxDriverApi_t * api = (PinMuxDriverApi_t *)dev->deviceApi;

	if(api) {
		ret = api->PinMuxDriverSetRowToAnalog(dev, bitOffset,len,acceptMask);
	} else {
		ret = kInvalidParam;
	}
	return(ret);
}

/*!
 * 	uLipeDriverPinMuxSetRowToGpio()
 *
 * 	\brief sets a specified gpio bit row to Digital I/O function
 *
 */
static inline OsStatus_t uLipeDriverPinMuxSetRowToGpio(Device_t * dev, uint8_t bitOffset, uint8_t len, uint32_t acceptMask)
{
	OsStatus_t ret;
	PinMuxDriverApi_t * api = (PinMuxDriverApi_t *)dev->deviceApi;

	if(api) {
		ret = api->PinMuxDriverSetRowToGpio(dev, bitOffset,len,acceptMask);
	} else {
		ret = kInvalidParam;
	}
	return(ret);
}


/*!
 * 	uLipeDriverPinMuxSetRowToAlt()
 *
 * 	\brief sets a specified gpio bit row to alternate function
 *
 */
static inline OsStatus_t uLipeDriverPinMuxSetRowToAlt(Device_t * dev, uint8_t bitOffset, uint8_t len,
			uint32_t acceptMask, Alternate_t alternateFun)
{
	OsStatus_t ret;
	PinMuxDriverApi_t * api = (PinMuxDriverApi_t *)dev->deviceApi;

	if(api) {
		ret = api->PinMuxDriverSetRowToAlternate(dev, bitOffset,len,acceptMask,(uint32_t)alternateFun);
	} else {
		ret = kInvalidParam;
	}
	return(ret);
}

#endif
#endif
