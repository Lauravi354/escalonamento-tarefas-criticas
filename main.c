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

typedef struct Bloco {
    int tarefa;
    int duracao;
    char status;
} Bloco;

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

int escolher_edf(Tarefa tarefas[], int qtd_tarefas) {

    int escolhida = -1;

    for (int i = 0; i < qtd_tarefas; i++) {

        if (tarefas[i].ativa == 0) {
            continue;
        }

        if (escolhida == -1) {
            escolhida = i;
        }
        else if (tarefas[i].deadline_absoluto < tarefas[escolhida].deadline_absoluto) {
            escolhida = i;
        }
    }

    return escolhida;
}

int verificar_deadlines(Tarefa tarefas[], int qtd_tarefas, int tempo, int tarefa_anterior) {

    int perdida = -1;

    for (int i = 0; i < qtd_tarefas; i++) {

        Tarefa *t = &tarefas[i];

        if (t->ativa == 1 &&
            t->deadline_absoluto == tempo &&
            t->restante > 0) {

            t->perdidas++;

            t->restante = 0;
            t->ativa = 0;

            if (i == tarefa_anterior) {
                perdida = i;
            }
        }
    }

    return perdida;
}

void executar_rate(Tarefa tarefas[], int qtd_tarefas, int tempo_total) {

    FILE *saida = fopen("rate_lvsa.out", "w");

    if (saida == NULL) {
        fprintf(stderr, "Erro: nao foi possivel criar o arquivo de saida.\n");
        return;
    }

    Bloco *blocos = malloc(sizeof(Bloco) * tempo_total);

    if (blocos == NULL) {
        fprintf(stderr, "Erro: falha de memoria.\n");
        return;
    }

    int qtd_blocos = 0;
    int tarefa_anterior = -2;

    for (int tempo = 0; tempo < tempo_total; tempo++) {

        int perdida = verificar_deadlines(tarefas, qtd_tarefas, tempo, tarefa_anterior);
        if (perdida == tarefa_anterior && qtd_blocos > 0) {
            Bloco *anterior = &blocos[qtd_blocos - 1];
            anterior->status = 'L';
        }

        if (tarefa_anterior >= 0 && tarefas[tarefa_anterior].ativa == 0) {
            tarefa_anterior = -2;
        }

        verificar_chegadas(tarefas, qtd_tarefas, tempo);

        int escolhida = escolher_rate(tarefas, qtd_tarefas);

        if (escolhida != tarefa_anterior) {

            if (tarefa_anterior >= 0 && tarefas[tarefa_anterior].ativa == 1) {
                Bloco *anterior = &blocos[qtd_blocos - 1];
                anterior->status = 'H';
            }

            Bloco *b = &blocos[qtd_blocos];

            b->tarefa = escolhida;
            b->duracao = 1;
            b->status = ' ';

            qtd_blocos++;
            tarefa_anterior = escolhida;
        }
        else {
            Bloco *b = &blocos[qtd_blocos - 1];
            b->duracao++;
        }

        if (escolhida != -1) {

            tarefas[escolhida].restante--;

            if (tarefas[escolhida].restante == 0) {

                tarefas[escolhida].completas++;
                tarefas[escolhida].ativa = 0;

                Bloco *b = &blocos[qtd_blocos - 1];
                b->status = 'F';

                tarefa_anterior = -2;
            }
        }
    }

    verificar_deadlines(tarefas, qtd_tarefas, tempo_total, tarefa_anterior);

    for (int i = 0; i < qtd_tarefas; i++) {

        Tarefa *t = &tarefas[i];

        if (t->ativa == 1 && t->restante > 0) {
            t->killed++;
            t->ativa = 0;
            t->restante = 0;
        }
    }

    fprintf(saida, "EXECUTION BY RATE\n");

    for (int i = 0; i < qtd_blocos; i++) {

        Bloco *b = &blocos[i];

        if (b->tarefa == -1) {
            fprintf(saida, "idle for %d units\n", b->duracao);
        }
        else {
            fprintf(saida, "[%s] for %d units - %c\n", tarefas[b->tarefa].nome, b->duracao, b->status);
        }
    }

    fprintf(saida, "LOST DEADLINES\n");

    for (int i = 0; i < qtd_tarefas; i++) {
        Tarefa *t = &tarefas[i];

        fprintf(saida, "[%s] %d\n", t->nome, t->perdidas);
    }

    fprintf(saida, "COMPLETE EXECUTION\n");

    for (int i = 0; i < qtd_tarefas; i++) {
        Tarefa *t = &tarefas[i];

        fprintf(saida, "[%s] %d\n", t->nome, t->completas);
    }

    fprintf(saida, "KILLED\n");

    for (int i = 0; i < qtd_tarefas; i++) {
        Tarefa *t = &tarefas[i];

        fprintf(saida, "[%s] %d\n",
                t->nome,
                t->killed);
    }

    free(blocos);
    fclose(saida);
}

