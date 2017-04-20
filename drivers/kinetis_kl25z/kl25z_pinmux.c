/*!
 *
 *                          ULIPE RTOS VERSION 4
 *
 *
 *  \file kl25z_pinmux.c
 *
 *  \brief kl25z specific pinmux device driver
 *
 *
 *  Author: FSN
 *
 */

#include "uLipeRtos4.h"
#include "kl25z_soc_config.h"

/* this driver can be only compiled if it is enabled on OsConfig.h*/
#if OS_USE_DEVICE_DRIVERS > 0
#if OS_USE_PINMUX_DRIVERS > 0
#if OS_USE_MCUEXPRESSO_FOR_KL25Z > 0

/* creates a structure to device custom data */
typedef struct {
	PORT_Type *port;
	clock_ip_name_t portClk;
}KL25ZPinMuxDevData_t;




static OsStatus_t KL25Z_PinMuxDriverInit(void *arg)
{
	OsStatus_t ret = kStatusOk;

	Device_t *dev = (Device_t *)arg;
	KL25ZPinMuxDevData_t *dat = (KL25ZPinMuxDevData_t *)dev->config->devConfigData;

	CLOCK_EnableClock(dat->portClk);
	return(ret);
}


static OsStatus_t KL25Z_PinMuxDriverSetToAnalog(Device_t * dev, uint8_t bitPos)
{
	OsStatus_t ret = kStatusOk;
	KL25ZPinMuxDevData_t *dat = (KL25ZPinMuxDevData_t *)dev->config->devConfigData;

	PORT_SetPinMux(dat->port, bitPos, kPORT_PinDisabledOrAnalog);

	return(ret);
}

static OsStatus_t KL25Z_PinMuxDriverSetToGpio (Device_t * dev, uint8_t bitPos)
{
	OsStatus_t ret = kStatusOk;
	KL25ZPinMuxDevData_t *dat = (KL25ZPinMuxDevData_t *)dev->config->devConfigData;
	PORT_SetPinMux(dat->port, bitPos, kPORT_MuxAsGpio);

	return(ret);
}
static OsStatus_t KL25Z_PinMuxDriverSetToAlternate(Device_t *dev, uint8_t bitPos, uint32_t alternateFun)
{
	OsStatus_t ret = kStatusOk;
	KL25ZPinMuxDevData_t *dat = (KL25ZPinMuxDevData_t *)dev->config->devConfigData;

	if(alternateFun >= kPORT_MuxAlt15) {
		port_mux_t ksdkAlt = (port_mux_t)alternateFun;
		PORT_SetPinMux(dat->port, bitPos, ksdkAlt);
	} else {
		ret = kNotImplementedForThisDevice;
	}

	return(ret);
}

static OsStatus_t KL25Z_PinMuxDriverSetRowToAnalog(Device_t * dev, uint8_t bitOffset, uint8_t len, uint32_t acceptMask)
{
	OsStatus_t ret = kStatusOk;
	KL25ZPinMuxDevData_t *dat = (KL25ZPinMuxDevData_t *)dev->config->devConfigData;

	for(uint8_t i = 0; i < len; i++, acceptMask >>= 1) {
		/* only modify desired pins passed with accept mask */
		if(acceptMask & 0x01) {
			PORT_SetPinMux(dat->port, i + bitOffset, kPORT_PinDisabledOrAnalog);
		}
	}
	return(ret);
}

static OsStatus_t KL25Z_PinMuxDriverSetRowToGpio(Device_t * dev, uint8_t bitOffset, uint8_t len, uint32_t acceptMask)
{
	OsStatus_t ret = kStatusOk;
	KL25ZPinMuxDevData_t *dat = (KL25ZPinMuxDevData_t *)dev->config->devConfigData;

	for(uint8_t i = 0; i < len; i++, acceptMask >>= 1) {
		/* only modify desired pins passed with accept mask */
		if(acceptMask & 0x01) {
			PORT_SetPinMux(dat->port, i + bitOffset, kPORT_MuxAsGpio);
		}
	}
	return(ret);
}

static OsStatus_t KL25Z_PinMuxDriverSetRowToAlternate(Device_t * dev, uint8_t bitOffset, uint8_t len, uint32_t acceptMask, uint32_t alternateFun)
{
	OsStatus_t ret = kStatusOk;
	KL25ZPinMuxDevData_t *dat = (KL25ZPinMuxDevData_t *)dev->config->devConfigData;
	bool notImplemented = false;

	for(uint8_t i = 0; i < len; i++, acceptMask >>= 1) {
		/* only modify desired pins passed with accept mask */
		if(acceptMask & 0x01) {
			if(alternateFun >= kPORT_MuxAlt15) {
				port_mux_t ksdkAlt = (port_mux_t)alternateFun;
				PORT_SetPinMux(dat->port, i + bitOffset, ksdkAlt);
			} else {
				notImplemented = true;
			}
		}
	}

	if(notImplemented) {
		/* this alternate is not implemented for this device */
		ret = kNotImplementedForThisDevice;
	}

	return(ret);
}

