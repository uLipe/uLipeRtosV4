
/*!
 *
 *                          ULIPE RTOS VERSION 4
 *
 *
 *  \file kl25z_uart.c
 *
 *  \brief kl25z specific uart device driver
 *
 *
 *  Author: FSN
 *
 */


/* BIG WARNING: the driver below only supports the standards UART1 and UART2 they are aliased to UART0:1 respectively*/

#include "uLipeRtos4.h"
#include "kl25z_soc_config.h"


/* this driver can be only compiled if it is enabled on OsConfig.h*/
#if OS_USE_DEVICE_DRIVERS > 0
#if OS_USE_UART_DRIVERS > 0
#if OS_USE_MCUEXPRESSO_FOR_KL25Z > 0

/* define the uart priority IRQ */
#define UART_DRIVER_UART_ISR_PRIORITY 0xFD

/* creates a structure to device custom data */
typedef struct {
	UART_Type *uart;
	clock_ip_name_t uartClk;
}KL25ZUartDevData_t;

typedef struct {
	UART0_Type *uart;
	clock_ip_name_t uartClk;
}KL25ZLpSciDevData_t;


typedef struct {
	bool enabled;
	bool busy;
	bool tx_sucess;
	bool rx_sucess;
	uart_handle_t handle;
}KL25ZCustomUartData_t;


/*!
 *  KL25Z_UartIsr()
 *  Platform specific uart ISR handle the syncronization
 */
