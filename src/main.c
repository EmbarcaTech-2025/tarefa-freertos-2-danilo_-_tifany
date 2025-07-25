#include <stdio.h>
#include "pico/stdlib.h"
#include "FreeRTOS.h"
#include "task.h"
#include "game.h"
#include "buzzer.h"
#include "rgb.h"
#include "pause.h"
#include "effects_task.h"

// Protótipos de funções de inicialização e tasks
void init_joystick_and_buttons(void);
void init_oled(void);
void init_buzzer_rgb(void);
void oled_display_task(void *);
void player_control_task(void *);
void bullet_logic_task(void *);
void alien_logic_task(void *);
void game_status_task(void *);
void pause_task(void *);

/**
 * @brief Função principal do programa.
 */
int main() {
    // Inicializa a comunicação serial padrão
    stdio_init_all();
    sleep_ms(1000);

    // Inicializa os periféricos de hardware
    init_joystick_and_buttons();
    init_oled();
    init_buzzer_rgb();
    effects_init();

    // Cria o mutex para proteger o estado do jogo
    g_game_state_mutex = xSemaphoreCreateMutex();
    if (g_game_state_mutex == NULL)
        while (1); // Falha ao criar o mutex

    // Define o estado inicial do jogo de forma segura
    if (xSemaphoreTake(g_game_state_mutex, portMAX_DELAY)) {
        g_game_state.current_game_internal_state = GAME_START_SCREEN;
        initialize_game_data_unsafe();
        xSemaphoreGive(g_game_state_mutex);
    }
    
    // Define uma cor inicial para o LED
    led_set_color(PURPLE);

    // Cria as tasks do FreeRTOS
    xTaskCreate(oled_display_task, "OLED", 512, NULL, 2, NULL);
    xTaskCreate(player_control_task, "Player", 256, NULL, 3, NULL);
    xTaskCreate(bullet_logic_task, "Bullets", 256, NULL, 2, NULL);
    xTaskCreate(alien_logic_task, "Aliens", 256, NULL, 2, NULL);
    xTaskCreate(game_status_task, "Status", 128, NULL, 1, NULL);
    xTaskCreate(pause_task, "Pause", 128, NULL, 3, NULL);
    xTaskCreate(effects_task, "Effects", 512, NULL, 3, NULL);

    // Inicia o escalonador do FreeRTOS
    vTaskStartScheduler();

    // O código nunca deve chegar aqui
    while (1);
}
