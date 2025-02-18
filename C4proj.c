#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include "pico/stdlib.h"
#include "hardware/pio.h"
#include "hardware/clocks.h"
#include "hardware/adc.h"
#include "pico/bootrom.h"

#include "C4proj.pio.h"

#define NUM_PIXELS 25

#define OUT_PIN 7
#define GREEN_PIN 11
#define BLUE_PIN 12
#define RED_PIN 13

const uint button_A = 5;
const uint button_B = 6;

// Vetores para criar os numeros na matriz de LED
double animacao[10][25] = {
    {1.0, 1.0, 1.0, 1.0, 1.0,
     1.0, 0.0, 0.0, 0.0, 1.0,
     1.0, 0.0, 0.0, 0.0, 1.0,
     1.0, 0.0, 0.0, 0.0, 1.0,
     1.0, 1.0, 1.0, 1.0, 1.0}, // 0

    {0.0, 0.0, 1.0, 0.0, 0.0,
     0.0, 0.0, 1.0, 1.0, 0.0,
     0.0, 0.0, 1.0, 0.0, 0.0,
     0.0, 0.0, 1.0, 0.0, 0.0,
     0.0, 1.0, 1.0, 1.0, 0.0}, // 1

    {1.0, 1.0, 1.0, 1.0, 1.0,
     1.0, 0.0, 0.0, 0.0, 0.0,
     1.0, 1.0, 1.0, 1.0, 1.0,
     0.0, 0.0, 0.0, 0.0, 1.0,
     1.0, 1.0, 1.0, 1.0, 1.0}, // 2

    {1.0, 1.0, 1.0, 1.0, 1.0,
     1.0, 0.0, 0.0, 0.0, 0.0,
     1.0, 1.0, 1.0, 1.0, 1.0,
     1.0, 0.0, 0.0, 0.0, 0.0,
     1.0, 1.0, 1.0, 1.0, 1.0}, // 3

    {1.0, 0.0, 0.0, 0.0, 1.0,
     1.0, 0.0, 0.0, 0.0, 1.0,
     1.0, 1.0, 1.0, 1.0, 1.0,
     1.0, 0.0, 0.0, 0.0, 0.0,
     0.0, 0.0, 0.0, 0.0, 1.0}, // 4

    {1.0, 1.0, 1.0, 1.0, 1.0,
     0.0, 0.0, 0.0, 0.0, 1.0,
     1.0, 1.0, 1.0, 1.0, 1.0,
     1.0, 0.0, 0.0, 0.0, 0.0,
     1.0, 1.0, 1.0, 1.0, 1.0}, // 5
    
    {1.0, 1.0, 1.0, 1.0, 1.0,
     0.0, 0.0, 0.0, 0.0, 1.0,
     1.0, 1.0, 1.0, 1.0, 1.0,
     1.0, 0.0, 0.0, 0.0, 1.0,
     1.0, 1.0, 1.0, 1.0, 1.0}, // 6

    {1.0, 1.0, 1.0, 1.0, 1.0,
     1.0, 0.0, 0.0, 0.0, 0.0,
     0.0, 0.0, 0.0, 1.0, 0.0,
     0.0, 0.0, 1.0, 0.0, 0.0,
     0.0, 1.0, 0.0, 0.0, 0.0}, // 7

    {1.0, 1.0, 1.0, 1.0, 1.0,
     1.0, 0.0, 0.0, 0.0, 1.0,
     1.0, 1.0, 1.0, 1.0, 1.0,
     1.0, 0.0, 0.0, 0.0, 1.0,
     1.0, 1.0, 1.0, 1.0, 1.0}, // 8

    {1.0, 1.0, 1.0, 1.0, 1.0,
     1.0, 0.0, 0.0, 0.0, 1.0,
     1.0, 1.0, 1.0, 1.0, 1.0,
     1.0, 0.0, 0.0, 0.0, 0.0,
     1.0, 1.0, 1.0, 1.0, 1.0}  // 9
};

