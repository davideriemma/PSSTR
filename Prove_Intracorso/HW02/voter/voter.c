/**
 * Author: Davide Riemma N46003702
 * email: da.riemma@studenti.unina.it
*/


#include <linux/module.h>
#include <asm/io.h>
//TODO: Aggiungere header file necessari

#include <asm/rtai.h>
#include <rtai_shm.h>
#include <rtai_sched.h>
#include <rtai_sem.h> //inclusione dell'header per l'utilizzo delle primitive semaforiche

#include "parameters.h"

#define N_TASKS 3

#define TRUE 1
#define FALSE 0

//TODO: definizioni variabili globali e puntatori alle shared memory

static struct raw_sensors_data * raw_data;
static struct processed_sensors_data * processed_data;

RT_TASK tasks_id[N_TASKS]; /*vettore degli identificativi dei task*/

/*prototipi per il vettore di puntatori*/
void altitude(long);
void temperature(long);
void speed(long);

/*il vettore di puntatori a funzioni non contiene allocazioni di memoria, dunque non entra in gioco
il gestore dell'heap*/
static void (*tasks_code[N_TASKS])(long) = {altitude, speed, temp};
//static unsigned int task_init_status[N_TASKS] = {TRUE, TRUE, TRUE};
//static int argc_array = 0;

/*create a status array*/
//module_param_array(task_init_status, unsigned int, &argc_array, 0000);
//MODULE_PARAM_DESC(task_init_status, "Stato dei tasks");

//TODO: implementazione funzioni dei task per temperature, speed, altitude

void temperature(long dummy)
{
	unsigned int cont = TRUE;

	int i; //dichiarazione compatibile del contatore

	/*acquisizione esclusiva della risorsa: siccome il controllo e la correzione dei dati è un'attività atomica
	(e.g. non può essere interrotta, soprattutto se questa interruzione comporta il cambiamento dei dati), abbiamo
	necessità di acquisire i semafori prima del for.
	Da notare che i deadlock vengono evitati, in quanto è imposta una ordinazione sull'acquisizione delle risorse: siccome
	si accede prima al semaforo dei sensori grezzi e poi al semaforo dei sensori processati, non può
	accadere che ci sia il semaforo 'raw_sensor_data_lock' bloccato e 'processed_sensor_data_lock' libero.*/

	rt_sem_wait(raw_data->raw_sensor_data_lock);
	rt_sem_wait(processed_data->processed_sensor_data_lock);
	for(i = 0; i < TEMP_SIZE && cont; ++i)
	{
		/*found 0?*/
		if(!(raw_data->temperature[i]))
		{
			/*ritorna l'elemento in posizione successiva allo zero, che è sicuramente buona. 
			Utilizza l'aritmetica modulare per eviter errore di overflow*/
			processed_data->temperature = raw_data->temperature[++i % TEMP_SIZE];
			cont = FALSE; /*risparmiamo iterazioni inutili*/
		}
	}

	/*operazione completata, rilascio del semaforo*/
	rt_sem_signal(processed_data->processed_sensor_data_lock);
	rt_sem_signal(raw_data->raw_sensor_data_lock);
	return;
}

void speed(long dummy)
{
	unsigned int cont = TRUE;

	int i;//dichiarazione compatibile del contatore
	//bloccaggio dei semafori, vedi funzione 'temperature()'
	rt_sem_wait(raw_data->raw_sensor_data_lock);
	rt_sem_wait(processed_data->processed_sensor_data_lock);
	for(i = 0; i < SPEED_SIZE && cont; ++i)
	{
		/*basta scorrere finquando non trovo due valori consecutivi uguali (sfruttando l'aritmetica modulare, vado
		a capo quando l'array termina di scorrere, quindi è garantito che questi due valori esistano consecutivi, 
		in quanto ciò è garantito per hp*/
		/*due valori consecutivi uguali?*/
		if(raw_data->speeds[i] == raw_data->speeds[++i % SPEED_SIZE])
		{
			processed_data->speed = raw_data->speeds[i]; /*se sì, allora il gioco è fatto, questa è quella corretta*/
			cont = FALSE;
		}
	}
	/*operazione completata, rilascio del semaforo*/
	rt_sem_signal(processed_data->processed_sensor_data_lock);
	rt_sem_signal(raw_data->raw_sensor_data_lock);
	return;
}

