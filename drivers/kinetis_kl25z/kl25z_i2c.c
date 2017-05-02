
/*!
 *
 *                          ULIPE RTOS VERSION 4
 *
 *
 *  \file kl25z_I2C.c
 *
 *  \brief kl25z specific I2C device driver
 *
 *
 *  Author: FSN
 *
 */


/* BIG WARNING: the driver below only supports the standards I2C1 and I2C2 they are aliased to I2C0:1 respectively*/

#include "uLipeRtos4.h"
#include "kl25z_soc_config.h"


/* dev driver can be only compiled if it is enabled on OsConfig.h*/
#if OS_USE_DEVICE_DRIVERS > 0
#if OS_USE_I2C_DRIVERS > 0
#if OS_USE_MCUEXPRESSO_FOR_KL25Z > 0

/* define the I2C priority IRQ */
#define I2C_DRIVER_I2C_ISR_PRIORITY 0xFD

/* creates a structure to device custom data */
typedef struct {
	I2C_Type *I2C;
	clock_ip_name_t I2CClk;
}KL25ZI2CDevData_t;


typedef struct {
	bool enabled;
	bool busy;
	bool tx_sucess;
	bool rx_sucess;
	bool master;
	i2c_master_handle_t mhandle;
	i2c_slave_handle_t  shnadle;
}KL25ZCustomI2CData_t;


/*!
 *  KL25Z_I2CIsr()
 *  Platform specific I2C ISR handle the syncronization
 */
static void KL25Z_MasterI2CIsr(I2C_Type *base, void *handle, status_t status, void *userData)
{
	Device_t *dev = (Device_t *)userData;
	KL25ZCustomI2CData_t *custom = (KL25ZCustomI2CData_t *)dev->deviceData;
	OsStatus_t err;

	/* operation fully completed */
	if(status == kStatus_Success) {
		custom->rx_sucess = true;
		custom->tx_sucess = true;
	} else {
		custom->rx_sucess = false;
		custom->tx_sucess = false;
	}

	/* finish syncro and give control to the waiting thread*/
	err = uLipeDeviceFinishSync(dev);
	uLipeAssert(err == kStatusOk);
}


static void KL25Z_SlaveI2CIsr(I2C_Type *base, i2c_slave_transfer_t xfer, void *userData)
{
	Device_t *dev = (Device_t *)userData;
	KL25ZCustomI2CData_t *custom = (KL25ZCustomI2CData_t *)dev->deviceData;
	OsStatus_t err;

	/* finish syncro and give control to the waiting thread*/
	err = uLipeDeviceFinishSync(dev);
	uLipeAssert(err == kStatusOk);
}



/*!
 * 	KL25Z_I2CDriverInit()
 *
 * \brief Early function that inits the LPSCI
 *
 */
static OsStatus_t KL25Z_I2CDriverInit(void *arg)
{
	OsStatus_t ret = kStatusOk;
	Device_t *dev = (Device_t *)arg;
	KL25ZI2CDevData_t *dat =  (KL25ZI2CDevData_t *)dev->config->devConfigData;

	/* gates the I2C peripheral clock */
	CLOCK_EnableClock(dat->I2CClk);

	return(ret);
}



