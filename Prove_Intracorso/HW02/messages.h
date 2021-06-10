//
// Created by Davide Riemma on 22/05/21.
//

#ifndef PSSRT_MESSAGES_H
#define PSSRT_MESSAGES_H

#define DEBUG

#define TBS_MBX_NAME 100105
#define TBS_BROKEN_MBX_NAME 280510 //delete

#define TBS_TEMP_ID 2
#define TBS_ALT_ID 0
#define TBS_SPEED_ID 1

struct tbs_mbx_message
{
    int task_id;
};

struct sensor_is_broken
{
    int task_id;
    int has_broken;
};

#endif //PSSRT_MESSAGES_H
