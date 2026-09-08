#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define MAX_TAREFAS 100
#define MAX_NOME 50

typedef struct Tarefa{
    char nome[MAX_NOME];
    int periodo;
    int deadline;
    int burst;
} Tarefa;

int main(int argc, char *argv[]) {
    if (argc != 3) {
        fprintf(stderr, "Erro: o número de argumentos esperado é 3.\n");
        return 1;
    }

    if (strcmp(argv[1], "rate") != 0 && strcmp(argv[1], "edf") != 0) {
        fprintf(stderr, "Erro: argumento invalido.\n");
        return 1;
    }

    return 0;
}