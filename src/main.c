#include <stdio.h>
#include <string.h> 
#include <stdlib.h> 
#include <time.h>

// Bibliotecas do Raspberry Pi Pico SDK.
#include "pico/stdlib.h"
#include "hardware/gpio.h"
#include "hardware/adc.h"
#include "hardware/i2c.h"
#include "hardware/pwm.h"
#include "hardware/clocks.h"
#include "pico/rand.h"

// Bibliotecas do FreeRTOS.
// Certifique-se de que o FreeRTOS está instalado corretamente no seu projeto no diretório `FreeRTOS`.
// git clone https://github.com/FreeRTOS/FreeRTOS-Kernel.git FreeRTOS
#include "FreeRTOS.h"
#include "task.h"
#include "semphr.h" 
#include "queue.h"  

// Biblioteca para controle do display OLED SSD1306.
#include "ssd1306.h"

/**
 * @file main.c
 * @brief Arquivo principal do jogo "BitDog Invaders", um clone de Space Invaders para Raspberry Pi Pico.
 * Este arquivo contém a lógica principal do jogo, incluindo inicialização de hardware,
 * gerenciamento de estado do jogo, renderização no display OLED e as tarefas FreeRTOS
 * que controlam o jogador, os alienígenas, os tiros e a interface do usuário.
 *
 * O jogo utiliza FreeRTOS para gerenciar múltiplas tarefas concorrentes:
 * - Uma tarefa para atualizar o display OLED.
 * - Uma tarefa para processar a entrada do jogador (joystick e botão).
 * - Uma tarefa para gerenciar a lógica dos tiros (do jogador e dos inimigos).
 * - Uma tarefa para gerenciar a lógica dos alienígenas (movimento e tiros).
 * - Uma tarefa para monitorar o status geral do jogo (atualmente com funcionalidade mínima).
 *
 * A comunicação com o display OLED SSD1306 é feita via I2C. A entrada do joystick
 * é lida através de um conversor Analógico-Digital (ADC).
 */

// --- Definições de Pinos ---

// Define os pinos para a comunicação I2C com o display OLED.
// O display OLED se comunica com o Pico usando o protocolo I2C.
// I2C requer dois pinos: SDA (Serial Data) e SCL (Serial Clock).
#define I2C_SDA_PIN 14    // Pino GPIO14 configurado como SDA para o barramento I2C1.
#define I2C_SCL_PIN 15    // Pino GPIO15 configurado como SCL para o barramento I2C1.
#define I2C_PORT i2c1     // Identificador do hardware I2C a ser usado (o Pico tem i2c0 e i2c1).

// Define o pino ADC conectado ao eixo X do joystick.
// O joystick analógico varia uma tensão, que é lida pelo ADC.
// O pino 27 corresponde ao canal ADC1.
#define JOYSTICK_VRX_PIN 27 // Pino GPIO27 (ADC1) para ler a posição horizontal do joystick.

// Define o pino para o botão de tiro / iniciar jogo.
// Este é um botão digital.
#define BTN_B_PIN 6 // Pino GPIO6 usado para o botão de ação (tiro/iniciar).

// --- Configurações do Display OLED ---

// Constantes específicas para o display OLED SSD1306.
#define OLED_ADDRESS 0x3C // Endereço I2C do display OLED. Este endereço é comum para muitos displays SSD1306.
#define OLED_WIDTH 128    // Largura do display OLED em pixels.
#define OLED_HEIGHT 64    // Altura do display OLED em pixels.

// --- Constantes do Jogo ---
// Estas constantes definem parâmetros fundamentais do jogo,
// como aparência e comportamento dos elementos.

// Constantes do Jogador
#define PLAYER_CHAR '^'     // Caractere usado para representar o jogador no display.
#define PLAYER_WIDTH 5      // Largura aproximada do jogador em pixels.
#define PLAYER_HEIGHT 8     // Altura aproximada do jogador em pixels.
#define PLAYER_Y_POS (OLED_HEIGHT - PLAYER_HEIGHT + 5) // Posição Y (vertical) fixa do jogador na parte inferior da tela.
#define PLAYER_MOVE_STEP 1  // Quantidade de pixels que o jogador se move a cada atualização de controle. Ajuste para mudar a sensibilidade.

// Constantes dos Tiros do Jogador
#define MAX_PLAYER_BULLETS 1   // Número máximo de tiros do jogador que podem estar na tela simultaneamente.
#define PLAYER_BULLET_CHAR '|' // Caractere usado para representar o tiro do jogador.
#define PLAYER_BULLET_SPEED 2  // Velocidade do tiro do jogador em pixels por atualização da lógica do jogo.

// Constantes dos Tiros dos Inimigos (Alienígenas)
#define MAX_ENEMY_BULLETS 4         // Número máximo de tiros de inimigos que podem estar na tela simultaneamente.
#define ENEMY_BULLET_CHAR '|'       // Caractere usado para representar o tiro do inimigo.
#define ENEMY_BULLET_SPEED 2        // Velocidade do tiro do inimigo em pixels por atualização da lógica.
#define ENEMY_SHOT_COOLDOWN_MS 100  // Tempo mínimo (em milissegundos) que um alienígena espera antes de poder decidir atirar novamente. Evita spam de tiros.
#define CHANCE_OF_ENEMY_SHOT 3     // Define a probabilidade de um alienígena atirar. A chance é 1 em CHANCE_OF_ENEMY_SHOT a cada ciclo de decisão de tiro.

// Constantes dos Alienígenas
#define ALIEN_CHAR 'x'        // Caractere usado para representar um alienígena.
#define ALIEN_WIDTH 5         // Largura aproximada do alienígena em pixels.
#define ALIEN_HEIGHT 8        // Altura aproximada do alienígena em pixels.
#define NUM_ALIEN_ROWS 2      // Número de linhas de alienígenas na formação.
#define NUM_ALIEN_COLS 10     // Número de colunas de alienígenas na formação.
#define ALIEN_SPACING_X 4     // Espaçamento horizontal (em pixels) entre alienígenas.
#define ALIEN_SPACING_Y 4     // Espaçamento vertical (em pixels) entre alienígenas.
#define ALIEN_INITIAL_Y 10    // Posição Y (vertical) inicial da primeira linha de alienígenas.
#define ALIEN_MOVE_SPEED_MS_INITIAL 700 // Velocidade inicial de movimento dos alienígenas (intervalo em milissegundos entre movimentos). Maior valor = mais lento.
#define ALIEN_MOVE_SPEED_DECREMENT 100  // Redução no intervalo de movimento (em ms) cada vez que os alienígenas descem. Isso acelera o movimento.
#define ALIEN_MOVE_SPEED_MIN 100       // Velocidade mínima de movimento dos alienígenas (intervalo mínimo em ms).
#define ALIEN_STEP_X 5        // Distância (em pixels) que os alienígenas se movem horizontalmente a cada passo.
#define ALIEN_STEP_Y 5        // Distância (em pixels) que os alienígenas se movem verticalmente quando atingem a borda da tela.

