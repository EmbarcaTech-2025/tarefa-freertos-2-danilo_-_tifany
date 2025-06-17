# 🕹️ Space Invaders com FreeRTOS - Raspberry Pi Pico W

**Tarefa: Roteiro de FreeRTOS #2 - EmbarcaTech 2025**

Autor: **Danilo Oliveira e Tífany Severo**

Curso: Residência Tecnológica em Sistemas Embarcados

Instituição: EmbarcaTech - HBr

Brasília, Junho de 2025

---

## 🎯 Objetivo do Projeto

Este projeto tem como objetivo desenvolver um jogo *Space Invaders* totalmente funcional para a plataforma Raspberry Pi Pico W (BitDogLab), explorando:

- Programação concorrente com **FreeRTOS**;
- Gerenciamento multitarefa em sistemas embarcados;
- Interação com múltiplos periféricos simultâneos;
- Organização modular de código em C;
- Controle de display OLED SSD1306, buzzer piezoelétrico, LED RGB, botões e joystick analógico.

---

## 🔧 Componentes Utilizados

- Raspberry Pi Pico W (RP2040)
- Display OLED SSD1306 (I2C)
- Buzzer Piezoelétrico
- LED RGB (3 pinos independentes)
- Joystick analógico (leitura via ADC)
- Botões físicos (Botão A e Botão Pause)

---

## 📌 Pinagem Utilizada

| Pino RP2040 | Função  | Descrição                 |
|-------------|---------|---------------------------|
| GPIO 10     | PWM     | Saída para o Buzzer       |
| GPIO 13     | GPIO    | LED RGB - RED             |
| GPIO 11     | GPIO    | LED RGB - GREEN           |
| GPIO 12     | GPIO    | LED RGB - BLUE            |
| GPIO 27     | ADC1    | Joystick eixo X           |
| GPIO 14     | I2C SDA | Comunicação com OLED      |
| GPIO 15     | I2C SCL | Comunicação com OLED      |
| GPIO 6      | GPIO    | Botão A (disparo e menu)  |
| GPIO 5      | GPIO    | Botão Pause (pausa do jogo)|

---

## 🧠 Descrição do Funcionamento

- Controle total do jogador com joystick analógico.
- Disparo de tiros com o Botão B.
- Aliens movimentam-se em grupo, descendo e aumentando velocidade gradualmente.
- Tiros do jogador podem acertar os aliens.
- Aliens também disparam aleatoriamente em direção ao jogador.
- Perda de vidas ao ser atingido. Game Over quando as vidas zeram.
- Vitória quando todos os aliens são destruídos.
- Sons e feedbacks visuais com LED RGB sincronizados com o gameplay.

---

## 📸 Demonstração do funcionamento

<video width="720" height="1280" src="https://github.com/user-attachments/assets/d2ec15b0-f799-453d-997e-a3c7e0892bc1.mp4"></video>

---

## 🚀 Importância do FreeRTOS neste Projeto

O uso do **FreeRTOS** foi essencial para possibilitar:

- Dividir responsabilidades em tarefas independentes.
- Permitir o funcionamento concorrente e sincronizado de:
  - Atualização de display OLED;
  - Leitura de botões e joystick;
  - Processamento dos tiros;
  - Lógica dos movimentos dos aliens;
  - Efeitos sonoros e visuais.
- Evitar travamentos ou bloqueios indesejados.
- Manter a atualização fluida da tela, mesmo durante sons e efeitos.

Sem FreeRTOS, seria muito mais complexo gerenciar todos os periféricos em tempo real, mantendo o jogo responsivo.

---

## 🗂 Estrutura Modular do Projeto

```bash
projeto/
│
├── include/
│   ├── buzzer.h
│   ├── rgb.h
│   ├── pause.h
│   ├── game.h
│   ├── effects_task.h
│   └── FreeRTOSConfig.h
│
├── src/
│   ├── drivers/
│   │   ├── buzzer.c
│   │   ├── rgb.c
│   │   └── hardware_init.c
│   │
│   ├── tasks/
│   │   ├── player_task.c
│   │   ├── bullet_task.c
│   │   ├── alien_task.c
│   │   ├── oled_task.c
│   │   ├── pause_task.c
│   │   ├── game_logic.c
│   │   └── effects_task.c
│   │
│   └── game.c
│   └── main.c
│
├── lib/
│   └── ssd1306/ (biblioteca externa para o display OLED)
│
├── FreeRTOS/(biblioteca externa para RTOS)
│
│
└── CMakeLists.txt
```

