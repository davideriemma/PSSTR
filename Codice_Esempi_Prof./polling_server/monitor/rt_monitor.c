#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/mman.h>
#include <rtai_lxrt.h>
#include <rtai_mbx.h>
#include <rtai_shm.h>
#include <rtai_sem.h>
#include <signal.h>
#include <sys/io.h>
#include "headers.h"

static RT_TASK* asyncTask;
static MBX* mbx;

static int start;

void sig_handler(int signum){
	start = 0;
}

int main(void){
	int data;
	signal(SIGINT, sig_handler);

	if(!(asyncTask = rt_task_init(nam2num("RT_MONITOR"),1,STACK_SIZE,0))){
		printf("failed creating rt task\n");
		exit(-1);
	}

	start = 1;
	mbx = rt_typed_named_mbx_init(MAILBOX_ID,MAILBOX_SIZE,FIFO_Q);
	while(start){
		if(!rt_mbx_receive(mbx,&data,sizeof(int)))
			printf("polling server processed an aperiodic request\n");
		else
			printf("failed retrieving message from mailbox\n");

	}

	rt_named_mbx_delete(mbx);
	rt_task_delete(asyncTask);

	return 0;
}
