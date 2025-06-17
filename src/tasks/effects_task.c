#include "pico/stdlib.h"
#include "FreeRTOS.h"
#include "queue.h"
#include "task.h"
#include "effects_task.h"
#include "buzzer.h"
#include "rgb.h"

// Definindo o tamanho da fila
#define EFFECT_QUEUE_LENGTH 10

static QueueHandle_t effects_queue;

void effects_init(void) {
    effects_queue = xQueueCreate(EFFECT_QUEUE_LENGTH, sizeof(effect_event_t));
}

bool effect_send(effect_event_t event) {
    if (effects_queue == NULL)
        return false;

    return (xQueueSend(effects_queue, &event, 0) == pdPASS);
}

static void execute_effect(effect_event_t event) {
    switch (event) {
        case EFFECT_PLAYER_SHOOT:
            buzzer_play(220, 50);
            led_set_color_rgb(1, 1, 1);
            vTaskDelay(pdMS_TO_TICKS(50));
            led_set_color_rgb(0, 0, 0);
            break;

        case EFFECT_ALIEN_HIT:
            buzzer_play(160, 100);
            led_set_color_rgb(1, 1, 0);
            vTaskDelay(pdMS_TO_TICKS(100));
            led_set_color_rgb(0, 0, 0);
            break;

        case EFFECT_PLAYER_HIT:
            buzzer_play(100, 200);
            led_set_color_rgb(1, 0, 1);
            vTaskDelay(pdMS_TO_TICKS(200));
            led_set_color_rgb(0, 0, 0);
            break;

        case EFFECT_GAME_OVER:
            buzzer_play(80, 500);
            led_set_color_rgb(1, 0, 0);
            break;

        case EFFECT_GAME_WIN:
            buzzer_play(880, 500);
            led_set_color_rgb(0, 0, 1);
            break;

        default:
            break;
    }
}

void effects_task(void *pvParameters) {
    effect_event_t received_event;

    while (1) {
        if (xQueueReceive(effects_queue, &received_event, portMAX_DELAY) == pdPASS) {
            execute_effect(received_event);
        }
    }
}
