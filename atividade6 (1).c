#include "raylib.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>

#define MAX 30
#define TAM_NOME 16
#define PLACAR "placar.txt"
#define SAVE "save.bin"

typedef enum { JOGADOR, INIMIGO, ITEM } Tipo;

typedef union {
    int dano;
    int valor;
} Extra;

typedef struct {
    Tipo tipo;
    Vector2 pos;
    float raio;
    int vida;
    Color cor;
    Extra extra;
} Entidade;

Entidade *v[MAX];
int total = 0;


/* Cria uma entidade zerada (calloc evita lixo no save.bin) */
Entidade *criarEntidade(Tipo tipo, float x, float y, float raio, Color cor) {
    Entidade *e = calloc(1, sizeof(Entidade));
    if (!e) return NULL;

    e->tipo = tipo;
    e->pos = (Vector2){x, y};
    e->raio = raio;
    e->cor = cor;
    return e;
}

/* Adiciona entidade. Assume a posse: se o vetor estiver cheio, libera. */
bool add(Entidade *e) {
    if (!e) return false;

    if (total >= MAX) {
        free(e);
        return false;
    }

    v[total++] = e;
    return true;
}

/* Libera entidades */
void liberar(void) {
    for (int i = 0; i < total; i++) {
        free(v[i]);
        v[i] = NULL;
    }
    total = 0;
}


/* F5 - salva nome e pontuação (texto, modo "a") */
void salvarPlacar(const char *nome, int pontos) {
    FILE *f = fopen(PLACAR, "a");
    if (!f) return;

    fprintf(f, "%s %d\n", nome, pontos);
    fclose(f);
}

/* Lê maior pontuação (texto, modo "r") */
int melhorPlacar(void) {
    FILE *f = fopen(PLACAR, "r");
    if (!f) return 0;

    char nome[TAM_NOME];
    int valor, melhor = 0;

    while (fscanf(f, "%15s %d", nome, &valor) == 2)
        if (valor > melhor)
            melhor = valor;

    fclose(f);
    return melhor;
}


/* F6 - salva jogo (binário, modo "wb") */
bool salvarJogo(void) {
    FILE *f = fopen(SAVE, "wb");
    if (!f) return false;

    fwrite(&total, sizeof(int), 1, f);

    for (int i = 0; i < total; i++)
        fwrite(v[i], sizeof(Entidade), 1, f);

    fclose(f);
    return true;
}

/* F9 - carrega jogo (binário, modo "rb").
   Só troca o estado atual depois de validar o arquivo inteiro. */
bool carregarJogo(void) {
    FILE *f = fopen(SAVE, "rb");
    if (!f) return false;

    int n;
    Entidade *tmp[MAX] = {0};

    if (fread(&n, sizeof(int), 1, f) != 1 || n < 1 || n > MAX) {
        fclose(f);
        return false;
    }

    for (int i = 0; i < n; i++) {
        tmp[i] = malloc(sizeof(Entidade));

        if (!tmp[i] ||
            fread(tmp[i], sizeof(Entidade), 1, f) != 1 ||
            (i == 0 && tmp[i]->tipo != JOGADOR)) {
            for (int j = 0; j <= i; j++) free(tmp[j]);
            fclose(f);
            return false;
        }
    }
    fclose(f);

    liberar();
    for (int i = 0; i < n; i++) add(tmp[i]);
    return true;
}


/* Coloca um inimigo em posição aleatória */
void reposicionar(Entidade *e) {
    e->pos = (Vector2){
        GetRandomValue(50, 750),
        GetRandomValue(50, 550)
    };
}


