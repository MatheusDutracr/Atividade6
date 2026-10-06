#include "raylib.h"
#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>

#define QUANTIDADE    60
#define PONTUACAO_MAX 100
#define LARGURA       800
#define ALTURA        600
#define BASE_Y        520   /* linha de base das barras */
#define ESCALA        3     /* pixels por ponto */

typedef struct {
    char nome[16];
    int pontuacao;
} Placar;

/* ---------- Vetor dinâmico ---------- */

Placar *criarPlacares(int quantidade) {
    Placar *placares = (Placar *) malloc(quantidade * sizeof(Placar));
    if (!placares) return NULL;

    for (int i = 0; i < quantidade; i++) {
        Placar *p = (placares + i);
        TextCopy(p->nome, TextFormat("J%02d", i + 1));
        p->pontuacao = GetRandomValue(10, PONTUACAO_MAX);
    }
    return placares;
}

Placar *copiarPlacares(Placar *origem, int n) {
    Placar *copia = (Placar *) malloc(n * sizeof(Placar));
    if (!copia) return NULL;

    for (int i = 0; i < n; i++) *(copia + i) = *(origem + i);
    return copia;
}

/* ---------- Ordenação ---------- */

void ordenarBubbleSort(Placar *vetor, int n, long *comparacoes, long *trocas) {
    *comparacoes = 0;
    *trocas = 0;

    for (int i = 0; i < n - 1; i++) {
        for (int j = 0; j < n - 1 - i; j++) {
            (*comparacoes)++;
            if (vetor[j].pontuacao > vetor[j + 1].pontuacao) {
                Placar temp = vetor[j];
                vetor[j] = vetor[j + 1];
                vetor[j + 1] = temp;
                (*trocas)++;
            }
        }
    }
}

/* Exercício 1: insertion sort */
void ordenarInsertionSort(Placar *vetor, int n, long *comparacoes, long *trocas) {
    *comparacoes = 0;
    *trocas = 0;

    for (int i = 1; i < n; i++) {
        Placar chave = vetor[i];
        int j = i - 1;

        while (j >= 0) {
            (*comparacoes)++;                       /* cada teste conta */
            if (vetor[j].pontuacao > chave.pontuacao) {
                vetor[j + 1] = vetor[j];            /* desloca para a direita */
                (*trocas)++;
                j--;
            } else {
                break;                              /* posição encontrada */
            }
        }
        vetor[j + 1] = chave;
    }
}

/* ---------- Buscas ---------- */

int buscaSequencial(Placar *vetor, int n, int alvo, long *comparacoes) {
    *comparacoes = 0;
    for (int i = 0; i < n; i++) {
        (*comparacoes)++;
        if (vetor[i].pontuacao == alvo) return i;
    }
    return -1;
}

int buscaBinaria(Placar *vetor, int n, int alvo, long *comparacoes) {
    *comparacoes = 0;
    int inicio = 0, fim = n - 1;

    while (inicio <= fim) {
        (*comparacoes)++;
        int meio = (inicio + fim) / 2;

        if (vetor[meio].pontuacao == alvo) return meio;
        if (vetor[meio].pontuacao < alvo) inicio = meio + 1;
        else fim = meio - 1;
    }
    return -1;
}

/* ---------- Programa ---------- */

