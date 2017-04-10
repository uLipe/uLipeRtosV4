# uLipeRtosV4
Simple preemptive, realtime, multitask kernel made just for fun.

uLipeRTOS Version 4 is a simple, but powerful real time kernel, currently implemented for arm cortex M3 and M4 processors.
The kernel uses a fully preemptive schedule policy, and supports up to 1024 priority levels.

# Low memory footprint:
  - 3.3KB of code with full modules enabled*; 
  - kernel objects are 48 bytes long or less (low ram consumption);

  *built on GCC_ARM 5.2 with -Os option
  
# Main Features

- Real time, preemptive kernel;
- Fast context switching time, below to 100ns @ 50MHz processor clock;
- O(1) Dynamic memory allocator based on powerful TLSF alghoritm
- Supports up to 1024 priority levels;
- Event flag groups, up to 32bits events;
- Counting semaphores;
- Binary semaphores;
- Queueing by reference support;
- Port file formed by two simple files in C and Assembly, simple to port;
- Unlimited kernel objects (limited by processor memory);
- Run time creation objects;
- Except port file, implementend in fully ANSI-C.
- Single header kernel, put on you application and enjoy.

# Basic Usage

- uLipeRtos was built to be simple in its essence;
- Add the folders to the include paths: uLipeRtosV4 and uLipeRtosV4/arch;
- Add the folders as sources: uLipeRtosV4/source and uLipeRtosV4/source/arch;
- Go to OsConfig.h and edit the following lines:
	-- Select the one of current supported architecture:
		#define OS_ARCH_CORTEX_M0     1
		#define OS_ARCH_CORTEX_M3     0
		#define OS_ARCH_CORTEX_M4     0
		#define OS_ARCH_CORTEX_M7     0

	-- Define the number of tasks (each task must have a unique priority):
		#define OS_NUMBER_OF_TASKS  8 

	-- Define how much heap(in bytes)to be used rtos memory allocation (use the suggested value):
		#define OS_HEAP_SIZE       4096

	-- Set the ticker frequency based on your target system clock
		#define OS_CPU_RATE  			48000000 //in Hz
		#define OS_TICK_RATE			1000	//in Hz


- Play witth the following demo:

...
#include "uLipeRtos4.h"

OsHandler_t r_sem;

void test_task1(void *args)
{
    uint32_t cntr = 0;

    /* creates a binary semaphore
     * to play syncrhonizing the two tasks
     */
    r_sem = uLipeSemCreate(0,1,NULL);
    uLipeAssert(r_sem != NULL)
    for(;;) {

        /* wait test_task2 to signal the semaphore to resume execution */
        uLipeSemTake(r_sem, 0);
        cntr++;
    }
}

void test_task2(void *args)
{
    uint32_t cntr = 0;

    for(;;) {
        cntr++;
        /* signal r_sem and wake-up the test_task1 */
        uLipeSemGive(r_sem, 1);
    }
}


int main(void)
{

    /* Init function always need to be called at first time*/
    uLipeRtosInit();

    /* create two test tasks */
    uLipeTaskCreate(&test_task1,128,2,NULL);
    uLipeTaskCreate(&test_task1,128,1,NULL);

    /* give to the kernel the control of processor */
    uLipeRtosStart();

    return 0;
}
...


- Build using your preferred IDE;
