#include "pause.h"
#include "hardware/gpio.h"
#include "FreeRTOS.h"
#include "task.h"

volatile bool g_game_paused = false;

void pause_task(void *pv) {
    bool prev = true, curr;
    const int PIN = 5;
    while (1) {
        curr = !gpio_get(PIN);
        if (curr && !prev) {
            g_game_paused = !g_game_paused;
        }
        prev = curr;
        vTaskDelay(pdMS_TO_TICKS(50));
    }
}
