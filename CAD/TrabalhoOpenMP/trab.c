/*
Giovanni dos Santos 13695341
Guilherme Augusto Fincatti da Silva 13676986
Marcelo Eduardo Reginato 13676965
Pedro Guilherme de Barros Zenatte 13676919
*/

// ATENÇÃO: O QUE ESTAMOS BUSCANDO COMO DESEMPENHO É TEMPO DE RESPOSTA

// Bibliotecas
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <omp.h>

// Constantes
#define NUM_THREADS 4 // Estamos considerando 4 núcleos físicos 
#define MAX_ASCII 95 // (126 - 32 + 1). Estamos considerando as faixas
#define TAM_INICIAL 1000 // Tamanho inicial da linha 
#define BLOCOS_INCREMENTAIS 64 // Caso temos uma linha com mais de 1000 caracteres, realocaremos a matriz com + 64 de espaço



// Estrutura para armazenar as frequências
typedef struct tipo_char_freq {
    int c; // Código ASCII do caracter
    int freq; // Frequência deste caractere na string
} char_frequencia;


// Função para ler a entrada padrão e armazenar em uma matriz de strings
char **ler_strings(int *num_linhas_out, int **num_colunas_out) {
    char **matriz = NULL; // Matriz de strings
    int *num_colunas = NULL; // Vetor com o número de colunas reais de cada linha

    int capacidade_linhas = 10; // Capacidade inicial de linhas
    int num_linhas = 0; // Quantidade real de linhas lidas

    // Alocação inicial dos vetores
    matriz = malloc(capacidade_linhas * sizeof(char*));
    num_colunas = malloc(capacidade_linhas * sizeof(int));

    // Alocação inicial para leitura da primeira linha
    char *linha_atual = malloc(TAM_INICIAL * sizeof(char));
    int capacidade_colunas = TAM_INICIAL; // Capacidade da linha atual
    int colunas = 0; // Quantos caracteres já foram lidos na linha atual

    int c;
    while ((c = getchar()) != EOF) {
        if (c == '\n') {
            // Alocamos espaço exato para essa linha
            char *linha_final = malloc(colunas * sizeof(char));
            for (int i = 0; i < colunas; i++) {
                linha_final[i] = linha_atual[i]; // Copia os caracteres
            }

            // Se a capacidade de linhas for insuficiente, aumentamos
            if (num_linhas == capacidade_linhas) {
                capacidade_linhas += 30;
                matriz = realloc(matriz, capacidade_linhas * sizeof(char*));
                num_colunas = realloc(num_colunas, capacidade_linhas * sizeof(int));
            }

            // Armazenamos a linha final na matriz e o número de colunas dela
            matriz[num_linhas] = linha_final;
            num_colunas[num_linhas] = colunas;
            num_linhas++;

            // Preparamos para a próxima linha
            capacidade_colunas = TAM_INICIAL;
            linha_atual = realloc(linha_atual, capacidade_colunas * sizeof(char));
            colunas = 0;
        } else {
            // Se a linha atual estiver cheia, aumentamos sua capacidade
            if (colunas == capacidade_colunas) {
                capacidade_colunas += BLOCOS_INCREMENTAIS;
                linha_atual = realloc(linha_atual, capacidade_colunas * sizeof(char));
            }
            linha_atual[colunas++] = c; // Armazena o caractere lido
        }
    }

    // Caso o arquivo termine sem uma quebra de linha final
    if (colunas > 0) {
        char *linha_final = malloc(colunas * sizeof(char));
        for (int i = 0; i < colunas; i++) {
            linha_final[i] = linha_atual[i];
        }

        if (num_linhas == capacidade_linhas) {
            capacidade_linhas += 10;
            matriz = realloc(matriz, capacidade_linhas * sizeof(char*));
            num_colunas = realloc(num_colunas, capacidade_linhas * sizeof(int));
        }

        matriz[num_linhas] = linha_final;
        num_colunas[num_linhas] = colunas;
        num_linhas++;
    }

    free(linha_atual); // Libera a linha temporária

    // Preenche os parâmetros de saída
    *num_linhas_out = num_linhas;
    *num_colunas_out = num_colunas;

    return matriz;
}

// Função merge
void merge(char_frequencia *vetor, int inicio, int meio, int fim) {
    // Calculo do tamanho das metades
    int tamanho_esquerda = meio - inicio + 1;
    int tamanho_direita = fim - meio;

    // Aloca vetores temporários para guardar os dados das metadas 
    char_frequencia *esquerda = malloc(tamanho_esquerda * sizeof(char_frequencia));
    char_frequencia *direita = malloc(tamanho_direita * sizeof(char_frequencia));

    // Copia os dados
    for (int i = 0; i < tamanho_esquerda; i++) esquerda[i] = vetor[inicio + i];
    for (int j = 0; j < tamanho_direita; j++) direita[j] = vetor[meio + 1 + j];

    // Intercala os vetores
    int i = 0, j = 0, k = inicio;
    while (i < tamanho_esquerda && j < tamanho_direita) {
        if (esquerda[i].freq <= direita[j].freq) vetor[k++] = esquerda[i++];
        else vetor[k++] = direita[j++];
    }

    while(i < tamanho_esquerda) { // Copia os elementos da metade esquerda
        vetor[k++] = esquerda[i++];
    } 
    while(j < tamanho_direita) { // Copia os elemntos da metade direita
        vetor[k++] = direita[j++];
    }

    free(esquerda);
    free(direita);
}

