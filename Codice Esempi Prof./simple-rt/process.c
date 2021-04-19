//------------------- RT_PROCESS.C ---------------------- 

#include <linux/module.h>

#include <asm/io.h>

#include <asm/rtai.h>

#include <rtai_shm.h>

#include <rtai_sched.h>

#include "parameters.h"

 

static RT_TASK my_task;

static struct data_str *data;

 

static void fun(int t)

{

    unsigned int count = 0;

    int val = 0;

    while (1) {

        data->indx_counter = count;

        if ((count%10)==0){ 
		val = val + 1;
		val = val % 2;
	}

        data->value = val;

        count++;

        rt_task_wait_period();

    }

}

int init_module(void)

{

    RTIME tick_period;


    rt_task_init(&my_task, (void *)fun, 1, STACK_SIZE, TASK_PRIORITY, 1, 0);

    data = rtai_kmalloc(SHMNAM, sizeof(struct data_str));

    tick_period = nano2count(TICK_PERIOD);

    rt_task_make_periodic(&my_task, rt_get_time() + tick_period, tick_period);

    return 0;

}

void cleanup_module(void)

{

    rt_task_delete(&my_task);

    rtai_kfree(SHMNAM);

    return;

}
