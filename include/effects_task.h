#ifndef EFFECTS_TASK_H
#define EFFECTS_TASK_H

#include <stdbool.h>

/**
 * @brief Enumeração dos possíveis eventos de efeito (som e luz).
 */
typedef enum {
    EFFECT_PLAYER_SHOOT,    // Efeito de tiro do jogador
    EFFECT_ALIEN_HIT,       // Efeito de alien atingido
    EFFECT_PLAYER_HIT,      // Efeito de jogador atingido
    EFFECT_GAME_OVER,       // Efeito de fim de jogo
    EFFECT_GAME_WIN         // Efeito de vitória
} effect_event_t;

/**
 * @brief Task principal que gerencia e executa os efeitos.
 * 
 * @param pvParameters Parâmetros da task (não utilizado).
 */
void effects_task(void *pvParameters);

/**
 * @brief Inicializa a fila de eventos de efeitos.
 */
void effects_init(void);

/**
 * @brief Envia um evento de efeito para a fila para ser processado.
 * 
 * @param event O evento de efeito a ser enviado.
 * @return true se o evento foi enviado com sucesso, false caso contrário.
 */
bool effect_send(effect_event_t event);

#endif
