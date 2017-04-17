# uLipeRtosV4
Simple preemptive, realtime, multitask kernel made just for fun.
uLipeRTOS Version 4 is a simple, but powerful real time kernel, currently implemented for arm cortex M0,M3,M4 and M7 architectures.
The kernel uses a fully preemptive schedule policy, and supports up to 1024 priority levels.

# Low memory footprint:
  - 3.3KB of code with full modules enabled*; 
  - kernel objects are 48 bytes long or less (low ram consumption);

  *built on GCC_ARM 5.2 with -Os option
  
# Main Features

- Real time, preemptive microkernel;
- Fast context switching time, below to 100ns @ 50MHz processor clock;
- O(1) Dynamic memory allocator based on powerful TLSF alghoritm optimized to low size pools as 64KB or 128KB;
- Supports up to 1024 priority levels ( highest prio is reserved for mutex and lowest to idle task);
- Event flag groups, up to 32bits events, support signaling with broadcast;
- Counting semaphores;
- Binary semaphores;
- Zero copy, type agnostic mailboxes / message queues;
- Device driver model (in development);
- Unlimited kernel objects / heap size (limited by processor memory);
- Run time creation objects;
- Port file formed by two simple files in C and Assembly, simple to port;
- Single header kernel, put on you application and enjoy.

# Recommended processor resources

- 4KB of Code targeted memory(ROM);
- 1.6KB of Data memory (RAM)¹²  

¹ considering 1KB of heap usage
² 650 Bytes is overhead used by the dynamic memory allocator.


# Basic Usage

- uLipeRtos was built to be simple in its essence;
- Add the folders to the include paths: uLipeRtosV4 and uLipeRtosV4/arch;
- Add the folders as sources: uLipeRtosV4/microkernel and uLipeRtosV4/microkernel/arch;
- Go to include/microkernel/OsConfig.h and edit the following lines:
```

//Set the ticker frequency based on your target system clock
#define OS_CPU_RATE  			48000000 //in Hz
#define OS_TICK_RATE			1000	//in Hz


//Select the one of current supported architecture:
#define OS_ARCH_CORTEX_M0     1
#define OS_ARCH_CORTEX_M3     0
#define OS_ARCH_CORTEX_M4     0
#define OS_ARCH_CORTEX_M7     0

//Define the number of tasks (each task must have a unique priority):
#define OS_NUMBER_OF_TASKS  8
 
//Define how much heap(in bytes)to be used rtos memory allocation (use the suggested value):
#define OS_HEAP_SIZE       2048

```

- Play witth the following demo:

```

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
    uLipeTaskCreate(&test_task2,128,1,NULL);

    /* give to the kernel the control of processor */
    uLipeRtosStart();

    return 0;
}
```

- Build using your preferred IDE;
