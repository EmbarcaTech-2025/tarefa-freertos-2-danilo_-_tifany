#ifndef EFFECTS_TASK_H
#define EFFECTS_TASK_H

#include <stdbool.h>

typedef enum {
    EFFECT_PLAYER_SHOOT,
    EFFECT_ALIEN_HIT,
    EFFECT_PLAYER_HIT,
    EFFECT_GAME_OVER,
    EFFECT_GAME_WIN
} effect_event_t;

void effects_task(void *pvParameters);
void effects_init(void);
bool effect_send(effect_event_t event);

#endif
