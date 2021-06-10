//
// Created by Davide Riemma on 22/05/21.
//

#include <pthread.h>
#include <stdio.h>
#include <stdlib.h> //for exit()
#include <signal.h> //for registering signal handlers
#include <rtai_lxrt.h>
#include <rtai_sem.h> //for semaphoric primitives
#include <rtai_shm.h>
#include <rtai_mbx.h>
#include <rtai_sched.h>

/* contiene nome della mailbox che permette la comunicazione tra i Voter ed il TBS */
#include "../include/common_mailbox.h"
/* contiene informazioni necessarie all'esecuzione del TBS e dei tasks di error detections */
#include "monitor.h"

/*bandwidth del TBS*/
#define TBS_BANDWIDTH 0.5

static RT_TASK * tasks[MONITOR_NUM_TASKS], * TBS_task;
static int exits = 0;

static double last_deadline = 0; //deadline dell task precedente

/*
static struct raw_sensors_data * raw_data;
static struct processed_sensors_data * proc_data;
 */

static MBX * proc_data_buddy_mbx, * raw_data_buddy_mbx; //buddy printing task mailboxes

static MBX * sensor_is_broken_mbx;

/* ------- VARIABILI NECESSARIE ALL'UTILIZZO ED ALLA CONFIGURAZIONE DEI TASKS DI ERROR DETECTION ------ */
/*
 * variabili necessarie a rappresentare i tasks realtime di error detection*/
static RT_TASK tbs_error_detection_tasks[TBS_NUM_TASKS];

/*
 * variabili necessarie a rappresentare la priorità dei tasks*/
static int tbs_error_detection_tasks_priorities[] = {TBS_ALT_PRIO, TBS_SPEED_PRIO, TBS_TEMP_PRIO};

/*
 * variabili necessarie a rappresentare il codice delle funzioni dei tasks*/
void (*tbs_error_detection_tasks_text[])(double) = {monitor_alt_task, monitor_speed_task, monitor_temp_task};

/*
 * variabile che rappresenta l'id del task TBS, necessaria per poterlo correttamente rimuovere e terminare,
 * oltre che utilizzata per la ricezione dei messaggi da parte dei task di error detection*/
static RT_TASK TBS_task;

/*----------------------------------------------------------------------------------------------------*/

/*
 * il main coincide con il task TBS
 * */
int main()
{
    /*
     * questa variabile conterrà il puntatore alla mailbox tra i Voters ed il TBS*/
    static MBX * voter_to_tbs_mbx;

    /*
     * questa variabile conterrà il contenuto del messaggio ricevuto dal TBS*/
    static struct TBS_message_type message_to_tbs;

    /*
     * registrazione dell'handler */
    puts("Registering SIGTERM handler...");
    signal(SIGTERM, signal_handler);

    /* dicharazione del task TBS come hard realtime: configurazione */
    if(!rt_task_init_schmod(nam2num("TBSTSK"), 0, 0, 0, SCHED_FIFO, 1))
    {
        puts("Unable to create TBS as an Hard Real-time task, exiting...");
    }
    else
    {
        /*
         * acquisizione dell'identificativo del task TBS (necessaria per poterlo correttamente rimuovere e terminare) */
        TBS_task = rt_whoami();

        /* dicharazione del task TBS come hard realtime: attivazione */
        rt_make_hard_real_time();

        /*collegamento alla mailbox tra il i Voter tasks ed il TBS*/
        voter_to_tbs_mbx = rt_typed_mbx_init(TBS_MBX_NAME, sizeof(struct TBS_message_type), PRIO_Q);

        /*creazione dei task aperiodici, schedulati sulla cpu 1*/
        for (int i = 0; i < TBS_NUM_TASKS; ++i)
        {
            rt_task_init_cpuid(&tbs_error_detection_tasks[i], tbs_error_detection_tasks_text[i], 0 /*no arguments passed*/, TBS_DEFAULT_STACK_SIZE, tbs_error_detection_tasks_priorities[i], 0 /*no fpu usage*/, 0 /*nothing tu run on ctx switch*/, 1);
        }

        double d_k_prev = 0;

        /* ciclo infinito: ricezione dei messaggi da parte dei task voter. */
        while(1)
        {
            /* attesa della ricezione di un messaggio*/
            rt_mbx_receive(voter_to_tbs_mbx, (void *) &message_to_tbs, sizeof(struct TBS_message_type));

            /* attiva il task in base alla richiesta, tramite messaggio diretto*/
            rt_send(&tbs_error_detection_tasks[message_to_tbs.task_to_wake], 0 /*dummy value*/);
        }
    }

    puts("Monitor exits...");

    return 0;
}