// --- Estruturas de Dados do Jogo ---

/**
 * @brief Estrutura para representar um objeto genérico do jogo.
 * Esta estrutura é usada para representar qualquer entidade no jogo que tenha
 * uma posição (x, y) e um estado de atividade (se está visível/ativo ou não).
 * Exemplos: jogador, tiros, alienígenas.
 */
typedef struct
{
    int x, y;     // Coordenadas (horizontal, vertical) do objeto na tela.
    bool active;  // Estado do objeto: 'true' se ativo/visível, 'false' caso contrário.
} GameObject;

/**
 * @brief Enumeração dos possíveis estados internos do jogo.
 * O jogo opera como uma máquina de estados finitos. Esta enumeração define
 * todos os estados possíveis em que o jogo pode se encontrar.
 */
typedef enum
{
    GAME_START_SCREEN, // Estado: Tela inicial do jogo, aguardando o jogador iniciar.
    GAME_PLAYING,      // Estado: Jogo em andamento, jogador controlando a nave e atirando.
    GAME_OVER,         // Estado: Fim de jogo devido à derrota do jogador (perdeu todas as vidas ou aliens invadiram).
    GAME_WIN           // Estado: Fim de jogo devido à vitória do jogador (todos os aliens destruídos).
} GameInternalState_e;

/**
 * @brief Estrutura principal que armazena todo o estado do jogo.
 * Esta é uma estrutura central que contém todas as informações dinâmicas
 * do jogo. É crucial para o funcionamento e para a comunicação entre as tarefas.
 * O acesso a esta estrutura por múltiplas tarefas é protegido por um mutex.
 */
typedef struct
{
    GameObject player_obj;                               // Objeto representando o jogador (posição, estado).
    GameObject bullets[MAX_PLAYER_BULLETS];              // Array para armazenar os tiros ativos do jogador.
    GameObject aliens[NUM_ALIEN_ROWS][NUM_ALIEN_COLS];   // Matriz 2D para armazenar todos os alienígenas.
    GameObject enemy_bullets[MAX_ENEMY_BULLETS];         // Array para armazenar os tiros ativos dos inimigos.
    int score;                                           // Pontuação atual do jogador.
    int lives;                                           // Número de vidas restantes do jogador.
    GameInternalState_e current_game_internal_state;     // Estado atual da máquina de estados do jogo (ex: GAME_PLAYING).
    int alien_dx;                                        // Direção do movimento horizontal dos alienígenas (+1 para direita, -1 para esquerda).
    uint32_t last_alien_move_time;                       // Timestamp (em ticks do sistema FreeRTOS) do último movimento dos alienígenas. Usado para controlar a velocidade.
    uint32_t current_alien_move_speed_ms;                // Intervalo atual (em milissegundos) entre os movimentos dos alienígenas. Diminui para acelerar o jogo.
    uint32_t last_enemy_shot_decision_time;              // Timestamp da última vez que um inimigo considerou atirar. Usado para o cooldown de tiro inimigo.
} GameState_t;

// --- Variáveis Globais ---
// Variáveis globais são geralmente evitadas, mas aqui são usadas para o estado central do jogo
// e para objetos de hardware/RTOS que precisam ser acessados por múltiplas funções/tarefas.

ssd1306_t oled_display; // Instância da estrutura de controle do display OLED. Usada pela biblioteca ssd1306.h.
GameState_t g_game_state; // Variável global que armazena TODO o estado atual do jogo. Protegida por mutex.
SemaphoreHandle_t g_game_state_mutex; // Handle para o mutex do FreeRTOS. Este mutex protege o acesso concorrente à `g_game_state`.
                                      // É ESSENCIAL para evitar condições de corrida quando múltiplas tarefas leem/escrevem em `g_game_state`.

// Handles das Tarefas FreeRTOS
// Estes handles são variáveis que armazenam um identificador para cada tarefa criada.
// Podem ser usados para controlar tarefas (ex: suspender, resumir, obter status),
// embora neste código sejam usados principalmente para verificar se a criação da tarefa foi bem-sucedida.
TaskHandle_t xOledDisplayTaskHandle = NULL;
TaskHandle_t xPlayerControlTaskHandle = NULL;
TaskHandle_t xBulletLogicTaskHandle = NULL;
TaskHandle_t xAlienLogicTaskHandle = NULL;
TaskHandle_t xGameStatusTaskHandle = NULL;

// --- Funções Auxiliares de Desenho ---
// Estas funções são responsáveis por desenhar os diferentes elementos do jogo no display OLED.
// Elas encapsulam as chamadas à biblioteca ssd1306.

/**
 * @brief Desenha o jogador na tela OLED.
 * @param player Ponteiro para o objeto GameObject que representa o jogador.
 *               A função verifica se o jogador está ativo antes de desenhar.
 */
void draw_player(const GameObject *player)
{
    if (player->active) // Só desenha se o jogador estiver ativo.
    {
        // Usa a função da biblioteca OLED para desenhar um caractere na posição (x, y) do jogador.
        // PLAYER_Y_POS é usado aqui porque a posição Y do jogador é fixa.
        // '1' é o fator de escala do caractere (tamanho normal).
        ssd1306_draw_char(&oled_display, player->x, PLAYER_Y_POS, 1, PLAYER_CHAR);
    }
}

/**
 * @brief Desenha um tiro (do jogador ou inimigo, dependendo do caractere) na tela OLED.
 * @param bullet Ponteiro para o objeto GameObject que representa o tiro.
 *               Verifica se o tiro está ativo antes de desenhar.
 */
void draw_bullet(const GameObject *bullet)
{
    if (bullet->active) // Só desenha se o tiro estiver ativo.
    {
        ssd1306_draw_char(&oled_display, bullet->x, bullet->y, 1, PLAYER_BULLET_CHAR);
    }
}

/**
 * @brief Desenha um alienígena na tela OLED.
 * @param alien Ponteiro para o objeto GameObject que representa o alienígena.
 *              Verifica se o alienígena está ativo antes de desenhar.
 */
void draw_alien(const GameObject *alien)
{
    if (alien->active) // Só desenha se o alienígena estiver ativo.
    {
        ssd1306_draw_char(&oled_display, alien->x, alien->y, 1, ALIEN_CHAR);
    }
}

/**
 * @brief Desenha um tiro inimigo na tela OLED.
 * @param bullet Ponteiro para o objeto GameObject que representa o tiro inimigo.
 *               Verifica se o tiro está ativo antes de desenhar.
 */
void draw_enemy_bullet(const GameObject *bullet)
{
    if (bullet->active) // Só desenha se o tiro inimigo estiver ativo.
    {
        ssd1306_draw_char(&oled_display, bullet->x, bullet->y, 1, ENEMY_BULLET_CHAR);
    }
}

// --- Inicializações de Periféricos ---
// Funções para configurar o hardware necessário para o jogo.

/**
 * @brief Inicializa o ADC para leitura do joystick e os pinos GPIO para os botões.
 * Configura o hardware de entrada do jogador.
 */
