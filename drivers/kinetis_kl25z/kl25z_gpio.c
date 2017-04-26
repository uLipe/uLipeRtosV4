/*!
 *
 *                          ULIPE RTOS VERSION 4
 *
 *
 *  \file kl25z_gpio.c
 *
 *  \brief kl25z specific gpio device driver
 *
 *
 *  Author: FSN
 *
 */

#include "uLipeRtos4.h"
#include "kl25z_soc_config.h"

#if OS_USE_DEVICE_DRIVERS > 0
#if OS_USE_GPIO_DRIVERS > 0
#if OS_USE_MCUEXPRESSO_FOR_KL25Z > 0


/*
 * device configuration data payload
 */
typedef struct {
	GPIO_Type *gpio;
	PORT_Type *port;
	clock_ip_name_t portClk;
}KL25ZGpioDevData_t;

static OsStatus_t KL25Z_GpioInit(void *arg)
{
	OsStatus_t ret = kStatusOk;
	Device_t *dev = (Device_t *)arg;
	KL25ZGpioDevData_t *dat = (KL25ZGpioDevData_t *)dev->config->devConfigData;

	/* gates the selectedd gpio clock */
	CLOCK_EnableClock(dat->portClk);

	return(ret);
}


static OsStatus_t KL25Z_GpioConfigureRow(Device_t *dev, uint8_t bitOffset ,uint8_t len, uint32_t acceptMask,uint32_t configMask )
{
	OsStatus_t ret = kStatusOk;
	KL25ZGpioDevData_t *dat = (KL25ZGpioDevData_t *)dev->config->devConfigData;
	gpio_pin_config_t cfg;

	for(uint8_t i = 0; i < len; i++, acceptMask >>= 1) {
		/* only modify desired pins passed with accept mask */
		if(acceptMask & 0x01) {
			/* select configuration */
			if(configMask & GPIO_DIR_OUT) {
				cfg.outputLogic = 0;
				cfg.pinDirection = kGPIO_DigitalOutput;
				/* select output type */
				if(configMask & GPIO_OPEN_DRAIN) {
					ret = kNotImplementedForThisDevice;
				}

			} else {
				cfg.outputLogic = 0;
				cfg.pinDirection = kGPIO_DigitalInput;
				if(configMask & GPIO_OPEN_DRAIN) {
					ret = kNotImplementedForThisDevice;
				} else if (configMask & GPIO_INT_ENABLED) {
					ret = kNotImplementedForThisDevice;
				}
			}

			if(ret != kNotImplementedForThisDevice) {
				GPIO_PinInit(dat->gpio, i + bitOffset, &cfg);
			}
		}
	}
	return(ret);
}


static OsStatus_t KL25Z_GpioConfigureSingle(Device_t *dev, uint8_t bitPos, uint32_t configMask )
{
	OsStatus_t ret = kStatusOk;
	KL25ZGpioDevData_t *dat = (KL25ZGpioDevData_t *)dev->config->devConfigData;
	gpio_pin_config_t cfg;

	/* select configuration */
	if(configMask & GPIO_DIR_OUT) {
		cfg.outputLogic = 0;
		cfg.pinDirection = kGPIO_DigitalOutput;
		/* select output type */
		if(configMask & GPIO_OPEN_DRAIN) {
			ret = kNotImplementedForThisDevice;
		}

	} else {
		cfg.outputLogic = 0;
		cfg.pinDirection = kGPIO_DigitalInput;
		if(configMask & GPIO_OPEN_DRAIN) {
			ret = kNotImplementedForThisDevice;
		} else if (configMask & GPIO_INT_ENABLED) {
			ret = kNotImplementedForThisDevice;
		}
	}

	if(ret != kNotImplementedForThisDevice) {
		GPIO_PinInit(dat->gpio, bitPos, &cfg);
	}


	return(ret);
}


static OsStatus_t KL25Z_GpioRegisterIntCallback(Device_t * dev, uint8_t bit, uLipeGpioCallBack_t cb)
{
	OsStatus_t ret = kNotImplementedForThisDevice;
	(void)dev;
	(void)bit;
	(void)cb;

	 /* TODO: Need to understand how Kinetis int pins works */
	return(ret);
}

static OsStatus_t KL25Z_GpioWriteRow(Device_t *dev,uint8_t bitOffset ,uint8_t len, uint32_t acceptMask, uint32_t value)
{
	OsStatus_t ret = kStatusOk;
	KL25ZGpioDevData_t *dat = (KL25ZGpioDevData_t *)dev->config->devConfigData;

	/* not necessary here */
	(void)len;
	(void)bitOffset;

	/* the kinetis sdk has no support for a write gpio data out register! bizarre, we need to deal
	 * directly with the register
	 */
	uint32_t tmp = dat->gpio->PDOR & (~acceptMask);
	dat->gpio->PDOR = (tmp | value );


	return(ret);

}

static OsStatus_t KL25Z_GpioWriteSingle(Device_t *dev, uint8_t bitPos, uint8_t value )
{
	OsStatus_t ret = kStatusOk;
	KL25ZGpioDevData_t *dat = (KL25ZGpioDevData_t *)dev->config->devConfigData;

	uint32_t tmp = dat->gpio->PDOR & (~(1 << bitPos));
	tmp |= ((value & 0x01) << bitPos);
	dat->gpio->PDOR = tmp;

	return(ret);
}