// Cores para cada quadro da animação
double cores[10][3] = {
    {1.0, 0.0, 0.0},
    {1.0, 0.0, 0.0},
    {1.0, 0.0, 0.0},
    {1.0, 0.0, 0.0},
    {0.0, 1.0, 0.0},
    {0.0, 1.0, 0.0},
    {0.0, 1.0, 0.0},
    {0.0, 1.0, 0.0},
    {0.0, 1.0, 0.0},
    {0.0, 1.0, 0.0}
};

// Variável global para o quadro atual
int quadro_atual = 0;

// Função de debouncing
bool debounce(uint gpio) {
    static uint32_t last_time = 0;
    uint32_t current_time = to_ms_since_boot(get_absolute_time());
    if (current_time - last_time < 200) {
        return false;
    }
    last_time = current_time;
    return true;
}

// Interrupção dos botões
/*
void gpio_irq_handler(uint gpio, uint32_t events) {
    if (!debounce(gpio)) return;

    if (gpio == button_A) {
        quadro_atual = (quadro_atual + 1) % 10;
    } else if (gpio == button_B) {
        quadro_atual = (quadro_atual - 1 + 10) % 10;
    }
}
*/

// Intensidade de cores
uint32_t matrix_rgb(double b, double r, double g) {
    unsigned char R, G, B;
    R = r * 255;
    G = g * 255;
    B = b * 255;
    return (G << 24) | (R << 16) | (B << 8);
}

// Matriz de LEDs WS2812
void desenho_pio(double *desenho, uint32_t valor_led, PIO pio, uint sm, double r, double g, double b) {
    for (int16_t i = 0; i < NUM_PIXELS; i++) {
        if (desenho[24 - i] > 0) {
            valor_led = matrix_rgb(b, r, g);
        } else {
            valor_led = matrix_rgb(0.0, 0.0, 0.0);
        }
        pio_sm_put_blocking(pio, sm, valor_led);
    }
}

// Função principal
int main() {
    PIO pio = pio0;
    bool ok;
    uint32_t valor_led;
    double r = 0.0, b = 0.0, g = 0.0;

    ok = set_sys_clock_khz(128000, false);

    stdio_init_all();

    printf("Iniciando a transmissão PIO\n");
    if (ok) printf("Clock set to %ld\n", clock_get_hz(clk_sys));

    // PIO
    uint offset = pio_add_program(pio, &C4proj_program);
    uint sm = pio_claim_unused_sm(pio, true);
    pio_matrix_program_init(pio, sm, offset, OUT_PIN);

    // Botões
    gpio_init(button_A);
    gpio_set_dir(button_A, GPIO_IN);
    gpio_pull_up(button_A);

    gpio_init(button_B);
    gpio_set_dir(button_B, GPIO_IN);
    gpio_pull_up(button_B);

    // Interrupções dos botões
    /*
    gpio_set_irq_enabled_with_callback(button_A, GPIO_IRQ_EDGE_FALL, true, &gpio_irq_handler);
    gpio_set_irq_enabled_with_callback(button_B, GPIO_IRQ_EDGE_FALL, true, &gpio_irq_handler);
    */

    // LED RGB
    gpio_init(RED_PIN);
    gpio_set_dir(RED_PIN, GPIO_OUT);
    gpio_init(GREEN_PIN);
    gpio_set_dir(GREEN_PIN, GPIO_OUT);
    gpio_init(BLUE_PIN);
    gpio_set_dir(BLUE_PIN, GPIO_OUT);

    //!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!
      
    while (true) {
        bool contagem = true;
    
        

        int tempolimit = 9;
        
        if(contagem){
            for (int i = tempolimit; i >= 0; i--) {
                printf("%d\n", i);
                
                if(i <= 10){
                quadro_atual = i;
                printf("%d\n", quadro_atual);
                desenho_pio(animacao[quadro_atual], valor_led, pio, sm, cores[quadro_atual][0], cores[quadro_atual][1], cores[quadro_atual][2]);
                };

                sleep_ms(500);
                

            
            };

            sleep_ms(1000);
        };
        
    
    };

}