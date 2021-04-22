/* Esempio di esecuzione di NTASKS con scheduling Rate Monotinic */

#include <linux/module.h>

#include <asm/io.h>

#include <asm/rtai.h>
#include <rtai_sched.h>

#define TICK_PERIOD 10000000

#define STACK_SIZE 2000

#define LOOPS 1000000000

#define NTASKS 8

static RT_TASK thread[NTASKS];

static RTIME tick_period;

static int cpu_used;

static void fun(long t)
{
	unsigned int loops = LOOPS;
	while(loops--) {
		cpu_used++;
		rt_printk("TASK %d %d\n", t, thread[t].priority);
		if (t == NTASKS-1) 
			rt_printk("\n\n"); 
		rt_task_wait_period();
	}
}


int init_module(void)
{
	RTIME now;
	int i;

	for (i = 0; i < NTASKS; i++) {
		rt_task_init_cpuid(&thread[i], fun, i, STACK_SIZE, NTASKS - i - 1, 0, 0, 0);
	}
	tick_period = nano2count(TICK_PERIOD);
	
	now = rt_get_time() + NTASKS*tick_period;
	for (i = 0; i < NTASKS; i++) {
		rt_task_make_periodic(&thread[i], now, (i+1)*tick_period);
	}
	// ordinamento secondo Rate Monotonic
	rt_spv_RMS(0);
	return 0;
}


void cleanup_module(void)
{
	int i;

	for (i = 0; i < NTASKS; i++) {
		rt_task_delete(&thread[i]);
	}
	printk("\n\nCPU USE SUMMARY: %d\n",cpu_used);
	
}
