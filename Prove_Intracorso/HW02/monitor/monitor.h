//
// Created by Davide Riemma on 22/05/21.
//

/*
 * error detection tasks priorities*/
#define TBS_TEMP_PRIO 2
#define TBS_ALT_PRIO 0
#define TBS_SPEED_PRIO 1

/* default stack size */
#define TBS_DEFAULT_STACK_SIZE 2000

#ifndef PSSRT_MONITOR_H
#define PSSRT_MONITOR_H

#define ALTITUDE_SIZE 5
#define SPEED_SIZE 3
#define TEMP_SIZE 3

#define RAW_SEN_SEM "240698"
#define PROC_SEN_SEM "100798"

#define TBS_NUM_TASKS 3
#define TBS_ALTITUDE_TASK 0
#define TBS_SPEED_TASK 1
#define TBS_TEMPERATURE_TASK 2

#define MONITOR_ALTITUDE_TASK_PRIO 1
#define MONITOR_SPEED_TASK_PRIO 2
#define MONITOR_TEMPERATURE_TASK_PRIO 3

#define RAW_SEN_SHM 121111
#define PROC_SEN_SHM 121112

#define RAW_SEN_SEM "240698"
#define PROC_SEN_SEM "100798"

#define MONITOR_ALTITUDE_N_OF_CALLS 10
#define MONITOR_SPEED_N_OF_CALLS 10
#define MONITOR_TEMP_N_OF_CALLS 20

#define now() rt_get_time()

struct raw_sensors_data
{
    unsigned int altitudes[ALTITUDE_SIZE]; 	//uptated every 250ms
    unsigned int speeds[SPEED_SIZE];		//updated every 500ms
    int temperatures[TEMP_SIZE];		//uptaded every	second
    SEM * raw_sensor_data_lock; //binary semaphore needed for proper access to resources
};

struct processed_sensors_data
{
    unsigned int altitude;
    unsigned int speed;
    int temperature;
    SEM * processed_sensor_data_lock; //binary semaphore needed for proper access to resources
};

_Noreturn void monitor_alt_task(double deadline); //task aperiodico per l'altitudine
void monitor_speed_task(double deadline); //task aperiodico per la velocità
void monitor_temp_task(double deadline); //task aperiodico per la temperatura
void signal_handler(int);

#endif //PSSRT_MONITOR_H
