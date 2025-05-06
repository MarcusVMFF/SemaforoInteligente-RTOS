#include "pico/stdlib.h"
#include "hardware/gpio.h"
#include "hardware/i2c.h"
#include "lib/ssd1306.h"
#include "lib/font.h"
#include "FreeRTOS.h"
#include "FreeRTOSConfig.h"
#include "task.h"
#include <stdio.h>
#include "blink.pio.h"
#include "hardware/pio.h"
#include "hardware/pwm.h"
#include "hardware/clocks.h"
#include "pico/bootrom.h"
#include "simbolos.h"
#define MATRIXPIO 7

#define I2C_PORT i2c1
#define I2C_SDA 14
#define I2C_SCL 15
#define endereco 0x3C

#define LED_PIN_GREEN 11
#define LED_PIN_RED 13

#define BUZZER_PIN 21

#define BUTTON_A 5
#define BUTTON_B 6

TaskHandle_t xHandleBlink1 = NULL;
TaskHandle_t xHandleBlink2 = NULL;
bool verde = 0;
bool amarelo = 0;
bool vermelho = 0;
bool flagModoNoturno = false;



void iniciar_pwm()
{
    // Configuração dos LEDs (mantida como está)
    gpio_init(LED_PIN_GREEN);
    gpio_set_dir(LED_PIN_GREEN, GPIO_OUT);
    gpio_init(LED_PIN_RED);
    gpio_set_dir(LED_PIN_RED, GPIO_OUT);
    
    gpio_set_function(LED_PIN_GREEN, GPIO_FUNC_PWM);
    gpio_set_function(LED_PIN_RED, GPIO_FUNC_PWM);

    uint slice_led_red = pwm_gpio_to_slice_num(LED_PIN_RED);
    uint slice_led_green = pwm_gpio_to_slice_num(LED_PIN_GREEN);

    pwm_set_clkdiv(slice_led_green, 500.0f);
    pwm_set_wrap(slice_led_green, 255);
    pwm_set_clkdiv(slice_led_red, 500.0f);
    pwm_set_wrap(slice_led_red, 255);

    pwm_set_enabled(slice_led_green, true);
    pwm_set_enabled(slice_led_red, true);

    // Configuração específica para o buzzer
    gpio_init(BUZZER_PIN);
    gpio_set_dir(BUZZER_PIN, GPIO_OUT);
    gpio_set_function(BUZZER_PIN, GPIO_FUNC_PWM);
    
    // Não habilitar o PWM do buzzer ainda - será feito na função beep_buzzer
}

void beep_buzzer(int duration_ms, int freq) {
    uint slice_buzzer = pwm_gpio_to_slice_num(BUZZER_PIN);
    uint channel_buzzer = pwm_gpio_to_channel(BUZZER_PIN);
    
    // Configuração do PWM para o buzzer
    pwm_config config = pwm_get_default_config();
    pwm_config_set_clkdiv(&config, 1.0f); // Divisor de clock 1 (125MHz)
    
    // Calcula o wrap value baseado na frequência desejada
    uint32_t wrap = (125000000 / freq) - 1;
    if (wrap > 65535) wrap = 65535; // Limite máximo para 16-bit
    
    pwm_config_set_wrap(&config, wrap);
    pwm_init(slice_buzzer, &config, true);
    
    // Configura duty cycle para 50%
    pwm_set_chan_level(slice_buzzer, channel_buzzer, wrap / 2);
    
    // Aguarda a duração do beep
    sleep_ms(duration_ms);
    
    // Desliga o buzzer
    pwm_set_chan_level(slice_buzzer, channel_buzzer, 0);
    pwm_set_enabled(slice_buzzer, false);
}

