/*!
 *
 *                          ULIPE RTOS VERSION 4
 *
 *
 *  \file OsDriverEnet.h
 *
 *  \brief generic Ethernet interface device driver
 *
 *  Author: FSN
 *
 */

#ifndef __OS_DRIVER_ENET_H_
#define __OS_DRIVER_ENET_H_

#if OS_USE_ENET_DRIVERS > 0

/*
 * Mac address structure
 */
typedef struct {
	uint8_t macAddr[6];
}EnetMacAddress_t;

/*
 * Enet driver api structure
 */
typedef struct {
	OsStatus_t (*EnetDriverInit)(Device_t *dev);
	OsStatus_t (*EnetDriverEnable)(Device_t *dev);
	OsStatus_t (*EnetDriverSetMac)(Device_t *dev, EnetMacAddress_t *mac);
	OsStatus_t (*EnetDriverTxPacket)(Device_t *dev, uint8_t *data, uint32_t len);
	OsStatus_t (*EnetDriverRxPacket)(Device_t *dev, uint8_t *data, uint32_t *actualLen);
}EnetDriverApi_t;


/*!
 * 	uLipeDriverEnetInit()
 *
 * 	\brief Inits the ethernet device
 *
 */
static inline OsStatus_t uLipeDriverEnetInit(Device_t *dev)
{
	OsStatus_t ret;
	EnetDriverApi_t *api = (EnetDriverApi_t *)dev->deviceApi;

	if(api) {
		ret = api->EnetDriverInit(dev);
	}else {
		ret = kInvalidParam;
	}
	return(ret);
}

/*!
 * 	uLipeDriverEnetEnable()
 *
 * 	\brief Enable the ethernet device
 *
 */
static inline OsStatus_t uLipeDriverEnetEnable(Device_t *dev)
{
	OsStatus_t ret;
	EnetDriverApi_t *api = (EnetDriverApi_t *)dev->deviceApi;

	if(api) {
		ret = api->EnetDriverEnable(dev);
	}else {
		ret = kInvalidParam;
	}
	return(ret);
}


/*!
 * 	uLipeDriverEnetSetMac()
 *
 * 	\brief sets the mac adress of the ethernet device
 *
 */
static inline OsStatus_t uLipeDriverEnetSetMac(Device_t *dev, EnetMacAddress_t *mac)
{
	OsStatus_t ret;
	EnetDriverApi_t *api = (EnetDriverApi_t *)dev->deviceApi;

	if(api) {
		ret = api->EnetDriverSetMac(dev,mac);
	}else {
		ret = kInvalidParam;
	}
	return(ret);
}

/*!
 * 	uLipeDriverEnetTxPacket()
 *
 * 	\brief Sends a packet using the ethernet device
 *
 */
static inline OsStatus_t uLipeDriverEnetTxPacket(Device_t *dev, uint8_t *data, uint32_t len)
{
	OsStatus_t ret;
	EnetDriverApi_t *api = (EnetDriverApi_t *)dev->deviceApi;

	if(api) {
		ret = api->EnetDriverTxPacket(dev, data, len);
	}else {
		ret = kInvalidParam;
	}
	return(ret);
}


/*!
 * 	uLipeDriverEnetRxPacket()
 *
 * 	\brief Receives a packet using the ethernet device
 *
 */
static inline OsStatus_t uLipeDriverEnetRxPacket(Device_t *dev, uint8_t *data, uint32_t *actualLen)
{
	OsStatus_t ret;
	EnetDriverApi_t *api = (EnetDriverApi_t *)dev->deviceApi;

	if(api) {
		ret = api->EnetDriverRxPacket(dev, data, actualLen);
	}else {
		ret = kInvalidParam;
	}
	return(ret);
}

#endif
#endif
