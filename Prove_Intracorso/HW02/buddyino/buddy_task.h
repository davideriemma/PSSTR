//
// Created by Davide Riemma on 22/05/21.
//

#ifndef PSSRT_BUDDY_TASK_H
#define PSSRT_BUDDY_TASK_H

#include "../include/sensor_data_shared_memory.h"

struct message_to_buddy_type
{
    struct raw_sensors_data raw_data;
    struct processed_sensors_data proc_data;
};

#endif //PSSRT_BUDDY_TASK_H
