#ifndef GAME_STATE_H
#define GAME_STATE_H

#include <stdint.h>
#include <stdbool.h>
#include "FreeRTOS.h"
#include "semphr.h"
#include "ssd1306.h"

// Definições de dimensões e constantes do jogo
#define OLED_WIDTH 128
#define OLED_HEIGHT 64
#define PLAYER_WIDTH 5
#define PLAYER_HEIGHT 8
#define PLAYER_Y_POS (OLED_HEIGHT - PLAYER_HEIGHT + 5)
#define ALIEN_WIDTH 5
#define ALIEN_HEIGHT 8
#define MAX_PLAYER_BULLETS 1
#define MAX_ENEMY_BULLETS 4
#define NUM_ALIEN_ROWS 2
#define NUM_ALIEN_COLS 10

/**
 * @brief Enumeração dos estados internos do jogo.
 */
typedef enum {
    GAME_START_SCREEN,  // Tela inicial
    GAME_PLAYING,       // Jogo em andamento
    GAME_OVER,          // Fim de jogo por derrota
    GAME_WIN            // Fim de jogo por vitória
} GameInternalState_e;

/**
 * @brief Estrutura para representar um objeto no jogo (jogador, alien, tiro).
 */
typedef struct {
    int x, y;       // Posição do objeto
    bool active;    // Status do objeto (ativo ou inativo)
} GameObject;

/**
 * @brief Estrutura principal que contém todo o estado do jogo.
 */
typedef struct {
    GameObject player_obj;                                  // Objeto do jogador
    GameObject bullets[MAX_PLAYER_BULLETS];                 // Tiros do jogador
    GameObject aliens[NUM_ALIEN_ROWS][NUM_ALIEN_COLS];      // Frota de aliens
    GameObject enemy_bullets[MAX_ENEMY_BULLETS];            // Tiros dos inimigos
    int score;                                              // Pontuação atual
    int lives;                                              // Vidas restantes do jogador
    GameInternalState_e current_game_internal_state;        // Estado atual do jogo
    int alien_dx;                                           // Direção do movimento dos aliens
    uint32_t last_alien_move_time;                          // Tempo do último movimento dos aliens
    uint32_t current_alien_move_speed_ms;                   // Velocidade de movimento dos aliens
    uint32_t last_enemy_shot_decision_time;                 // Tempo da última decisão de tiro inimigo
} GameState_t;

// Variáveis globais externas
extern ssd1306_t oled_display;          // Instância do display OLED
extern GameState_t g_game_state;        // Instância global do estado do jogo
extern SemaphoreHandle_t g_game_state_mutex; // Mutex para proteger o acesso ao estado do jogo

/**
 * @brief Inicializa/reseta os dados do jogo para um novo começo.
 * @note Esta função não é segura para threads e deve ser chamada dentro de uma seção crítica (protegida por mutex).
 */
void initialize_game_data_unsafe(void);

#endif
