#include "game.h"

ssd1306_t oled_display;
GameState_t g_game_state;
SemaphoreHandle_t g_game_state_mutex;