void MatrixPIOTask() {
    PIO pio = pio0;
    uint offset = pio_add_program(pio, &blink_program);
    uint sm = pio_claim_unused_sm(pio, true);
    blink_program_init(pio, sm, offset, MATRIXPIO);
    double *simbolos[3] = {simbolo1, simbolo2, simbolo3};

    while (true) {
        for (int i = 0; i < NUM_PIXELS; i++) {
            double r = 0.0;
            double g = simbolos[2][24 - i];
            double b = 0.0;
            unsigned char R = r * 255;
            unsigned char G = g * 255;
            unsigned char B = b * 255;
            uint32_t valor_led = (G << 24) | (R << 16) | (B << 8);
            pio_sm_put_blocking(pio, sm, valor_led);
        }
        for (int i = 0; i < 10; i += 1)
                vTaskDelay(pdMS_TO_TICKS(100));
    }
}
            
void ModoNormal()
{
    while(true){
        // Verde – 1 beep curto por segundo
        verde = true;
        amarelo = false;
        vermelho = false;
        pwm_set_gpio_level(LED_PIN_GREEN,255);

        beep_buzzer(100, 1000);
        vTaskDelay(pdMS_TO_TICKS(900));
        pwm_set_gpio_level(LED_PIN_GREEN, 0);


        // Amarelo – beeps rápidos intermitentes
        verde = false;
        amarelo = true;
        vermelho = false;
        for (int i = 0; i < 7; i++) {
            pwm_set_gpio_level(LED_PIN_GREEN,255);
            pwm_set_gpio_level(LED_PIN_RED,255);

            beep_buzzer(100, 1500);
            vTaskDelay(pdMS_TO_TICKS(100));
        }
        pwm_set_gpio_level(LED_PIN_GREEN, 0);
        pwm_set_gpio_level(LED_PIN_RED, 0);


        // Vermelho – tom contínuo curto
        verde = false;
        amarelo = false;
        vermelho = true;
        pwm_set_gpio_level(LED_PIN_RED,255);

        beep_buzzer(500, 600);
        vTaskDelay(pdMS_TO_TICKS(1500));
        pwm_set_gpio_level(LED_PIN_RED, 0);

    }
}

void ModoNoturno()
{
    const int delay_step_ms = 20;  // Intervalo entre passos
    const int max_intensity = 255;

    while (true)
    {
        // Efeito de fade-in (do escuro até a intensidade máxima)
        for (int i = 0; i <= max_intensity; i += 5)
        {
            pwm_set_gpio_level(LED_PIN_GREEN, i);
            pwm_set_gpio_level(LED_PIN_RED, i);
            vTaskDelay(pdMS_TO_TICKS(delay_step_ms));
        }

        vTaskDelay(pdMS_TO_TICKS(100)); // Mantém acesso total por um tempo

        // Emitir beep suave no modo noturno
        beep_buzzer(300, 800);  // Beep suave, 100ms, 800Hz
        vTaskDelay(pdMS_TO_TICKS(2000));  // Pausa de 2 segundos

        // Efeito de fade-out (da intensidade máxima até escuro)
        for (int i = max_intensity; i >= 0; i -= 5)
        {
            pwm_set_gpio_level(LED_PIN_GREEN, i);
            pwm_set_gpio_level(LED_PIN_RED, i);
            vTaskDelay(pdMS_TO_TICKS(delay_step_ms));
        }

        vTaskDelay(pdMS_TO_TICKS(100)); // Pausa antes do próximo ciclo
    }
}

