#ifndef HEADERS_H
#define HEADERS_H

#define PERIOD_NS 1000000
#define STACK_SIZE 2048

#define NTASKS 15

#define SHARED_ID 12345
#define MUTEX_ID "myMutex"

#define MAILBOX_ID "MyMailbox"
#define MAILBOX_SIZE 256

typedef struct{
	long Cs;
	long Ts;
}TaskInfo;

typedef struct{
	int req;
	int res;
}SharedMemory;

#endif