static uint32_t  KL25Z_GpioReadRow(Device_t *dev,uint8_t bitOffset ,uint8_t len, uint32_t acceptMask, OsStatus_t *err)
{
	uint32_t ret = 0;
	KL25ZGpioDevData_t *dat = (KL25ZGpioDevData_t *)dev->config->devConfigData;
	/* not necessary here */
	(void)len;
	(void)bitOffset;

	ret = dat->gpio->PDIR & acceptMask;

	if(err != NULL) {
		*err = kStatusOk;
	}

	return(ret);
}

static bool KL25Z_GpioReadSingle (Device_t *dev, uint8_t bitPos, OsStatus_t *err )
{
	bool ret = false;
	KL25ZGpioDevData_t *dat = (KL25ZGpioDevData_t *)dev->config->devConfigData;
	ret = GPIO_ReadPinInput(dat->gpio, bitPos);

	if(err != NULL) {
		*err = kStatusOk;
	}

	return(ret);
}

/* link the specific api with generic api template */
GpioDeviceApi_t kl25zGpioDeviceApi = {
		.uLipeGpioConfigureRow=KL25Z_GpioConfigureRow,
		.uLipeGpioConfigureSingle=KL25Z_GpioConfigureSingle,
		.uLipeGpioRegisterIntCallback=KL25Z_GpioRegisterIntCallback,
		.uLipeGpioWriteRow=KL25Z_GpioWriteRow,
		.uLipeGpioWriteSingle=KL25Z_GpioWriteSingle,
		.uLipeGpioReadRow=KL25Z_GpioReadRow,
		.uLipeGpioReadSingle=KL25Z_GpioReadSingle
};

/* create data device and each drivers instances
 * use: ULIPE_DEVICE_DECLARE(DevName, DevConfig, DriverData, DriverApi)
 */

static KL25ZGpioDevData_t kl25zGpioData_a = {
		.gpio = GPIOA,
		.portClk = kCLOCK_PortA,
};

static DeviceConfig_t kl25zGpioCfg_a = {
		.name=GPIOA_GPIO_DEVICE_NAME,
		.driverIsr=NULL,
		.earlyInitFcn=KL25Z_GpioInit,
		.irqOffset=0,
		.refCount=0,
		.devConfigData=&kl25zGpioData_a,
};

ULIPE_DEVICE_DECLARE(gpioGpio0, &kl25zGpioCfg_a, NULL, &kl25zGpioDeviceApi);


/* instance the further existing gpio controllers */


#if OS_USE_GPIOB_GPIO > 0
static KL25ZGpioDevData_t kl25zGpioData_b = {
		.gpio = GPIOB,
		.portClk = kCLOCK_PortB,

};

static DeviceConfig_t kl25zGpioCfg_b = {
		.name=GPIOB_GPIO_DEVICE_NAME,
		.driverIsr=NULL,
		.earlyInitFcn=KL25Z_GpioInit,
		.irqOffset=0,
		.refCount=0,
		.devConfigData=&kl25zGpioData_b,
};

ULIPE_DEVICE_DECLARE(gpioGpio1, &kl25zGpioCfg_b, NULL, &kl25zGpioDeviceApi);
#endif


#if OS_USE_GPIOC_GPIO > 0
static KL25ZGpioDevData_t kl25zGpioData_c = {
		.gpio = GPIOC,
		.portClk = kCLOCK_PortC

};

static DeviceConfig_t kl25zGpioCfg_c = {
		.name=GPIOC_GPIO_DEVICE_NAME,
		.driverIsr=NULL,
		.earlyInitFcn=KL25Z_GpioInit,
		.irqOffset=0,
		.refCount=0,
		.devConfigData=&kl25zGpioData_c,
};

ULIPE_DEVICE_DECLARE(gpioGpio2, &kl25zGpioCfg_c, NULL, &kl25zGpioDeviceApi);
#endif


#if OS_USE_GPIOD_GPIO > 0
static KL25ZGpioDevData_t kl25zGpioData_d = {
		.gpio = GPIOD,
		.portClk = kCLOCK_PortD
};


static DeviceConfig_t kl25zGpioCfg_d = {
		.name=GPIOD_GPIO_DEVICE_NAME,
		.driverIsr=NULL,
		.earlyInitFcn=KL25Z_GpioInit,
		.irqOffset=0,
		.refCount=0,
		.devConfigData=&kl25zGpioData_d,
};

ULIPE_DEVICE_DECLARE(gpioGpio3, &kl25zGpioCfg_d, NULL, &kl25zGpioDeviceApi);
#endif



#if OS_USE_GPIOE_GPIO > 0
static KL25ZGpioDevData_t kl25zGpioData_e = {
		.gpio = GPIOE,
		.portClk = kCLOCK_PortE

};


static DeviceConfig_t kl25zGpioCfg_e = {
		.name=GPIOE_GPIO_DEVICE_NAME,
		.driverIsr=NULL,
		.earlyInitFcn=KL25Z_GpioInit,
		.irqOffset=0,
		.refCount=0,
		.devConfigData=&kl25zGpioData_e,
};

ULIPE_DEVICE_DECLARE(gpioGpio4, &kl25zGpioCfg_e, NULL, &kl25zGpioDeviceApi);
#endif


#endif
#endif
#endif
