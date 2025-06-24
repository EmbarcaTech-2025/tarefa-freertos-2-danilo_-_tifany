#ifndef LED_H
#define LED_H

typedef enum { RED, GREEN, BLUE, PURPLE, AQUA, WHITE, OFF } led_color_t;

void led_set_color(led_color_t color);
void led_init(void);

#endif
