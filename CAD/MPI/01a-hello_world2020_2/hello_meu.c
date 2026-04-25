#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "mpi.h"


int main() {
    printf("\n");
    int MENSAGEM = 27; 
    int nprocessos; // número de processos.
	int meu_rank; // rank do processo, por exemplo, rank 0.
	int src; // processo de origem de uma mensagem.
	int dest; // processo de destino de uma mensagem.
	int msgtag; // é a tag da mensagem, usada para identificar o tipo de mensagem.
	int ret; // guarda o valor de retorno de chamadas do MPI, sendo usada para checar erros
	MPI_Status status; // estrutura que armazena informações sobre a mensagem recebida (quem enviou, tag, tamanho real, etc.).

    MPI_Init(NULL, NULL); // Inicializa o ambiente MPI dentro de cada processo já existente
    // Desse ponto em diante o programa consegue fazer as chamadas como MPI_Comm_rank, send, receive e etc.
    MPI_Comm_size(MPI_COMM_WORLD, &nprocessos); 
    MPI_Comm_rank(MPI_COMM_WORLD, &meu_rank);

    int *bufrecv = (int *) malloc(sizeof(int)); // Buffer para receber a mensagem
    
    msgtag = 1;
    if(meu_rank == 0) { // Processo 0 executa esse trecho do código
        src = 1;
        dest = 1; // tanto src quanto destino são iguais a 1, pois estaremos enviando uma mensagem para o processo de rank 1 (dest = 1), e também esperaremos uma mensagem do processo 1 (por isso src = 1).
        // Vamos enviar uma mensagem para o processo 1
        MPI_Send(&MENSAGEM, 1, MPI_INT, dest, msgtag, MPI_COMM_WORLD);
        printf("Sou o processo %d e a mensagem enviada eh %d\n", meu_rank, MENSAGEM); 
        //msgtag = 2; // Agora vamos receber esperar uma mensagem com a tag = 2

    }
    else {
        src = 0; 
        MPI_Recv(bufrecv, 1, MPI_INT, src, msgtag, MPI_COMM_WORLD, &status);
        printf("Sou o processo %d e a mensagem recebida eh %d\n", meu_rank, *bufrecv);
    }
    free(bufrecv);
    ret = MPI_Finalize(); // Essa função encerra o ambiente de comunicação global e retorna um inteiro
    if(ret == MPI_SUCCESS) { // MPI_SUCESS é uma constante que contém o valor que deu certo, normalmente é 0
        printf("MPI encerrado com sucesso\n");
    }

    return 0;
}