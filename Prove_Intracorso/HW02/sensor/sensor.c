//------------------- SENSOR.C ---------------------- 

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/mman.h>
#include <rtai_lxrt.h>
#include <rtai_shm.h>
#include <rtai_sem.h> //inclusione dell'header per l'utilizzo delle primitive semaforiche
#include <sys/io.h>
#include <signal.h>
#include "parameters.h"

#define CPUMAP 0x00 //modifica della cpu per far eseguire tutti sulla cpu 0

#define NUM_TASKS

//emulates the plant to be controlled

static RT_TASK *main_Task;
static RT_TASK *temp_Task;
static RT_TASK *alt_Task;
static RT_TASK *speed_Task;
static int keep_on_running = 1;

static pthread_t temp;
static pthread_t alt;
static pthread_t speed;
static RTIME expected;
static RTIME base_period;

static void endme(int dummy) {keep_on_running = 0;}

struct raw_sensors_data* raw_sensor;
struct processed_sensors_data* proc_sensor;

static void * temp_loop(void * par) {
	
	if (!(temp_Task = rt_task_init_schmod(nam2num("TEMP"), 22, 0, 0, SCHED_FIFO, CPUMAP))) {
		printf("CANNOT INIT PERIODIC TASK\n");
		exit(1);
	}

	unsigned int iseed = (unsigned int)rt_get_time();
  	srand (iseed);

	expected = rt_get_time() + base_period;
	rt_task_make_periodic(temp_Task, expected, 4*base_period);
	rt_make_hard_real_time();

	int value = -50;
	int count = 1;
	int broken_sensor = (int)(((float)TEMP_SIZE) * rand() / ( RAND_MAX + 1.0 ));
	int when_to_break = 1 + (int)( 10.0 * rand() / ( RAND_MAX + 1.0 ));
	while (keep_on_running)
	{
		value += -1 + (int)(3.0 * rand() / ( RAND_MAX + 1.0 ));
		int i;
		/*acquire lock on raw data
		 * PLEASE, NOTE that, even if we acquire no lock on processed data (i.e. we use only one semaphore), we cause no
		 * deadlock, as the raw data semaphore is the first to be locked in any case, so the order is always respected.*/
        rt_sem_wait(raw_sensor->raw_sensor_data_lock);
		for (i=0; i<TEMP_SIZE; i++) raw_sensor->temperatures[i]=value;


		if (count%when_to_break==0) {
			raw_sensor->temperatures[broken_sensor]=0; // random stuck at zero...
			when_to_break = 1 + (int)( 10.0 * rand() / ( RAND_MAX + 1.0 ));
			count = when_to_break;
		}
        /*all done, release the lock on the structure*/
        rt_sem_signal(raw_sensor->raw_sensor_data_lock);
		count++;
		rt_task_wait_period();
	}
	rt_task_delete(temp_Task);
	return 0;
}

static void * alt_loop(void * par) {
	
	if (!(alt_Task = rt_task_init_schmod(nam2num("ALT"), 20, 0, 0, SCHED_FIFO, CPUMAP))) {
		printf("CANNOT INIT PERIODIC TASK\n");
		exit(1);
	}

	unsigned int iseed = (unsigned int)rt_get_time();
  	srand (iseed);

	expected = rt_get_time() + base_period;
	rt_task_make_periodic(alt_Task, expected, base_period);
	rt_make_hard_real_time();

	int value = 11000;
	int count = 1;
	int broken_sensor_1 = (int)(((float)ALTITUDE_SIZE) * rand() / ( RAND_MAX + 1.0 ));
	int broken_sensor_2 = (int)(((float)ALTITUDE_SIZE) * rand() / ( RAND_MAX + 1.0 ));
	int when_to_break = 1 + (int)( 10.0 * rand() / ( RAND_MAX + 1.0 ));
	while (keep_on_running)
	{
		value += -1 + (int)(3.0 * rand() / ( RAND_MAX + 1.0 ));
		int i;
		/*lock the resource, see temp_loop for details*/
        rt_sem_wait(raw_sensor->raw_sensor_data_lock);
		for (i=0; i<ALTITUDE_SIZE; i++) raw_sensor->altitudes[i]=value;


		if (count%when_to_break==0) {
			raw_sensor->altitudes[broken_sensor_1]
				+= -3 + (int)(7.0 * rand() / ( RAND_MAX + 1.0 )); // random value failure
			raw_sensor->altitudes[broken_sensor_2] = 0; // random stuck at zero
			when_to_break = 1 + (int)( 10.0 * rand() / ( RAND_MAX + 1.0 ));
			count = when_to_break;
		}
        /*all done, release the lock on the structure*/
        rt_sem_signal(raw_sensor->raw_sensor_data_lock);
		count++;
		rt_task_wait_period();
	}
	rt_task_delete(alt_Task);
	return 0;
}

