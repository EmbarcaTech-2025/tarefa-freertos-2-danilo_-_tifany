#include "pico/stdlib.h"
#include "hardware/gpio.h"
#include "hardware/adc.h"
#include "hardware/i2c.h"
#include "game.h"
#include "buzzer.h"
#include "rgb.h"

#define JOYSTICK_VRX_PIN 27
#define BTN_B_PIN 6
#define I2C_SDA_PIN 14
#define I2C_SCL_PIN 15
#define I2C_PORT i2c1
#define OLED_ADDRESS 0x3C

void init_joystick_and_buttons(void) {
    adc_init();
    adc_gpio_init(JOYSTICK_VRX_PIN);
    gpio_init(BTN_B_PIN);
    gpio_set_dir(BTN_B_PIN, GPIO_IN);
    gpio_pull_up(BTN_B_PIN);
    gpio_init(5);
    gpio_set_dir(5, GPIO_IN);
    gpio_pull_up(5);
}

void init_oled(void) {
    i2c_init(I2C_PORT, 400 * 1000);
    gpio_set_function(I2C_SDA_PIN, GPIO_FUNC_I2C);
    gpio_set_function(I2C_SCL_PIN, GPIO_FUNC_I2C);
    gpio_pull_up(I2C_SDA_PIN);
    gpio_pull_up(I2C_SCL_PIN);
    if (!ssd1306_init(&oled_display, OLED_WIDTH, OLED_HEIGHT, OLED_ADDRESS, I2C_PORT)) {
        while (1);
    }
    ssd1306_clear(&oled_display);
    ssd1306_show(&oled_display);
}

void init_buzzer_rgb(void) {
    led_init();
    buzzer_init();
}
