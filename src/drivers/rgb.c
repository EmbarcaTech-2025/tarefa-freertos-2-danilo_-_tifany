#include "pico/stdlib.h"
#include "hardware/gpio.h"
#include "hardware/pwm.h"
#include "rgb.h"

#define RED_PIN 13
#define GREEN_PIN 11
#define BLUE_PIN 12

#define PWM_WRAP 255

static uint slice_red, slice_green, slice_blue;

/*
    Função responsável por inicializar o LED RGB, configurando os pinos
    como saídas PWM e zerando o nível de brilho inicial.
*/
void led_init(void) {
    gpio_set_function(RED_PIN, GPIO_FUNC_PWM);
    gpio_set_function(GREEN_PIN, GPIO_FUNC_PWM);
    gpio_set_function(BLUE_PIN, GPIO_FUNC_PWM);

    slice_red = pwm_gpio_to_slice_num(RED_PIN);
    slice_green = pwm_gpio_to_slice_num(GREEN_PIN);
    slice_blue = pwm_gpio_to_slice_num(BLUE_PIN);

    pwm_config config = pwm_get_default_config();
    pwm_config_set_wrap(&config, PWM_WRAP);
    
    pwm_init(slice_red, &config, true);
    pwm_init(slice_green, &config, true);
    pwm_init(slice_blue, &config, true);

    pwm_set_gpio_level(RED_PIN, 0);
    pwm_set_gpio_level(GREEN_PIN, 0);
    pwm_set_gpio_level(BLUE_PIN, 0);
}

/*
    Função responsável por definir a cor do LED RGB com base no valor
    passado como parâmetro. O brilho de cada cor é ajustado utilizando
    a modulação por largura de pulso (PWM).
*/
void led_set_color(led_color_t color) {
    pwm_set_gpio_level(RED_PIN, 0);
    pwm_set_gpio_level(GREEN_PIN, 0);
    pwm_set_gpio_level(BLUE_PIN, 0);

    uint16_t level = PWM_WRAP / 16;
    uint16_t level_green = 20000 / 16; // Utiliza o mesmo canal do buzzer
    

    switch (color) {
        case RED:
            pwm_set_gpio_level(RED_PIN, level);
            break;
        case GREEN:
            pwm_set_gpio_level(GREEN_PIN, level_green);
            break;
        case BLUE:
            pwm_set_gpio_level(BLUE_PIN, level);
            break;
        case PURPLE:
            pwm_set_gpio_level(RED_PIN, level);
            pwm_set_gpio_level(BLUE_PIN, level);
            break;
        case AQUA:  
            pwm_set_gpio_level(GREEN_PIN, level_green);
            pwm_set_gpio_level(BLUE_PIN, level);
            break;
        case WHITE:
            pwm_set_gpio_level(RED_PIN, level);
            pwm_set_gpio_level(GREEN_PIN, level_green); 
            pwm_set_gpio_level(BLUE_PIN, level);
            break;
        case OFF:
            break;
        default:
            break;
    }
}