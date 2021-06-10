//---------------- PARAMETERS.H ----------------------- 

#include <rtai_sem.h> //inclusione dell'header per l'utilizzo delle primitive semaforice

#define TICK_TIME 250000000

#define STACK_SIZE 10000

#include "../include/sensor_data_shared_memory.h" //avoid duplicate structure definition