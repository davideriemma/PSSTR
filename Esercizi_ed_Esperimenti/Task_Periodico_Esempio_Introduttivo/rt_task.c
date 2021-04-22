#include <asm/rtai.h>
#include <rtai_shm.h>
#include <rtai_sched.h>

#include "rt_task.h"

/*Dichiarazione di un task*/
static RT_TASK my_task;

static shmem_t * data;

static void task(int param)
{

    /*do something*/

    return;
}

int init_module(void)
{

    data = rtai_kmalloc(SHM_ID, sizeof(shmem_t));

    return 0;
}
