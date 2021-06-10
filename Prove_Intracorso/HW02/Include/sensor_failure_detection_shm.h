//
// Created by Davide Riemma on 10/06/21.
//

#ifndef PSSTR_SENSOR_FAILURE_DETECTION_SHM_H
#define PSSTR_SENSOR_FAILURE_DETECTION_SHM_H

#include <rtai_sem.h>

#define TBS_STOP_TO_VOTERS_SHM 280510
#define TBS_STOP_TO_VOTERS_SEM 280511

struct tbs_stop_to_voters_type
{
    int stop_altitude;
    int stop_speed;
    int stop_temperature
    SEM * tbs_stop_to_voters_sem;
};

#endif //PSSTR_SENSOR_FAILURE_DETECTION_MBX_H
