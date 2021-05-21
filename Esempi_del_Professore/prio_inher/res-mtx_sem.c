/*
COPYRIGHT (C) 2001  Paolo Mantegazza (mantegazza@aero.polimi.it)

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

/* An adaption of the test res-mtx_sem, by Marcello Cinque */

#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/version.h>
#include <linux/errno.h>

#include <asm/rtai.h>
#include <rtai_sched.h>
#include <rtai_sem.h>

#define RT_STACK 2000

#define DEFAULT_TICK_PERIOD 1000000

#define T_BUFFER_SIZE 256

#define NUM_TASKS 10

#define TID 1000

#define RT_PRINTK rt_printk

struct op_buffer {
	int t_buffer_data[T_BUFFER_SIZE];
	long t_buffer_tid[T_BUFFER_SIZE];
	int t_buffer_pos;
	SEM lock;
};

static struct op_buffer test_op;

static RT_TASK control_task;

void task_func(long tid)
{
	RT_TASK *task;
	task = rt_whoami();
	if (tid == 1005) { // task 1005 does a busy sleep to test a priority inversion
		
		RT_PRINTK("task: %d, priority: %d BUSY SLEEPING\n", tid, task->priority);
		rt_busy_sleep(1000000);
	
	} else {
		
		int i;

		
	  	RT_PRINTK("task: %d, priority: %d\n", tid, task->priority);
		RT_PRINTK("task: %d, about to lock mutex...\n", tid);

		rt_sem_wait(&test_op.lock);

		RT_PRINTK("task: %d, mutex locked!\n", tid);
	
		for (i = 0; i < 5; i++) {
			RT_PRINTK("task: %d, loop count %d\n", tid, i);
			test_op.t_buffer_data[test_op.t_buffer_pos] = tid + i;
			test_op.t_buffer_tid[test_op.t_buffer_pos] = tid;
			test_op.t_buffer_pos++;
		}
	  	
		RT_PRINTK("task: %d, about to unlock mutex.\n", tid);
		rt_sem_signal(&test_op.lock);
		if (tid == 1010) RT_PRINTK("Control task: priority back to %d.\n", control_task.priority);
	}
	rt_task_suspend(task);
}

static RT_TASK tasks[NUM_TASKS];

void control_func(long tid)
{
	int i;
	RT_TASK *task;

	task = rt_whoami();
	RT_PRINTK("task: %d, priority: %d\n", tid, task->priority);

	//rt_typed_sem_init(&test_op.lock, 1, BIN_SEM | FIFO_Q);
	//rt_typed_sem_init(&test_op.lock, 1, BIN_SEM | PRIO_Q);
	rt_typed_sem_init(&test_op.lock, 1, RES_SEM); 

	test_op.t_buffer_pos = 0;
	for (i = 0; i < T_BUFFER_SIZE; i++) {
		test_op.t_buffer_data[i] = 0;
		test_op.t_buffer_tid[i] = 0;
	}
	for (i = 0; i < NUM_TASKS; i++) {
		if (rt_task_init(&tasks[i], task_func, TID + i + 1, RT_STACK, NUM_TASKS - i, 0, 0)) {
			RT_PRINTK("resource semaphore priority inheritance test - Application task creation failed\n");
		}
	}
	
	RT_PRINTK("task: %d, about to lock mutex...\n", tid);

	rt_sem_wait(&test_op.lock);

	RT_PRINTK("task: %d, mutex locked!\n", tid);
	
	for (i = 0; i < NUM_TASKS; i++) {
		if ((TID + i + 1) != 1005) { // task 1005 is delayed (to have a priority inversion)
			rt_task_resume(&tasks[i]);
			RT_PRINTK("task: %d, priority raised to: %d\n", tid, task->priority);
		}
	}
	// resume task 1005
	rt_task_resume(&tasks[4]);
	
	RT_PRINTK("task: %d, about to unlock mutex.\n", tid);
	rt_sem_signal(&test_op.lock);
	RT_PRINTK("task: %d, priority should now be back to original.\n", tid);
	RT_PRINTK("task: %d, priority back to %d.\n", tid, task->priority);
	rt_task_suspend(task);
}



int init_module(void)
{
	printk("resource semaphores priority inheritance test program.\n");
// Create a control pthread.
	if (rt_task_init(&control_task, control_func, TID, RT_STACK, 1000, 0, 0)) {
		printk("resource semaphores priority inheritance test - Control task creation failed\n");
		return -1;
	}
	rt_task_resume(&control_task);
	return 0;
}

void cleanup_module(void)
{
	int i;
	printk("resource semaphore test - Contents of test_op buffer:\n");
	for (i = 0; i < test_op.t_buffer_pos; i++) {
		printk("Position: %d, Value: %d, Task id: %ld\n", i + 1, test_op.t_buffer_data[i], test_op.t_buffer_tid[i]);
	}
	rt_task_delete(&control_task);
	for (i = 0; i < NUM_TASKS; i++) {
		rt_task_delete(&tasks[i]);
	}
	rt_sem_delete(&test_op.lock);
	printk("resource semaphores test module removed.\n");
}
