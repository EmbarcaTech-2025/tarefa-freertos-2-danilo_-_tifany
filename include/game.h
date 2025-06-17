#ifndef GAME_STATE_H
#define GAME_STATE_H

#include <stdint.h>
#include <stdbool.h>
#include "FreeRTOS.h"
#include "semphr.h"
#include "ssd1306.h"

#define OLED_WIDTH 128
#define OLED_HEIGHT 64
#define PLAYER_WIDTH 5
#define PLAYER_HEIGHT 8
#define PLAYER_Y_POS (OLED_HEIGHT - PLAYER_HEIGHT + 5)
#define ALIEN_WIDTH 5
#define ALIEN_HEIGHT 8
#define MAX_PLAYER_BULLETS 1
#define MAX_ENEMY_BULLETS 4
#define NUM_ALIEN_ROWS 2
#define NUM_ALIEN_COLS 10

typedef enum {
    GAME_START_SCREEN,
    GAME_PLAYING,
    GAME_OVER,
    GAME_WIN
} GameInternalState_e;

typedef struct {
    int x, y;
    bool active;
} GameObject;

typedef struct {
    GameObject player_obj;
    GameObject bullets[MAX_PLAYER_BULLETS];
    GameObject aliens[NUM_ALIEN_ROWS][NUM_ALIEN_COLS];
    GameObject enemy_bullets[MAX_ENEMY_BULLETS];
    int score;
    int lives;
    GameInternalState_e current_game_internal_state;
    int alien_dx;
    uint32_t last_alien_move_time;
    uint32_t current_alien_move_speed_ms;
    uint32_t last_enemy_shot_decision_time;
} GameState_t;

extern ssd1306_t oled_display;
extern GameState_t g_game_state;
extern SemaphoreHandle_t g_game_state_mutex;

void initialize_game_data_unsafe(void);

#endif
