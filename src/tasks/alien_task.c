#include <stdio.h>
#include "FreeRTOS.h"
#include "task.h"
#include "pico/rand.h"
#include "game.h"
#include "effects_task.h"

#define ALIEN_STEP_X 5
#define ALIEN_STEP_Y 5
#define ALIEN_MOVE_SPEED_DECREMENT 100
#define ALIEN_MOVE_SPEED_MIN 100
#define ENEMY_SHOT_COOLDOWN_MS 100
#define CHANCE_OF_ENEMY_SHOT 3

void alien_logic_task(void *pvParameters) {
    bool move_down_next = false;

    while (1) {
        if (xSemaphoreTake(g_game_state_mutex, pdMS_TO_TICKS(50)) == pdTRUE) {

            if (g_game_state.current_game_internal_state == GAME_PLAYING) {

                if (xTaskGetTickCount() - g_game_state.last_alien_move_time > pdMS_TO_TICKS(g_game_state.current_alien_move_speed_ms)) {
                    g_game_state.last_alien_move_time = xTaskGetTickCount();

                    bool edge_hit = false;

                    if (move_down_next) {
                        for (int r = 0; r < NUM_ALIEN_ROWS; ++r) {
                            for (int c = 0; c < NUM_ALIEN_COLS; ++c) {
                                if (g_game_state.aliens[r][c].active) {
                                    g_game_state.aliens[r][c].y += ALIEN_STEP_Y;

                                    if (g_game_state.aliens[r][c].y + ALIEN_HEIGHT >= g_game_state.player_obj.y) {
                                        g_game_state.current_game_internal_state = GAME_OVER;
                                        effect_send(EFFECT_GAME_OVER);
                                        break;
                                    }
                                }
                            }
                        }

                        g_game_state.alien_dx *= -1;

                        if (g_game_state.current_alien_move_speed_ms > ALIEN_MOVE_SPEED_MIN) {
                            g_game_state.current_alien_move_speed_ms -= ALIEN_MOVE_SPEED_DECREMENT;
                            if (g_game_state.current_alien_move_speed_ms < ALIEN_MOVE_SPEED_MIN)
                                g_game_state.current_alien_move_speed_ms = ALIEN_MOVE_SPEED_MIN;
                        }
                        move_down_next = false;
                    } else {
                        for (int r = 0; r < NUM_ALIEN_ROWS; ++r) {
                            for (int c = 0; c < NUM_ALIEN_COLS; ++c) {
                                if (g_game_state.aliens[r][c].active) {
                                    g_game_state.aliens[r][c].x += g_game_state.alien_dx * ALIEN_STEP_X;
                                }
                            }
                        }

                        int min_x = OLED_WIDTH;
                        int max_x = 0;

                        for (int r = 0; r < NUM_ALIEN_ROWS; ++r) {
                            for (int c = 0; c < NUM_ALIEN_COLS; ++c) {
                                if (g_game_state.aliens[r][c].active) {
                                    if (g_game_state.aliens[r][c].x < min_x)
                                        min_x = g_game_state.aliens[r][c].x;
                                    if (g_game_state.aliens[r][c].x + ALIEN_WIDTH > max_x)
                                        max_x = g_game_state.aliens[r][c].x + ALIEN_WIDTH;
                                }
                            }
                        }

                        if ((g_game_state.alien_dx < 0 && min_x <= 0) ||
                            (g_game_state.alien_dx > 0 && max_x >= OLED_WIDTH)) {
                            edge_hit = true;
                        }
                    }

                    if (edge_hit)
                        move_down_next = true;

                    // Tiro dos alienígenas
                    if (xTaskGetTickCount() - g_game_state.last_enemy_shot_decision_time > pdMS_TO_TICKS(ENEMY_SHOT_COOLDOWN_MS)) {
                        g_game_state.last_enemy_shot_decision_time = xTaskGetTickCount();

                        for (int c = 0; c < NUM_ALIEN_COLS; ++c) {
                            for (int r = NUM_ALIEN_ROWS - 1; r >= 0; --r) {
                                if (g_game_state.aliens[r][c].active) {
                                    if ((get_rand_32() % CHANCE_OF_ENEMY_SHOT) == 0) {
                                        for (int i = 0; i < MAX_ENEMY_BULLETS; ++i) {
                                            if (!g_game_state.enemy_bullets[i].active) {
                                                g_game_state.enemy_bullets[i].active = true;
                                                g_game_state.enemy_bullets[i].x = g_game_state.aliens[r][c].x + (ALIEN_WIDTH / 2);
                                                g_game_state.enemy_bullets[i].y = g_game_state.aliens[r][c].y + ALIEN_HEIGHT;
                                                break;
                                            }
                                        }
                                    }
                                    break;
                                }
                            }
                        }
                    }

                    // Verifica vitória
                    bool all_destroyed = true;
                    for (int r = 0; r < NUM_ALIEN_ROWS; ++r) {
                        for (int c = 0; c < NUM_ALIEN_COLS; ++c) {
                            if (g_game_state.aliens[r][c].active) {
                                all_destroyed = false;
                                break;
                            }
                        }
                    }

                    if (all_destroyed) {
                        g_game_state.current_game_internal_state = GAME_WIN;
                        effect_send(EFFECT_GAME_WIN);
                    }
                }
            }
            xSemaphoreGive(g_game_state_mutex);
        }
        vTaskDelay(pdMS_TO_TICKS(50));
    }
}
