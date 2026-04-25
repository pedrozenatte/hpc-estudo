// para compilar: gcc prodcons_N_threads_varcond.c -o prodcons_N_threads_varcond -lpthread
// para executar: prodcons_N_threads_varcond
//
#include <stdlib.h>
#include <stdio.h>
#include <pthread.h>

#define THREADS 3			// define o nr de produtores e consumidores

#define MAX_PRODUCED 50	// define o nr de itens que devem ser produzidos no total
#define MAX_QUEUE 5		// define o nr de itens que podem ser armazenados no buffer

pthread_cond_t cond_queue_empty, cond_queue_full;
pthread_mutex_t item_queue_cond_lock;

int queue[MAX_QUEUE], item_available=0, produced=0, consumed=0;

/* other data structures here */

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

	do {
		item = create_item();
		pthread_mutex_lock(&item_queue_cond_lock);
		while (item_available == MAX_QUEUE)
			pthread_cond_wait(&cond_queue_empty, &item_queue_cond_lock);
		if (produced < MAX_PRODUCED) {
			insert_into_queue(item, p);
        		pthread_cond_broadcast(&cond_queue_full);
		}
		else {
			stay = 0;
		}
        	pthread_mutex_unlock(&item_queue_cond_lock);
	}while (stay);   //produced < MAX_PRODUCED );
	pthread_exit(0);
} // fim producer
void *consumer(void *p_arg) {
	int my_item, p, stay = 1, printed;
	p = (int) *((int *)p_arg);
	do {
		pthread_mutex_lock(&item_queue_cond_lock);
        	while (item_available == 0 && consumed < MAX_PRODUCED)
        		pthread_cond_wait(&cond_queue_full, &item_queue_cond_lock);
		if (consumed < MAX_PRODUCED) {
        		my_item = extract_from_queue(p);
			printed = consumed;		// usado so para mostrar o nr do item sendo impresso. Usado em process_item()
									// printed precisa ser local!
        		pthread_cond_broadcast(&cond_queue_empty);
		}
		else {
			stay = 0;
		}
        	pthread_mutex_unlock(&item_queue_cond_lock);
		if (stay) {
        		process_item(my_item, p, printed);
		}
	}while (stay);   //consumed < MAX_PRODUCED);

	pthread_exit(0);

} // fim_consumer


int main(void) {
	pthread_t prod_handle[ THREADS ], cons_handle[ THREADS ];

	int i, p[ THREADS ];

	/* declarations and initializations */
    	item_available = 0;

    	pthread_cond_init(&cond_queue_empty, NULL);
    	pthread_cond_init(&cond_queue_full, NULL);
    	pthread_mutex_init(&item_queue_cond_lock, NULL);
    	/* create and join producer and consumer threads */

	for (i=0; i < THREADS; i++) {
		p[i] = i;
		if (pthread_create(&cons_handle[ i ], 0, (void *) consumer, (void *) &p[ i ] ) != 0) { 
			printf("Error creating thread consumer %d! Exiting! \n", i);
			fflush(0);
			exit(0);
		}
	} 
	
	for (i=0; i < THREADS; i++) {
		p[i] = i;
		if (pthread_create(&prod_handle[ i ], 0, (void *) producer, (void *) &p[ i ] ) != 0) { 
			printf("Error creating thread producer %d! Exiting! \n", i);
			fflush(0);
			exit(0);
		}
	} 

	for (i=0; i < THREADS; i++) {
		pthread_join(prod_handle[ i ], 0);
		printf("Producer %d out! \n", i);
		fflush(0);
	}

	for (i=0; i < THREADS; i++) {
		pthread_join(cons_handle[ i ], 0);
		printf("Consumer %d out! \n", i);
		fflush(0);
	}

	exit(0);	
} // main()
