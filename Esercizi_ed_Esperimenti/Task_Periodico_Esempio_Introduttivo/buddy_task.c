#include <stdio.h>
#include <signal.h>

#include "/usr/realtime/include/rtai_shm.h"

#include "rt_task.h"

void my_handler(int); /*dichiarazione della funzione di handling*/

static int termination_variable  = 1;
static shmem_t * data; /*utilizzo dell'aspetto di visibilità di static: data è visibile solo in questo
file e non entra in conflitto con il 'data' del task RT*/

int main()
{
    /*registra l'handler per il segnale SIGINT*/
    signal(SIGINT, my_handler);
    
    /*attach della shared memory*/
    data = rtai_malloc(SHM_ID, 1);

    while (termination_variable)
    {
        /*accede alla shared memory e legge.
        NOTA CHE QUESTO È UNO SCHEMA PRODUTTORE CONSUMATORE SENZA MUTUA ESCLUSIONE, ASPETTARSI RACE
        CONDITIONS A GOGO*/
        printf("Lettera nella memoria condivisa: [%c]\n", *data);

    }

    /*stacca la shared memory dal buddy task*/
    rtai_free(SHM_ID, &data);

    return 0;
}

/*definisce la funzione di handler*/
void my_handler(int a)
{
    termination_variable = !termination_variable;
    printf("Terminating task\n");
    return;
}