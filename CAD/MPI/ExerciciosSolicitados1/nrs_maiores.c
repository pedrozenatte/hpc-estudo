#include <stdio.h>
#include <stdlib.h>
#include "mpi.h"
#include <time.h>

#define TAM 100
#define K 500

int main() {
    printf("\n");
    srand(time(NULL));

    // Variáveis para o MPI
    int nprocessos; // número de processos.
	int meu_rank; // rank do processo, por exemplo, rank 0.
	int src; // processo de origem de uma mensagem.
	int dest; // processo de destino de uma mensagem.
	int tag_mensagem; // é a tag da mensagem, usada para identificar o tipo de mensagem.
	int ret; // guarda o valor de retorno de chamadas do MPI, sendo usada para checar erros
    int buffer; // buffer para armazenar o contador de cada processo
	MPI_Status status; // estrutura que armazena informações sobre a mensagem recebida (quem enviou, tag, tamanho real, etc.).

    MPI_Init(NULL, NULL);
    MPI_Comm_size(MPI_COMM_WORLD, &nprocessos); 
    MPI_Comm_rank(MPI_COMM_WORLD, &meu_rank);

    // Variáveis específicas relacionadas ao vetor para cada processo (balanceamento de carga)
    int inicio = (meu_rank * TAM) / nprocessos; // Marca o inicio do for de cada processo
    int fim = ((meu_rank + 1) * TAM) / nprocessos; // Marca o fim do for de cada processo
    int tamanho_local = fim - inicio; // Tamanho de cada vetor para cada processo (lembrando que isso é importante, pois cada processo pode receber um valor um pouco diferente, caso a divisão pelo tamanho não dê resto 0)
    int contador = 0; 
    
    // Vetor com tamanho menor que o total
    int *vetor_local = (int *) malloc(tamanho_local * sizeof(int)); // Buffer para receber uma parte do vetor

    if(meu_rank == 0) {
        // Criação_Do_Vetor
        int *vetor = (int *) malloc(TAM * sizeof(int));
        printf("VETOR: ");
        for(int i = 0; i < TAM; i++) {
            vetor[i] = rand() % 1001;
            printf("%d[%d] ", vetor[i], i); 
        }
        printf("\n");

        // Fazendo a parte do rank 0 - Análise dos maiores que K e marcando com 1 as posições;
        for(int i = 0; i < tamanho_local; i++) {
            if(vetor[i] > K) { // Como estamos no rank 0, não precisamos nos preocupar onde inicia e termina, apenas a quantidade/tamanho_local que será analisada
                contador++;
                vetor[i] = 1;
            }
        }

        // Enviando uma parte do vetor para os outros processos
        tag_mensagem = 0; // Valor da tag para enviar partes do vetor maior
        for(dest = 1; dest < nprocessos; dest++) { // Começa em 1, pois o processo 0 (que é este próprio) faz a sua parte, além disso não faz sentido enviar uma mensagem para si mesmo
            int inicioI = (dest * TAM) / nprocessos; // Inicio do processo destino
            int fimI = ((dest + 1) * TAM) / nprocessos; // Fim do processo destino
            int quantidadeI = fimI - inicioI; // Quantidade de elementos a serem enviados para o processo destino
            ret = MPI_Send(&vetor[inicioI], quantidadeI, MPI_INT, dest, tag_mensagem, MPI_COMM_WORLD);
            if(ret != MPI_SUCCESS) {
                printf("Erro no envio da mensagem\n");
            }
        }
        
        // Recebendo os contadores de cada processo, isto é, quantidade de elementos maiores que K
        tag_mensagem = 1; // Valor da tag para receber a quantidade de elementos maiores que K
        for(src = 1; src < nprocessos; src++) {
            // Recebendo o contador de cada processo
            ret = MPI_Recv(&buffer, 1, MPI_INT, src, tag_mensagem, MPI_COMM_WORLD, &status);
            
            if(ret != MPI_SUCCESS) {
                printf("Erro no recebimento do contador\n"); 
            }
        
            contador += buffer; // Recebendo do buffer e somando no contador
        } 

        tag_mensagem = 2; // Valor da tag para receber o vetor com as posições maiores que K marcadas 
        for(src = 1; src < nprocessos; src++) {
            int inicioI = (src * TAM) / nprocessos; // Inicio vetor que vem do processo origem
            int fimI = ((src + 1) * TAM) / nprocessos; // Fim vetor que vem do processo origem
            int quantidadeI = fimI - inicioI; // Quantidade de elementos a serem recebidos do processo origem
            
            // Recebendo a parte atualizada do vetor de cada processo
            ret = MPI_Recv(&vetor[inicioI], quantidadeI, MPI_INT, src, tag_mensagem, MPI_COMM_WORLD, &status);
            
            if(ret != MPI_SUCCESS) {
                printf("Erro no recebimento do vetor atualizado\n");
            }
        }
          

        // Imprimindo o resultado
        printf("Quantidade de elementos maiore que %d: %d\n", K, contador);
        printf("Posicoes: ");
        for(int i = 0; i < TAM; i++) {
            if(vetor[i] == 1) {
                printf("%d ", i);
            }
        }
        printf("\n");
        
        free(vetor);
    }
    else {
        // Recebendo parte do vetor total
        tag_mensagem = 0; // Valor da tag para receber partes do vetor maior
        src = 0;
        ret = MPI_Recv(&vetor_local[0], tamanho_local, MPI_INT, src, tag_mensagem, MPI_COMM_WORLD, &status);

        if(ret != MPI_SUCCESS) {
            printf("Erro no recebimento de parte do vetor\n");
        }

        for(int i = 0; i < tamanho_local; i++) {
            if(vetor_local[i] > K) {
                contador++;
                vetor_local[i] = 1; 
            }
        }

        // Enviando o contador
        tag_mensagem = 1; // Valor da tag para enviar a quantidade de elementos maiores que K
        dest = 0;
        ret = MPI_Send(&contador, 1, MPI_INT, dest, tag_mensagem, MPI_COMM_WORLD);

        if(ret != MPI_SUCCESS) {
            printf("Erro no envio do contador\n");
        }

        // Enviando parte do vetor atualizado
        tag_mensagem = 2; // Valor da tag para enviar parte do vetor com as posições maiores que K marcadas
        dest = 0; 
        ret = MPI_Send(&vetor_local[0], tamanho_local, MPI_INT, dest, tag_mensagem, MPI_COMM_WORLD);

        if(ret != MPI_SUCCESS) {
            printf("Erro no envio de parte do vetor\n");
        }
        
    }
    
    free(vetor_local);

    ret = MPI_Finalize();

    if(ret == MPI_SUCCESS) { // MPI_SUCESS é uma constante que contém o valor que deu certo, normalmente é 0
        printf("MPI encerrado com sucesso\n");
    }



    return 0;
}