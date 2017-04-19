/*!
 *
 * 							ULIPE RTOS VERSION 4
 *
 *
 *  \file uLipeRtos4.h
 *
 *  \brief this file is treated as a master file
 *
 *	In this file all the other header files is included so the user
 *  should only include this master file
 *
 *  Author: FSN
 *
 */

#ifndef __ULIPE_RTOS_4_H
#define __ULIPE_RTOS_4_H

 /*
  *  all the microkernel include files here:
  */

#include "include/microkernel/OsBase.h"
#include "include/microkernel/OsPort.h"
#include "include/microkernel/OsKernel.h"
#include "include/microkernel/OsTask.h"
#include "include/microkernel/OsFlags.h"
#include "include/microkernel/OsQueue.h"
#include "include/microkernel/OsMutex.h"
#include "include/microkernel/OsSem.h"
#include "include/microkernel/OsMem.h"
#include "include/microkernel/OsDeviceDriver.h"

/*
 *  all device drivers
 */
#include "include/drivers/OsDriverUart.h"
#include "include/drivers/OsDriverPinMux.h"
#include "include/drivers/OsDriverGpio.h"
#include "include/drivers/OsDriverAnalog.h"
#include "include/drivers/OsDriverSPI.h"
#include "include/drivers/OsDriverI2C.h"

#endif
