// para compilar: gcc prodcons_n_threads_sem.c -o prodcons_n_threads_sem -lpthread
// para executar: prodcons_n_threads_sem
//
#include <stdlib.h>
#include <stdio.h>
#include <pthread.h>
#include <semaphore.h>

#define THREADS 7
#define MAX_PRODUCED 27
#define MAX_QUEUE 17

sem_t  mutex, empty, full;

int queue[MAX_QUEUE], item_available=0, produced=0, consumed=0;


int create_item(void) {

	return(rand()%1000);

} //fim  create_item()

void insert_into_queue(int item, int p_arg) {
	
	queue[item_available++] = item;
	produced++;
	printf("Producer %d producing item:%d, value:%d, queued:%d \n", p_arg, produced, item, item_available); 
	fflush(0);
	return;

} // fim insert_into_queue()

int extract_from_queue(int p_arg) {

	consumed++;
	printf("Consumer %d cosuming item:%d, value:%d, queued:%d \n", p_arg, consumed, queue[item_available-1], item_available-1);
	fflush(0);
	
	return(queue[--item_available]);

} // fim extract_from_queue()

void process_item(int my_item, int p_arg, int printed) {

	printf("Thread %d printed:%d, value:%d, queued:%d \n", p_arg, printed, my_item, item_available);
	fflush(0);

	return;

} // fim_process_item()

void *producer(void *p_arg) {
	int item, p, stay = 1;

	p = (int) *((int *)p_arg);

	printf("Producer %d  \n", p);
	fflush(0);

	do {
		item = create_item();
		sem_wait(&empty);
		sem_wait(&mutex);
		if (produced < MAX_PRODUCED) {
				insert_into_queue(item, p);
			}
			else {
				stay = 0;
			}
        	sem_post(&mutex);
        	sem_post(&full);
	} while (stay);
	pthread_exit(0);
} // fim producer

void *consumer(void *p_arg) {
	int my_item, p, stay = 1, printed;
	p = (int) *((int *)p_arg);

//	printf("Consumer %d  \n", p);
//	fflush(0);

	do {
		sem_wait(&full);
		sem_wait(&mutex);
			if (consumed < MAX_PRODUCED) {
				my_item = extract_from_queue(p);
				printed = consumed;
			}
			else {
				stay = 0;
			}
		sem_post(&mutex);
		sem_post(&empty);
		if (stay ) {
			process_item(my_item, p, printed);
		}
	} while (stay);

	// ao serem consumidos todos os itens, alguns consumidores ainda podem estar bloqueados no sem_wait&full), 
	// esperando mais itens sejam produzidos. O sem_post(&full) abaixo libera esses consumidores para que saiam
	// do loop e finalizem sozinhos. Cada consumidor que sai tenta avisar outro que todos os itens ja foram consumidos
	// assim podera haver mais sem_post(&full) que o necessario. Um if e um contador resolveriam esse problema.
	sem_post(&full);
	pthread_exit(0);
} // fim_consumer


int main(void) {
	pthread_t prod_handle[THREADS], cons_handle[THREADS];

	int i, p [THREADS];

	/* declarations and initializations */
	item_available = 0;

	sem_init (&mutex, 0 , 1);
	sem_init(&empty, 0, MAX_QUEUE);
	sem_init(&full, 0, 0);

	/* create and join producer and consumer threads */

	for (i=0; i < THREADS; i++) {
		p[i] = i;
		if (pthread_create(&prod_handle[ i ], 0, (void *) producer, (void *) &i /*&p[ i ] */) != 0) { 
			printf("Error creating thread producer %d! Exiting! \n", i);
			fflush(0);
			exit(0);
		}
	}

	for (i=0; i < THREADS; i++) {
		p[i] = i;
		if (pthread_create(&cons_handle[ i ], 0, (void *) consumer, (void *) &p[ i ] ) != 0) { 
			printf("Error creating thread consumer %d! Exiting! \n", i);
			fflush(0);
			exit(0);
		}
	}


	for (i=0; i < THREADS; i++) {
		pthread_join(prod_handle[ i ], 0);
		//printf("Producer %d out! \n", i);
	}

	for (i=0; i < THREADS; i++) {
		pthread_join(cons_handle[ i ], 0);
		//printf("Consumer %d out! \n", i);
	}

	exit(0);
} // main()
