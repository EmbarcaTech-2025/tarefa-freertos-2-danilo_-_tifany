#include <stdio.h>
#include "FreeRTOS.h"
#include "task.h"
#include "game.h"

// Esta função é o seu initialize_game_data_unsafe() adaptado
void initialize_game_data_unsafe() {
    g_game_state.player_obj.x = OLED_WIDTH / 2 - PLAYER_WIDTH / 2;
    g_game_state.player_obj.y = PLAYER_Y_POS;
    g_game_state.player_obj.active = true;
    for (int i = 0; i < MAX_PLAYER_BULLETS; ++i)
        g_game_state.bullets[i].active = false;
    for (int i = 0; i < MAX_ENEMY_BULLETS; ++i)
        g_game_state.enemy_bullets[i].active = false;
    for (int r = 0; r < NUM_ALIEN_ROWS; ++r)
        for (int c = 0; c < NUM_ALIEN_COLS; ++c) {
            g_game_state.aliens[r][c].x = c * (ALIEN_WIDTH + 4) + 15;
            g_game_state.aliens[r][c].y = r * (ALIEN_HEIGHT + 4) + 10;
            g_game_state.aliens[r][c].active = true;
        }
    g_game_state.alien_dx = 1;
    g_game_state.last_alien_move_time = xTaskGetTickCount();
    g_game_state.current_alien_move_speed_ms = 700;
    g_game_state.score = 0;
    g_game_state.lives = 3;
}

// Esta função é responsável pela tarefa de status do jogo
void game_status_task(void *pvParameters) {
    while (1)
        vTaskDelay(pdMS_TO_TICKS(500));
}
