#include <stdio.h>
#include "FreeRTOS.h"
#include "task.h"
#include "game.h"

#define PLAYER_BULLET_SPEED 2
#define ENEMY_BULLET_SPEED 2

void bullet_logic_task(void *pvParameters) {
    while (1) {
        if (xSemaphoreTake(g_game_state_mutex, pdMS_TO_TICKS(20)) == pdTRUE) {
            if (g_game_state.current_game_internal_state == GAME_PLAYING) {

                // Lógica dos tiros do jogador
                for (int i = 0; i < MAX_PLAYER_BULLETS; ++i) {
                    if (g_game_state.bullets[i].active) {
                        g_game_state.bullets[i].y -= PLAYER_BULLET_SPEED;
                        if (g_game_state.bullets[i].y < 0)
                            g_game_state.bullets[i].active = false;

                        for (int r = 0; r < NUM_ALIEN_ROWS; ++r) {
                            for (int c = 0; c < NUM_ALIEN_COLS; ++c) {
                                if (g_game_state.aliens[r][c].active) {
                                    int bx = g_game_state.bullets[i].x;
                                    int by = g_game_state.bullets[i].y;
                                    int ax = g_game_state.aliens[r][c].x;
                                    int ay = g_game_state.aliens[r][c].y;

                                    if (bx >= ax && bx < (ax + ALIEN_WIDTH) &&
                                        by >= ay && by < (ay + ALIEN_HEIGHT)) {
                                        g_game_state.bullets[i].active = false;
                                        g_game_state.aliens[r][c].active = false;
                                        g_game_state.score += 10;
                                        break;
                                    }
                                }
                            }
                        }
                    }
                }

                // Lógica dos tiros inimigos
                for (int i = 0; i < MAX_ENEMY_BULLETS; ++i) {
                    if (g_game_state.enemy_bullets[i].active) {
                        g_game_state.enemy_bullets[i].y += ENEMY_BULLET_SPEED;
                        if (g_game_state.enemy_bullets[i].y > OLED_HEIGHT)
                            g_game_state.enemy_bullets[i].active = false;

                        int bx = g_game_state.enemy_bullets[i].x;
                        int by = g_game_state.enemy_bullets[i].y;
                        int px = g_game_state.player_obj.x;
                        int py = g_game_state.player_obj.y;

                        if (g_game_state.player_obj.active &&
                            bx >= px && bx < (px + PLAYER_WIDTH) &&
                            by >= py && by < (py + PLAYER_HEIGHT)) {
                            g_game_state.enemy_bullets[i].active = false;
                            g_game_state.lives--;
                            if (g_game_state.lives <= 0) {
                                g_game_state.current_game_internal_state = GAME_OVER;
                                g_game_state.player_obj.active = false;
                            }
                        }
                    }
                }
            }
            xSemaphoreGive(g_game_state_mutex);
        }
        vTaskDelay(pdMS_TO_TICKS(30));
    }
}
