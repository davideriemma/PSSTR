//
// Created by Davide Riemma on 10/06/21.
//

#ifndef PSSTR_SENSOR_DATA_SHARED_MEMORY_H
#define PSSTR_SENSOR_DATA_SHARED_MEMORY_H

#include <rtai_sem.h>

#define ALTITUDE_SIZE 5
#define SPEED_SIZE 3
#define TEMP_SIZE 3

#define RAW_SEN_SHM 121111
#define PROC_SEN_SHM 121112

#define RAW_SEN_SEM "240698"
#define PROC_SEN_SEM "100798"


struct raw_sensors_data
{
    unsigned int altitudes[ALTITUDE_SIZE]; 	//updated every 250ms
    unsigned int speeds[SPEED_SIZE];		//updated every 500ms
    int temperatures[TEMP_SIZE];		//updated every	second
    SEM * raw_sensor_data_lock; //binary semaphore needed for proper access to resources
};

struct processed_sensors_data
{
    unsigned int altitude;
    unsigned int speed;
    int temperature;
    SEM * processed_sensor_data_lock; //binary semaphore needed for proper access to resources
};

#endif //PSSTR_SENSOR_DATA_SHARED_MEMORY_H