void init_joystick_and_buttons()
{
    adc_init(); // Inicializa o sistema ADC do Pico.
    adc_gpio_init(JOYSTICK_VRX_PIN); // Configura o pino GPIO especificado (JOYSTICK_VRX_PIN) para ser usado como entrada ADC.

    gpio_init(BTN_B_PIN);             // Inicializa o pino GPIO para o botão.
    gpio_set_dir(BTN_B_PIN, GPIO_IN); // Configura o pino do botão como entrada.
    gpio_pull_up(BTN_B_PIN);          // Habilita o resistor de pull-up interno para o pino do botão.
                                      // Isso significa que o pino lerá 'alto' (1) quando o botão não estiver pressionado
                                      // e 'baixo' (0) quando pressionado (se o botão conectar ao GND).
}

/**
 * @brief Inicializa a comunicação I2C e o display OLED.
 * Configura o hardware de saída visual do jogo.
 */
void init_oled()
{
    // Inicializa o barramento I2C especificado (I2C_PORT) com uma velocidade de 400 kHz.
    i2c_init(I2C_PORT, 400 * 1000);

    // Configura as funções dos pinos GPIO para I2C.
    // Os pinos SDA e SCL precisam ser explicitamente configurados para operar no modo I2C.
    gpio_set_function(I2C_SDA_PIN, GPIO_FUNC_I2C);
    gpio_set_function(I2C_SCL_PIN, GPIO_FUNC_I2C);

    // Habilita resistores de pull-up internos para os pinos I2C.
    // Isso é frequentemente necessário para a comunicação I2C estável.
    gpio_pull_up(I2C_SDA_PIN);
    gpio_pull_up(I2C_SCL_PIN);

    // Inicializa o driver do display OLED (ssd1306).
    // Passa a estrutura de controle, dimensões, endereço I2C e o barramento I2C.
    if (!ssd1306_init(&oled_display, OLED_WIDTH, OLED_HEIGHT, OLED_ADDRESS, I2C_PORT))
    {
        // Se a inicialização falhar, imprime uma mensagem crítica e trava.
        // Em um produto final, um tratamento de erro mais robusto seria ideal.
        printf("CRITICAL: Falha ao inicializar SSD1306!\n");
        while (1); // Loop infinito para indicar falha crítica.
    }
    ssd1306_clear(&oled_display); // Limpa o buffer do display.
    ssd1306_show(&oled_display);  // Envia o buffer limpo para o display (mostra uma tela vazia).
    printf("OLED Inicializado.\n");
}

// --- Lógica de Inicialização do Jogo ---

/**
 * @brief Configura os dados iniciais para um novo jogo ou para reiniciar o jogo.
 * @note Esta função é chamada de "_unsafe" porque ela modifica diretamente `g_game_state`
 *       e NÃO adquire nem libera o mutex `g_game_state_mutex`.
 *       Portanto, a tarefa que chama esta função DEVE já ter adquirido o mutex
 *       para garantir a segurança dos dados em um ambiente multitarefa.
 *       Esta função não altera o `current_game_internal_state`.
 */
void initialize_game_data_unsafe()
{
    printf("UNSAFE: Inicializando dados do jogo...\n"); // Log para depuração.

    // Configura a posição inicial e estado do jogador.
    g_game_state.player_obj.x = OLED_WIDTH / 2 - PLAYER_WIDTH / 2; // Centraliza o jogador horizontalmente.
    g_game_state.player_obj.y = PLAYER_Y_POS;                      // Posição Y fixa.
    g_game_state.player_obj.active = true;                         // Jogador começa ativo.

    // Desativa todos os tiros do jogador.
    for (int i = 0; i < MAX_PLAYER_BULLETS; ++i)
        g_game_state.bullets[i].active = false;

    // Desativa todos os tiros dos inimigos.
    for (int i = 0; i < MAX_ENEMY_BULLETS; ++i)
        g_game_state.enemy_bullets[i].active = false;

    // Configura a formação inicial dos alienígenas.
    for (int r = 0; r < NUM_ALIEN_ROWS; ++r) // Para cada linha de alienígenas.
    {
        for (int c = 0; c < NUM_ALIEN_COLS; ++c) // Para cada coluna de alienígenas.
        {
            // Calcula a posição X com base na coluna, largura e espaçamento.
            g_game_state.aliens[r][c].x = c * (ALIEN_WIDTH + ALIEN_SPACING_X) + 15; // O '+15' é um offset inicial.
            // Calcula a posição Y com base na linha, altura, espaçamento e posição Y inicial.
            g_game_state.aliens[r][c].y = r * (ALIEN_HEIGHT + ALIEN_SPACING_Y) + ALIEN_INITIAL_Y;
            g_game_state.aliens[r][c].active = true; // Todos os alienígenas começam ativos.
        }
    }

    // Configurações iniciais do movimento dos alienígenas.
    g_game_state.alien_dx = 1; // Começam movendo para a direita (+1).
    g_game_state.last_alien_move_time = xTaskGetTickCount(); // Registra o tempo atual para o próximo movimento.
    g_game_state.current_alien_move_speed_ms = ALIEN_MOVE_SPEED_MS_INITIAL; // Define a velocidade inicial.

    // Reseta a pontuação e as vidas do jogador.
    g_game_state.score = 0;
    g_game_state.lives = 3; // Número padrão de vidas.

    printf("UNSAFE: Dados do jogo inicializados.\n"); // Log para depuração.
}

// --- Tarefas FreeRTOS ---
// O coração da concorrência do jogo. Cada tarefa executa uma parte específica da lógica do jogo.

/**
 * @brief Tarefa responsável por atualizar o display OLED.
 *        Esta tarefa lê o estado atual do jogo (protegido por mutex) e desenha
 *        todos os elementos visuais na tela (jogador, alienígenas, tiros, placar, etc.).
 *        Tenta manter uma taxa de atualização razoável (aproximadamente 30 FPS).
 * @param pvParameters Parâmetros da tarefa (não utilizados neste caso, então é NULL).
 */
