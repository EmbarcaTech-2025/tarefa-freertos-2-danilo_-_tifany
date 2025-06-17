#include "pico/stdlib.h"
#include "hardware/gpio.h"
#include "rgb.h"

#define RED_PIN 13
#define GREEN_PIN 11
#define BLUE_PIN 12

void led_init(void) {
    gpio_init(RED_PIN);
    gpio_set_dir(RED_PIN, GPIO_OUT);
    gpio_init(GREEN_PIN);
    gpio_set_dir(GREEN_PIN, GPIO_OUT);
    gpio_init(BLUE_PIN);
    gpio_set_dir(BLUE_PIN, GPIO_OUT);
}

void led_set_color(led_color_t color) {
    gpio_put(RED_PIN, 1);
    gpio_put(GREEN_PIN, 1);
    gpio_put(BLUE_PIN, 1);

    switch (color) {
        case RED:
            gpio_put(RED_PIN, 0);
            break;
        case GREEN:
            gpio_put(GREEN_PIN, 0);
            break;
        case BLUE:
            gpio_put(BLUE_PIN, 0);
            break;
        default:
            break;
    }
}

void led_set_color_rgb(int r, int g, int b) {
    gpio_put(RED_PIN, !r);
    gpio_put(GREEN_PIN, !g);
    gpio_put(BLUE_PIN, !b);
}
