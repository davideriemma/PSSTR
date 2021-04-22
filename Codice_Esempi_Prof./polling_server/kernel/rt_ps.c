#include <linux/module.h>
#include <asm/io.h>
#include <asm/rtai.h>
#include <rtai_shm.h>
#include <rtai_mbx.h>
#include <rtai_sem.h>
#include <rtai_sched.h>

#include "headers.h"

static RT_TASK tasks[NTASKS];
static TaskInfo infos[NTASKS];

static RT_TASK psTask;
static TaskInfo psInfo;

static RTIME startTime,period;

static SharedMemory* shared;
static SEM* mutex;

static MBX* mbx;

//period_ns is 1ms (1^6 ns)
void initInfos(void){
	int i;
	for(i=0;i<5;i++){
		infos[i].Cs = 4*PERIOD_NS;
		infos[i].Ts = nano2count(100*PERIOD_NS);
	}

	for(i=5;i<9;i++){
		infos[i].Cs = 2*PERIOD_NS;
		infos[i].Ts = nano2count(50*PERIOD_NS);
	}

	for(i=9;i<13;i++){
		infos[i].Cs = 1*PERIOD_NS;
		infos[i].Ts = nano2count(30*PERIOD_NS);
	}

	for(i=13;i<15;i++){
		infos[i].Cs = 2*PERIOD_NS;
		infos[i].Ts = nano2count(40*PERIOD_NS);
	}

	//0.59 + x <= 0.70
	//x <= 0.10
	//Cs=1 T=10

	psInfo.Cs = 1*PERIOD_NS; 
	psInfo.Ts = nano2count(10*PERIOD_NS);

}

void aperiodic_fun(void){
	int msg = 1;
	rt_mbx_send(mbx,&msg,sizeof(int));
	printk("serving aperiodic request\n");
	rt_busy_sleep(psInfo.Cs);
}

void ps(long t){
	while(1){
		rt_sem_wait(mutex); //waiting on mutex
		//critical section
		if(shared->req == 1){
			aperiodic_fun();
			shared->req = 0;
			shared->res = 1;
		}
		//end of critical section
		rt_sem_signal(mutex); //awaking any waiting task

		
		rt_task_wait_period();
	}

}

void periodic_fun(long t){
	while(1){
		//printk("Task %ld executing\n",t);
		rt_busy_sleep(infos[t].Cs);
		rt_task_wait_period();
	}
}


int init_module(void){
	int i;
	
	printk("RT kernel module started!\n");
	initInfos();

	shared = (SharedMemory*)rtai_kmalloc(SHARED_ID,sizeof(SharedMemory));
	shared->req = 0;
	shared->res = 0;

	if(!shared){
		printk("Shared memory segment creation failed!\n");
		return -1;
	}

	mutex = rt_typed_named_sem_init(MUTEX_ID,1,BIN_SEM | FIFO_Q);
	mbx = rt_typed_named_mbx_init(MAILBOX_ID,MAILBOX_SIZE,FIFO_Q);

	period = nano2count(PERIOD_NS);

	for(i=0;i<NTASKS;i++){
		rt_task_init_cpuid(&tasks[i],periodic_fun,i,STACK_SIZE,1,0,0,0x00); //prio 1 ma verra' modificata da rm _cpuid
	}
	rt_task_init_cpuid(&psTask,ps,(i+1),STACK_SIZE,1,0,0,0x00);

	startTime = rt_get_time() + (NTASKS+1) *period;

	for(i=0;i<NTASKS;i++){
		rt_task_make_periodic(&tasks[i],startTime, infos[i].Ts);
	}

	rt_task_make_periodic(&psTask,startTime,psInfo.Ts);
	
	rt_spv_RMS(0x00);

	return 0;
}

void cleanup_module(){
	int i;

	printk("End of RT kernel module\n");
	stop_rt_timer();
	for(i=0;i<NTASKS;i++)
		rt_task_delete(&tasks[i]);

	rt_task_delete(&psTask);

	rt_named_sem_delete(mutex);

	rt_named_mbx_delete(mbx);

	if(shared){
		rtai_kfree(SHARED_ID);
	}
}