static void * speed_loop(void * par) {
	
	if (!(speed_Task = rt_task_init_schmod(nam2num("SPEED"), 21, 0, 0, SCHED_FIFO, CPUMAP))) {
		printf("CANNOT INIT PERIODIC TASK\n");
		exit(1);
	}

	unsigned int iseed = (unsigned int)rt_get_time();
  	srand (iseed);

	expected = rt_get_time() + base_period;
	rt_task_make_periodic(speed_Task, expected, 2*base_period);
	rt_make_hard_real_time();

	int value = 800;
	int count = 1;
	int broken_sensor = (int)(((float)SPEED_SIZE) * rand() / ( RAND_MAX + 1.0 ));
	int when_to_break = 1 + (int)( 10.0 * rand() / ( RAND_MAX + 1.0 ));
	while (keep_on_running)
	{
		value += -2 + (int)(5.0 * rand() / ( RAND_MAX + 1.0 ));
		int i;
        /*lock the resource, see temp_loop for details*/
        rt_sem_wait(raw_sensor->raw_sensor_data_lock);
		for (i=0; i<SPEED_SIZE; i++) raw_sensor->speeds[i]=value;


		if (count%when_to_break==0) {
			raw_sensor->speeds[broken_sensor]
				+= -2 + (int)(5.0 * rand() / ( RAND_MAX + 1.0 )); // random value failure
			when_to_break = 1 + (int)( 10.0 * rand() / ( RAND_MAX + 1.0 ));
			count = when_to_break;
		}
        /*all done, release the lock on the structure*/
        rt_sem_signal(raw_sensor->raw_sensor_data_lock);
		count++;
		rt_task_wait_period();
	}
	rt_task_delete(speed_Task);
	return 0;
}

int main(void)
{
	printf("Sensor STARTED!\n");
 	signal(SIGINT, endme);

	if (!(main_Task = rt_task_init_schmod(nam2num("MNTSK"), 0, 0, 0, SCHED_FIFO, 0xF))) {
		printf("CANNOT INIT MAIN TASK\n");
		exit(1);
	}

	//attach to data shared with the voter
	raw_sensor = rtai_malloc(RAW_SEN_SHM, sizeof(struct raw_sensors_data));
    /*acquire pointer to semaphores, as the two versions of the structure are unrelated, though they are the same thing*/
    raw_sensor->raw_sensor_data_lock = rt_typed_named_sem_init(RAW_SEN_SEM, 1, RES_SEM); //initialize as a priority inheritance semaphore
	proc_sensor = rtai_malloc(PROC_SEN_SHM, sizeof(struct processed_sensors_data));
    /*acquire pointer to semaphores, as the two versions of the structure are unrelated, though they are the same thing*/
    proc_sensor->processed_sensor_data_lock = rt_typed_named_sem_init(PROC_SEN_SEM, 1, RES_SEM); //initialize as a priority inheritance semaphore

	base_period = nano2count(TICK_TIME);
	pthread_create(&temp, NULL, temp_loop, NULL);
	pthread_create(&alt, NULL, alt_loop, NULL);
	pthread_create(&speed, NULL, speed_loop, NULL);
	int i;
	while (keep_on_running) {
	    /*acquire pointer on the two shared memories (again, printing the data is a logical atomic action, so it's better to
	    * perform it in a single wait/signal cycle)*/

        rt_sem_wait(raw_sensor->raw_sensor_data_lock);
        rt_sem_wait(proc_sensor->processed_sensor_data_lock);

		printf("Raw Temp:\t");
		for (i=0; i<TEMP_SIZE-1; i++)
			printf(" %d,",raw_sensor->temperatures[i]);
		printf(" %d\n",	raw_sensor->temperatures[TEMP_SIZE-1]);

		printf("Raw Speed:\t");
		for (i=0; i<SPEED_SIZE-1; i++)
			printf(" %d,",raw_sensor->speeds[i]);
		printf(" %d\n",	raw_sensor->speeds[SPEED_SIZE-1]);

		printf("Raw Alt:\t");
		for (i=0; i<ALTITUDE_SIZE-1; i++)
			printf(" %d,",raw_sensor->altitudes[i]);
		printf(" %d\n",	raw_sensor->altitudes[ALTITUDE_SIZE-1]);
		
		printf("    Temp:\t %d\n",proc_sensor->temperature);
		printf("    Speed:\t %d\n",proc_sensor->speed);
		printf("    Alt:\t %d\n",proc_sensor->altitude);

		/*now release the semaphore in inverse order*/
        rt_sem_signal(proc_sensor->processed_sensor_data_lock);
        rt_sem_signal(raw_sensor->raw_sensor_data_lock);

		rt_sleep(250000000);
	}

	/*release (detach) semaphores*/
    rt_sem_delete(raw_sensor->raw_sensor_data_lock);
    rt_sem_delete(proc_sensor->processed_sensor_data_lock);

    /*now delete everything else*/
	rt_shm_free(RAW_SEN_SHM);
	rt_shm_free(PROC_SEN_SHM);
	rt_task_delete(main_Task);
 	printf("Sensor STOPPED\n");
	return 0;
}




