//
// Created by Davide Riemma on 22/05/21.
//

#include <pthread.h>
#include <stdio.h>
#include <stdlib.h> //for exit()
#include <signal.h> //for registering signal handlers
#include <rtai_lxrt.h>

#include "../messages.h"
#include "monitor.h" //defines e prototipi per il monitor

static pthread_t TBS, threads[MONITOR_NUM_TASKS];
static int exits = 0;
/*
 * il main si limita solo a creare il thread tbs, registrare la callback e stampare qualche messaggio di debug*/
int main()
{
    /*inizilizzazione del task TBS*/
#ifdef DEBUG
    puts("Registering SIGTERM handler...");
#endif
    signal(SIGTERM, signal_handler);
#ifdef DEBUG
    puts("Monitor has started, generating TBS thread...");
#endif
    pthread_create(&TBS, NULL, TBS_func, NULL);
    while(!exits){}
#ifdef DEBUG
    puts("Monitor exits...");
#endif
    return 0;
}

void signal_handler(int)
{
    /*TODO: deletes the mailboxes and sets the exit value to 1*/
    exits = 1;
    return;
}

_Noreturn void * TBS_func(void *)
{
    /*dichiarazione del task come real time*/
    if(!rt_task_init_schmod(nam2num("TBSTSK"), 0, 0, 0, SCHED_FIFO, 1))
    {
        puts("Unable to create TBS task, exiting...");
        exit(1);
    }
    else
    {
        /*dichiarazione dei tasks*/
        RT_TASK tasks[MONITOR_NUM_TASKS];
        char ** task_names[] = {"ALTTSK", "SPEEDTSK", "TEMPTSK"};
        for (int i = 0; i < MONITOR_NUM_TASKS; ++i)
        {
            if(!rt_task_init_schmod(nam2num(task_names[i]), i + 1, 0, 0, SCHED_FIFO, 1))
            {
                puts("Unable to initialize tasks, exiting...");
                exit(1);
            }
        }

        /*TODO: avviare finalmente il task come hard realtime*/
        rt_make_hard_real_time();

        while(1)
        {
            /*TODO: inizializzazione della coda dei messaggi*/
            /*TODO: attesa della ricezione di un messaggio*/
            /*TODO: attiva il task in base alla richiesta.*/
        }
    }

    return (void *) 0;
}