void signal_handler(int)
{
    /*
     * eliminazione / detach della mailbox da voter*/
    rt_mbx_delete(TBS_MBX_NAME);

    /*
     * eliminazione delgi error detection tasks*/
    for (int i = 0; i < MONITOR_NUM_TASKS; ++i)
    {
        rt_task_delete(tbs_error_detection_tasks[i]);
    }

    /*
     * eliminazione del task TBS*/
    rt_task_delete(TBS_task);

    /*chiusura delle mailboxes del buddy task
     * TODO: sistemare*/
    rt_mbx_delete(raw_data_buddy_mbx);
    rt_mbx_delete(proc_data_buddy_mbx);

    exits = 1;
    return;
}

#include "../buddyino/buddy_task.h"
/*
 * necessaria per l'id della mailbox di invio segnale di stop ai voters*/
#include "../include/sensor_failure_detection_shm.h"

_Noreturn void monitor_alt_task(double deadline)
{
    /*
     * variabile che indica il numero di notifiche ricevute*/
    static number_of_calls = 0;

    /*
     * variabile dummy che contiene il messaggio ricevuto dal TBS*/
    unsigned int dummy;

    /*
     * variabile contentente l'id del buddy task*/
    extern RT_TASK Buddy_Task_task;

    /*
     * ACTIVITY: configurazione del task*/

    /*
     * attivazione del task come realtime*/
    rt_make_hard_real_time();
    /*
     * puntatori alle shared memory dei dati grezzi ed elaborati, rispettivamente */
    struct raw_sensors_data * raw_sensor_data_altitude;
    struct processed_sensors_data * processed_sensor_data_altitude;

    /*
     * copie locali dei dati condivisi da inviare al buddy*/
    struct message_to_buddy_type message_to_buddy;

    /*
     * puntatore alla shared memory per l'invio del segnale di terminazione ai task voter*/
    struct tbs_stop_to_voters_type * stop_voters;

    /*
     * collegamento alla memoria condivisa dei dati grezzi ed elaborati */
    raw_sensor_data_altitude = (struct raw_sensors_data *) rtai_malloc(RAW_SEN_SHM, sizeof(struct raw_sensors_data));
    processed_sensor_data_altitude = (struct processed_sensors_data *) rtai_malloc(PROC_SEN_SHM, sizeof(struct processed_sensors_data));

    /*
     * collegamento alla memoria condivisa per l'invio del segnale di terminazione ai task voter*/
    stop_voters = (struct tbs_stop_to_voters_type *) rtai_malloc(TBS_STOP_TO_VOTERS_SHM, sizeof(struct tbs_stop_to_voters_type));

    /*
     * attivazione dei semafor per l'accesso alla memoria condivisa per l'invio del segnale di terminazione ai task voter*/
    rt_typed_sem_init(stop_voters->tbs_stop_to_voters_sem, 1, RES_SEM);

    while (number_of_calls <= MONITOR_ALTITUDE_N_OF_CALLS)
    {

        /*
        * ACT: attesa per la ricezione del messaggio*/
        rt_receive(&TBS_task, &dummy);

        /*
         * ACT: schedulazione autonoma del task*/

        /*reset schedule for EDF*/
        deadline = last_deadline + TBS_BANDWIDTH;
        last_deadline = deadline; //assume that this is the last deadline. This value will be updated whenever a new
        //(last) task arrives, hence keeping the last deadline always updated.
        rt_task_set_resume_end_times(now(), deadline);

        /*
        * ACT: accesso alla shared memory e copia dei dati*/
        /*acquire lock on resources*/
        rt_sem_wait(raw_data->raw_sensor_data_lock);
        rt_sem_wait(proc_data->processed_sensor_data_lock);

        /*do the actual data printing*/

        /*
         * copia i dati nelle variabili locali*/
        message_to_buddy.proc_data.altitude = processed_sensor_data_altitude->altitude;
        message_to_buddy.proc_data.speed = processed_sensor_data_altitude->speed;
        message_to_buddy.proc_data.altitude = processed_sensor_data_altitude->altitude;

        /*
         * copia i valori grezzi dell'altitudine*/
        for (int i = 0; i < ALTITUDE_SIZE; ++i)
        {
            message_to_buddy.raw_data.altitudes[i] = raw_sensor_data_altitude->altitudes[i];
        }

        /*
         * copia i valori grezzi della temperatura*/
        for (int i = 0; i < TEMP_SIZE; ++i)
        {
            message_to_buddy.raw_data.temperatures[i] = raw_sensor_data_altitude->temperatures[i];
        }

        /*
         * copia i valori grezzi della velocità*/
        for (int i = 0; i < SPEED_SIZE; ++i)
        {
            message_to_buddy.raw_data.speeds[i] = raw_sensor_data_altitude->speeds[i];
        }

        /*release lock on resources*/
        rt_sem_signal(proc_data->processed_sensor_data_lock);
        rt_sem_signal(raw_data->raw_sensor_data_lock);

        /*il task invia una copia della struct al buddy task, che la stampa*/
        rt_send(&Buddy_Task_task, 0); /*bisogna trovare un modo per implementare l'invio tramite messaggio diretto, dato che
                                        rtai non supporta l'invio diretto di messaggi strutturati o puntatori.*/

        number_of_calls = number_of_calls++; //incrementa il numero di notifiche ricevute

        /*il processo si auto-sospende*/
        rt_task_suspend(rt_whoami());
    }

    /*invio del messaggio di terminazione*/
    rt_sem_wait(stop_voters->tbs_stop_to_voters_sem);
    stop_voters->stop_altitude = 1;
    rt_sem_signal(stop_voters->tbs_stop_to_voters_sem);

    return;
}