---

## 📄 O que cada arquivo faz:


* `main.c` — Inicialização geral do sistema e criação das tarefas FreeRTOS.

### Núcleo do jogo (`game.c`, `game_logic.c`, `game.h`)

* Estrutura de dados com o estado global do jogo.
* Controle de score, vidas, posição dos aliens, tiros, etc.

### Drivers (`drivers/`)

* `buzzer.c` / `buzzer.h` — Controle PWM para geração de tons no buzzer.
* `rgb.c` / `rgb.h` — Controle direto dos pinos GPIO para o LED RGB.
* `hardware_init.c` — Inicialização dos periféricos: ADC, I2C, botões e OLED.

### Tasks (`tasks/`)

* `player_task.c` — Leitura do joystick e controle do jogador (movimento e disparo).
* `bullet_task.c` — Controle dos tiros (jogador e aliens) e detecção de colisões.
* `alien_task.c` — Movimento dos aliens e controle da dificuldade.
* `oled_task.c` — Desenho gráfico do estado do jogo no display OLED.
* `pause_task.c` — Leitura do botão de pausa e alternância entre pausar e retomar o jogo.
* `effects_task.c` — Task dedicada para gerenciar os efeitos sonoros e visuais via fila (`Queue`).

### Efeitos assíncronos (`effects_task.c`)

* Recebe comandos das demais tasks via `QueueHandle_t` do FreeRTOS.
* Executa os efeitos de buzzer e LED RGB de forma assíncrona, evitando travamentos de tela.
* Permitiu eliminar as piscadas indesejadas no display durante a execução dos sons.

---

## ⚙️ Como Compilar

Pré-requisitos:

* Raspberry Pi Pico SDK configurado
* FreeRTOS Kernel adicionado ao projeto
* VSCode com extensão CMake Tools
* ARM Toolchain (arm-none-eabi)

Passos:

```bash
git clone <repositório>
cd projeto
export PICO_SDK_PATH=/caminho/para/pico-sdk
mkdir build
cd build
cmake ..
make
```

---

## ▶️ Como Rodar

1. Conecte a Raspberry Pi Pico W no modo BOOTSEL.
2. Copie o arquivo `.uf2` gerado para a placa.
3. O jogo iniciará com a tela inicial aguardando o botão B para começar.

---

## Resultados

O projeto resultou em uma implementação completa e funcional do jogo Space Invaders na plataforma Raspberry Pi Pico W. A utilização do FreeRTOS foi fundamental para alcançar uma jogabilidade fluida e responsiva, com todas as funcionalidades propostas operando de forma concorrente e sincronizada.

- **Gameplay Completo:** O jogo inclui movimentação do jogador via joystick, disparos, hordas de aliens com movimento progressivo, sistema de pontuação e vidas.
- **Multitarefa Eficiente:** A divisão do sistema em tarefas independentes (jogador, aliens, tiros, display, efeitos) garantiu uma execução sem travamentos, mesmo durante a ocorrência de múltiplos eventos simultâneos.
- **Integração de Periféricos:** O display OLED, buzzer, LED RGB, joystick e botões foram integrados com sucesso, proporcionando uma experiência interativa com feedback visual e sonoro em tempo real.
- **Comunicação Assíncrona:** A implementação de uma task de efeitos (`effects_task`) com uma fila (`Queue`) para gerenciar sons e luzes de forma assíncrona foi uma solução eficaz que eliminou gargalos de processamento e piscadas no display.

O resultado final é um sistema embarcado coeso que demonstra a aplicação prática e os benefícios de um RTOS no gerenciamento de aplicações interativas complexas.

---

## 💡 Principais Aprendizados

* Utilização eficiente de FreeRTOS com prioridades e delays cooperativos.
* Uso correto de mutex (`SemaphoreHandle_t`) para proteger o estado global do jogo.
* Gerenciamento de filas (`QueueHandle_t`) para efeitos assíncronos.
* Controle total de hardware embarcado com GPIO, PWM, ADC e I2C.

---




## 📜 Licença
GNU GPL-3.0.
