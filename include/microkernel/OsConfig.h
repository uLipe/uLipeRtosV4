/*!
 *
 * 							ULIPE RTOS VERSION 4
 *
 *
 *  \file OsConfig.h
 *
 *  \brief uLipe Rtos config file
 *
 *  In this file the user can select and specify the quantity of kernel objects
 *  and which code is generated for these modules, in this file also the user
 *  should provide the cpu clock rate and the ticks per second count
 *
 *  Author: FSN
 *
 */

#ifndef __OS_CONFIG_H
#define __OS_CONFIG_H

/*
 * 	clock system and ticks per seconds
 */

#define OS_CPU_RATE  			48000000 //in Hz
#define OS_TICK_RATE			1000	//in Hz
#define OS_DEBUG
/*
 *  Architecture selection:
 */

//
// ARM Cortex:
//
#define OS_ARCH_CORTEX_M0 	  1
#define OS_ARCH_CORTEX_M3 	  0
#define OS_ARCH_CORTEX_M4     0
#define OS_ARCH_CORTEX_M7     0


//
// Other archs TBD
//

/*
 * 	Kernel configurations:
 */
#define OS_IDLE_TASK_HOOK_EN		0
#define OS_FAST_SCHED           	0
#define OS_MINIMAL_STACK            32

/*
 * 	task kernel objects and generation code:
 */

#define OS_NUMBER_OF_TASKS 			  	8 //MUST BE > 0
#define OS_TASK_MODULE_EN			    1 //Gererate code for task management


/*
 * specifies system heap size bytes
 */
#define OS_HEAP_SIZE                    4096

/*
 *  timers and delays:
 */
#define OS_DELAY_TIME_BASE 		(10000/(OS_TICKS_PER_SECOND)) //In steps of 0.1ms

/*
 * event flags kernel objects and code:
 */
#define OS_FLAGS_MODULE_EN			  1

/*
 * semaphore kernel objects and code
 */
#define OS_SEM_MODULE_EN			  1

/*
 * Mutex kernel objects and code
 */
#define OS_MTX_MODULE_EN			  1

/*
 * Queue kernel objects and code
 */
#define OS_QUEUE_MODULE_EN		      1



/*
 * Device drivers:
 */
#define OS_USE_DEVICE_DRIVERS           0
#define OS_DEVICE_SECTION_NAME      ".device_driver"

/*
 *
 *  To use device model place the following snippet on your linker script on .data section

					__OsDeviceTblStart = .;
					KEEP(*(.device_driver))
					__OsDeviceTblEnd  = .;


 *
 */


/*
 * External soc manufacturer sw support selection:
 */
#define OS_USE_MCUEXPRESSO_FOR_KL25Z	1
#define OS_USE_MCUEXPRESSO_FOR_K64F		0


/*
 * Enable use of pin mux drivers
 */
#define OS_USE_PINMUX_DRIVERS				1

#if OS_USE_PINMUX_DRIVERS > 0

	#define OS_USE_PORTB_PIN_MUX			1
	#define OS_USE_PORTC_PIN_MUX			1
	#define OS_USE_PORTD_PIN_MUX			1
	#define OS_USE_PORTE_PIN_MUX			1

	#define PORTA_PIN_MUX_DEVICE_NAME		"pmux0"

	#if(OS_USE_PORTB_PIN_MUX > 0)
	#define PORTB_PIN_MUX_DEVICE_NAME		"pmux1"
	#endif

	#if(OS_USE_PORTC_PIN_MUX > 0)
	#define PORTC_PIN_MUX_DEVICE_NAME		"pmux2"
	#endif

	#if(OS_USE_PORTD_PIN_MUX > 0)
	#define PORTD_PIN_MUX_DEVICE_NAME		"pmux3"
	#endif

	#if(OS_USE_PORTE_PIN_MUX > 0)
	#define PORTE_PIN_MUX_DEVICE_NAME		"pmux4"
	#endif

#endif


/*
 * Enable use of GPIO Drivers
 */
#define OS_USE_GPIO_DRIVERS				1

#if OS_USE_GPIO_DRIVERS > 0
	#define OS_USE_GPIOB_GPIO 			1
	#define OS_USE_GPIOC_GPIO			1
	#define OS_USE_GPIOD_GPIO			1
	#define OS_USE_GPIOE_GPIO			1


	#define GPIOA_GPIO_DEVICE_NAME		"gpio0"

	#if (OS_USE_GPIOB_GPIO > 0)
	#define GPIOB_GPIO_DEVICE_NAME		"gpio1"
	#endif

	#if (OS_USE_GPIOC_GPIO > 0)
	#define GPIOC_GPIO_DEVICE_NAME		"gpio2"
	#endif

	#if (OS_USE_GPIOD_GPIO > 0)
	#define GPIOD_GPIO_DEVICE_NAME		"gpio3"
	#endif

	#if (OS_USE_GPIOE_GPIO > 0)
	#define GPIOE_GPIO_DEVICE_NAME		"gpio4"
	#endif
#endif

/*
 * Enable use of uart drivers
 */
#define OS_USE_UART_DRIVERS				1

#if OS_USE_UART_DRIVERS > 0

	#define OS_USE_UART1_UART			1
	#define OS_USE_UART2_UART			1


	#define UART0_UART_DEVICE_NAME		"uart0"

	#if(OS_USE_UART1_UART)
	#define UART1_UART_DEVICE_NAME		"uart1"
	#endif

	#if(OS_USE_UART2_UART)
	#define UART2_UART_DEVICE_NAME		"uart2"
	#endif


#endif



/*
 * Enable use of SPI drivers
 */
#define OS_USE_SPI_DRIVERS				0

/*
 * Enable use of I2C drivers
 */
#define OS_USE_I2C_DRIVERS				1

#if OS_USE_I2C_DRIVERS > 0

	#define OS_USE_I2C1_I2C			1


	#define I2C0_I2C_DEVICE_NAME		"i2c0"

	#if(OS_USE_I2C1_I2C)
	#define I2C1_I2C_DEVICE_NAME		"i2c1"
	#endif

#endif

/*
 * Enable use of AIO drivers
 */
#define OS_USE_AIO_DRIVERS				0

/*
 * Enable use of ENET drivers
 */
#define OS_USE_ENET_DRIVERS				0

/*
 * Enable use of multicore drivers
 */
#define OS_USE_ASSI_MULTICORE_DRIVERS	0


/*
 * Enable kernel console
 */
#define OS_CONSOLE_ENABLE			   0


#if (OS_CONSOLE_ENABLE > 0)
#if (OS_USE_DEVICE_DRIVERS > 0) &&  (OS_USE_UART_DRIVERS > 0)
	#define OS_CONSOLE_DRIVER					UART0_UART_DEVICE_NAME
	#define OS_CONSOLE_CONFIG_VALID 			1
	#define OS_CONSOLE_DRIVER_PINMUX_NAME		PORTA_PIN_MUX_DEVICE_NAME
	#define OS_CONSOLE_DRIVER_PIN_OFFSET	    1
	#define OS_CONSOLE_DRIVER_PINS_LEN			2
	#define OS_CONSOLE_ACCEPT_MASK				0x03
	#define OS_CONSOLE_ALTERNATE				0x02

#else
	#error "Console needs to a I/O device driver, please provide one"
#endif
#endif


#endif
