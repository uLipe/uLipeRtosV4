/*!
 *
 *                          ULIPE RTOS VERSION 4
 *
 *
 *  \file OsDriverAnalog.h
 *
 *  \brief generic Analog device driver
 *
 *
 *  Author: FSN
 *
 */

#ifndef __OS_DRIVER_ANALOG_H_
#define __OS_DRIVER_ANALOG_H_

#if OS_USE_AIO_DRIVERS > 0

/*
 * Analog sequencer configuration structure
 */
typedef struct {
	uint8_t channelStart;
	uint8_t channelEnd;
	bool	interleaved;
	uint8_t noofConversions;
	uint16_t *adcData;
}AdcSequencer_t;


/*
 * Analog device Api structure
 */
typedef struct {
	OsStatus_t (*AnalogInDriverConfig)(Device_t *dev, uint32_t expectedSampleRate, uint32_t *actualSampleRate, uint32_t configMask);
	OsStatus_t (*AnalogInDriverEnable)(Device_t *dev);
	OsStatus_t (*AnalogInDriverDisable)(Device_t *dev);
	OsStatus_t (*AnalogInDriverSingleConversion)(Device_t *dev, uint16_t *result, uint8_t channel);
	OsStatus_t (*AnalogInDriverStreamConversion)(Device_t *dev, AdcSequencer_t *seq);
}AnalogDriverApi_t;


/*
 * Analog input driver configuration mask
 */
#define ANALOG_INPUT_SINGLE_CONVERSION			0x00
#define ANALOG_INPUT_CONTINUOUS_CONVERSION		0x01
#define ANALOG_INPUT_USE_EXTERNAL_VREF			(0x00 << 2)
#define ANALOG_INPUT_USE_INTERNAL_VREF			(0x01 << 1)



/*!
 *	uLipeDriverAnalogInConfig()
 *
 *	\brief Configure the analog to ditial converter device
 */
static __inline OsStatus_t uLipeDriverAnalogInConfig(Device_t *dev, uint32_t expectedSampleRate, uint32_t *actualSampleRate,
			uint32_t configMask)
{
	OsStatus_t ret;
	AnalogDriverApi_t *api = (AnalogDriverApi_t *)dev->deviceApi;

	if(api) {
		ret = api->AnalogInDriverConfig(dev, expectedSampleRate,actualSampleRate, configMask);
	} else {
		ret = kInvalidParam;
	}

	return(ret);
}


/*!
 *	uLipeDriverAnalogInEnable()
 *
 *	\brief Enable the analog to ditial converter device
 */
static __inline OsStatus_t uLipeDriverAnalogInEnable(Device_t *dev)
{
	OsStatus_t ret;
	AnalogDriverApi_t *api = (AnalogDriverApi_t *)dev->deviceApi;

	if(api) {
		ret = api->AnalogInDriverEnable(dev);
	} else {
		ret = kInvalidParam;
	}

	return(ret);
}

/*!
 *	uLipeDriverAnalogInDisable()
 *
 *	\brief Disables the analog to ditial converter device
 */
static __inline OsStatus_t uLipeDriverAnalogInDisable(Device_t *dev)
{
	OsStatus_t ret;
	AnalogDriverApi_t *api = (AnalogDriverApi_t *)dev->deviceApi;

	if(api) {
		ret = api->AnalogInDriverDisable(dev);
	} else {
		ret = kInvalidParam;
	}

	return(ret);
}

/*!
 *	uLipeDriverAnalogInSingleConv()
 *
 *	\brief starts a single conversion on the analog to ditial converter device
 */
static __inline OsStatus_t uLipeDriverAnalogInSingleConv(Device_t *dev, uint16_t *result, uint8_t channel)
{
	OsStatus_t ret;
	AnalogDriverApi_t *api = (AnalogDriverApi_t *)dev->deviceApi;

	if(api) {
		ret = api->AnalogInDriverSingleConversion(dev,result,channel);
	} else {
		ret = kInvalidParam;
	}

	return(ret);
}


/*!
 *	uLipeDriverAnalogInStreamConv()
 *
 *	\brief starts a sequence ruled conversion on the analog to ditial converter
 */
static __inline OsStatus_t uLipeDriverAnalogInStreamConv(Device_t *dev, AdcSequencer_t *seq)
{
	OsStatus_t ret;
	AnalogDriverApi_t *api = (AnalogDriverApi_t *)dev->deviceApi;

	if(api) {
		ret = api->AnalogInDriverStreamConversion(dev, seq);
	} else {
		ret = kInvalidParam;
	}

	return(ret);
}

#endif
#endif
