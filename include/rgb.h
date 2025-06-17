#ifndef LED_H
#define LED_H

typedef enum { RED, GREEN, BLUE } led_color_t;

void led_set_color(led_color_t color);
void led_set_color_rgb(int r, int g, int b);
void led_init(void);

#endif
