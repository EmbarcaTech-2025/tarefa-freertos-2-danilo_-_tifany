#ifndef PAUSE_H
#define PAUSE_H

#include <stdbool.h>

/**
 * @brief Flag global volátil que indica se o jogo está pausado.
 */
extern volatile bool g_game_paused;

/**
 * @brief Task que monitora o botão de pausa e atualiza a flag g_game_paused.
 * 
 * @param pv Parâmetros da task (não utilizado).
 */
void pause_task(void *pv);

#endif
