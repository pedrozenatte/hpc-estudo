#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <semaphore.h>

#define THREADS 4 // Define a quantidade de threads

sem_t mutex[THREADS]; // Um mutex para cada thread

int VLR = 0; // Variável compartilhada

int soma_item(int vlr) {
    return vlr + 1;
}

void *inserir_item(void *thread_especifica) {
    int id = *(int *)thread_especifica;

    sem_wait(&mutex[id]); // Espera sua vez para executar

    VLR = soma_item(VLR);
    printf("Thread %d executou, VLR: %d\n", id + 1, VLR);
    
    sem_post(&mutex[(id + 1) % THREADS]); // Para permanecer no intervalo entre 0 a THREADS - 1
    
    pthread_exit(0);
}


int main() {
    pthread_t threads[THREADS - 1]; // A principal já conta como uma thread

    int ids[THREADS];

    // Inicializando os semáforos
    for(int i= 0; i < THREADS; i++) {
        sem_init (&mutex[i], 0 , 0); // Começar com 0 garante que TODAS as threads começem bloqueadas, exceto a principal
        ids[i] = i; 
    }
    
    // Criando as três outras threads
    for (int i = 0; i < THREADS - 1; i++) {
		if (pthread_create(&threads[i], 0, (void *) inserir_item, (void *) /*&i*/ &ids[i + 1]) != 0) { //i + 1, pois a thread principal já é a primeira (thread 0)
			printf("Erro ao criar as threads %d! Saindoo! \n", i);
			fflush(0);
			exit(0);
		}
	}

    // Liberando a segunda thread (lembrando que a primeira é a principal)
    sem_post(&mutex[1]);

    // Essa thread (a principal) também precisa esperar a sua vez
    sem_wait(&mutex[0]);
    VLR = soma_item(VLR);
    printf("Thread principal executou. Valor final de VLR = %d\n", VLR);

    // Espera todas as threads terminarem
    for (int i = 0; i < THREADS - 1; i++) {
        pthread_join(threads[i], NULL);
    }

    // Destruindo os semáforos
    for(int i = 0; i < THREADS; i++) {
        sem_destroy(&mutex[i]);
    }

    return 0; 
}