_Noreturn void monitor_speed_task(double deadline)
{
    static number_of_calls = 0; //numero di notifiche ricevute

    while (number_of_calls <= MONITOR_SPEED_N_OF_CALLS)
    {
        if (deadline == 0)
        {
            tasks[MONITOR_SPEED_TASK] = rt_whoami(); //inizializza e mette a disposizione il nome del task, in modo tale da poterlo far ripartire
            rt_task_init_schmod(nam2num("ALTTSK"), MONITOR_SPEED_TASK_PRIO, 0, 0, SCHED_FIFO, 1);
            rt_typed_named_sem_init(RAW_SEN_SEM, 1, RES_SEM);
            rt_typed_named_sem_init(PROC_SEN_SEM, 1, RES_SEM);
            raw_data_buddy_mbx = rt_typed_mbx_init(BUDDY_TASK_MBX_1, sizeof(struct raw_sensors_data), PRIO_Q);
            proc_data_buddy_mbx = rt_typed_mbx_init(BUDDY_TASK_MBX_2, sizeof(struct processed_sensors_data), PRIO_Q);
            sensor_is_broken_mbx = rt_typed_mbx_init(TBS_BROKEN_MBX_NAME, sizeof(struct sensor_is_broken), PRIO_Q);
        }
        else
        {
            /*reset schedule for EDF*/
            deadline = last_deadline + TBS_BANDWIDTH;
            last_deadline = deadline;
            rt_task_set_resume_end_times(now(), deadline);

            /*acquire lock on resources*/
            rt_sem_wait(raw_data->raw_sensor_data_lock);
            rt_sem_wait(proc_data->processed_sensor_data_lock);

            /*do the actual data printing*/

            /*il task invia una copia della struct al buddy task, che la stampa*/
            rt_mbx_send(raw_data_buddy_mbx, (void *) raw_data, sizeof(struct raw_sensors_data));
            rt_mbx_send(proc_data_buddy_mbx, (void *) proc_data, sizeof(struct processed_sensors_data));

            /*release lock on resources*/
            rt_sem_signal(proc_data->processed_sensor_data_lock);
            rt_sem_signal(raw_data->raw_sensor_data_lock);

            number_of_calls = number_of_calls++; //incrementa il numero di notifiche ricevute
        }

        /*il numero degli errori è stato accumulato, inviare il messaggio di terminazione*/
        struct sensor_is_broken sensorIsBroken;
        sensorIsBroken.task_id = TBS_SPEED_ID;
        sensorIsBroken.has_broken = 1;
        rt_mbx_send(sensor_is_broken_mbx, (void *) &sensorIsBroken, sizeof(struct sensor_is_broken));

        /*il processo si auto-sospende*/
        rt_task_suspend(rt_whoami());
    }

    return;
}

