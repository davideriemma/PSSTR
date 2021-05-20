#include <string.h> //for strncpy
#include <stdio.h> //for printf>
#include <rtai_lxrt.h> //fort rtai userspace related tasks
#include <rtai_shm.h>

#include "thread.h"
#include "task_info.h"

static RT_TASK * power_presence_checker_task;

void power_presence_checker_init_struct(power_failure_t * data)
{
    data->flag = 0;
    strncpy(data->message, "No power failure detected.", POWER_FAILURE_MESSAGE_SIZE);
    return;
}

unsigned int has_power_failed(void) //simulates random power failure, then returns 1 if the power has failed
{
    static unsigned int i = 1;
    i <<= 1;
    return i;
}

void * power_presence_checker(void * dummy)
{
    char message[] = "Power Has failed, please, intervene! 40 minutes to go";
    //inizializza il task
    if(!(power_presence_checker_task = rt_task_init_schmod(nam2num("PWCHK"), 5, 0, 0, SCHED_FIFO, 0)))
    {
        puts("UNABLE TO INITIATE POWER CHECKER TASK");
        exit(1); //not very good practice
    }
    else
    {
        //TBD: acquisizione del puntatore alla shmem -> done
        //nota: la shmem contiene sia il dato, che il flag di terminazione
        power_failure_t * data = rt_shm_alloc(SHARED_MEMORY_NAME, sizeof(power_failure_t), USE_VMALLOC);

        //attiva il task periodico
        rt_task_make_periodic_relative_ns(power_presence_checker_task, 0, nano2count(T_c));
        //rendilo hard real time
        rt_make_hard_real_time();

        /*task vero e proprio*/
        while(has_power_failed() != (1 << 24)){/*do nothing*/}

        /*TBD: power has failed, procedi -> done*/
        data->flag = 1;
        
        //la funzione di copia la si implementa a mano, onde evitare system calls (che non so perché debbano)
        //esserci in una funzione come strncpy, ma non si può mai sapere

        for(int i = 0; i < sizeof(message); ++i) //copia anche \0.
        {
            data->message[i] = message[i];
        }

        /*operazioni di pulizia*/
        //TBD procedere con la chiusura delle shmem ->done
        rt_shm_free(SHARED_MEMORY_NAME);
        //eliminazione del tasks
        rt_task_delete(power_presence_checker_task);

    }

    return (void *) 0;
}