// Função recursiva de merge sort com paralelismo
void merge_sort(char_frequencia *vetor, int inicio, int fim) {
    if (inicio < fim) {
        int meio = inicio + (fim - inicio) / 2;

        // Divide recursivamente as metades
        #pragma omp task shared(vetor) // Ordena metade direita
        merge_sort(vetor, inicio, meio);

        #pragma omp task shared(vetor) // Ordena metade esquerda
        merge_sort(vetor, meio + 1, fim);

        #pragma omp taskwait // Junta as metades
        merge(vetor, inicio, meio, fim);
    }
}


int main() {
    double inicio, fim; // Variáveis para calcular tempo de resposta
    inicio = omp_get_wtime();  // Início do tempo de resposta

//------------Referentes à matriz de strings------------------------------
    int num_linhas; // Quantidade total de linhas lidas
    int *num_colunas; // Vetor com a quantidade de colunas de cada linha

    // Montando a matriz com os caracteres
    char **matriz_caracteres = ler_strings(&num_linhas, &num_colunas);

//----------- Referentes ao paralelismo-----------------------------------
    int **matriz_freq_linhas = malloc(num_linhas * sizeof(int*));
    for (int i = 0; i < num_linhas; i++) {
        matriz_freq_linhas[i] = calloc(MAX_ASCII, sizeof(int)); // Vetor de frequências global iniciados com 0
    }

    char_frequencia **frequencias_globais_por_linhas = malloc(num_linhas * sizeof(char_frequencia*)); // Vetor auxiliar para armazenar os resultados por linha
//--------------------------PARALELISMO-----------------------------------
    #pragma omp parallel num_threads(NUM_THREADS) 
    {
        #pragma omp for // Cada thread fica responsável por uma linha/ou_quantidade_de_linhas da matriz 
        for(int i = 0; i < num_linhas; i++) {
            int codigo_asc[num_colunas[i]]; // Para utilizar SIMD
            
            #pragma omp simd
            for(int j = 0; j < num_colunas[i]; j++) {
                codigo_asc[j] = (int)matriz_caracteres[i][j] - 32; // Pegando o código ASCII do caractere da linha i e coluna j e shiftando em 32
            }

            for(int j = 0; j < num_colunas[i]; j++) {
                matriz_freq_linhas[i][codigo_asc[j]]++; // Somando um na posição do código ASC
            }
        }
    }
    // Agora temos cada linha da matriz desordenada na frequência, precisamos ordenar

    // Ordenando cada linha da matriz 
    // Para cada linha, cria vetor de char_frequencia e ordena com merge sort paralelo
    #pragma omp parallel for num_threads(NUM_THREADS)
    for (int i = 0; i < num_linhas; i++) {
        frequencias_globais_por_linhas[i] = malloc(MAX_ASCII * sizeof(char_frequencia)); // Alocando a linha para cada thread preencher de forma independente 

        // Associa o código ASCII original com a frequência (por isso soma 32)
        for (int c = 0; c < MAX_ASCII; c++) {
            frequencias_globais_por_linhas[i][c].c = c + 32; // Caractere da linha i (ajuste com + 32)
            frequencias_globais_por_linhas[i][c].freq = matriz_freq_linhas[i][c]; // Frequência desse caractere na linha i
        }

        merge_sort(frequencias_globais_por_linhas[i], 0, MAX_ASCII - 1); // Ordena os pares (caractere, frequência) da linha i por frequência descrecente
    }   

    // Agora imprime em ordem
    for (int i = 0; i < num_linhas; i++) {
        for (int c = 0; c < MAX_ASCII; c++) {
            if (frequencias_globais_por_linhas[i][c].freq > 0)
                printf("%d %d\n", frequencias_globais_por_linhas[i][c].c, frequencias_globais_por_linhas[i][c].freq);
        }
        printf("\n");
        free(frequencias_globais_por_linhas[i]);
    }
    free(frequencias_globais_por_linhas);
    


//-------------------Libera a memória alocada-----------------------------
    // Libera matriz
    #pragma omp parallel for num_threads(NUM_THREADS)
    for (int i = 0; i < num_linhas; i++) {
        free(matriz_caracteres[i]);  // Libera cada linha da matriz que foi formada pelas strings
        free(matriz_freq_linhas[i]); // O mesmo, mas para a matriz de frequências
    }

    free(matriz_caracteres); // Libera o vetor de ponteiros
    free(num_colunas);    // Libera o vetor de tamanhos das colunas nas determinadas linhas
    free(matriz_freq_linhas); 

    fim = omp_get_wtime(); // Fim do tempo de resposta
    printf("Tempo de resposta total: %.6f segundos\n", fim - inicio);


    return 0;
}