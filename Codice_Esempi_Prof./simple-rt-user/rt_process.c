//------------------- RT_PROCESS.C ---------------------- 

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/mman.h>
#include <rtai_lxrt.h>
#include <rtai_shm.h>
#include <sys/io.h>
#include <signal.h>
#include "parameters.h"


static RT_TASK *main_Task;
static RT_TASK *loop_Task;
static int keep_on_running = 1;

static pthread_t main_thread;
static RTIME expected;
static RTIME sampl_interv;

static struct data_str *data;

static void endme(int dummy) {keep_on_running = 0;}

static void *main_loop() {
	
	if (!(loop_Task = rt_task_init_schmod(nam2num("RTAI01"), 2, 0, 0, SCHED_FIFO, 0))) {
		printf("CANNOT INIT PERIODIC TASK\n");
		exit(1);
	}
	expected = rt_get_time() + 100*sampl_interv;
	rt_task_make_periodic(loop_Task, expected, sampl_interv);
	rt_make_hard_real_time();

	unsigned int count = 0;
	int val = 0;
	while (keep_on_running)
	{
		//printf("ciclo n. %d\n",count);
		data->indx_counter = count;
       		if ((count%10)==0) val = (++val)%2;
        	data->value = val;
		count++;
		rt_task_wait_period();
	}
	rt_task_delete(loop_Task);
	return 0;
}

int main(void)
{
	printf("Simple RT Task (user mode) STARTED\n");
 	signal(SIGQUIT, endme);

	if (!(main_Task = rt_task_init_schmod(nam2num("MNTSK"), 0, 0, 0, SCHED_FIFO, 0))) {
		printf("CANNOT INIT MAIN TASK\n");
		exit(1);
	}

	data = rt_shm_alloc(SHMNAM, sizeof(struct data_str), USE_VMALLOC);

	sampl_interv = nano2count(TICK_TIME);
	pthread_create(&main_thread, NULL, main_loop, NULL);
	while (keep_on_running) {
		sampl_interv = sampl_interv; //do nothing!
		rt_sleep(nano2count(100000000));
	}

    	stop_rt_timer();
	rt_shm_free(SHMNAM);
	rt_task_delete(main_Task);
 	printf("Simple RT Task (user mode) STOPPED\n");
	return 0;
}




