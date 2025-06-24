#ifndef LED_H
#define LED_H

/**
 * @brief Enumeração das cores disponíveis para o LED RGB.
 */
typedef enum { RED, GREEN, BLUE, PURPLE, AQUA, WHITE, OFF } led_color_t;

/**
 * @brief Define a cor do LED RGB.
 * 
 * @param color A cor desejada, conforme a enumeração led_color_t.
 */
void led_set_color(led_color_t color);

/**
 * @brief Inicializa o hardware do LED RGB (PWM).
 */
void led_init(void);

#endif
