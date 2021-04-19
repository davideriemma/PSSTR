/*
Marcello Cinque - 2018

Corso di Progetto e Sviluppo di Sistemi in Tempo Reale

A simple example of aperiodic task scheduling with EDF
*/
#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/version.h>
#include <linux/errno.h>

#include <asm/rtai.h>
#include <rtai_sched.h>

#define RT_STACK 2000

#define NUM_CYCLES 50

#define DEFAULT_TICK_PERIOD 100000

#define NUM_TASKS 2

RTIME now;

void ape_task (long tid)
{	

	RT_TASK* task = rt_whoami();

	// Setting the deadline to 61ms for T1 and to 101ms for T0
	rt_task_set_resume_end_times(now,now+((NUM_TASKS-tid)*NUM_CYCLES*nano2count(1000000)+nano2count(1000000)*(1+tid*10)));

	// T0 sleeps 1ms, T1 sleeps 11ms
	rt_sleep(nano2count(1000000)*(1+tid*10));

	int i;
	for (i = 0; i < NUM_CYCLES; i++) {
		rt_printk("task %d executing, elapsed %lu\n", tid, count2nano(rt_get_time()-now));
		rt_busy_sleep(1000000);
	}

	rt_task_suspend(task);
}

static RT_TASK tasks[NUM_TASKS];



int init_module(void)
{
	printk("aperiodic tasks with EDF test program.\n");
	int i;
	for (i = 0; i < NUM_TASKS; i++) {
		if (rt_task_init_cpuid(&tasks[i], ape_task, i, RT_STACK, i, 0, 0, 0)) {
			rt_printk("task creation failed\n");
		}
	}

	now = rt_get_time();
	
	for (i = 0; i < NUM_TASKS; i++) {
		rt_task_resume(&tasks[i]);
	}
	return 0;
}

void cleanup_module(void)
{
	int i;
	for (i = 0; i < NUM_TASKS; i++) {
		rt_task_delete(&tasks[i]);
	}
	printk("aperiodic task test module removed.\n");
}
