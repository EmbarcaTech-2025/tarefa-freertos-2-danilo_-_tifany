#ifndef BUZZER_H
#define BUZZER_H

/**
 * @brief Inicializa o hardware do buzzer (PWM).
 */
void buzzer_init(void);

/**
 * @brief Toca um som no buzzer com uma frequência e duração específicas.
 * 
 * @param freq Frequência do som em Hz.
 * @param duration_ms Duração do som em milissegundos.
 */
void buzzer_play(int freq, int duration_ms);

#endif
