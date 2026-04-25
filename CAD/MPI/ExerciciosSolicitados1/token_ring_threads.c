#include <stdio.h>
#include <stdlib.h>
#include <mpi.h>
#include <omp.h>

#define NUM_THREADS 4

int main() {
    printf("\n");
    // Variáveis para o MPI
    int nprocessos; // número de processos.
	int meu_rank; // rank do processo, por exemplo, rank 0.
	int src; // processo de origem de uma mensagem.
	int dest; // processo de destino de uma mensagem.
	int tag_mensagem; // é a tag da mensagem, usada para identificar o tipo de mensagem.
	int ret; // guarda o valor de retorno de chamadas do MPI, sendo usada para checar erros
    int buffer; // buffer para guardar a mensagem
	MPI_Status status; // estrutura que armazena informações sobre a mensagem recebida (quem enviou, tag, tamanho real, etc.).
    int provid; 

    MPI_Init_thread(NULL, NULL, MPI_THREAD_MULTIPLE, &provid); 
    //MPI_Init(NULL, NULL);
    MPI_Comm_size(MPI_COMM_WORLD, &nprocessos); 
    MPI_Comm_rank(MPI_COMM_WORLD, &meu_rank);

    int contador = 0;

    if(meu_rank == 0) {
        #pragma omp parallel num_threads(NUM_THREADS)
        {
            #pragma omp single
            {
                for(int i = 0; i < NUM_THREADS; i++) {
                    if(i == 0) {
                        #pragma omp task depend(out: contador) // Task 1
                        {
                            contador += 1;
                            printf("PROCESSO %d: Sou a TASK %d e estou com o contador: %d\n", meu_rank, i + 1, contador); 
                        }
                    }
                    else if(i != 0 && i != NUM_THREADS - 1) {
                        #pragma omp task depend(inout: contador) // Tasks do meio
                        {
                            contador += 1;
                            printf("PROCESSO %d: Sou a TASK %d e estou com o contador: %d\n", meu_rank, i + 1, contador); 
                        }
                    }
                    else {
                        #pragma omp task depend(in: contador) // Task NUM_THREADS (última task)
                        {
                            contador += 1;
                            printf("PROCESSO %d: Sou a TASK %d e estou com o contador: %d\n", meu_rank, i + 1, contador); 
                        }
                    }
                }
            }
        }   

        printf("Sou o processo %d e estou com o contador = %d\n", meu_rank, contador); 

        tag_mensagem = 0; // Tag para enviar do precesso 0 para o processo 1
        dest = 1; 

        ret = MPI_Send(&contador, 1, MPI_INT, dest, tag_mensagem, MPI_COMM_WORLD);
            if(ret != MPI_SUCCESS) {
                printf("Erro no envio do contador pelo processo 0\n");
            }
        
        tag_mensagem = nprocessos - 1; // Tag para receber do último processo para o processo 0
        src = nprocessos - 1; 

        ret = MPI_Recv(&buffer, 1, MPI_INT, src, tag_mensagem, MPI_COMM_WORLD, &status);  
        if(ret != MPI_SUCCESS) {
            printf("Erro no recebimento do contador final\n"); 
        }

        printf("Contador final: %d\n", buffer);
    }
    else {
        tag_mensagem = meu_rank - 1; 
        src = meu_rank - 1; 
        ret = MPI_Recv(&buffer, 1, MPI_INT, src, tag_mensagem, MPI_COMM_WORLD, &status);
            if(ret != MPI_SUCCESS) {
                printf("Erro no recebimento do contador\n");
            }
        
        contador += buffer;

        #pragma omp parallel num_threads(NUM_THREADS)
        {
            #pragma omp single
            {
                for(int i = 0; i < NUM_THREADS; i++) {
                    if(i == 0) {
                        #pragma omp task depend(out: contador) // Task 1
                        {
                            contador += 1;
                            printf("PROCESSO %d: Sou a TASK %d e estou com o contador: %d\n", meu_rank, i + 1, contador); 
                        }
                    }
                    else if(i != 0 && i != NUM_THREADS - 1) {
                        #pragma omp task depend(inout: contador) // Tasks do meio
                        {
                            contador += 1;
                            printf("PROCESSO %d: Sou a TASK %d e estou com o contador: %d\n", meu_rank, i + 1, contador); 
                        }
                    }
                    else {
                        #pragma omp task depend(in: contador) // Task NUM_THREADS (última task)
                        {
                            contador += 1;
                            printf("PROCESSO %d: Sou a TASK %d e estou com o contador: %d\n", meu_rank, i + 1, contador); 
                        }
                    }
                }
            }
        }      
        
        printf("Sou o processo %d e estou com o contador = %d\n", meu_rank, contador); 

        if(meu_rank == nprocessos - 1) { // O último processo envia para o processo 0
            tag_mensagem = meu_rank; 
            dest = 0;
            ret = MPI_Send(&contador, 1, MPI_INT, dest, tag_mensagem, MPI_COMM_WORLD);
            if(ret != MPI_SUCCESS) {
                printf("Erro no envio do contador pelo processo 0\n");
            }
            
        }
        else {
            tag_mensagem = meu_rank;
            dest = meu_rank + 1; 
            ret = MPI_Send(&contador, 1, MPI_INT, dest, tag_mensagem, MPI_COMM_WORLD);
            if(ret != MPI_SUCCESS) {
                printf("Erro no envio do contador pelo processo 0\n");
            }
        }
    }

    ret = MPI_Finalize(); // Essa função encerra o ambiente de comunicação global e retorna um inteiro
    if(ret == MPI_SUCCESS) { // MPI_SUCESS é uma constante que contém o valor que deu certo, normalmente é 0
        printf("MPI encerrado com sucesso\n");
    }

    return 0; 
}

/*
DUVIDA
se eu fizer
#pragma omp task depend(out: contador) // Task 1
    {
        contador += 1;
    }
#pragma omp task depend(inout: contador) // Tasks 2
    {
        contador += 1;
    }
#pragma omp task depend(in: contador) // Task 3 
    {
        contador += 1;
    }
A task 1 ocorrerá antes da 2, a 2 antes da 3 e a 3 por último? Por ser a mesma variável, esse dependência acontece em ordem?
*/