/* prepare this device linking its api  */
static PinMuxDriverApi_t kl25zPinMuxApi = {
		.PinMuxDriverSetToAnalog = KL25Z_PinMuxDriverSetToAnalog,
		.PinMuxDriverSetToGpio = KL25Z_PinMuxDriverSetToGpio,
		.PinMuxDriverSetToAlternate = KL25Z_PinMuxDriverSetToAlternate,
		.PinMuxDriverSetRowToAnalog = KL25Z_PinMuxDriverSetRowToAnalog,
		.PinMuxDriverSetRowToGpio = KL25Z_PinMuxDriverSetRowToGpio,
		.PinMuxDriverSetRowToAlternate = KL25Z_PinMuxDriverSetRowToAlternate
};



/* create data device and each drivers instances
 * use: ULIPE_DEVICE_DECLARE(DevName, DevConfig, DriverData, DriverApi)
 */
static KL25ZPinMuxDevData_t kl25zPinMuxData_a = {
		.port = PORTA,
		.portClk = kCLOCK_PortA,
};


static DeviceConfig_t kl25zPinMuxCfg_a = {
		.name = PORTA_PIN_MUX_DEVICE_NAME,
		.refCount = 0,
		.devConfigData = &kl25zPinMuxData_a,
		.driverIsr = NULL,
		.earlyInitFcn = KL25Z_PinMuxDriverInit
};


ULIPE_DEVICE_DECLARE(pinMuxPorta, &kl25zPinMuxCfg_a, NULL, &kl25zPinMuxApi);





#if OS_USE_PORTB_PIN_MUX > 0

static KL25ZPinMuxDevData_t kl25zPinMuxData_b = {
		.port = PORTB,
		.portClk = kCLOCK_PortB,
};


static DeviceConfig_t kl25zPinMuxCfg_b = {
		.name = PORTB_PIN_MUX_DEVICE_NAME,
		.refCount = 0,
		.devConfigData = &kl25zPinMuxData_b,
		.driverIsr = NULL,
		.earlyInitFcn = KL25Z_PinMuxDriverInit,
};

ULIPE_DEVICE_DECLARE(pinMuxPortb, &kl25zPinMuxCfg_b, NULL, &kl25zPinMuxApi);

#endif

#if OS_USE_PORTC_PIN_MUX > 0

static KL25ZPinMuxDevData_t kl25zPinMuxData_c = {
		.port = PORTC,
		.portClk = kCLOCK_PortC,
};


static DeviceConfig_t kl25zPinMuxCfg_c = {
		.name = PORTC_PIN_MUX_DEVICE_NAME,
		.refCount = 0,
		.devConfigData = &kl25zPinMuxData_c,
		.driverIsr = NULL,
		.earlyInitFcn = KL25Z_PinMuxDriverInit,
};

ULIPE_DEVICE_DECLARE(pinMuxPortc, &kl25zPinMuxCfg_c, NULL, &kl25zPinMuxApi);
#endif


#if OS_USE_PORTD_PIN_MUX > 0

static KL25ZPinMuxDevData_t kl25zPinMuxData_d = {
		.port = PORTD,
		.portClk = kCLOCK_PortD,
};


static DeviceConfig_t kl25zPinMuxCfg_d = {
		.name = PORTD_PIN_MUX_DEVICE_NAME,
		.refCount = 0,
		.devConfigData = &kl25zPinMuxData_d,
		.driverIsr = NULL,
		.earlyInitFcn = KL25Z_PinMuxDriverInit,
};

ULIPE_DEVICE_DECLARE(pinMuxPortd, &kl25zPinMuxCfg_d, NULL, &kl25zPinMuxApi);
#endif


#if OS_USE_PORTE_PIN_MUX > 0

static KL25ZPinMuxDevData_t kl25zPinMuxData_e = {
		.port = PORTE,
		.portClk = kCLOCK_PortE,
};


static DeviceConfig_t kl25zPinMuxCfg_e = {
		.name = PORTE_PIN_MUX_DEVICE_NAME,
		.refCount = 0,
		.devConfigData = &kl25zPinMuxData_e,
		.driverIsr = NULL,
		.earlyInitFcn = KL25Z_PinMuxDriverInit,
};
ULIPE_DEVICE_DECLARE(pinMuxPorte, &kl25zPinMuxCfg_e, NULL, &kl25zPinMuxApi);

#endif


#endif
#endif
#endif
