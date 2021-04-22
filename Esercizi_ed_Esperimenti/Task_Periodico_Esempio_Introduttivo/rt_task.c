#include <linux/module.h>
#include <linux/init.h> /*needed for the module characterization macros*/

#include <asm/rtai.h>
#include <rtai_shm.h>
#include <rtai_sched.h>

#include <stdbool.h>

#include "rt_task.h"

/*informazioni sul modulo*/
MODULE_AUTHOR("Davide Riemma N46003702 UNINA");
MODULE_DESCRIPTION("Esercizio su RTAI");
MODULE_LICENSE("GPL");

/*Dichiarazione di un task*/
static RT_TASK my_task;

static shmem_t * data;

static void task(int param)
{
    char letters[] = {'d', 'a', 'v', 'i', 'd', 'e'}; /*6 out of 8 bytes used*/
    unsigned short int i = 0; /*short int occupies 2 bytes, and here we have our 8 bytes!*/

    while (1)
    {
        /*put a character in the shared memory*/
        *data = letters[i++ % sizeof(letters)]; /*cicla utilizzando l'aritmetica modulare*/

        /*put the task to sleep until next period*/
        rt_task_wait_period();
    }
    

    return;
}

int init_module(void)
{
    RTIME task_period_in_ticks; /*conterrà il valore del periodo in rappresentazione interna di RTAI*/

    /*create the shared memory before the creation of the task, so that no guesses should be made on 
    whether the task starts its execution before the shared memory is created and the pointer is valid.*/
    data = rtai_kmalloc(SHM_ID, sizeof(shmem_t));

    /*RT Task initialization*/
    rt_task_init(&my_task, (void *) task, DUMMY_VALUE, TSK_WA_SIZE, TSK_PRIO, true, NULL);

    /*generating the start time relative to the task's phase*/
    task_period_in_ticks = nano2count(TSK_TICK);

    /*now start the periodic RT task*/
    rt_task_make_periodic(&my_task, rt_get_time() + task_period_in_ticks, task_period_in_ticks);

    return 0;
}