void oled_display_task(void *pvParameters)
{
    char score_str[20]; // Buffer para formatar a string da pontuação.
    char lives_str[10]; // Buffer para formatar a string das vidas.
    printf("OLED Display Task Iniciada\n");

    while (1) // Loop infinito da tarefa.
    {
        // Limpa o buffer interno do display antes de desenhar os novos elementos.
        // Isso evita que elementos antigos permaneçam na tela.
        ssd1306_clear(&oled_display);

        // Tenta adquirir o mutex para ler `g_game_state` com segurança.
        // pdMS_TO_TICKS(50) define um timeout de 50ms. Se o mutex não for obtido
        // nesse tempo, a tarefa não bloqueia indefinidamente.
        if (xSemaphoreTake(g_game_state_mutex, pdMS_TO_TICKS(50)) == pdTRUE)
        {
            // Mutex adquirido com sucesso! Agora é seguro ler `g_game_state`.

            // A renderização depende do estado interno atual do jogo.
            switch (g_game_state.current_game_internal_state)
            {
            case GAME_START_SCREEN:
                // Desenha a tela inicial.
                ssd1306_draw_string(&oled_display, 10, 20, 1, "BitDog Invaders");
                ssd1306_draw_string(&oled_display, 10, 35, 1, "Pressione B");
                break;

            case GAME_PLAYING:
                // Desenha todos os elementos do jogo ativo.
                draw_player(&g_game_state.player_obj); // Desenha o jogador.

                // Desenha os tiros do jogador.
                for (int i = 0; i < MAX_PLAYER_BULLETS; ++i)
                    draw_bullet(&g_game_state.bullets[i]);

                // Desenha os tiros dos inimigos.
                for (int i = 0; i < MAX_ENEMY_BULLETS; ++i)
                    draw_enemy_bullet(&g_game_state.enemy_bullets[i]);

                // Desenha todos os alienígenas.
                for (int r = 0; r < NUM_ALIEN_ROWS; ++r)
                {
                    for (int c = 0; c < NUM_ALIEN_COLS; ++c)
                        draw_alien(&g_game_state.aliens[r][c]);
                }

                // Prepara e desenha a pontuação.
                sprintf(score_str, "Score: %d", g_game_state.score);
                ssd1306_draw_string(&oled_display, 0, 0, 1, score_str);

                // Prepara e desenha o número de vidas.
                sprintf(lives_str, "Vidas: %d", g_game_state.lives);
                ssd1306_draw_string(&oled_display, OLED_WIDTH - 50, 0, 1, lives_str); // Posiciona à direita.
                break;

            case GAME_OVER:
                // Desenha a tela de "Game Over".
                ssd1306_draw_string(&oled_display, 30, 20, 1, "GAME OVER");
                sprintf(score_str, "Final: %d", g_game_state.score); // Mostra a pontuação final.
                ssd1306_draw_string(&oled_display, 30, 35, 1, score_str);
                break;

            case GAME_WIN:
                // Desenha a tela de vitória.
                ssd1306_draw_string(&oled_display, 25, 20, 1, "VOCE VENCEU!");
                sprintf(score_str, "Final: %d", g_game_state.score); // Mostra a pontuação final.
                ssd1306_draw_string(&oled_display, 30, 35, 1, score_str);
                break;

            default:
                // Caso o estado do jogo seja desconhecido (erro).
                ssd1306_draw_string(&oled_display, 0, 0, 1, "Estado Desconhecido");
                break;
            }
            // Libera o mutex, permitindo que outras tarefas acessem `g_game_state`.
            xSemaphoreGive(g_game_state_mutex);
        }
        else
        {
            // Não conseguiu obter o mutex dentro do timeout.
            // Isso pode indicar que outra tarefa está segurando o mutex por muito tempo.
            printf("OLED: Mutex timeout!\n");
        }

        // Envia o conteúdo do buffer de desenho para o display OLED físico.
        // Todas as operações de `ssd1306_draw_*` modificam um buffer na memória;
        // `ssd1306_show` é que efetivamente atualiza a tela.
        ssd1306_show(&oled_display);

        // Pausa a tarefa por um curto período.
        // pdMS_TO_TICKS(33) converte 33 milissegundos para ticks do sistema FreeRTOS.
        // Isso resulta em uma taxa de atualização de aproximadamente 1000/33 = 30 FPS.
        // Também permite que outras tarefas de menor prioridade executem.
        vTaskDelay(pdMS_TO_TICKS(33));
    }
}

/**
 * @brief Tarefa responsável por ler a entrada do jogador (joystick e botão de tiro)
 *        e atualizar o estado do jogador e do jogo conforme as ações.
 *        Modifica `g_game_state` (posição do jogador, tiros, estado do jogo),
 *        portanto, usa o mutex para proteção.
 * @param pvParameters Parâmetros da tarefa (não utilizados).
 */
void player_control_task(void *pvParameters)
{
    uint16_t adc_x_raw; // Variável para armazenar o valor bruto lido do ADC (joystick X).
    bool shoot_button_pressed_prev = false; // Estado anterior do botão de tiro. Usado para detectar a borda de subida (pressionamento).
    TickType_t last_shot_time = 0;          // Timestamp do último tiro. Usado para implementar um debounce/cooldown para os tiros.
    const TickType_t shot_debounce_ms = 250; // Intervalo mínimo (em ms) entre tiros do jogador para evitar spam.

    printf("Player Control Task Iniciada\n");

    while (1) // Loop infinito da tarefa.
    {
        // Seleciona o canal ADC para o eixo X do joystick.
        // JOYSTICK_VRX_PIN (GPIO27) corresponde ao ADC input 1. Os inputs ADC são numerados de 0 a N.
        // Se JOYSTICK_VRX_PIN é 26, input é 0. Se 27, input é 1. Se 28, input é 2.
        adc_select_input(JOYSTICK_VRX_PIN - 26); // Subtrai 26 para mapear o número do pino GPIO para o número do canal ADC.
        adc_x_raw = adc_read(); // Lê o valor do ADC (tipicamente 0-4095 para 12 bits).

        // Lê o estado atual do botão de tiro.
        // `!gpio_get(BTN_B_PIN)` é usado porque o pino tem pull-up.
        // Botão não pressionado = pino em HIGH (1). Botão pressionado = pino em LOW (0).
        // Então, `true` significa que o botão está pressionado.
        bool shoot_button_current = !gpio_get(BTN_B_PIN);

        // Tenta adquirir o mutex para modificar `g_game_state`.
        // Timeout de 20ms.
        if (xSemaphoreTake(g_game_state_mutex, pdMS_TO_TICKS(20)) == pdTRUE)
        {
            // Mutex adquirido.

            // Lógica de controle baseada no estado atual do jogo.
            if (g_game_state.current_game_internal_state == GAME_START_SCREEN)
            {
                // Se na tela inicial, verifica se o botão de tiro foi pressionado para iniciar o jogo.
                // `shoot_button_current && !shoot_button_pressed_prev` detecta a borda de subida (momento do pressionamento).
                if (shoot_button_current && !shoot_button_pressed_prev)
                {
                    printf("PLAYER: Botao B pressionado na tela de inicio.\n");
                    g_game_state.current_game_internal_state = GAME_PLAYING; // Muda o estado para "jogando".
                    initialize_game_data_unsafe(); // Prepara os dados para um novo jogo (posições, vidas, etc.).
                                                   // Chamada "_unsafe" porque o mutex já está adquirido.
                    printf("PLAYER: Estado mudado para GAME_PLAYING.\n");
                }
            }
            else if (g_game_state.current_game_internal_state == GAME_PLAYING)
            {
                // Se o jogo está em andamento, processa o movimento e o tiro do jogador.

                // Lógica de movimento do jogador com base no joystick.
                int dead_zone = 200; // Zona morta para o joystick para evitar movimento com pequenas flutuações.
                                     // O valor do ADC (0-4095) tem centro em ~2048.
                if (adc_x_raw < (2048 - dead_zone)) // Joystick movido para a esquerda.
                {
                    g_game_state.player_obj.x -= PLAYER_MOVE_STEP;
                }
                else if (adc_x_raw > (2048 + dead_zone)) // Joystick movido para a direita.
                {
                    g_game_state.player_obj.x += PLAYER_MOVE_STEP;
                }

                // Garante que o jogador permaneça dentro dos limites da tela.
                if (g_game_state.player_obj.x < 0)
                    g_game_state.player_obj.x = 0; // Limite esquerdo.
                if (g_game_state.player_obj.x > OLED_WIDTH - PLAYER_WIDTH) // PLAYER_WIDTH é a largura do caractere do jogador.
                {
                    g_game_state.player_obj.x = OLED_WIDTH - PLAYER_WIDTH; // Limite direito.
                }

                // Lógica de tiro do jogador.
                TickType_t current_time = xTaskGetTickCount(); // Pega o tempo atual do sistema.
                // Verifica se o botão foi pressionado (borda de subida) E se o tempo de debounce passou.
                if (shoot_button_current && !shoot_button_pressed_prev && (current_time - last_shot_time > pdMS_TO_TICKS(shot_debounce_ms)))
                {
                    last_shot_time = current_time; // Atualiza o tempo do último tiro.
                    // Procura um slot de tiro inativo para ativar.
                    for (int i = 0; i < MAX_PLAYER_BULLETS; ++i)
                    {
                        if (!g_game_state.bullets[i].active) // Se encontrou um slot de tiro inativo.
                        {
                            g_game_state.bullets[i].active = true; // Ativa o tiro.
                            // Define a posição inicial do tiro (centro do jogador, um pouco acima).
                            g_game_state.bullets[i].x = g_game_state.player_obj.x + (PLAYER_WIDTH / 2);
                            g_game_state.bullets[i].y = PLAYER_Y_POS - 1; // PLAYER_Y_POS é a base do jogador.
                            break; // Sai do loop, pois apenas um tiro é disparado por vez.
                        }
                    }
                }
            }
            else if (g_game_state.current_game_internal_state == GAME_OVER || g_game_state.current_game_internal_state == GAME_WIN)
            {
                // Se na tela de Game Over ou Vitória, verifica se o botão foi pressionado para voltar à tela inicial.
                if (shoot_button_current && !shoot_button_pressed_prev)
                {
                    printf("PLAYER: Botao B pressionado em GAME_OVER/WIN.\n");
                    g_game_state.current_game_internal_state = GAME_START_SCREEN; // Volta para a tela inicial.
                    // Não precisa chamar initialize_game_data_unsafe() aqui, pois isso será feito
                    // quando o jogador pressionar o botão na tela inicial para começar um novo jogo.
                    printf("PLAYER: Estado mudado para GAME_START_SCREEN.\n");
                }
            }

            // Libera o mutex.
            xSemaphoreGive(g_game_state_mutex);
        }
        else
        {
            // Não conseguiu obter o mutex.
            printf("PLAYER: Mutex timeout!\n"); // Depuração.
        }

        // Atualiza o estado anterior do botão para a próxima iteração (para detecção de borda).
        shoot_button_pressed_prev = shoot_button_current;

        // Pequeno delay para tornar o controle responsivo sem sobrecarregar o CPU.
        // 20ms = 50Hz de taxa de leitura de input.
        vTaskDelay(pdMS_TO_TICKS(20));
    }
}