static OsStatus_t KL25Z_I2CConfig(Device_t *dev, uint32_t expectedSpeed, uint32_t *actualSpeed, uint32_t configMask, uint16_t slaveAddr)
{
	OsStatus_t ret = kStatusOk;
	KL25ZI2CDevData_t *dat =  (KL25ZI2CDevData_t *)dev->config->devConfigData;
	KL25ZCustomI2CData_t *custom = (KL25ZCustomI2CData_t *)dev->deviceData;
	DeviceConfig_t *dcfg = (DeviceConfig_t *)dev->config;

	(void)actualSpeed;

	if(custom->busy) {
		/* device busy, exit */
		ret = kDeviceBusy;
		goto cleanup;
	}

	custom->busy = true;

	/* select the mode of I2C operation */
	if(configMask & I2C_DEVICE_SLAVE) {
		custom->master = false;
		I2C_SlaveTransferCreateHandle(dat->I2C, &custom->shnadle,KL25Z_SlaveI2CIsr , dev);

		custom->enabled = false;

		i2c_slave_config_t cfg;
		I2C_SlaveGetDefaultConfig(&cfg);

		cfg.enableHighDrive = false;

		/* perform addressing mode setup */
		if(configMask & I2C_10BIT_ADDR) {
			cfg.addressingMode = kI2C_Address7bit;
		} else {
			cfg.addressingMode = kI2C_RangeMatch;
		}
		cfg.slaveAddress = slaveAddr;


		/* perfom I2C configuration */
		I2C_SlaveInit(dat->I2C,&cfg,OS_CPU_RATE/2);


	} else {
		custom->master = true;
		I2C_MasterTransferCreateHandle(dat->I2C, &custom->mhandle, KL25Z_MasterI2CIsr, dev);
		custom->enabled = false;

		i2c_master_config_t cfg;
		I2C_MasterGetDefaultConfig(&cfg);
		cfg.baudRate_Bps = expectedSpeed;
		cfg.enableMaster = false;

		/* perfom I2C configuration */
		I2C_MasterInit(dat->I2C,&cfg,OS_CPU_RATE/2);
	}


	/* keeps the I2C disabled */
	NVIC_ClearPendingIRQ(dcfg->irqOffset);
	NVIC_DisableIRQ(dcfg->irqOffset);
	I2C_SlaveClearStatusFlags(dat->I2C, 0xFFFFFFFF);
	I2C_DisableInterrupts(dat->I2C, 0xFFFFFFFF);
	custom->busy = false;


cleanup:
	return(ret);
}



static OsStatus_t KL25Z_I2CSendByte(Device_t *dev, uint16_t slaveAddr, uint32_t addr, uint8_t addrSize, uint8_t c, uint16_t timeout)
{
	OsStatus_t ret = kStatusOk;
	KL25ZI2CDevData_t *dat =  (KL25ZI2CDevData_t *)dev->config->devConfigData;
	KL25ZCustomI2CData_t *custom = (KL25ZCustomI2CData_t *)dev->deviceData;


	if(custom->busy) {
		/* device busy, exit */
		ret = kDeviceBusy;
		goto cleanup;
	}

	if(!custom->enabled) {
		/* device already enabled, discard */
		ret = kDeviceDisabled;
		goto cleanup;
	}


	/* lock device */
	custom->busy = true;

	if(custom->master) {
		i2c_master_transfer_t xfer;
		xfer.data = &c;
		xfer.dataSize = sizeof(uint8_t);
		xfer.direction = kI2C_Write;
		xfer.slaveAddress = slaveAddr;
		xfer.subaddress = addr;
		xfer.subaddressSize = addrSize;
		custom->tx_sucess = false;

		/* request a write from SoC I2C controller */
		/* request a write from SoC I2C controller */
		if(I2C_MasterTransferNonBlocking(dat->I2C, &custom->mhandle, &xfer) != kStatus_Success) {
			ret = kDeviceIoError;
		} else {

			/* transfer started, wait response from hardware */
			ret = uLipeDeviceStartSync(dev, timeout);
			if(ret == kStatusOk) {
				if(!custom->rx_sucess) {
					/* get how much data was arrived */
					I2C_MasterTransferAbort(dat->I2C, &custom->mhandle);
					ret = kDeviceIoError;
				}
			} else {
				I2C_MasterTransferAbort(dat->I2C, &custom->mhandle);
				ret = kTimeout;
			}
		}
	} else {

		/* todo, implement the slave driver */
		ret = kNotImplementedForThisDevice;
	}


	custom->busy = false;

cleanup:
	return(ret);
}

