//TBD: tutto

#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <unistd.h>
#include <rtai_lxrt.h>
#include <rtai_shm.h>

#include "task_info.h"
#include "thread.h"

static RT_TASK * main_task;
static pthread_t rt_task;
static power_failure_t * data, * data_bkp;

int main()
{
    data_bkp = malloc(sizeof(power_failure_t)); //acquisisci memoria per il backup
    /*the main function simply initializes the shared memory and creates the thread*/
    if(!(main_task = rt_task_init_schmod(nam2num("MAINTASK"), 0, 0, 0, SCHED_FIFO, 0)))
    {
        perror("Unable to promote main task to rtai task");
        exit(1);
    }
    else
    {
        //inizializza la shared memory
        data = rt_shm_alloc(SHARED_MEMORY_NAME, sizeof(power_failure_t), USE_VMALLOC);
        //crea e fa partire il thread
        pthread_create(&rt_task, NULL, power_presence_checker, (void *) 0);
        while(!pthread_join(rt_task, NULL))
        {
            printf("I am waiting\n");
            sleep(1);
        }

        //effettua il backup
        data_bkp->flag = data->flag;

        for(int i = 0; i < POWER_FAILURE_MESSAGE_SIZE; ++i)
        {
            data_bkp->message[i] = data->message[i];
        }

        //stampa i contenuti della memoria condivisa ->non può! basta un accesso ad una syscall che lxrt butta fuori, anche se il task non è
        //hard realtime!!!
        //printf("contents: [%d]: %s\n", data->flag, data->message);

        //effettua pulizia
        rt_shm_free(SHARED_MEMORY_NAME);
        rt_task_delete(main_task);

        //ora può stampare i caratteri
        printf("contents: [%d]: %s\n", data_bkp->flag, data_bkp->message);
        free(data_bkp);
    }

    return 0;
}