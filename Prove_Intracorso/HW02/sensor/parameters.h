//---------------- PARAMETERS.H ----------------------- 

#include <rtai_sem.h> //needed to use semaphores

#define TICK_TIME 250000000

#define STACK_SIZE 10000

#define ALTITUDE_SIZE 5
#define SPEED_SIZE 3
#define TEMP_SIZE 3

#define RAW_SEN_SHM 121111
#define PROC_SEN_SHM 121112

#define RAW_SEN_SEM "240698"
#define PROC_SEN_SEM "100798"

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