static OsStatus_t KL25Z_I2CSendStream(Device_t *dev,uint16_t slaveAddr, uint16_t base_addr,uint8_t addrSize, void *data, uint32_t len, uint16_t timeout)
{

	OsStatus_t ret = kStatusOk;
	KL25ZI2CDevData_t *dat =  (KL25ZI2CDevData_t *)dev->config->devConfigData;
	KL25ZCustomI2CData_t *custom = (KL25ZCustomI2CData_t *)dev->deviceData;

	if(custom->busy) {
		/* device busy, exit */
		ret = kDeviceBusy;
		goto cleanup;
	}

	if(!custom->enabled) {
		/* device already enabled, discard */
		ret = kDeviceDisabled;
		goto cleanup;
	}

	if(data == NULL) {
		ret = kInvalidParam;
		goto cleanup;
	}

	if(len == 0) {
		ret = kInvalidParam;
		goto cleanup;
	}

	if(addrSize == 0) {
		ret = kInvalidParam;
		goto cleanup;
	}

	custom->busy = true;
	if(custom->master) {
		i2c_master_transfer_t xfer;
		xfer.data = (uint8_t *)data;
		xfer.dataSize = len;
		xfer.direction = kI2C_Write;
		xfer.slaveAddress = slaveAddr;
		xfer.subaddress = base_addr;
		xfer.subaddressSize = addrSize;
		custom->tx_sucess = false;

		/* request a write from SoC I2C controller */
		if(I2C_MasterTransferNonBlocking(dat->I2C, &custom->mhandle, &xfer) != kStatus_Success) {
			ret = kDeviceIoError;
		} else {

			/* transfer started, wait response from hardware */
			ret = uLipeDeviceStartSync(dev, timeout);
			if(ret == kStatusOk) {
				if(!custom->rx_sucess) {
					/* get how much data was arrived */
					I2C_MasterTransferAbort(dat->I2C, &custom->mhandle);
					ret = kDeviceIoError;
				}
			} else {
				I2C_MasterTransferAbort(dat->I2C, &custom->mhandle);
				ret = kTimeout;
			}
		}
	} else {

		/* todo, implement the slave driver */
		ret = kNotImplementedForThisDevice;
	}


	custom->busy = false;




cleanup:
	return(ret);
}

static OsStatus_t KL25Z_I2CReadByte (Device_t *dev, uint16_t slaveAddr, uint16_t addr, uint8_t addrSize, uint8_t *c, uint16_t timeout)
{
	OsStatus_t ret = kStatusOk;
	KL25ZI2CDevData_t *dat =  (KL25ZI2CDevData_t *)dev->config->devConfigData;
	KL25ZCustomI2CData_t *custom = (KL25ZCustomI2CData_t *)dev->deviceData;

	if(custom->busy) {
		/* device busy, exit */
		ret = kDeviceBusy;
		goto cleanup;
	}

	if(!custom->enabled) {
		/* device already enabled, discard */
		ret = kDeviceDisabled;
		goto cleanup;
	}


	if(c == NULL) {
		ret = kInvalidParam;
		goto cleanup;
	}

	if(addrSize == 0) {
		ret = kInvalidParam;
		goto cleanup;
	}



	custom->busy = true;

	if(custom->master) {
		i2c_master_transfer_t xfer;
		xfer.data = (uint8_t *)c;
		xfer.dataSize = sizeof(uint8_t);
		xfer.direction = kI2C_Read;
		xfer.slaveAddress = slaveAddr;
		xfer.subaddress = addr;
		xfer.subaddressSize = addrSize;
		custom->rx_sucess = false;

		/* request a read from SoC I2C controller */
		if(I2C_MasterTransferNonBlocking(dat->I2C, &custom->mhandle, &xfer) != kStatus_Success) {
			ret = kDeviceIoError;
		} else {

			/* transfer started, wait response from hardware */
			ret = uLipeDeviceStartSync(dev, timeout);
			if(ret == kStatusOk) {
				if(!custom->rx_sucess) {
					/* get how much data was arrived */
					I2C_MasterTransferAbort(dat->I2C, &custom->mhandle);
					ret = kDeviceIoError;
				}
			} else {
				I2C_MasterTransferAbort(dat->I2C, &custom->mhandle);
				ret = kTimeout;
			}
		}
	} else {

		/* todo, implement the slave driver */
		ret = kNotImplementedForThisDevice;
	}



	custom->busy = false;

cleanup:
	return(ret);

}

