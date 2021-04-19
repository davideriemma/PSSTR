/*
 * Copyright (C) 1999 Paolo Mantegazza <mantegazza@aero.polimi.it>
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License as
 * published by the Free Software Foundation; either version 2 of the
 * License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA 02111-1307, USA.
 */

#include <linux/kernel.h>
#include <linux/module.h>
#include <linux/init.h>

#include <rtai.h>
#include <rtai_sched.h>

MODULE_LICENSE("GPL");

#define STACKSIZE 4096

#define SLOWMUL  25

#define USEDFRAC 2

#define TICK_TIME 10000000

static RT_TASK Slow_Task, Fast_Task;

static int slowon;

static void Slow_Thread(long dummy)
{
        while (1) {  
                rt_busy_sleep((SLOWMUL*TICK_TIME)/USEDFRAC);
		rt_printk("SLOW THREAD SLEEPS.\n"); 
		slowon = 0;
                rt_task_wait_period();
		rt_printk("SLOW THREAD RESUMES.\n"); 
		slowon = 1;
        }
}                                        

static void Fast_Thread(long dummy) 
{                             
        while (1) {
		if (slowon) rt_printk("     FAST THREAD SLEEPS.\n"); 
                rt_task_wait_period();                                        
		if (slowon) rt_printk("     FAST THREAD RESUMES.\n"); 
        }                      
}

static int __preempt_init(void)
{
	RTIME start;
	int period = nano2count(TICK_TIME);

	rt_task_init_cpuid(&Fast_Task, Fast_Thread, 0, STACKSIZE, 1, 0, 0, 0);
	rt_task_init_cpuid(&Slow_Task, Slow_Thread, 0, STACKSIZE, 2, 0, 0, 0);
	start = rt_get_time() + SLOWMUL*period;
	rt_task_make_periodic(&Fast_Task, start, period);
	rt_task_make_periodic(&Slow_Task, start, SLOWMUL*period);

	return 0;
}

static void __preempt_exit(void)
{
	rt_task_delete(&Slow_Task);
	rt_task_delete(&Fast_Task);

	return;
}

module_init(__preempt_init);
module_exit(__preempt_exit);
