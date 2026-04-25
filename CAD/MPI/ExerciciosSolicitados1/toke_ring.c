#include <stdio.h>
#include <stdlib.h>
#include "mpi.h"



int main() {
    // Variáveis para o MPI
    int nprocessos; // número de processos.
	int meu_rank; // rank do processo, por exemplo, rank 0.
	int src; // processo de origem de uma mensagem.
	int dest; // processo de destino de uma mensagem.
	int tag_mensagem; // é a tag da mensagem, usada para identificar o tipo de mensagem.
	int ret; // guarda o valor de retorno de chamadas do MPI, sendo usada para checar erros
    int buffer; // buffer para guardar a mensagem
	MPI_Status status; // estrutura que armazena informações sobre a mensagem recebida (quem enviou, tag, tamanho real, etc.).

    MPI_Init(NULL, NULL);
    MPI_Comm_size(MPI_COMM_WORLD, &nprocessos); 
    MPI_Comm_rank(MPI_COMM_WORLD, &meu_rank);

    for(int i = 0; i < nprocessos; i++) {
        
    }



    return 0;
}