#include <string.h> //for strncpy
#include <stdio.h> //for printf
#include <rtai_lxrt.h> //fort rtai userspace related tasks
#include <rtai_shm.h>

#include "thread.h"

static RT_TASK * power_presence_checker_task;

void power_presence_checker_init_struct(power_failure_t * data)
{
    data->flag = 0;
    strncpy(data->message, "No power failure detected.", POWER_FAILURE_MESSAGE_SIZE);
    return;
}

unsigned int has_power_failded(void) //simulates random power failure, then returns 1 if the power has failed
{
    //TBD: simulare un power failure
    return 0;
}

void * power_presence_checker(void *)
{
    //inizializza il task
    if(!(power_presence_checker_task = rt_task_init_schmod(nam2num("PWCHK"), 5, 0, 0, SCHED_FIFO, 0)))
    {
        puts("UNABLE TO INITIATE POWER CHECKER TASK");
        exit(1); //not very good practice
    }
    else
    {
        //TBD: acquisizione del puntatore alla shmem
        //nota: la shmem contiene sia il dato, che il flag di terminazione

        //attiva il task periodico
        rt_task_make_periodic_relative_ns(power_presence_checker_task, 0, nano2count(T_c));
        //rendilo hard real time
        rt_make_hard_real_time();

        /*task vero e proprio*/
        while(!has_power_failded()){/*do nothing*/}

        /*TBD: power has failed, procedi*/

        /*operazioni di pulizia*/
        //TBD procedere con la chiusura delle shmem

        //eliminazione del tasks
        rt_task_delete(power_presence_checker_task);

    }

    return (void *) 0;
}