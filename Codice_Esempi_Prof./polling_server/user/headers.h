#ifndef HEADERS_H
#define HEADERS_H

#define STACK_SIZE 10000
#define SLEEP_TIME_S 1 

#define SHARED_ID 12345
#define MUTEX_ID "myMutex"

typedef struct{
	int req;
	int res;
}SharedMemory;

#endif