void altitude(long dummy)
{
	/*- le misure possono non essere consecutive
	  - non è sempre detto che ci sia uno zero.*/

	unsigned int cont =  TRUE;

	/*devi trovare solo due misure consecutive, diverse da 0: 
	G b G b G
	G G G b b
	b b G G G
	b G G b G
	b G G G b
	G G b G b
	...

	cmq, escono sempre due uguali, che rappresentano la misura corretta. Se il valore random è 0, il check sulla non-nullità del valore è sufficiente
	se il valore random è diverso da 0, allora sarà sicuramente diverso dal successivo, ed il test per l'uguaglianza di due valori consecutivi lo sgama*/

	int i; //dichiarazione compatibile del contatore
	
	//bloccaggio dei semafori, vedi funzione 'temperature()'
	rt_sem_wait(raw_data->raw_sensor_data_lock);
	rt_sem_wait(processed_data->processed_sensor_data_lock);

	for(i = 0; i < ALTITUDE_SIZE && cont; ++i)
	{
		if(!(raw_data->altitudes[i]) && raw_data->altitudes[i] == raw_data->altitudes[++i % ALTITUDE_SIZE])
		{
			processed_data->altitude = raw_data->altitudes[i];
			cont = FALSE;
		}
	}

    /*operazione completata, rilascio del semaforo*/
    rt_sem_signal(processed_data->processed_sensor_data_lock);
    rt_sem_signal(raw_data->raw_sensor_data_lock);

	return;
}

int init_module(void)
{
	RTIME phase, now;

	phase = nano2count(TICK_TIME);

	//TODO: inizializzazione task
	int i; // dichiarazione compatibile del contatore
	for(i = 0; i < N_TASKS; ++i)
	{
//		if(task_init_status[i])
			rt_task_init_cpuid(&tasks_id[i], tasks_code[i], i, STACK_SIZE, 1, 0, 0, 0);
	}
	//TODO: allocazione shared memory
	raw_data = rtai_kmalloc(RAW_SEN_SHM, sizeof(struct raw_sensors_data));
    /*Semaphore Inizializazion*/
    raw_data->raw_sensor_data_lock = rt_typed_named_sem_init(RAW_SEN_SEM, 1, RES_SEM); //priority inheritance accessed semaphore
	processed_data = rtai_kmalloc(PROC_SEN_SHM, sizeof(struct processed_sensors_data));
	/*Semaphore Inizializazion*/
	processed_data->processed_sensor_data_lock = rt_typed_named_sem_init(PROC_SEN_SEM, 1, RES_SEM); //priority inheritance accessed semaphore
	//TODO: avvio task periodici
	now = rt_get_time();

	for(i = 0; i < N_TASKS; ++i)
	{
//		if(task_init_status[i])
			rt_task_make_periodic(&tasks_id[i], now + phase, TICK_TIME);
	}
	//TODO: scheduling con rate monotonic
	rt_spv_RMS(0);

	return 0;
}


void cleanup_module(void)
{	
	//TODO: rimozione task e shared memory

	int i; //dichiarazione compatibile del contatore
	for(i = 0; i < N_TASKS; ++i)
	{
		rt_task_delete(&tasks_id[i]);
	}

	/*eliminazione dei semafori*/
	rt_named_sem_delete(raw_data->raw_sensor_data_lock);
	rt_named_sem_delete(processed_data->processed_sensor_data_lock);
	/*eliminazione delle memorie condivise*/
	rtai_kfree(RAW_SEN_SHM);
	rtai_kfree(PROC_SEN_SHM);
	
}