/**
 * @brief Tarefa responsável pela lógica dos tiros (do jogador e dos inimigos).
 *        Move os tiros, detecta colisões entre tiros e alvos (aliens ou jogador),
 *        e atualiza o estado do jogo (pontuação, vidas, desativação de objetos).
 *        Modifica `g_game_state`, portanto, usa o mutex.
 * @param pvParameters Parâmetros da tarefa (não utilizados).
 */
void bullet_logic_task(void *pvParameters)
{
    printf("Bullet Logic Task Iniciada\n");
    while (1) // Loop infinito da tarefa.
    {
        // Tenta adquirir o mutex. Timeout de 20ms.
        if (xSemaphoreTake(g_game_state_mutex, pdMS_TO_TICKS(20)) == pdTRUE)
        {
            // Mutex adquirido.
            // A lógica dos tiros só é processada se o jogo estiver no estado GAME_PLAYING.
            if (g_game_state.current_game_internal_state == GAME_PLAYING)
            {
                // --- Lógica dos tiros do JOGADOR ---
                for (int i = 0; i < MAX_PLAYER_BULLETS; ++i) // Itera sobre todos os slots de tiro do jogador.
                {
                    if (g_game_state.bullets[i].active) // Processa apenas tiros ativos.
                    {
                        g_game_state.bullets[i].y -= PLAYER_BULLET_SPEED; // Move o tiro para cima.
                        if (g_game_state.bullets[i].y < 0) // Se o tiro saiu da tela por cima.
                        {
                            g_game_state.bullets[i].active = false; // Desativa o tiro.
                        }

                        // Detecção de colisão: tiro do jogador vs. alienígenas.
                        for (int r = 0; r < NUM_ALIEN_ROWS; ++r) // Para cada linha de alienígenas.
                        {
                            for (int c = 0; c < NUM_ALIEN_COLS; ++c) // Para cada coluna de alienígenas.
                            {
                                if (g_game_state.aliens[r][c].active) // Se o alienígena está ativo.
                                {
                                    // Pega as coordenadas do tiro (bx, by) e do alienígena (ax, ay).
                                    int bx = g_game_state.bullets[i].x;
                                    int by = g_game_state.bullets[i].y;
                                    int ax = g_game_state.aliens[r][c].x;
                                    int ay = g_game_state.aliens[r][c].y;

                                    // Verificação de colisão retangular simples.
                                    // O tiro (bx, by) está dentro da caixa delimitadora do alienígena (ax, ay, ALIEN_WIDTH, ALIEN_HEIGHT)?
                                    if (bx >= ax && bx < (ax + ALIEN_WIDTH) &&
                                        by >= ay && by < (ay + ALIEN_HEIGHT))
                                    {
                                        g_game_state.bullets[i].active = false;   // Desativa o tiro.
                                        g_game_state.aliens[r][c].active = false; // Desativa o alienígena atingido.
                                        g_game_state.score += 10;                 // Aumenta a pontuação.

                                        break; // Tiro já colidiu, não precisa checar mais alienígenas para ESTE tiro.
                                    }
                                }
                            }
                            if (!g_game_state.bullets[i].active) // Se o tiro foi desativado (colidiu),
                                break;                           // sai do loop de linhas de alienígenas.
                        }
                    }
                }

                // --- Lógica dos tiros dos INIMIGOS ---
                for (int i = 0; i < MAX_ENEMY_BULLETS; ++i) // Itera sobre todos os slots de tiro dos inimigos.
                {
                    if (g_game_state.enemy_bullets[i].active) // Processa apenas tiros ativos.
                    {
                        g_game_state.enemy_bullets[i].y += ENEMY_BULLET_SPEED; // Move o tiro inimigo para baixo.

                        if (g_game_state.enemy_bullets[i].y > OLED_HEIGHT) // Se o tiro saiu da tela por baixo.
                        {
                            g_game_state.enemy_bullets[i].active = false; // Desativa o tiro.
                        }

                        // Detecção de colisão: tiro inimigo vs. jogador.
                        // Pega as coordenadas do tiro inimigo (bex, bey) e do jogador (px, py).
                        int bex = g_game_state.enemy_bullets[i].x;
                        int bey = g_game_state.enemy_bullets[i].y;
                        int px = g_game_state.player_obj.x;
                        int py = g_game_state.player_obj.y; // PLAYER_Y_POS poderia ser usado aqui também.

                        // Verificação de colisão retangular.
                        // O tiro inimigo (bex, bey) está dentro da caixa delimitadora do jogador (px, py, PLAYER_WIDTH, PLAYER_HEIGHT)?
                        if (g_game_state.player_obj.active && // Jogador só pode ser atingido se estiver ativo.
                            bex >= px && bex < (px + PLAYER_WIDTH) &&
                            bey >= py && bey < (py + PLAYER_HEIGHT)) // A altura do jogador é PLAYER_HEIGHT.
                        {
                            g_game_state.enemy_bullets[i].active = false; // Desativa o tiro inimigo.
                            g_game_state.lives--;                         // Decrementa as vidas do jogador.
                            printf("BULLET: Jogador atingido! Vidas: %d\n", g_game_state.lives);

                            if (g_game_state.lives <= 0) // Se o jogador ficou sem vidas.
                            {
                                g_game_state.current_game_internal_state = GAME_OVER; // Muda o estado para Game Over.
                                g_game_state.player_obj.active = false; // Desativa o jogador.

                                printf("BULLET: GAME OVER por falta de vidas.\n");
                            }
                        }
                    }
                }
            }
            // Libera o mutex.
            xSemaphoreGive(g_game_state_mutex);
        }
        // Pausa a tarefa. 30ms = ~33Hz de atualização da lógica dos tiros.
        vTaskDelay(pdMS_TO_TICKS(30));
    }
}

