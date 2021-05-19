/**
 * informazioni specifiche per il thread
*/
#ifndef _THREAD_H_
#define _THREAD_H_

/**
 * il task power_presence_cecker controlla ogni 10ms che ci sia presenza di alimentaizone sulla rete elettrica, 
 * settando un flag di errore e scrivendo un messaggio, che sarà poi raccolto dal main. La struttura 'message'
 * fa ciò.
*/

#define T_c 10000000  //periodo del task

#define POWER_FAILURE_MESSAGE_SIZE 256

typedef struct
{
    char message[POWER_FAILURE_MESSAGE_SIZE];
    unsigned int flag;

} power_failure_t;

//funzione di utility per inizializzare la stuttura
void power_presence_checker_init_struct(power_failure_t *);

void * power_presence_checker(void *);

#endif