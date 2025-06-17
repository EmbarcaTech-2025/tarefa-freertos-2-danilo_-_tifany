#include "pico/stdlib.h"
#include "hardware/pwm.h"
#include "hardware/gpio.h"
#include "buzzer.h"

// Ajuste o pino conforme seu hardware
#define BUZZER_PIN 10

static uint slice;

void buzzer_init() {
    gpio_set_function(BUZZER_PIN, GPIO_FUNC_PWM);
    slice = pwm_gpio_to_slice_num(BUZZER_PIN);

    pwm_config config = pwm_get_default_config();
    pwm_config_set_wrap(&config, 20000);  // Valor de wrap fixo (para 1kHz base, por exemplo)
    pwm_config_set_clkdiv(&config, 1.0f); 
    pwm_init(slice, &config, true);

    pwm_set_gpio_level(BUZZER_PIN, 0);  // Começa desligado
}

void buzzer_play(int freq, int duration_ms) {
    if (freq <= 0) {
        pwm_set_gpio_level(BUZZER_PIN, 0);
        return;
    }

    // Calcula o wrap para a frequência desejada
    uint32_t sys_clk = 125000000;  // Clock padrão do RP2040
    uint32_t divider16 = (sys_clk << 4) / (freq * 20000);
    float div_real = divider16 / 16.0f;

    if (div_real < 1.0f) div_real = 1.0f;
    if (div_real > 256.0f) div_real = 256.0f;

    pwm_config config = pwm_get_default_config();
    pwm_config_set_clkdiv(&config, div_real);
    pwm_config_set_wrap(&config, 20000);
    pwm_init(slice, &config, true);

    pwm_set_gpio_level(BUZZER_PIN, 10000); // 50% duty cycle (20000/2)

    sleep_ms(duration_ms);

    pwm_set_gpio_level(BUZZER_PIN, 0);
}
