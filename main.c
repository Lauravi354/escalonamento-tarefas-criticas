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

    int restante;
    int proxima_chegada;
    int deadline_absoluto;
    int ativa;

    int completas;
    int perdidas;
    int killed;
} Tarefa;


void verificar_chegadas(Tarefa tarefas[], int qtd_tarefas, int tempo) {

    for (int i = 0; i < qtd_tarefas; i++) {

        if (tempo == tarefas[i].proxima_chegada) {

            tarefas[i].restante = tarefas[i].burst;

            tarefas[i].deadline_absoluto = tempo + tarefas[i].deadline;

            tarefas[i].ativa = 1;

            tarefas[i].proxima_chegada += tarefas[i].periodo;
        }
    }
}

int escolher_rate(Tarefa tarefas[], int qtd_tarefas) {

    int escolhida = -1;

    for (int i = 0; i < qtd_tarefas; i++) {

        if (tarefas[i].ativa == 0) {
            continue;
        }

        if (escolhida == -1) {
            escolhida = i;
        }
        else if (tarefas[i].periodo < tarefas[escolhida].periodo) {
            escolhida = i;
        }
    }

    return escolhida;
}

void verificar_deadlines(Tarefa tarefas[], int qtd_tarefas, int tempo) {

    for (int i = 0; i < qtd_tarefas; i++) {

        if (tarefas[i].ativa == 1 && tarefas[i].deadline_absoluto == tempo && tarefas[i].restante > 0) {

            tarefas[i].perdidas++;

            tarefas[i].restante = 0;
            tarefas[i].ativa = 0;
        }
    }
}

void executar_rate(Tarefa tarefas[], int qtd_tarefas, int tempo_total) {

    for (int tempo = 0; tempo < tempo_total; tempo++) {

        verificar_deadlines(tarefas, qtd_tarefas, tempo);

        verificar_chegadas(tarefas, qtd_tarefas, tempo);

        int escolhida = escolher_rate(tarefas, qtd_tarefas);

        if (escolhida != -1) {

            tarefas[escolhida].restante--;

            if (tarefas[escolhida].restante == 0) {
                tarefas[escolhida].completas++;
                tarefas[escolhida].ativa = 0;
            }
        }
    }
}

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

        t->restante = 0;
        t->proxima_chegada = 0;
        t->deadline_absoluto = 0;
        t->ativa = 0;
        
        t->completas = 0;
        t->perdidas = 0;
        t->killed = 0;
        
        qtd_tarefas++;
    }

    if (qtd_tarefas == 0) {
        fprintf(stderr, "Erro: nenhuma tarefa encontrada no arquivo.\n");
        fclose(arquivo);
        return 1;
    }

    fclose(arquivo);

    if (strcmp(argv[1], "rate") == 0) {
        executar_rate(tarefas, qtd_tarefas, tempo_total);
    }

    return 0;
}