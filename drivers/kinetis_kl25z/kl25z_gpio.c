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
	clock_ip_name_t portClk;
}KL25ZGpioDevData_t;

static OsStatus_t KL25Z_GpioInit(void *arg)
{
	OsStatus_t ret = kStatusOk;
	return(ret);
}


static OsStatus_t KL25Z_GpioConfigureRow(Device_t *this, uint8_t bitOffset ,uint8_t len, uint32_t acceptMask,uint32_t configMask )
{
	OsStatus_t ret = kStatusOk;
	return(ret);
}


static OsStatus_t KL25Z_GpioConfigureSingle(Device_t *this, uint8_t bitPos, uint32_t configMask )
{
	OsStatus_t ret = kStatusOk;
	return(ret);
}


static OsStatus_t KL25Z_GpioRegisterIntCallback(Device_t * this, uint8_t bit, uLipeGpioCallBack_t cb)
{
	OsStatus_t ret = kStatusOk;
	return(ret);
}

static OsStatus_t KL25Z_GpioWriteRow(Device_t *this,uint8_t bitOffset ,uint8_t len, uint32_t acceptMask, uint32_t value)
{
	OsStatus_t ret = kStatusOk;
	return(ret);
}

static OsStatus_t KL25Z_GpioWriteSingle(Device_t *this, uint8_t bitPos, uint8_t value )
{
	OsStatus_t ret = kStatusOk;
	return(ret);
}

static uint32_t  KL25Z_GpioReadRow(Device_t *this,uint8_t bitOffset ,uint8_t len, uint32_t acceptMask, OsStatus_t *err)
{
	uint32_t ret = 0;
	return(ret);

}

static bool KL25Z_GpioReadSingle (Device_t *this, uint8_t bitPos, OsStatus_t *err )
{
	bool ret = false;
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
		.portClk = kCLOCK_PortA
};

static DeviceConfig_t kl25zGpioCfg_a = {
		.name=GPIOA_GPIO_DEVICE_NAME,
		.driverIsr=NULL,
		.earlyInitFcn=KL25Z_GpioInit,
		.irqOffset=0,
		.refCount=0,
		.devConfigData=&kl25zGpioCfg_a,
};

ULIPE_DEVICE_DECLARE(gpioGpio0, &kl25zGpioCfg_a, NULL, &kl25zGpioDeviceApi);


/* instance the further existing gpio controllers */


#if OS_USE_GPIOB_GPIO > 0
static KL25ZGpioDevData_t kl25zGpioData_b = {
		.gpio = GPIOB,
		.portClk = kCLOCK_PortB
};

static DeviceConfig_t kl25zGpioCfg_b = {
		.name=GPIOB_GPIO_DEVICE_NAME,
		.driverIsr=NULL,
		.earlyInitFcn=KL25Z_GpioInit,
		.irqOffset=0,
		.refCount=0,
		.devConfigData=&kl25zGpioCfg_b,
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
		.devConfigData=&kl25zGpioCfg_c,
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
		.devConfigData=&kl25zGpioCfg_d,
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
		.devConfigData=&kl25zGpioCfg_e,
};

ULIPE_DEVICE_DECLARE(gpioGpio4, &kl25zGpioCfg_e, NULL, &kl25zGpioDeviceApi);
#endif


#endif
#endif
#endif