void DisplayTask()
{
    // I2C Initialisation. Using it at 400Khz.
    i2c_init(I2C_PORT, 400 * 1000);
    gpio_set_function(I2C_SDA, GPIO_FUNC_I2C);                    // Set the GPIO pin function to I2C
    gpio_set_function(I2C_SCL, GPIO_FUNC_I2C);                    // Set the GPIO pin function to I2C
    gpio_pull_up(I2C_SDA);                                        // Pull up the data line
    gpio_pull_up(I2C_SCL);                                        // Pull up the clock line
    ssd1306_t ssd;                                                // Inicializa a estrutura do display
    ssd1306_init(&ssd, WIDTH, HEIGHT, false, endereco, I2C_PORT); // Inicializa o display
    ssd1306_config(&ssd);                                         // Configura o display
    ssd1306_send_data(&ssd);                                      // Envia os dados para o display
    // Limpa o display. O display inicia com todos os pixels apagados.
    ssd1306_fill(&ssd, false);
    ssd1306_send_data(&ssd);

   
    bool cor = true;
    while (true)
    {
        if(verde && flagModoNoturno == false){
        ssd1306_fill(&ssd, !cor);
        ssd1306_draw_string(&ssd, "LIVRE", 45, 40); // Desenha uma string
        }
        if(amarelo && flagModoNoturno == false){
        ssd1306_fill(&ssd, !cor);
        ssd1306_draw_string(&ssd, "ATENCAO", 35, 40); // Desenha uma string
        }
        if(vermelho && flagModoNoturno == false){
        ssd1306_fill(&ssd, !cor);
        ssd1306_draw_string(&ssd, "PARE", 45, 40); // Desenha uma string
        }
        if(flagModoNoturno == true){
        ssd1306_fill(&ssd, !cor);
        ssd1306_draw_string(&ssd, "MODO NOTURNO", 15, 40); // Desenha uma string
        }
        ssd1306_draw_string(&ssd, "CEPEDI   TIC37", 8, 6); // Desenha uma string
        ssd1306_draw_string(&ssd, "EMBARCATECH", 20, 16);  // Desenha uma string
        ssd1306_rect(&ssd, 3, 3, 122, 60, cor, !cor);
        ssd1306_send_data(&ssd);                           // Atualiza o display
        sleep_ms(735);
    }
}


// Rotina de interrupção para o botão A e botão B com debouncing
uint32_t last_time_sw = 0;  
uint32_t last_time_btn = 0; 
void gpio_irq_handler(uint gpio, uint32_t events) {
    uint32_t current_time = to_us_since_boot(get_absolute_time());
    if (gpio == BUTTON_A && (current_time - last_time_btn > 300000)) {
        last_time_btn = current_time;

        // Alterna flag
        flagModoNoturno = !flagModoNoturno;

        if (flagModoNoturno) {
            vTaskSuspend(xHandleBlink1);
            vTaskResume(xHandleBlink2);
        } else {
            vTaskSuspend(xHandleBlink2);
            vTaskResume(xHandleBlink1);
        }
    }
    if (gpio == BUTTON_B) {
        last_time_btn = current_time;
        reset_usb_boot(0, 0);
    }
}

int main()
{
    stdio_init_all();
    iniciar_pwm();
    gpio_init(BUTTON_B);
    gpio_set_dir(BUTTON_B, GPIO_IN);
    gpio_pull_up(BUTTON_B);
    gpio_init(BUTTON_A);
    gpio_set_dir(BUTTON_A, GPIO_IN);
    gpio_pull_up(BUTTON_A);
    gpio_set_irq_enabled_with_callback(BUTTON_B, GPIO_IRQ_EDGE_FALL, true, &gpio_irq_handler);
    gpio_set_irq_enabled_with_callback(BUTTON_A, GPIO_IRQ_EDGE_FALL, true, &gpio_irq_handler);
    xTaskCreate(ModoNormal, "Blink Task 1", configMINIMAL_STACK_SIZE, NULL, 2, &xHandleBlink1);
    xTaskCreate(ModoNoturno, "Blink Task 2", configMINIMAL_STACK_SIZE, NULL, 2, &xHandleBlink2);
    xTaskCreate(DisplayTask, "DisplayTask", configMINIMAL_STACK_SIZE, NULL, 1, NULL);
    xTaskCreate(MatrixPIOTask, "Matriz Task", configMINIMAL_STACK_SIZE, NULL, 1, NULL);
    vTaskSuspend(xHandleBlink2);
    vTaskStartScheduler();
    panic_unsupported();
}
