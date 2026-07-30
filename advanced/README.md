# Under the Hood

The chapters so far have treated FreeRTOS through its API: create a task, send
to a queue, take a semaphore. This closing chapter looks beneath that surface
at four things the API quietly depends on: the Cortex-M hardware features the
port is built on, the heap that every `Create` call draws from, the hook
functions through which the kernel calls back into the application, and the
CMSIS-RTOS layer that vendor tools wrap around the native API. It is a
reference chapter, all prose and no apps. The scheduling policies themselves,
preemption, time slicing, and cooperative scheduling, have their own chapter
under [Understanding the Scheduler](../scheduler/README.md).