int main(void) {

    InitWindow(800, 600, "Atividade 6");
    SetTargetFPS(60);

    /* ---- Exercício 1: pede o nome do jogador ---- */
    char nome[TAM_NOME] = "";
    int len = 0;
    bool digitando = true;

    while (digitando && !WindowShouldClose()) {
        int c = GetCharPressed();
        while (c > 0) {
            /* só caracteres visíveis, sem espaço (fscanf usa %s) */
            if (c > 32 && c < 127 && len < TAM_NOME - 1) {
                nome[len++] = (char)c;
                nome[len] = '\0';
            }
            c = GetCharPressed();
        }

        if (IsKeyPressed(KEY_BACKSPACE) && len > 0)
            nome[--len] = '\0';

        if (IsKeyPressed(KEY_ENTER) && len > 0)
            digitando = false;

        BeginDrawing();
        ClearBackground(RAYWHITE);
        DrawText("Digite seu nome e pressione ENTER:", 200, 240, 24, BLACK);
        DrawText(TextFormat("%s_", nome), 200, 290, 30, BLUE);
        EndDrawing();
    }

    if (digitando) {            /* janela fechada antes de começar */
        CloseWindow();
        return 0;
    }

    int pontos = 0;
    char mensagem[64] = "";
    float tempoMensagem = 0;

    /* Jogador */
    Entidade *jogador = criarEntidade(JOGADOR, 400, 300, 20, BLUE);
    if (!jogador || !add(jogador)) {
        CloseWindow();
        return 1;
    }
    jogador->vida = 3;

    /* Alguns inimigos */
    for (int i = 0; i < 5; i++) {
        Entidade *e = criarEntidade(INIMIGO, 0, 0, 15, RED);
        if (!e) break;
        reposicionar(e);
        e->vida = 1;
        e->extra.dano = 10;     /* pontos ganhos ao encostar */
        add(e);
    }

    int melhor = melhorPlacar();

    while (!WindowShouldClose()) {

        /* Movimento */
        if (IsKeyDown(KEY_RIGHT)) jogador->pos.x += 3;
        if (IsKeyDown(KEY_LEFT))  jogador->pos.x -= 3;
        if (IsKeyDown(KEY_UP))    jogador->pos.y -= 3;
        if (IsKeyDown(KEY_DOWN))  jogador->pos.y += 3;

        /* Mantém o jogador dentro da tela */
        if (jogador->pos.x < jogador->raio) jogador->pos.x = jogador->raio;
        if (jogador->pos.x > 800 - jogador->raio) jogador->pos.x = 800 - jogador->raio;
        if (jogador->pos.y < jogador->raio) jogador->pos.y = jogador->raio;
        if (jogador->pos.y > 600 - jogador->raio) jogador->pos.y = 600 - jogador->raio;

        /* Colisão com inimigos: ganha pontos e o inimigo reaparece */
        for (int i = 1; i < total; i++) {
            if (v[i]->tipo == INIMIGO &&
                CheckCollisionCircles(jogador->pos, jogador->raio,
                                      v[i]->pos, v[i]->raio)) {
                pontos += v[i]->extra.dano;
                reposicionar(v[i]);
            }
        }


        /* F5 - salva placar */
        if (IsKeyPressed(KEY_F5)) {
            salvarPlacar(nome, pontos);
            melhor = melhorPlacar();
            strcpy(mensagem, "Placar salvo!");
            tempoMensagem = 2.0f;
        }

        /* F6 - salva jogo */
        if (IsKeyPressed(KEY_F6)) {
            strcpy(mensagem, salvarJogo() ? "Jogo salvo!"
                                          : "Erro ao salvar o jogo");
            tempoMensagem = 2.0f;
        }

        /* F9 - carrega jogo */
        if (IsKeyPressed(KEY_F9)) {
            if (carregarJogo()) {
                jogador = v[0];
                strcpy(mensagem, "Jogo carregado!");
            } else {
                strcpy(mensagem, "Nenhum save valido encontrado");
            }
            tempoMensagem = 2.0f;
        }

        /* ---- Exercício 2: DELETE apaga o save ---- */
        if (IsKeyPressed(KEY_DELETE)) {
            if (remove(SAVE) == 0)
                strcpy(mensagem, "Save apagado!");
            else
                strcpy(mensagem, "Nenhum save encontrado");
            tempoMensagem = 2.0f;
        }


        BeginDrawing();
        ClearBackground(RAYWHITE);

        /* Desenha entidades */
        for (int i = 0; i < total; i++)
            DrawCircleV(v[i]->pos, v[i]->raio, v[i]->cor);

        DrawText(TextFormat("Nome: %s", nome), 10, 10, 20, BLACK);
        DrawText(TextFormat("Pontos: %d", pontos), 10, 35, 20, BLACK);
        DrawText(TextFormat("Recorde: %d", melhor), 10, 60, 20, BLACK);

        /* Mensagem temporária */
        if (tempoMensagem > 0) {
            tempoMensagem -= GetFrameTime();
            DrawText(mensagem, 10, 520, 22, DARKGREEN);
        }

        DrawText(
            "F5=Placar  F6=Salvar  F9=Carregar  DELETE=Apagar",
            10, 560, 18, DARKGRAY
        );

        EndDrawing();
    }

    liberar();
    CloseWindow();

    return 0;
}
