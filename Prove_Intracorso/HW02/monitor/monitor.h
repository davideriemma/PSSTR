//
// Created by Davide Riemma on 22/05/21.
//

#ifndef PSSRT_MONITOR_H
#define PSSRT_MONITOR_H

#define MONITOR_NUM_TASKS 3
#define MONITOR_ALTITUDE_TASK 0
#define MONITOR_SPEED_TASK 1
#define MONITOR_TEMPERATURE_TASK 2

_Noreturn void * TBS_func(void *); //funzione che implementa il Total Bandwidth Server;
void monitor_alt_taks(long); //task aperiodico per l'altitudine
void monitor_speed_task(long); //task aperiodico per la velocità
void monitor_temp_task(long); //task aperiodico per la temperatura
void signal_handler(int);

#endif //PSSRT_MONITOR_H