void executar_edf(Tarefa tarefas[], int qtd_tarefas, int tempo_total) {

    FILE *saida = fopen("edf_lvsa.out", "w");

    if (saida == NULL) {
        fprintf(stderr, "Erro: nao foi possivel criar o arquivo de saida.\n");
        return;
    }

    Bloco *blocos = malloc(sizeof(Bloco) * tempo_total);

    if (blocos == NULL) {
        fprintf(stderr, "Erro: falha de memoria.\n");
        return;
    }

    int qtd_blocos = 0;
    int tarefa_anterior = -2;

    for (int tempo = 0; tempo < tempo_total; tempo++) {

        int perdida = verificar_deadlines(tarefas, qtd_tarefas, tempo, tarefa_anterior);

        if (perdida == tarefa_anterior && qtd_blocos > 0) {
            Bloco *anterior = &blocos[qtd_blocos - 1];
            anterior->status = 'L';
        }

        if (tarefa_anterior >= 0 && tarefas[tarefa_anterior].ativa == 0) {
            tarefa_anterior = -2;
        }

        verificar_chegadas(tarefas, qtd_tarefas, tempo);

        int escolhida = escolher_edf(tarefas, qtd_tarefas);

        if (escolhida != tarefa_anterior) {

            if (tarefa_anterior >= 0 && tarefas[tarefa_anterior].ativa == 1) {
                Bloco *anterior = &blocos[qtd_blocos - 1];
                anterior->status = 'H';
            }

            Bloco *b = &blocos[qtd_blocos];

            b->tarefa = escolhida;
            b->duracao = 1;
            b->status = ' ';

            qtd_blocos++;
            tarefa_anterior = escolhida;
        }
        else {
            Bloco *b = &blocos[qtd_blocos - 1];
            b->duracao++;
        }

        if (escolhida != -1) {

            tarefas[escolhida].restante--;

            if (tarefas[escolhida].restante == 0) {

                tarefas[escolhida].completas++;
                tarefas[escolhida].ativa = 0;

                Bloco *b = &blocos[qtd_blocos - 1];
                b->status = 'F';

                tarefa_anterior = -2;
            }
        }
    }

    verificar_deadlines(tarefas, qtd_tarefas, tempo_total, tarefa_anterior);

    for (int i = 0; i < qtd_tarefas; i++) {

        Tarefa *t = &tarefas[i];

        if (t->ativa == 1 && t->restante > 0) {
            t->killed++;
            t->ativa = 0;
            t->restante = 0;
        }
    }

    fprintf(saida, "EXECUTION BY EDF\n");

    for (int i = 0; i < qtd_blocos; i++) {

        Bloco *b = &blocos[i];

        if (b->tarefa == -1) {
            fprintf(saida, "idle for %d units\n", b->duracao);
        }
        else {
            fprintf(saida, "[%s] for %d units - %c\n", tarefas[b->tarefa].nome, b->duracao, b->status);
        }
    }

    fprintf(saida, "LOST DEADLINES\n");

    for (int i = 0; i < qtd_tarefas; i++) {
        Tarefa *t = &tarefas[i];

        fprintf(saida, "[%s] %d\n", t->nome, t->perdidas);
    }

    fprintf(saida, "COMPLETE EXECUTION\n");

    for (int i = 0; i < qtd_tarefas; i++) {
        Tarefa *t = &tarefas[i];

        fprintf(saida, "[%s] %d\n", t->nome, t->completas);
    }

    fprintf(saida, "KILLED\n");

    for (int i = 0; i < qtd_tarefas; i++) {
        Tarefa *t = &tarefas[i];

        fprintf(saida, "[%s] %d\n", t->nome, t->killed);
    }

    free(blocos);
    fclose(saida);
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

    else if (strcmp(argv[1], "edf") == 0) {
    executar_edf(tarefas, qtd_tarefas, tempo_total);
}

    return 0;
}