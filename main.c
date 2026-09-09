#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define MAX_TAREFAS 100
#define MAX_NOME 50

typedef struct Tarefa {
    char nome[MAX_NOME];
    int periodo;
    int deadline;
    int burst;
} Tarefa;

int main(int argc, char *argv[]) {

    if (argc != 3) {
        fprintf(stderr, "Erro: o numero de argumentos esperado é 3.\n");
        return 1;
    }

    if (strcmp(argv[1], "rate") != 0 && strcmp(argv[1], "edf") != 0) {
        fprintf(stderr, "Erro: argumento invalido.\n");
        return 1;
    }

    FILE *arquivo = fopen(argv[2], "r");

    if (arquivo == NULL) {
        fprintf(stderr, "Erro: nao foi possivel abrir o arquivo.\n");
        return 1;
    }

    int tempo_total;

    if (fscanf(arquivo, "%d", &tempo_total) != 1) {
        fprintf(stderr, "Erro: tempo total invalido.\n");
        fclose(arquivo);
        return 1;
    }

    if (tempo_total <= 0) {
        fprintf(stderr, "Erro: tempo total deve ser positivo.\n");
        fclose(arquivo);
        return 1;
    }

    Tarefa tarefas[MAX_TAREFAS];
    int qtd_tarefas = 0;

    while (qtd_tarefas < MAX_TAREFAS) {

        Tarefa *t = &tarefas[qtd_tarefas];

        int resultado = fscanf(arquivo, "%49s %d %d %d", t->nome, &t->periodo, &t->deadline, &t->burst);

        if (resultado == EOF) {
            break;
        }

        if (resultado != 4) {
            fprintf(stderr, "Erro: tarefa malformada.\n");
            fclose(arquivo);
            return 1;
        }

        if (t->periodo <= 0 || t->deadline <= 0 || t->burst <= 0) {
            fprintf(stderr, "Erro: os valores da tarefa devem ser positivos.\n");
            fclose(arquivo);
            return 1;
        }

        if (t->deadline > t->periodo) {
            fprintf(stderr, "Erro: deadline maior que o periodo.\n");
            fclose(arquivo);
            return 1;
        }

        if (t->burst > t->deadline) {
            fprintf(stderr, "Erro: burst maior que o deadline.\n");
            fclose(arquivo);
            return 1;
        }

        qtd_tarefas++;
    }

    if (qtd_tarefas == 0) {
        fprintf(stderr, "Erro: nenhuma tarefa encontrada no arquivo.\n");
        fclose(arquivo);
        return 1;
    }

    fclose(arquivo);

    return 0;
}