/**
 * @brief Tarefa responsável pela lógica dos alienígenas.
 *        Controla o movimento horizontal e vertical da formação de alienígenas,
 *        a aceleração gradual deles, a decisão e execução dos tiros dos alienígenas,
 *        e verifica condições de vitória (todos os aliens destruídos) ou derrota
 *        (aliens alcançam a linha do jogador).
 *        Modifica `g_game_state`, portanto, usa o mutex.
 * @param pvParameters Parâmetros da tarefa (não utilizados).
 */
void alien_logic_task(void *pvParameters)
{
    printf("Alien Logic Task Iniciada\n");
    bool move_down_on_next = false; // Flag: 'true' se os aliens devem descer no próximo ciclo de movimento.

    while (1) // Loop infinito da tarefa.
    {
        // Tenta adquirir o mutex. Timeout de 50ms.
        if (xSemaphoreTake(g_game_state_mutex, pdMS_TO_TICKS(50)) == pdTRUE)
        {
            // Mutex adquirido.
            // A lógica dos alienígenas só é processada se o jogo estiver no estado GAME_PLAYING.
            if (g_game_state.current_game_internal_state == GAME_PLAYING)
            {
                // Verifica se é hora de mover os alienígenas com base no tempo e na velocidade atual.
                if (xTaskGetTickCount() - g_game_state.last_alien_move_time > pdMS_TO_TICKS(g_game_state.current_alien_move_speed_ms))
                {
                    g_game_state.last_alien_move_time = xTaskGetTickCount(); // Atualiza o tempo do último movimento.
                    bool edge_hit_this_frame = false; // Flag: 'true' se uma borda da tela foi atingida neste movimento.

                    if (move_down_on_next) // Se os alienígenas devem descer (porque atingiram a borda no movimento anterior).
                    {
                        for (int r = 0; r < NUM_ALIEN_ROWS; ++r)
                        {
                            for (int c = 0; c < NUM_ALIEN_COLS; ++c)
                            {
                                if (g_game_state.aliens[r][c].active)
                                {
                                    g_game_state.aliens[r][c].y += ALIEN_STEP_Y; // Move o alienígena para baixo.
                                    // Verifica se algum alienígena alcançou a linha do jogador.
                                    if (g_game_state.aliens[r][c].y + ALIEN_HEIGHT >= g_game_state.player_obj.y)
                                    {
                                        g_game_state.current_game_internal_state = GAME_OVER; // Fim de jogo.

                                        printf("ALIEN: Aliens invadiram! GAME OVER.\n");
                                        break; // Sai do loop de colunas.
                                    }
                                }
                            }
                            if (g_game_state.current_game_internal_state == GAME_OVER) break; // Sai do loop de linhas.
                        }

                        if (g_game_state.current_game_internal_state == GAME_PLAYING) // Se o jogo não acabou após o movimento para baixo.
                        {
                            g_game_state.alien_dx *= -1; // Inverte a direção horizontal para o próximo movimento.
                            // Aumenta a velocidade dos alienígenas (diminuindo o intervalo de movimento).
                            if (g_game_state.current_alien_move_speed_ms > ALIEN_MOVE_SPEED_MIN)
                            {
                                g_game_state.current_alien_move_speed_ms -= ALIEN_MOVE_SPEED_DECREMENT;
                                if (g_game_state.current_alien_move_speed_ms < ALIEN_MOVE_SPEED_MIN)
                                {
                                    g_game_state.current_alien_move_speed_ms = ALIEN_MOVE_SPEED_MIN; // Limita à velocidade mínima.
                                }
                            }
                        }
                        move_down_on_next = false; // Reseta a flag, pois já desceram.
                    }
                    else // Caso contrário, move horizontalmente.
                    {
                        // Move todos os alienígenas ativos horizontalmente.
                        for (int r = 0; r < NUM_ALIEN_ROWS; ++r)
                        {
                            for (int c = 0; c < NUM_ALIEN_COLS; ++c)
                            {
                                if (g_game_state.aliens[r][c].active)
                                {
                                    g_game_state.aliens[r][c].x += g_game_state.alien_dx * ALIEN_STEP_X;
                                }
                            }
                        }

                        // Encontra as extremidades X (esquerda e direita) do bloco de alienígenas ativos.
                        // Isso é para detectar se o *bloco inteiro* atingiu a borda, não apenas um alienígena individual.
                        int min_alien_x = OLED_WIDTH;   // Inicializa com o máximo possível (borda direita).
                        int max_alien_x_right_edge = 0; // Inicializa com o mínimo possível (borda esquerda).

                        for (int r = 0; r < NUM_ALIEN_ROWS; ++r)
                        {
                            for (int c = 0; c < NUM_ALIEN_COLS; ++c)
                            {
                                if (g_game_state.aliens[r][c].active)
                                {
                                    if (g_game_state.aliens[r][c].x < min_alien_x)
                                    {
                                        min_alien_x = g_game_state.aliens[r][c].x; // Alien mais à esquerda.
                                    }
                                    if (g_game_state.aliens[r][c].x + ALIEN_WIDTH > max_alien_x_right_edge)
                                    {
                                        // Borda direita do alien mais à direita.
                                        max_alien_x_right_edge = g_game_state.aliens[r][c].x + ALIEN_WIDTH;
                                    }
                                }
                            }
                        }

                        // Verifica se o bloco de alienígenas atingiu as bordas da tela.
                        if (g_game_state.alien_dx < 0 && min_alien_x <= 0) // Movendo para esquerda e atingiu a borda esquerda.
                        {
                            edge_hit_this_frame = true;
                            int overshoot = 0 - min_alien_x; // Calcula o quanto ultrapassou a borda.
                            if (overshoot > 0) // Se houve ultrapassagem.
                            {
                                // Corrige a posição de todos os aliens para que o bloco fique alinhado com a borda.
                                for (int r_corr = 0; r_corr < NUM_ALIEN_ROWS; ++r_corr)
                                {
                                    for (int c_corr = 0; c_corr < NUM_ALIEN_COLS; ++c_corr)
                                    {
                                        if (g_game_state.aliens[r_corr][c_corr].active)
                                        {
                                            g_game_state.aliens[r_corr][c_corr].x += overshoot;
                                        }
                                    }
                                }
                            }
                        }
                        else if (g_game_state.alien_dx > 0 && max_alien_x_right_edge >= OLED_WIDTH) // Movendo para direita e atingiu a borda direita.
                        {
                            edge_hit_this_frame = true;
                            int overshoot = max_alien_x_right_edge - OLED_WIDTH; // Calcula o quanto ultrapassou.
                            if (overshoot > 0) // Se houve ultrapassagem.
                            {
                                // Corrige a posição.
                                for (int r_corr = 0; r_corr < NUM_ALIEN_ROWS; ++r_corr)
                                {
                                    for (int c_corr = 0; c_corr < NUM_ALIEN_COLS; ++c_corr)
                                    {
                                        if (g_game_state.aliens[r_corr][c_corr].active)
                                        {
                                            g_game_state.aliens[r_corr][c_corr].x -= overshoot;
                                        }
                                    }
                                }
                            }
                        }
                    } // Fim do else (movimento horizontal)

                    if (edge_hit_this_frame && g_game_state.current_game_internal_state == GAME_PLAYING)
                    {
                        move_down_on_next = true; // Prepara para descer no próximo ciclo de movimento.
                    }

                    // --- Lógica de Tiro dos Alienígenas ---
                    // Verifica se passou o cooldown para decisão de tiro.
                    if (g_game_state.current_game_internal_state == GAME_PLAYING &&
                        xTaskGetTickCount() - g_game_state.last_enemy_shot_decision_time > pdMS_TO_TICKS(ENEMY_SHOT_COOLDOWN_MS))
                    {
                        g_game_state.last_enemy_shot_decision_time = xTaskGetTickCount(); // Atualiza o tempo da decisão.

                        int potential_shooters_col[NUM_ALIEN_COLS]; // Array para guardar as colunas com aliens que podem atirar.
                        int num_potential_shooters = 0; // Contador de colunas potenciais.

                        // Identifica colunas que têm alienígenas ativos na linha mais baixa.
                        // Apenas o alienígena mais baixo de uma coluna pode atirar.
                        for (int c = 0; c < NUM_ALIEN_COLS; ++c) { // Para cada coluna.
                            for (int r = NUM_ALIEN_ROWS - 1; r >= 0; --r) { // Verifica da linha mais baixa para cima.
                                if (g_game_state.aliens[r][c].active) {
                                    potential_shooters_col[num_potential_shooters++] = c; // Adiciona a coluna à lista.
                                    break; // Encontrou o alien mais baixo ativo nesta coluna, vai para a próxima coluna.
                                }
                            }
                        }

                        if (num_potential_shooters > 0) { // Se há alienígenas que podem atirar.
                            // Sorteia uma das colunas potenciais para atirar.
                            int shooter_col_idx = get_rand_32() % num_potential_shooters; // Índice aleatório na lista de colunas.
                            int actual_shooter_col = potential_shooters_col[shooter_col_idx]; // A coluna real que vai atirar.

                            // Encontra o alienígena mais baixo na coluna sorteada (ele será o atirador).
                            for (int r = NUM_ALIEN_ROWS - 1; r >= 0; --r) {
                                if (g_game_state.aliens[r][actual_shooter_col].active) {
                                    // Verifica a chance de atirar (ex: 1 em CHANCE_OF_ENEMY_SHOT).
                                    if ((get_rand_32() % CHANCE_OF_ENEMY_SHOT) == 0) {
                                        // Encontra um slot de tiro inimigo inativo.
                                        for (int i = 0; i < MAX_ENEMY_BULLETS; ++i) {
                                            if (!g_game_state.enemy_bullets[i].active) {
                                                g_game_state.enemy_bullets[i].active = true;
                                                // Posição inicial do tiro: abaixo e no centro do alienígena atirador.
                                                g_game_state.enemy_bullets[i].x = g_game_state.aliens[r][actual_shooter_col].x + (ALIEN_WIDTH / 2);
                                                g_game_state.enemy_bullets[i].y = g_game_state.aliens[r][actual_shooter_col].y + ALIEN_HEIGHT;
                                                printf("ALIEN: Alien (%d,%d) atirou!\n", r, actual_shooter_col);

                                                break; // Slot de tiro encontrado e usado.
                                            }
                                        }
                                    }
                                    break; // Apenas um tiro por coluna sorteada por ciclo de decisão.
                                }
                            }
                        }
                    } // Fim da lógica de tiro dos alienígenas.

                    // --- Verifica condição de VITÓRIA ---
                    // Se todos os alienígenas foram destruídos.
                    bool all_aliens_gone = true;
                    if (g_game_state.current_game_internal_state == GAME_PLAYING) {
                        for (int r = 0; r < NUM_ALIEN_ROWS; ++r) {
                            for (int c = 0; c < NUM_ALIEN_COLS; ++c) {
                                if (g_game_state.aliens[r][c].active) { // Se encontrar qualquer alienígena ativo.
                                    all_aliens_gone = false; // Ainda não ganhou.
                                    break;
                                }
                            }
                            if (!all_aliens_gone) break;
                        }
                        if (all_aliens_gone) {
                            g_game_state.current_game_internal_state = GAME_WIN; // Muda o estado para Vitória.

                            printf("ALIEN: Todos os aliens destruidos! VOCE VENCEU.\n");
                        }
                    }
                } // Fim do if (é hora de mover os alienígenas).
            } // Fim do if (GAME_PLAYING).
            // Libera o mutex.
            xSemaphoreGive(g_game_state_mutex);
        }
        // Pausa a tarefa. 50ms = 20Hz de atualização da lógica dos alienígenas.
        // A velocidade real do movimento dos aliens é controlada por `current_alien_move_speed_ms`.
        vTaskDelay(pdMS_TO_TICKS(50));
    }
}