static OsStatus_t KL25Z_I2CReadStream(Device_t *dev,uint16_t slaveAddr, uint16_t base_addr, uint8_t addrSize, void *data, uint32_t expectedLen, uint32_t *actualLen, uint16_t timeout)
{
	OsStatus_t ret = kStatusOk;
	KL25ZI2CDevData_t *dat =  (KL25ZI2CDevData_t *)dev->config->devConfigData;
	KL25ZCustomI2CData_t *custom = (KL25ZCustomI2CData_t *)dev->deviceData;

	if(custom->busy) {
		/* device busy, exit */
		ret = kDeviceBusy;
		goto cleanup;
	}

	if(!custom->enabled) {
		/* device already enabled, discard */
		ret = kDeviceDisabled;
		goto cleanup;
	}


	if(data == NULL) {
		ret = kInvalidParam;
		goto cleanup;
	}

	if(expectedLen == 0) {
		ret = kInvalidParam;
		goto cleanup;
	}

	if(addrSize == 0) {
		ret = kInvalidParam;
		goto cleanup;
	}

	if(actualLen == NULL) {
		ret = kInvalidParam;
		goto cleanup;
	}


	custom->busy = true;

	if(custom->master) {
		i2c_master_transfer_t xfer;
		xfer.data = (uint8_t *)data;
		xfer.dataSize = expectedLen;
		xfer.direction = kI2C_Read;
		xfer.slaveAddress = slaveAddr;
		xfer.subaddress = base_addr;
		xfer.subaddressSize = addrSize;
		custom->rx_sucess = false;

		/* request a read from SoC I2C controller */
		if(I2C_MasterTransferNonBlocking(dat->I2C, &custom->mhandle, &xfer) != kStatus_Success) {
			ret = kDeviceIoError;
		} else {

			/* transfer started, wait response from hardware */
			ret = uLipeDeviceStartSync(dev, timeout);
			if(ret == kStatusOk) {
				if(!custom->rx_sucess) {
					/* get how much data was arrived */
					I2C_MasterTransferAbort(dat->I2C, &custom->mhandle);
					ret = kDeviceIoError;
				} else {
					*actualLen = expectedLen;
				}
			} else {
				I2C_MasterTransferGetCount(dat->I2C, &custom->mhandle, actualLen);
				I2C_MasterTransferAbort(dat->I2C, &custom->mhandle);
				ret = kTimeout;
			}
		}
	} else {

		/* todo, implement the slave driver */
		ret = kNotImplementedForThisDevice;
	}


	custom->busy = false;

cleanup:
	return(ret);
}




static OsStatus_t KL25Z_I2CEnable(Device_t *dev)
{
	OsStatus_t ret = kStatusOk;
	KL25ZI2CDevData_t *dat =  (KL25ZI2CDevData_t *)dev->config->devConfigData;
	KL25ZCustomI2CData_t *custom = (KL25ZCustomI2CData_t *)dev->deviceData;
	DeviceConfig_t *cfg = (DeviceConfig_t *)dev->config;

	if(custom->busy) {
		/* device busy, exit */
		ret = kDeviceBusy;
		goto cleanup;
	}

	if(custom->enabled) {
		/* device already enabled, discard */
		ret = kDeviceEnabled;
		goto cleanup;
	}

	/* lock device */
	custom->busy = true;

	NVIC_ClearPendingIRQ(cfg->irqOffset);
	I2C_Enable(dat->I2C, true);
	NVIC_EnableIRQ(cfg->irqOffset);
	custom->enabled = true;

	/* unlock device */
	custom->busy = false;

cleanup:
	return(ret);
}

static OsStatus_t KL25Z_I2CDisable(Device_t *dev)
{
	OsStatus_t ret = kStatusOk;
	KL25ZI2CDevData_t *dat =  (KL25ZI2CDevData_t *)dev->config->devConfigData;
	KL25ZCustomI2CData_t *custom = (KL25ZCustomI2CData_t *)dev->deviceData;
	DeviceConfig_t *cfg = (DeviceConfig_t *)dev->config;

	if(custom->busy) {
		/* device busy, exit */
		ret = kDeviceBusy;
		goto cleanup;
	}

	if(!custom->enabled) {
		/* device already enabled, discard */
		ret = kDeviceDisabled;
		goto cleanup;
	}

	/* lock device */
	custom->busy = true;

	NVIC_ClearPendingIRQ(cfg->irqOffset);
	NVIC_DisableIRQ(cfg->irqOffset);
	I2C_Enable(dat->I2C,false);
	custom->enabled = false;

	/* unlock device */
	custom->busy = false;

cleanup:
	return(ret);
}


