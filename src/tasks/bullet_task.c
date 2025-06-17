#include "pico/stdlib.h"
#include "FreeRTOS.h"
#include "task.h"
#include "game.h"
#include "effects_task.h"

#define PLAYER_BULLET_SPEED 2
#define ENEMY_BULLET_SPEED 2

void bullet_logic_task(void *pvParameters)
{
    while (1)
    {
        if (xSemaphoreTake(g_game_state_mutex, pdMS_TO_TICKS(20)) == pdTRUE)
        {
            if (g_game_state.current_game_internal_state == GAME_PLAYING)
            {
                // Movimentação dos tiros do jogador
                for (int i = 0; i < MAX_PLAYER_BULLETS; ++i)
                {
                    if (g_game_state.bullets[i].active)
                    {
                        g_game_state.bullets[i].y -= PLAYER_BULLET_SPEED;
                        if (g_game_state.bullets[i].y < 0)
                            g_game_state.bullets[i].active = false;

                        // Colisão com alienígenas
                        for (int r = 0; r < NUM_ALIEN_ROWS; ++r)
                        {
                            for (int c = 0; c < NUM_ALIEN_COLS; ++c)
                            {
                                if (g_game_state.aliens[r][c].active)
                                {
                                    int bx = g_game_state.bullets[i].x;
                                    int by = g_game_state.bullets[i].y;
                                    int ax = g_game_state.aliens[r][c].x;
                                    int ay = g_game_state.aliens[r][c].y;

                                    if (bx >= ax && bx < (ax + ALIEN_WIDTH) &&
                                        by >= ay && by < (ay + ALIEN_HEIGHT))
                                    {
                                        g_game_state.bullets[i].active = false;
                                        g_game_state.aliens[r][c].active = false;
                                        g_game_state.score += 10;
                                        effect_send(EFFECT_ALIEN_HIT);  // 🔧 Chamada centralizada de efeito
                                        break;
                                    }
                                }
                            }
                            if (!g_game_state.bullets[i].active)
                                break;
                        }
                    }
                }

                // Movimentação dos tiros dos inimigos
                for (int i = 0; i < MAX_ENEMY_BULLETS; ++i)
                {
                    if (g_game_state.enemy_bullets[i].active)
                    {
                        g_game_state.enemy_bullets[i].y += ENEMY_BULLET_SPEED;

                        if (g_game_state.enemy_bullets[i].y > OLED_HEIGHT)
                        {
                            g_game_state.enemy_bullets[i].active = false;
                        }

                        int bex = g_game_state.enemy_bullets[i].x;
                        int bey = g_game_state.enemy_bullets[i].y;
                        int px = g_game_state.player_obj.x;
                        int py = g_game_state.player_obj.y;

                        if (g_game_state.player_obj.active &&
                            bex >= px && bex < (px + PLAYER_WIDTH) &&
                            bey >= py && bey < (py + PLAYER_HEIGHT))
                        {
                            g_game_state.enemy_bullets[i].active = false;
                            g_game_state.lives--;
                            effect_send(EFFECT_PLAYER_HIT);  // 🔧 Chamada centralizada de efeito

                            if (g_game_state.lives <= 0)
                            {
                                g_game_state.current_game_internal_state = GAME_OVER;
                                effect_send(EFFECT_GAME_OVER);  // 🔧 Chamada de efeito de game over
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
