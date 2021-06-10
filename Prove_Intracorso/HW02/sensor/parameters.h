//---------------- PARAMETERS.H ----------------------- 

#include <rtai_sem.h> //needed to use semaphores

#define TICK_TIME 250000000

#define STACK_SIZE 10000

#include "../include/sensor_data_shared_memory.h" //avoid duplicate structures definitions