/* prepare dev device linking its api  */
static I2cDriverApi_t kl25zI2CApi = {
		.I2cDriverConfig = KL25Z_I2CConfig,
		.I2cDriverWriteByte= KL25Z_I2CSendByte,
		.I2cDriverWriteStream= KL25Z_I2CSendStream,
		.I2cDriverReadByte= KL25Z_I2CReadByte,
		.I2cDriverReadStream=KL25Z_I2CReadStream,
		.I2cDriverEnable=KL25Z_I2CEnable,
		.I2cDriverDisable=KL25Z_I2CDisable
};



/* create data device and each drivers instances
 * use: ULIPE_DEVICE_DECLARE(DevName, DevConfig, DriverData, DriverApi)
 */
static KL25ZI2CDevData_t kl25zI2CData_a = {
		.I2C = I2C0,
		.I2CClk = kCLOCK_I2c0,
};

static KL25ZCustomI2CData_t kl25zI2CCustom_a = {
		.busy = false,
		.enabled = false,
};


static DeviceConfig_t kl25zI2CCfg_a = {
		.devConfigData = &kl25zI2CData_a,
		.name = I2C0_I2C_DEVICE_NAME,
		.refCount = 0,
		.earlyInitFcn = KL25Z_I2CDriverInit,
		.irqOffset = I2C0_IRQn,
};


ULIPE_DEVICE_DECLARE(I2cI2c0, &kl25zI2CCfg_a, &kl25zI2CCustom_a, &kl25zI2CApi);


/*!
 * I2C0_IRQHandler()
 */
void I2C0_IRQHandler(void)
{
	KL25ZI2CDevData_t *dat =  (KL25ZI2CDevData_t *)I2cI2c0.config->devConfigData;
	KL25ZCustomI2CData_t *custom = (KL25ZCustomI2CData_t *)I2cI2c0.deviceData;

	uLipeKernelIrqIn();
	/* select the operation of I2C */
	if(custom->master) {
		I2C_MasterTransferHandleIRQ(dat->I2C, &custom->mhandle);
	} else {
		I2C_SlaveTransferHandleIRQ(dat->I2C, &custom->mhandle);
	}
	uLipeKernelIrqOut();
}



#if OS_USE_I2C1_I2C > 0

static KL25ZI2CDevData_t kl25zI2CData_b = {
		.I2C = I2C1,
		.I2CClk = kCLOCK_I2c1,
};

static KL25ZCustomI2CData_t kl25zI2CCustom_b = {
		.busy = false,
		.enabled = false,
};


static DeviceConfig_t kl25zI2CCfg_b = {
		.devConfigData = &kl25zI2CData_b,
		.name = I2C1_I2C_DEVICE_NAME,
		.refCount = 0,
		.earlyInitFcn = KL25Z_I2CDriverInit,
		.irqOffset = I2C1_IRQn,

};


ULIPE_DEVICE_DECLARE(I2cI2c1, &kl25zI2CCfg_b, &kl25zI2CCustom_b, &kl25zI2CApi);

/*!
 * I2C1_IRQHandler()
 */
void I2C1_IRQHandler(void)
{
	KL25ZI2CDevData_t *dat =  (KL25ZI2CDevData_t *)I2cI2c0.config->devConfigData;
	KL25ZCustomI2CData_t *custom = (KL25ZCustomI2CData_t *)I2cI2c0.deviceData;

	uLipeKernelIrqIn();
	/* select the operation of I2C */
	if(custom->master) {
		I2C_MasterTransferHandleIRQ(dat->I2C, &custom->mhandle);
	} else {
		I2C_SlaveTransferHandleIRQ(dat->I2C, &custom->mhandle);
	}
	uLipeKernelIrqOut();

}

#endif



#endif
#endif
#endif
