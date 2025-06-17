#ifndef PAUSE_H
#define PAUSE_H

#include <stdbool.h>


extern volatile bool g_game_paused;
void pause_task(void *pv);

#endif
