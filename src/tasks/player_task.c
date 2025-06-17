#include <stdio.h>
#include "FreeRTOS.h"
#include "task.h"
#include "hardware/adc.h"
#include "hardware/gpio.h"
#include "game.h"

#define JOYSTICK_VRX_PIN 27
#define BTN_B_PIN 6
#define PLAYER_MOVE_STEP 1

void player_control_task(void *pvParameters) {
    uint16_t adc_x_raw;
    bool shoot_button_prev = false;
    TickType_t last_shot_time = 0;
    const TickType_t shot_debounce_ms = 250;

    while (1) {
        adc_select_input(JOYSTICK_VRX_PIN - 26);
        adc_x_raw = adc_read();
        bool shoot_button_curr = !gpio_get(BTN_B_PIN);

        if (xSemaphoreTake(g_game_state_mutex, pdMS_TO_TICKS(20)) == pdTRUE) {

            if (g_game_state.current_game_internal_state == GAME_START_SCREEN) {
                if (shoot_button_curr && !shoot_button_prev) {
                    g_game_state.current_game_internal_state = GAME_PLAYING;
                    initialize_game_data_unsafe();
                }
            }
            else if (g_game_state.current_game_internal_state == GAME_PLAYING) {

                // Movimento do player
                int dead_zone = 200;
                if (adc_x_raw < (2048 - dead_zone))
                    g_game_state.player_obj.x -= PLAYER_MOVE_STEP;
                else if (adc_x_raw > (2048 + dead_zone))
                    g_game_state.player_obj.x += PLAYER_MOVE_STEP;

                if (g_game_state.player_obj.x < 0)
                    g_game_state.player_obj.x = 0;
                if (g_game_state.player_obj.x > OLED_WIDTH - PLAYER_WIDTH)
                    g_game_state.player_obj.x = OLED_WIDTH - PLAYER_WIDTH;

                // Lógica de tiro
                TickType_t current_time = xTaskGetTickCount();
                if (shoot_button_curr && !shoot_button_prev &&
                    (current_time - last_shot_time > pdMS_TO_TICKS(shot_debounce_ms))) {

                    last_shot_time = current_time;

                    for (int i = 0; i < MAX_PLAYER_BULLETS; ++i) {
                        if (!g_game_state.bullets[i].active) {
                            g_game_state.bullets[i].active = true;
                            g_game_state.bullets[i].x = g_game_state.player_obj.x + (PLAYER_WIDTH / 2);
                            g_game_state.bullets[i].y = PLAYER_Y_POS - 1;
                            break;
                        }
                    }
                }
            }

            else if (g_game_state.current_game_internal_state == GAME_OVER ||
                     g_game_state.current_game_internal_state == GAME_WIN) {

                if (shoot_button_curr && !shoot_button_prev) {
                    g_game_state.current_game_internal_state = GAME_START_SCREEN;
                }
            }

            xSemaphoreGive(g_game_state_mutex);
        }

        shoot_button_prev = shoot_button_curr;
        vTaskDelay(pdMS_TO_TICKS(20));
    }
}
