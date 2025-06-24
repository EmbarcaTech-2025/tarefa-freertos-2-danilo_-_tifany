#include "game.h"

/**
 * @brief Instância global da estrutura de controle do display OLED.
 */
ssd1306_t oled_display;

/**
 * @brief Instância global da estrutura que contém todo o estado do jogo.
 */
GameState_t g_game_state;

/**
 * @brief Handle para o mutex que protege o acesso à estrutura g_game_state.
 */
SemaphoreHandle_t g_game_state_mutex;
