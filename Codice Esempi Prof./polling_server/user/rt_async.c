#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/mman.h>
#include <rtai_lxrt.h>
#include <rtai_shm.h>
#include <rtai_sem.h>
#include <sys/io.h>
#include "headers.h"

static RT_TASK* asyncTask;
static SharedMemory* shared;
static SEM* mutex;

void async_request(void){
	int done = 0;

	rt_sem_wait(mutex);
	//start of critical section
	shared->req = 1;
	//end of critical section
	rt_sem_signal(mutex);

	do{
		sleep(SLEEP_TIME_S);
		rt_sem_wait(mutex);
		
		if(shared->res == 1){
			shared->res = 0;
			done = 1;
		}

		rt_sem_signal(mutex);

	}while(!done);

}


int main(void){
	int start;

	if(!(asyncTask = rt_task_init(nam2num("RT_ASYNC"),1,STACK_SIZE,0))){
		printf("failed creating rt task\n");
		exit(-1);
	}

	mutex = rt_typed_named_sem_init(MUTEX_ID,1,BIN_SEM | FIFO_Q);
	shared = (SharedMemory*)rtai_malloc(SHARED_ID,sizeof(SharedMemory));

	start = 1;
	do{
		printf("which will it be?\n");
		printf("0.exit\n");
		printf("1.start aperiodic request\n");
		printf(">");
		scanf("%d",&start);
		if(start){
			async_request();
		}else
			start = 0; //if you enter something which is not 1

	}while(start);

	rt_named_sem_delete(mutex);
	rtai_free(SHARED_ID,&shared);

	rt_task_delete(asyncTask);

	return 0;
}
