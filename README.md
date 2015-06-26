# uLipeRtosV4
Simple preemptive, realtime, multitask kernel made just for fun.

uLipeRTOS Version 4 is a simple, but powerful real time kernel, currently implemented for arm cortex M3 and M4 processors.
The kernel uses a fully preemptive schedule policy, and supports up to 1024 priority levels.

Main features:

- Real time, preemptive kernel;
- Fast context switching time, below to 100ns @ 50MHz processor clock;
- Fully configurable thru a config header file;
- Supports up to 1024 priority levels;
- Event flag groups, up to 32bits events;
- Counting semaphores;
- Binary semaphores;
- Queueing by reference support;
- Port file formed by two simple files in C and Assembly, simple to port;
- Unlimited kernel objects (limited by processor memory);
- Run time creation objects;
- Except port file, implementend in fully ANSI-C.