int main(void) {
    InitWindow(LARGURA, ALTURA, "Atividade 7");
    SetTargetFPS(60);

    int n = QUANTIDADE;
    Placar *placares = criarPlacares(n);
    if (!placares) {
        CloseWindow();
        return 1;
    }

    bool ordenado = false;

    /* custo dos dois algoritmos de ordenação (Exercícios 1 e 2) */
    long compBubble = 0, trocasBubble = 0;
    long compInsertion = 0, trocasInsertion = 0;
    double tempoBubble = 0.0, tempoInsertion = 0.0;

    /* busca */
    int alvo = 0, indiceEncontrado = -1;
    long compBusca = 0;
    const char *tipoBusca = "";

    char mensagem[64] = "";
    float tempoMensagem = 0;

    while (!WindowShouldClose()) {

        /* R: novo vetor aleatório */
        if (IsKeyPressed(KEY_R)) {
            free(placares);
            placares = criarPlacares(n);
            if (!placares) break;

            ordenado = false;
            compBubble = trocasBubble = compInsertion = trocasInsertion = 0;
            tempoBubble = tempoInsertion = 0.0;
            indiceEncontrado = -1;
            tipoBusca = "";
        }

        /* B: bubble sort no vetor; insertion sort roda numa cópia do mesmo vetor */
        if (IsKeyPressed(KEY_B)) {
            Placar *copia = copiarPlacares(placares, n);
            if (copia) {
                double inicio = GetTime();
                ordenarBubbleSort(placares, n, &compBubble, &trocasBubble);
                double fim = GetTime();
                tempoBubble = (fim - inicio) * 1000.0;

                inicio = GetTime();
                ordenarInsertionSort(copia, n, &compInsertion, &trocasInsertion);
                fim = GetTime();
                tempoInsertion = (fim - inicio) * 1000.0;

                free(copia);
                ordenado = true;
                indiceEncontrado = -1;
            }
        }

        /* I: insertion sort no vetor; bubble sort roda numa cópia do mesmo vetor */
        if (IsKeyPressed(KEY_I)) {
            Placar *copia = copiarPlacares(placares, n);
            if (copia) {
                double inicio = GetTime();
                ordenarInsertionSort(placares, n, &compInsertion, &trocasInsertion);
                double fim = GetTime();
                tempoInsertion = (fim - inicio) * 1000.0;

                inicio = GetTime();
                ordenarBubbleSort(copia, n, &compBubble, &trocasBubble);
                fim = GetTime();
                tempoBubble = (fim - inicio) * 1000.0;

                free(copia);
                ordenado = true;
                indiceEncontrado = -1;
            }
        }

        /* S: busca sequencial (funciona em qualquer vetor) */
        if (IsKeyPressed(KEY_S)) {
            alvo = placares[GetRandomValue(0, n - 1)].pontuacao;
            indiceEncontrado = buscaSequencial(placares, n, alvo, &compBusca);
            tipoBusca = "Sequencial";
        }

        /* W: busca binária (só em vetor ordenado) */
        if (IsKeyPressed(KEY_W)) {
            if (ordenado) {
                alvo = placares[GetRandomValue(0, n - 1)].pontuacao;
                indiceEncontrado = buscaBinaria(placares, n, alvo, &compBusca);
                tipoBusca = "Binaria";
            } else {
                TextCopy(mensagem, "Ordene o vetor antes (B ou I)!");
                tempoMensagem = 2.0f;
            }
        }

        /* ---------- Desenho ---------- */
        BeginDrawing();
        ClearBackground(RAYWHITE);

        float larguraBarra = (float) LARGURA / n;
        for (int i = 0; i < n; i++) {
            int altura = placares[i].pontuacao * ESCALA;
            Color cor = ordenado ? SKYBLUE : LIGHTGRAY;
            if (i == indiceEncontrado) cor = RED;

            DrawRectangle((int)(i * larguraBarra), BASE_Y - altura,
                          (int) larguraBarra - 1, altura, cor);
        }

        DrawText(TextFormat("Bubble:    %ld comp | %ld trocas | %.4f ms",
                 compBubble, trocasBubble, tempoBubble), 10, 10, 20, DARKGRAY);
        DrawText(TextFormat("Insertion: %ld comp | %ld trocas | %.4f ms",
                 compInsertion, trocasInsertion, tempoInsertion), 10, 35, 20, DARKGRAY);
        DrawText(TextFormat("Vetor: %s", ordenado ? "ordenado" : "desordenado"),
                 10, 60, 20, DARKGRAY);

        if (tipoBusca[0] != '\0') {
            DrawText(TextFormat("Busca %s: alvo=%d | indice=%d | %ld comp",
                     tipoBusca, alvo, indiceEncontrado, compBusca),
                     10, 85, 20, MAROON);
        }

        if (tempoMensagem > 0) {
            tempoMensagem -= GetFrameTime();
            DrawText(mensagem, 10, 540, 22, DARKGREEN);
        }

        DrawText("B=Bubble I=Insertion S=Busca sequencial W=Busca binaria R=Novo vetor",
                 10, 570, 18, DARKGRAY);

        EndDrawing();
    }

    free(placares);
    CloseWindow();
    return 0;
}
