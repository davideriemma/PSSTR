//
// Created by Davide Riemma on 22/05/21.
//

/*TODO: buddy task*/

#include "buddy_task.h"
#include <stdio.h>
#include <rtai_mbx.h>
#include <rtai_xlrt.h>
#include <signal.h>

#include "../voter/parameters.h"

static MBX * raw_buddy_mbx, * proc_buddy_mbx;
static int continue_buddy = 1;

/*
 * variabile che permette l'individuazione del task buddy da parte dei task di error detection, contenente l'identificativo
 * rt del task.*/
RT_TASK Buddy_Task_task;

static void signal_hand(int);

_Noreturn int main()
{
    /*registra l'handler*/
    signal(SIGTERM, signal_hand);
    /*attach della mailbox*/
    raw_buddy_mbx = rt_typed_mbx_init(BUDDY_TASK_MBX_1, sizeof(struct raw_sensors_data), PRIO_Q);
    proc_buddy_mbx = rt_typed_mbx_init(BUDDY_TASK_MBX_2, sizeof(struct processed_sensors_data), PRIO_Q);
    /*do the buddy task job*/

    struct raw_sensors_data raw_data;
    struct processed_sensors_data proc_data;

    while(continue_buddy)
    {
        /*ricevi e stampa i dati*/
        rt_mbx_receive(raw_buddy_mbx, (void *) &raw_data, sizeof(struct raw_sensors_data));
        for (int i = 0; i < ALTITUDE_SIZE; ++i) {printf("[%d]\t", raw_data.altitudes[i]);}
        printf("\nNow Reading Raw Speeds: ");
        for (int i = 0; i < SPEED_SIZE; ++i) {printf("[%d]\t", raw_data.speeds[i]);}
        printf("\nNow Reading Raw Temperatures: ");
        for (int i = 0; i < TEMP_SIZE; ++i) {printf("[%d\t", raw_data.altitudes[i]);}
        rt_mbx_receive(proc_buddy_mbx, (void  *) &proc_data, sizeof(struct processed_sensors_data));
        printf("\nNow Reading Processed Altitudes: [%d]\n", proc_data.altitude);
        printf("Now Reading Processed Speed: [%d]\n", proc_data.speed);
        printf("Now Reading Proecessed Temperatures: [%d]\n", proc_data.temperature);

    }

    return 0;
}