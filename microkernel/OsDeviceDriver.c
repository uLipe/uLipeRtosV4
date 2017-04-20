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

#include "uLipeRtos4.h"

#if OS_USE_DEVICE_DRIVERS > 0

/** private variables */
extern Device_t __OsDeviceTblStart[];
extern Device_t __OsDeviceTblEnd[];


/** public functions */
void uLipeDeviceTblInit(void)
{
    Device_t *devices = NULL;

    /* searche for a device entry on device table */
    for( devices = __OsDeviceTblStart; devices != __OsDeviceTblEnd; devices++) {
        if(devices->config->earlyInitFcn != NULL) {
            devices->config->earlyInitFcn(devices);
        }
    }
}


Device_t* uLipeDeviceOpen(const char *devName, OsStatus_t *err)
{
    Device_t *ret = NULL;
    bool found = false;

    /* searche for a device entry on device table */
    for( ret = __OsDeviceTblStart; ret != __OsDeviceTblEnd; ret++) {
        if((ret->deviceApi != NULL) && (!strcmp(devName, ret->config->name))){
            /* if this driver is a first instance, then create the sync object */
            if(ret->config->refCount == 0) {
                ret->deviceSync = uLipeSemCreate(1,0,NULL);
            }


            /* if sync object is fully created mark as device used */
            if(ret->deviceSync != NULL) {
                if(ret->config->refCount < 0xFFFFFFFF)
                    ret->config->refCount++;
                found = true;
            }
            break;
        }
    }

    if(found != false) {
        if(err != NULL) *err = kStatusOk;

    } else {
        ret = NULL;
        if(err != NULL) *err = kDeviceNotFound;
    }
    return(ret);
}


OsStatus_t uLipeDeviceStartSync(Device_t *dev)
{
    OsStatus_t ret = kStatusOk;

    /* check parameters */
    if(dev == NULL) {
        ret = kInvalidParam;
        goto cleanup;
    }

    /* device has a sync object */
    if(dev->deviceSync == NULL) {
        ret = kInvalidParam;
        goto cleanup;
    }

    /* start the sync waiting for the I/O signal */
    uLipeSemTake(dev->deviceSync, 0);

cleanup:
    return(ret);
}


OsStatus_t uLipeDeviceFinishSync(Device_t *dev)
{
    OsStatus_t ret = kStatusOk;

    /* check parameters */
    if(dev == NULL) {
        ret = kInvalidParam;
        goto cleanup;
    }

    /* device has a sync object */
    if(dev->deviceSync == NULL) {
        ret = kInvalidParam;
        goto cleanup;
    }

    /* start the sync waiting for the I/O signal */
    uLipeSemGive(dev->deviceSync, 1);

cleanup:
    return(ret);
}

#endif
