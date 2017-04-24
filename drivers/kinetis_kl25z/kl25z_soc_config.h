/*!
 *
 * 							ULIPE RTOS VERSION 4
 *
 *
 *  \file kl25z_soc_config.h
 *
 *  \brief chip external driver configuration file
 *
 *	Use this file to provide access to third party software to help
 *	you to implement uLipe like device drivers
 *
 *  Author: FSN
 *
 */

#ifndef KL25Z_SOC_CONFIG_H_
#define KL25Z_SOC_CONFIG_H_


#if OS_USE_DEVICE_DRIVERS > 0
#if OS_USE_MCUEXPRESSO_FOR_KL25Z > 0

/* basic system setup */
#include "MKL25Z4.h"
#include "system_MKL25Z4.h"

/* mcu xpresso sdk external files */
#include "fsl_i2c.h"
#include "fsl_spi.h"
#include "fsl_gpio.h"
#include "fsl_adc16.h"
#include "fsl_uart.h"
#include "fsl_port.h"
#include "fsl_clock.h"
#include "fsl_lpsci.h"

#endif
#endif

#endif /*KL25Z_SOC_CONFIG_H_ */
