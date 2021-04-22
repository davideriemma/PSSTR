#ifndef _RT_TASK_H_
#define _RT_TASK_H_

#define TSK_WA_SIZE 8UL /*8 bytes*/
#define TSK_PRIO 1UL /*1 less thant the highest priority*/
#define TSK_TICK 1000000UL  /*1 millisecond*/
#define SHM_ID 1824UL /*shared memory id*/

typedef unsigned int shmem_t;

#endif