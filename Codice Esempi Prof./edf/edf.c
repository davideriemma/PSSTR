/*
COPYRIGHT (C) 1999  Paolo Mantegazza (mantegazza@aero.polimi.it)

This library is free software; you can redistribute it and/or
modify it under the terms of the GNU Lesser General Public
License as published by the Free Software Foundation; either
version 2 of the License, or (at your option) any later version.

This library is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
Lesser General Public License for more details.

You should have received a copy of the GNU Lesser General Public
License along with this library; if not, write to the Free Software
Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA.
*/


/* This example features NTASKS realtime tasks running periodically in EDF mode.
The task are given a priority ordered in such a way that low numbered tasks
have the lowest priority. However the tasks execute for a duration proportional 
to their number so that, under EDF, the lowest priority tasks run first.
So if they appear increasingly ordered on the screen EDF should be working.*/

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
		if (t == NTASKS-1) { 
			rt_printk("\n\n"); 
			rt_task_wait_period();
		} else {
			rt_task_set_resume_end_times(-NTASKS*tick_period, -(t + 1)*tick_period);
		}
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
		rt_task_make_periodic(&thread[NTASKS - i - 1], now, NTASKS*tick_period);
	}
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
