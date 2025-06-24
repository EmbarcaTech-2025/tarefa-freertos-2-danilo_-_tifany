#include "pause.h"
#include "hardware/gpio.h"
#include "FreeRTOS.h"
#include "task.h"

volatile bool g_game_paused = false;

// Task responsável pela pausa do jogo
void pause_task(void *pv) {
    bool prev = true, curr;
    const int PIN = 5;
    while (1) {
        curr = !gpio_get(PIN);
        // Verifica transição de nível para alterar estado de pausa
        if (curr && !prev) {
            g_game_paused = !g_game_paused;
        }
        prev = curr;
        vTaskDelay(pdMS_TO_TICKS(50));
    }
}
