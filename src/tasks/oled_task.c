#include <stdio.h>
#include "FreeRTOS.h"
#include "task.h"
#include "game.h"

// Desenha o player
static void draw_player(const GameObject *player) {
    if (player->active)
        ssd1306_draw_char(&oled_display, player->x, PLAYER_Y_POS, 1, '^');
}

// Desenha o tiro do jogador
static void draw_bullet(const GameObject *bullet) {
    if (bullet->active)
        ssd1306_draw_char(&oled_display, bullet->x, bullet->y, 1, '|');
}

// Desenha alien
static void draw_alien(const GameObject *alien) {
    if (alien->active)
        ssd1306_draw_char(&oled_display, alien->x, alien->y, 1, 'x');
}

// Desenha o tiro do alien
static void draw_enemy_bullet(const GameObject *bullet) {
    if (bullet->active)
        ssd1306_draw_char(&oled_display, bullet->x, bullet->y, 1, '|');
}

void oled_display_task(void *pvParameters) {
    char score_str[20];
    char lives_str[10];

    while (1) {
        ssd1306_clear(&oled_display);

        if (xSemaphoreTake(g_game_state_mutex, pdMS_TO_TICKS(50)) == pdTRUE) {
            switch (g_game_state.current_game_internal_state) {

                case GAME_START_SCREEN:
                    ssd1306_draw_string(&oled_display, 10, 20, 1, "BitDog Invaders");
                    ssd1306_draw_string(&oled_display, 10, 35, 1, "Pressione B");
                    break;

                case GAME_PLAYING:
                    draw_player(&g_game_state.player_obj);

                    // Desenha tiros do jogador
                    for (int i = 0; i < MAX_PLAYER_BULLETS; ++i)
                        draw_bullet(&g_game_state.bullets[i]);

                    // Desenha tiros dos inimigos
                    for (int i = 0; i < MAX_ENEMY_BULLETS; ++i)
                        draw_enemy_bullet(&g_game_state.enemy_bullets[i]);

                    // Desenha aliens
                    for (int r = 0; r < NUM_ALIEN_ROWS; ++r)
                        for (int c = 0; c < NUM_ALIEN_COLS; ++c)
                            draw_alien(&g_game_state.aliens[r][c]);

                    sprintf(score_str, "Score: %d", g_game_state.score);
                    ssd1306_draw_string(&oled_display, 0, 0, 1, score_str);
                    sprintf(lives_str, "Vidas: %d", g_game_state.lives);
                    ssd1306_draw_string(&oled_display, OLED_WIDTH - 50, 0, 1, lives_str);
                    break;

                case GAME_OVER:
                    ssd1306_draw_string(&oled_display, 30, 20, 1, "GAME OVER");
                    sprintf(score_str, "Final: %d", g_game_state.score);
                    ssd1306_draw_string(&oled_display, 30, 35, 1, score_str);
                    break;

                case GAME_WIN:
                    ssd1306_draw_string(&oled_display, 25, 20, 1, "VOCE VENCEU!");
                    sprintf(score_str, "Final: %d", g_game_state.score);
                    ssd1306_draw_string(&oled_display, 30, 35, 1, score_str);
                    break;

                default:
                    ssd1306_draw_string(&oled_display, 0, 0, 1, "Estado Desconhecido");
                    break;
            }
            xSemaphoreGive(g_game_state_mutex);
        }

        ssd1306_show(&oled_display);
        vTaskDelay(pdMS_TO_TICKS(33));
    }
}