_Noreturn void monitor_temp_task(double deadline)
{
    static number_of_calls = 0; //numero di notifiche ricevute

    while (number_of_calls <= MONITOR_TEMP_N_OF_CALLS)
    {
        if (deadline == 0)
        {
            tasks[MONITOR_TEMPERATURE_TASK] = rt_whoami(); //inizializza e mette a disposizione il nome del task, in modo tale da poterlo far ripartire
            rt_task_init_schmod(nam2num("ALTTSK"), MONITOR_TEMPERATURE_TASK_PRIO, 0, 0, SCHED_FIFO, 1);
            rt_typed_named_sem_init(RAW_SEN_SEM, 1, RES_SEM);
            rt_typed_named_sem_init(PROC_SEN_SEM, 1, RES_SEM);
            raw_data_buddy_mbx = rt_typed_mbx_init(BUDDY_TASK_MBX_1, sizeof(struct raw_sensors_data), PRIO_Q);
            proc_data_buddy_mbx = rt_typed_mbx_init(BUDDY_TASK_MBX_2, sizeof(struct processed_sensors_data), PRIO_Q);
            sensor_is_broken_mbx = rt_typed_mbx_init(TBS_BROKEN_MBX_NAME, sizeof(struct sensor_is_broken), PRIO_Q);
        }
        else
        {
            /*reset schedule for EDF*/
            deadline = last_deadline + TBS_BANDWIDTH;
            last_deadline = deadline;
            rt_task_set_resume_end_times(now(), deadline);

            /*acquire lock on resources*/
            rt_sem_wait(raw_data->raw_sensor_data_lock);
            rt_sem_wait(proc_data->processed_sensor_data_lock);

            /*do the actual data printing*/

            /*il task invia una copia della struct al buddy task, che la stampa*/
            rt_mbx_send(raw_data_buddy_mbx, (void *) raw_data, sizeof(struct raw_sensors_data));
            rt_mbx_send(proc_data_buddy_mbx, (void *) proc_data, sizeof(struct processed_sensors_data));

            /*release lock on resources*/
            rt_sem_signal(proc_data->processed_sensor_data_lock);
            rt_sem_signal(raw_data->raw_sensor_data_lock);

            number_of_calls = number_of_calls++; //incrementa il numero di notifiche ricevute
        }
        /*il numero degli errori è stato accumulato, inviare il messaggio di terminazione*/
        struct sensor_is_broken sensorIsBroken;
        sensorIsBroken.task_id = TBS_SPEED_ID;
        sensorIsBroken.has_broken = 1;
        rt_mbx_send(sensor_is_broken_mbx, (void *) &sensorIsBroken, sizeof(struct sensor_is_broken));
        /*il processo si auto-sospende*/
        rt_task_suspend(rt_whoami());
    }
}