/**
 * @brief Tarefa de status do jogo.
 *        Atualmente, esta tarefa tem funcionalidade mínima (apenas um delay).
 *        Poderia ser expandida no futuro para monitorar o desempenho,
 *        gerenciar níveis de dificuldade, power-ups, ou outros aspectos globais do jogo
 *        que não se encaixam diretamente nas outras tarefas.
 * @param pvParameters Parâmetros da tarefa (não utilizados).
 */
void game_status_task(void *pvParameters)
{
    printf("Game Status Task Iniciada\n");
    while (1)
    {
        // Esta tarefa atualmente não faz muito, apenas cede tempo de CPU.
        // Em um jogo mais complexo, poderia verificar condições globais,
        // como tempo de jogo, ou gerenciar eventos que afetam todo o jogo.
        vTaskDelay(pdMS_TO_TICKS(500)); // Executa a cada 500ms.
    }
}

// --- Função Principal ---
/**
 * @brief Ponto de entrada principal do programa.
 *        Inicializa o sistema, periféricos, FreeRTOS (mutex e tarefas)
 *        e então inicia o escalonador do FreeRTOS.
 * @return int Código de retorno (nunca é alcançado, pois o escalonador FreeRTOS não retorna).
 */
int main()
{
    // Inicializa todas as E/S padrão configuradas (ex: USB serial para printf).
    stdio_init_all();

    // Pequeno delay para permitir que o terminal serial se conecte após o boot/flash.
    // Útil para ver as mensagens de inicialização.
    sleep_ms(2000);
    printf("===================================\n");
    printf("Inicializando BitDog Invaders v0.2\n");
    printf("===================================\n");

    // Inicializa os periféricos de hardware.
    init_joystick_and_buttons(); // Configura ADC e GPIOs para entrada.
    init_oled();                 // Configura I2C e o display OLED.

    // Cria o mutex para proteger o acesso à estrutura global `g_game_state`.
    // Este é um passo CRÍTICO em sistemas multitarefa para evitar corrupção de dados.
    g_game_state_mutex = xSemaphoreCreateMutex();
    if (g_game_state_mutex == NULL) // Verifica se a criação do mutex falhou.
    {
        printf("CRITICAL: Falha ao criar mutex!\n");
        while (1); // Trava o sistema se o mutex não puder ser criado (erro fatal).
    }
    printf("Mutex criado.\n");

    // Configuração inicial do estado e dados do jogo.
    // É importante fazer isso ANTES de iniciar as tarefas que dependem desses dados.
    // Adquire o mutex antes de modificar `g_game_state`. `portMAX_DELAY` significa
    // que a tarefa esperará indefinidamente até que o mutex esteja disponível.
    if (xSemaphoreTake(g_game_state_mutex, portMAX_DELAY) == pdTRUE)
    {
        printf("MAIN: Mutex pego para init.\n");
        // Define o estado inicial do jogo para a tela de início.
        g_game_state.current_game_internal_state = GAME_START_SCREEN;
        // Inicializa as variáveis do jogo (posições, vidas, etc.).
        // A função "_unsafe" é chamada aqui porque o mutex já foi adquirido.
        initialize_game_data_unsafe();
        // Garante que a velocidade inicial dos aliens seja a definida.
        g_game_state.current_alien_move_speed_ms = ALIEN_MOVE_SPEED_MS_INITIAL;

        printf("MAIN: Estado inicial %d. Liberando mutex.\n", g_game_state.current_game_internal_state);
        // Libera o mutex após a inicialização.
        xSemaphoreGive(g_game_state_mutex);
    }
    else
    {
        // Isso não deveria acontecer com portMAX_DELAY, a menos que haja um erro no FreeRTOS.
        printf("CRITICAL: Falha ao pegar mutex no main para init!\n");
        while (1); // Trava.
    }
    printf("Estado e dados do jogo inicializados.\n");

    BaseType_t status; // Variável para armazenar o status de retorno da criação de tarefas.

    // --- Criação das Tarefas FreeRTOS ---
    // Cada tarefa é criada com xTaskCreate, especificando:
    // - A função da tarefa.
    // - Um nome descritivo (para depuração).
    // - O tamanho da pilha (stack size) em palavras (words, não bytes). 1 word = 4 bytes em ARM Cortex-M0+.
    //   É crucial alocar pilha suficiente para evitar stack overflow.
    // - Parâmetros para a tarefa (NULL se não houver).
    // - A prioridade da tarefa (números maiores = maior prioridade).
    // - Um handle para a tarefa (opcional, mas útil para controle futuro).

    // Tarefa do Display OLED: prioridade 2.
    // Stack de 512 words (2048 bytes) é geralmente seguro para tarefas com printf e manipulação de strings/display.
    status = xTaskCreate(oled_display_task, "OLEDTask", 512, NULL, 2, &xOledDisplayTaskHandle);
    if (status != pdPASS)
        printf("Falha ao criar OLEDTask\n");
    else
        printf("OLEDTask criada.\n");

    // Tarefa de Controle do Jogador: prioridade 3 (mais alta que display e lógica, para responsividade).
    // Stack de 256 words (1024 bytes).
    status = xTaskCreate(player_control_task, "PlayerCtrlTask", 256, NULL, 3, &xPlayerControlTaskHandle);
    if (status != pdPASS)
        printf("Falha ao criar PlayerCtrlTask\n");
    else
        printf("PlayerCtrlTask criada.\n");

    // Tarefa da Lógica dos Tiros: prioridade 2.
    // Stack de 256 words.
    status = xTaskCreate(bullet_logic_task, "BulletLogicTask", 256, NULL, 2, &xBulletLogicTaskHandle);
    if (status != pdPASS)
        printf("Falha ao criar BulletLogicTask\n");
    else
        printf("BulletLogicTask criada.\n");

    // Tarefa da Lógica dos Alienígenas: prioridade 2.
    // Stack de 256 words.
    status = xTaskCreate(alien_logic_task, "AlienLogicTask", 256, NULL, 2, &xAlienLogicTaskHandle);
    if (status != pdPASS)
        printf("Falha ao criar AlienLogicTask\n");
    else
        printf("AlienLogicTask criada.\n");

    // Tarefa de Status do Jogo: prioridade 1 (mais baixa).
    // Stack de 128 words (512 bytes), pois faz pouco.
    status = xTaskCreate(game_status_task, "GameStatusTask", 128, NULL, 1, &xGameStatusTaskHandle);
    if (status != pdPASS)
        printf("Falha ao criar GameStatusTask\n");
    else
        printf("GameStatusTask criada.\n");

    printf("Tarefas criadas. Iniciando scheduler...\n");
    // Inicia o escalonador do FreeRTOS.
    // A partir deste ponto, o FreeRTOS toma controle do processador e começa a executar as tarefas
    // de acordo com suas prioridades e o algoritmo de escalonamento.
    // Esta função NUNCA deve retornar.
    vTaskStartScheduler();

    // O código abaixo só seria alcançado se houvesse um erro crítico e o escalonador parasse,
    // ou se não houvesse memória suficiente para a tarefa Idle do FreeRTOS.
    while (true)
        ; // Loop infinito de segurança.
    return 0; // Nunca alcançado.
}