static uart_transfer_callback_t KL25Z_UartIsr(UART_Type *base, uart_handle_t *handle, status_t status, void *userData)
{
	Device_t *dev = (Device_t *)userData;
	KL25ZCustomUartData_t *custom = (KL25ZUartDevData_t *)dev->deviceData;
	OsStatus_t err;

	/* transmission or reception goes ok ? */
	if((status == kStatus_UART_TxIdle) || (status == kStatus_UART_RxIdle)) {
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


/*!
 *  KL25Z_UartIsr()
 *  Platform specific uart ISR handle the syncronization
 */
static lpsci_transfer_callback_t KL25Z_LpSciIsr(UART0_Type *base, uart_handle_t *handle, status_t status, void *userData)
{
	Device_t *dev = (Device_t *)userData;
	KL25ZCustomUartData_t *custom = (KL25ZUartDevData_t *)dev->deviceData;
	OsStatus_t err;

	/* transmission or reception goes ok ? */
	if((status == kStatus_LPSCI_TxIdle) || (status == kStatus_LPSCI_RxIdle)) {
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


/*!
 * 	KL25Z_UartDriverInit()
 *
 * \brief Early function that inits the UART
 *
 */
static OsStatus_t KL25Z_LpSciDriverInit(void *arg)
{
	OsStatus_t ret = kStatusOk;
	Device_t *dev = (Device_t *)arg;
	KL25ZLpSciDevData_t *dat =  (KL25ZLpSciDevData_t *)dev->config->devConfigData;
	KL25ZCustomUartData_t *custom = (KL25ZUartDevData_t *)dev->deviceData;

	/* sets the LPSCI clock source */
	CLOCK_SetLpsci0Clock(1);

	/* gates the uart peripheral clock */
	CLOCK_EnableClock(dat->uartClk);

	LPSCI_TransferCreateHandle((UART0_Type *)dat->uart, (lpsci_handle_t *)&custom->handle, &KL25Z_LpSciIsr, (void *)dev);

	return(ret);
}



/*!
 * 	KL25Z_UartDriverInit()
 *
 * \brief Early function that inits the LPSCI
 *
 */
static OsStatus_t KL25Z_UartDriverInit(void *arg)
{
	OsStatus_t ret = kStatusOk;
	Device_t *dev = (Device_t *)arg;
	KL25ZUartDevData_t *dat =  (KL25ZUartDevData_t *)dev->config->devConfigData;
	KL25ZCustomUartData_t *custom = (KL25ZUartDevData_t *)dev->deviceData;

	/* gates the uart peripheral clock */
	CLOCK_EnableClock(dat->uartClk);

	UART_TransferCreateHandle(dat->uart, &custom->handle, &KL25Z_UartIsr, (void *)dev);

	return(ret);
}



static OsStatus_t KL25Z_UartConfig(Device_t *this, uint32_t baud, uint32_t configMask)
{
	OsStatus_t ret = kStatusOk;
	KL25ZUartDevData_t *dat =  (KL25ZUartDevData_t *)this->config->devConfigData;

	if((UART0_Type *)dat->uart == UART0) {
		lpsci_config_t cfg;

		LPSCI_GetDefaultConfig(&cfg);

		/* select the new uart configuration based on configuration mask and user selected baud rate*/
		cfg.baudRate_Bps = baud;

		if(configMask & STOP_BIT_LEN_2) {
			cfg.stopBitCount = 2;
		}

		if(configMask & UART_PARITY_ODD) {
			cfg.parityMode = kUART_ParityOdd;
		} else if (configMask & UART_PARITY_EVEN) {
			cfg.parityMode = kUART_ParityEven;
		}

		/* defaults disabled */
		cfg.enableRx = false;
		cfg.enableRx = false;


		/* Set its configuration on NVIC */
	    NVIC_DisableIRQ(this->config->irqOffset);
	    NVIC_SetPriority(this->config->irqOffset, UART_DRIVER_UART_ISR_PRIORITY);


		/* Disable the tranmisstion and clear all uart events */
		LPSCI_DisableInterrupts((UART0_Type *)dat->uart, 0xFFFFFFFF);
		LPSCI_ClearStatusFlags((UART0_Type *)dat->uart, 0xFFFFFFFF);
		LPSCI_EnableRx((UART0_Type *)dat->uart, false);
		LPSCI_EnableTx((UART0_Type *)dat->uart, false);


		if(LPSCI_Init((UART0_Type *)dat->uart, &cfg, OS_CPU_RATE) != kStatus_Success) {
			ret = kNotImplementedForThisDevice;
		}

	} else {
		uart_config_t cfg;

		UART_GetDefaultConfig(&cfg);

		/* select the new uart configuration based on configuration mask and user selected baud rate*/
		cfg.baudRate_Bps = baud;

		if(configMask & STOP_BIT_LEN_2) {
			cfg.stopBitCount = 2;
		}

		if(configMask & UART_PARITY_ODD) {
			cfg.parityMode = kUART_ParityOdd;
		} else if (configMask & UART_PARITY_EVEN) {
			cfg.parityMode = kUART_ParityEven;
		}

		/* defaults disabled */
		cfg.enableRx = false;
		cfg.enableRx = false;


		/* Set its configuration on NVIC */
	    NVIC_DisableIRQ(this->config->irqOffset);
	    NVIC_SetPriority(this->config->irqOffset, UART_DRIVER_UART_ISR_PRIORITY);


		/* Disable the tranmisstion and clear all uart events */
		UART_DisableInterrupts(dat->uart, 0xFFFFFFFF);
		UART_ClearStatusFlags(dat->uart, 0xFFFFFFFF);
		UART_EnableRx(dat->uart, false);
		UART_EnableTx(dat->uart, false);


		if(UART_Init(dat->uart, &cfg, OS_CPU_RATE/2) != kStatus_Success) {
			ret = kNotImplementedForThisDevice;
		}

	}

	return(ret);
}

static OsStatus_t KL25Z_UartSendByte(Device_t *this, uint8_t c, uint16_t timeout )
{
	OsStatus_t ret = kStatusOk;
	KL25ZUartDevData_t *dat =  (KL25ZUartDevData_t *)this->config->devConfigData;
	KL25ZCustomUartData_t *custom = (KL25ZUartDevData_t *)this->deviceData;

	if(custom->busy){
		ret = kDeviceBusy;
		goto cleanup;
	}

	if(!custom->enabled) {
		/* devicemust be enabled to send byte */
		ret = kDeviceDisabled;
		goto cleanup;
	}


	if((UART0_Type *)dat->uart == UART0) {
		lpsci_transfer_t xfer;

		/* lock device */
		custom->busy = true;

		xfer.data = &c;
		xfer.dataSize = sizeof(uint8_t);

		/* dispatch the packet */
		LPSCI_TransferSendNonBlocking((UART0_Type *)dat->uart, (lpsci_handle_t *)&custom->handle, &xfer);
		ret = uLipeDeviceStartSync(this,timeout);

		if(ret != kStatusOk) {
			/* transmisstion timed out stop all uart activity*/
			LPSCI_TransferAbortSend((UART0_Type *)dat->uart, (lpsci_handle_t *)&custom->handle);
			custom->busy = false;
			goto cleanup;
		}


		if(custom->tx_sucess != true) {
			LPSCI_TransferAbortSend((UART0_Type *)dat->uart, (lpsci_handle_t *)&custom->handle);
			ret = kDeviceIoError;
		}

		custom->busy = false;

	} else {
		uart_transfer_t xfer;

		/* lock device */
		custom->busy = true;

		xfer.data = &c;
		xfer.dataSize = sizeof(uint8_t);

		/* dispatch the packet */
		UART_TransferSendNonBlocking(dat->uart, &custom->handle, &xfer);
		ret = uLipeDeviceStartSync(this,timeout);

		if(ret != kStatusOk) {
			/* transmisstion timed out stop all uart activity*/
			UART_TransferAbortSend(dat->uart, &custom->handle);
			custom->busy = false;
			goto cleanup;
		}


		if(custom->tx_sucess != true) {
			UART_TransferAbortSend(dat->uart, &custom->handle);
			ret = kDeviceIoError;
		}

		custom->busy = false;

	}


cleanup:
	return(ret);
}

static OsStatus_t KL25Z_UartSendStream(Device_t *this, void *data, uint32_t size, uint16_t timeout)
{

	OsStatus_t ret = kStatusOk;
	KL25ZUartDevData_t *dat =  (KL25ZUartDevData_t *)this->config->devConfigData;
	KL25ZCustomUartData_t *custom = (KL25ZUartDevData_t *)this->deviceData;

	if(custom->busy){
		ret = kDeviceBusy;
		goto cleanup;
	}

	if(!custom->enabled) {
		/* devicemust be enabled to send byte */
		ret = kDeviceDisabled;
		goto cleanup;
	}

	if(data == NULL) {
		ret = kInvalidParam;
		goto cleanup;
	}

	if(!size) {
		ret = kInvalidParam;
		goto cleanup;
	}

	if((UART0_Type *)dat->uart == UART0) {
		lpsci_transfer_t xfer;

		/* lock device */
		custom->busy = true;

		xfer.data = (uint8_t *)data;
		xfer.dataSize = size;

		/* dispatch the packet */
		LPSCI_TransferSendNonBlocking((UART0_Type *)dat->uart,(lpsci_handle_t *) &custom->handle, &xfer);
		ret = uLipeDeviceStartSync(this,timeout);

		if(ret != kStatusOk) {
			/* transmisstion timed out stop all uart activity*/
			LPSCI_TransferAbortSend((UART0_Type *)dat->uart, (lpsci_handle_t *)&custom->handle);
			custom->busy = false;
			goto cleanup;
		}

		if(custom->tx_sucess != true) {
			LPSCI_TransferAbortSend((UART0_Type *)dat->uart, (lpsci_handle_t *)&custom->handle);
			ret = kDeviceIoError;
		}
		custom->busy = false;


	} else {
		uart_transfer_t xfer;


		/* lock device */
		custom->busy = true;

		xfer.data = (uint8_t *)data;
		xfer.dataSize = size;

		/* dispatch the packet */
		UART_TransferSendNonBlocking(dat->uart, &custom->handle, &xfer);
		ret = uLipeDeviceStartSync(this,timeout);

		if(ret != kStatusOk) {
			/* transmisstion timed out stop all uart activity*/
			UART_TransferAbortSend(dat->uart, &custom->handle);
			custom->busy = false;
			goto cleanup;
		}

		if(custom->tx_sucess != true) {
			UART_TransferAbortSend(dat->uart, &custom->handle);
			ret = kDeviceIoError;
		}
		custom->busy = false;
	}



cleanup:
	return(ret);
}

static OsStatus_t KL25Z_UartReadByte (Device_t *this, uint8_t *c, uint16_t timeout)
{
	OsStatus_t ret = kStatusOk;
	KL25ZUartDevData_t *dat =  (KL25ZUartDevData_t *)this->config->devConfigData;
	KL25ZCustomUartData_t *custom = (KL25ZUartDevData_t *)this->deviceData;

	if(custom->busy){
		ret = kDeviceBusy;
		goto cleanup;
	}

	if(!custom->enabled) {
		/* devicemust be enabled to send byte */
		ret = kDeviceDisabled;
		goto cleanup;
	}

	if(c == NULL) {
		ret = kInvalidParam;
		goto cleanup;
	}


	if((UART0_Type *)dat->uart == UART0) {
		lpsci_transfer_t xfer;

		/* lock device */
		custom->busy = true;

		xfer.data = c;
		xfer.dataSize = sizeof(uint8_t);

		/* dispatch the packet */
		size_t expectedReceive = xfer.dataSize;
		LPSCI_TransferReceiveNonBlocking((UART0_Type *)dat->uart, (lpsci_handle_t *)&custom->handle, &xfer, &expectedReceive);
		ret = uLipeDeviceStartSync(this,timeout);


		if(ret != kStatusOk) {
			/* transmisstion timed out stop all uart activity*/
			LPSCI_TransferAbortReceive((UART0_Type *)dat->uart, (lpsci_handle_t *)&custom->handle);
			custom->busy = false;
			goto cleanup;
		}

		if(custom->rx_sucess != true) {
			LPSCI_TransferAbortReceive((UART0_Type *)dat->uart, (lpsci_handle_t *)&custom->handle);
			ret = kDeviceIoError;
		}

		custom->busy = false;

	} else {
		uart_transfer_t xfer;


		/* lock device */
		custom->busy = true;

		xfer.data = c;
		xfer.dataSize = sizeof(uint8_t);

		/* dispatch the packet */
		size_t expectedReceive = xfer.dataSize;
		UART_TransferReceiveNonBlocking(dat->uart, &custom->handle, &xfer, &expectedReceive);
		ret = uLipeDeviceStartSync(this,timeout);


		if(ret != kStatusOk) {
			/* transmisstion timed out stop all uart activity*/
			UART_TransferAbortReceive(dat->uart, &custom->handle);
			custom->busy = false;
			goto cleanup;
		}


		if(custom->rx_sucess != true) {
			UART_TransferAbortReceive(dat->uart, &custom->handle);
			ret = kDeviceIoError;
		}

		custom->busy = false;

	}


cleanup:
	return(ret);

}

static OsStatus_t KL25Z_UartReadStream(Device_t *this, void *data, uint32_t expected_size, uint32_t *actual_size, uint16_t timeout)
{
	OsStatus_t ret = kStatusOk;
	KL25ZUartDevData_t *dat =  (KL25ZUartDevData_t *)this->config->devConfigData;
	KL25ZCustomUartData_t *custom = (KL25ZUartDevData_t *)this->deviceData;

	if(custom->busy){
		ret = kDeviceBusy;
		goto cleanup;
	}

	if(!custom->enabled) {
		/* devicemust be enabled to send byte */
		ret = kDeviceDisabled;
		goto cleanup;
	}

	if(data == NULL) {
		ret = kInvalidParam;
		goto cleanup;
	}

	if(expected_size == 0) {
		ret = kInvalidParam;
		goto cleanup;
	}

	if(actual_size == NULL) {
		ret = kInvalidParam;
		goto cleanup;
	}

	if((UART0_Type *)dat->uart == UART0) {
		lpsci_transfer_t xfer;

		/* lock device */
		custom->busy = true;

		xfer.data = (uint8_t *)data;
		xfer.dataSize = expected_size;

		/* dispatch the packet */
		LPSCI_TransferReceiveNonBlocking((UART0_Type *)dat->uart, (lpsci_handle_t *)&custom->handle, &xfer, (size_t *)actual_size);
		ret = uLipeDeviceStartSync(this,timeout);


		if(ret != kStatusOk) {
			/* transmisstion timed out stop all uart activity*/
			LPSCI_TransferAbortReceive((UART0_Type *)dat->uart, (lpsci_handle_t *)&custom->handle);
			custom->busy = false;
			goto cleanup;
		}


		if(custom->rx_sucess != true) {
			LPSCI_TransferAbortReceive((UART0_Type *)dat->uart, (lpsci_handle_t *)&custom->handle);
			ret = kDeviceIoError;
		}

		custom->busy = false;


	} else {
		uart_transfer_t xfer;

		/* lock device */
		custom->busy = true;

		xfer.data = (uint8_t *)data;
		xfer.dataSize = expected_size;

		/* dispatch the packet */
		UART_TransferReceiveNonBlocking(dat->uart, &custom->handle, &xfer, (size_t *)actual_size);
		ret = uLipeDeviceStartSync(this,timeout);


		if(ret != kStatusOk) {
			/* transmisstion timed out stop all uart activity*/
			UART_TransferAbortReceive(dat->uart, &custom->handle);
			custom->busy = false;
			goto cleanup;
		}


		if(custom->rx_sucess != true) {
			UART_TransferAbortReceive(dat->uart, &custom->handle);
			ret = kDeviceIoError;
		}

		custom->busy = false;

	}


cleanup:
	return(ret);
}

static OsStatus_t KL25Z_UartEnable(Device_t *this)
{
	OsStatus_t ret = kStatusOk;
	KL25ZUartDevData_t *dat =  (KL25ZUartDevData_t *)this->config->devConfigData;
	KL25ZCustomUartData_t *custom = (KL25ZUartDevData_t *)this->deviceData;

	if(custom->busy){
		ret = kDeviceBusy;
		goto cleanup;
	}

	if(custom->enabled) {
		/* device already enabled */
		ret = kDeviceEnabled;
		goto cleanup;
	}

	if((UART0_Type *)dat->uart == UART0) {
		/* lock device */
		custom->busy = true;

		/* enable TX, RX and uart related interrupts */
		LPSCI_EnableRx((UART0_Type *)dat->uart, true);
		LPSCI_EnableTx((UART0_Type *)dat->uart, true);

	} else {
		/* lock device */
		custom->busy = true;

		/* enable TX, RX and uart related interrupts */
		UART_EnableRx(dat->uart, true);
		UART_EnableTx(dat->uart, true);
	}

	custom->enabled = true;
	NVIC_ClearPendingIRQ(this->config->irqOffset);
	NVIC_EnableIRQ(this->config->irqOffset);

	custom->busy = false;

cleanup:
	return(ret);
}

static OsStatus_t KL25Z_UartDisable(Device_t *this)
{
	OsStatus_t ret = kStatusOk;
	KL25ZUartDevData_t *dat =  (KL25ZUartDevData_t *)this->config->devConfigData;
	KL25ZCustomUartData_t *custom = (KL25ZUartDevData_t *)this->deviceData;

	if(custom->busy){
		ret = kDeviceBusy;
		goto cleanup;
	}

	if(!custom->enabled) {
		/* device already enabled */
		ret = kDeviceDisabled;
		goto cleanup;
	}

	/* lock device */
	custom->busy = true;
	/* enable TX, RX and uart related interrupts */
	NVIC_DisableIRQ(this->config->irqOffset);
	NVIC_ClearPendingIRQ(this->config->irqOffset);


	if((UART0_Type *)dat->uart == UART0) {
		LPSCI_DisableInterrupts((UART0_Type *)dat->uart, 0xFFFFFFFF);
		LPSCI_ClearStatusFlags((UART0_Type *)dat->uart, 0xFFFFFFFF);
		LPSCI_EnableRx((UART0_Type *)dat->uart, false);
		LPSCI_EnableTx((UART0_Type *)dat->uart, false);

	} else {
		UART_DisableInterrupts(dat->uart, 0xFFFFFFFF);
		UART_ClearStatusFlags(dat->uart, 0xFFFFFFFF);
		UART_EnableRx(dat->uart, false);
		UART_EnableTx(dat->uart, false);
	}

	custom->enabled = false;
	custom->busy = false;

cleanup:
	return(ret);
}


/* prepare this device linking its api  */
static UartDeviceApi_t kl25zUartApi = {
		.uLipeUartConfig = KL25Z_UartConfig,
		.uLipeUartSendByte= KL25Z_UartSendByte,
		.uLipeUartSendStream= KL25Z_UartSendStream,
		.uLipeUartReadByte= KL25Z_UartReadByte,
		.uLipeUartReadStream=KL25Z_UartReadStream,
		.uLipeUartEnable=KL25Z_UartEnable,
		.uLipeUartDisable=KL25Z_UartDisable
};



/* create data device and each drivers instances
 * use: ULIPE_DEVICE_DECLARE(DevName, DevConfig, DriverData, DriverApi)
 */
static KL25ZLpSciDevData_t kl25zUartData_a = {
		.uart = UART0,
		.uartClk = kCLOCK_Uart0,
};

static KL25ZCustomUartData_t kl25zUartCustom_a = {
		.busy = false,
		.enabled = false,
};


static DeviceConfig_t kl25zUartCfg_a = {
		.devConfigData = &kl25zUartData_a,
		.name = UART0_UART_DEVICE_NAME,
		.refCount = 0,
		.earlyInitFcn = KL25Z_LpSciDriverInit,
		.irqOffset = UART0_IRQn,
};


ULIPE_DEVICE_DECLARE(uartUart0, &kl25zUartCfg_a, &kl25zUartCustom_a, &kl25zUartApi);


/*!
 * UART0_IRQHandler()
 */
void UART0_IRQHandler(void)
{
	KL25ZUartDevData_t *dat =  (KL25ZUartDevData_t *)uartUart0.config->devConfigData;
	KL25ZCustomUartData_t *custom = (KL25ZUartDevData_t *)uartUart0.deviceData;

	uLipeKernelIrqIn();
	LPSCI_TransferHandleIRQ((UART0_Type *)dat->uart, &custom->handle);
	uLipeKernelIrqOut();
}



#if OS_USE_UART1_UART > 0

static KL25ZUartDevData_t kl25zUartData_b = {
		.uart = UART1,
		.uartClk = kCLOCK_Uart1,
};

static KL25ZCustomUartData_t kl25zUartCustom_b = {
		.busy = false,
		.enabled = false,
};


static DeviceConfig_t kl25zUartCfg_b = {
		.devConfigData = &kl25zUartData_b,
		.name = UART1_UART_DEVICE_NAME,
		.refCount = 0,
		.earlyInitFcn = KL25Z_UartDriverInit,
		.irqOffset = UART1_IRQn,

};


ULIPE_DEVICE_DECLARE(uartUart1, &kl25zUartCfg_b, &kl25zUartCustom_b, &kl25zUartApi);

/*!
 * UART1_IRQHandler()
 */
void UART1_IRQHandler(void)
{
	KL25ZUartDevData_t *dat =  (KL25ZUartDevData_t *)uartUart1.config->devConfigData;
	KL25ZCustomUartData_t *custom = (KL25ZUartDevData_t *)uartUart1.deviceData;

	uLipeKernelIrqIn();
	UART_TransferHandleIRQ(dat->uart, &custom->handle);
	uLipeKernelIrqOut();

}

#endif


#if OS_USE_UART2_UART > 0

static KL25ZUartDevData_t kl25zUartData_c = {
		.uart = UART2,
		.uartClk = kCLOCK_Uart2,
};

static KL25ZCustomUartData_t kl25zUartCustom_c = {
		.busy = false,
		.enabled = false,
};


static DeviceConfig_t kl25zUartCfg_c = {
		.devConfigData = &kl25zUartData_c,
		.name = UART2_UART_DEVICE_NAME,
		.refCount = 0,
		.earlyInitFcn = KL25Z_UartDriverInit,
		.irqOffset = UART2_IRQn,

};


ULIPE_DEVICE_DECLARE(uartUart2, &kl25zUartCfg_c, &kl25zUartCustom_c, &kl25zUartApi);

/*!
 * UART1_IRQHandler()
 */
void UART2_IRQHandler(void)
{
	KL25ZUartDevData_t *dat =  (KL25ZUartDevData_t *)uartUart2.config->devConfigData;
	KL25ZCustomUartData_t *custom = (KL25ZUartDevData_t *)uartUart2.deviceData;

	uLipeKernelIrqIn();
	UART_TransferHandleIRQ(dat->uart, &custom->handle);
	uLipeKernelIrqOut();

}

#endif


#endif
#endif
#endif
