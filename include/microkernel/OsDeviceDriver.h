/*!
 *
 *                          ULIPE RTOS VERSION 4
 *
 *
 *  \file OsBase.h
 *
 *  \brief uLipe device model
 *
 *  Author: FSN
 *
 */
#ifndef OS_DEVICE_DRIVER_H_
#define OS_DEVICE_DRIVER_H_

/*
 * Device driver configuration structure
 */
typedef struct {
    void *devConfigData;
    char *name;
    uint32_t refCount;
    OsStatus_t (*earlyInitFcn) (void *arg);
    void (*driverIsr) (void *arg);
    uint32_t irqOffset;
}DeviceConfig_t;


/*
 * device driver generic data structure
 */
typedef struct {
    DeviceConfig_t *config;
    void *deviceApi;
    void *deviceData;
    OsHandler_t deviceSync;
}Device_t;

#if OS_USE_DEVICE_DRIVERS > 0

/*!
 *  uLipeDeviceOpen()
 *
 *  \brief Inits the device driver table
 *
 */
void uLipeDeviceTblInit(void);


/*!
 *  uLipeDeviceOpen()
 *
 *  \brief Gets a particular device instance
 *
 */
Device_t* uLipeDeviceOpen(const char *devName, OsStatus_t *err);



/*!
 *  uLipeDeviceStartSync()
 *
 *  \brief Start device syncrhonization with I/O operation
 *
 */
OsStatus_t uLipeDeviceStartSync(Device_t *dev, uint16_t timeout);


/*!
 *  uLipeDeviceFinishSync
 *
 *  \brief Signals the waiting device that synchronization is complete
 *
 */
OsStatus_t uLipeDeviceFinishSync(Device_t *dev);

#endif

/*
 * Macro to define a device
 */
#define ULIPE_DEVICE_DECLARE(DevName, DevConfig, DriverData, DriverApi)                              \
   Device_t __attribute__ ((section(OS_DEVICE_SECTION_NAME),__used__)) DevName   = { 		 		 \
            .config = DevConfig,	                                                                 \
            .deviceApi = DriverApi,                                                                  \
            .deviceData = DriverData,																 \
			.deviceSync = NULL																		 \
     }                                                                                               \

#endif
