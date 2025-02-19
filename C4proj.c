#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include "pico/stdlib.h"
#include "hardware/pio.h"
#include "hardware/clocks.h"
#include "hardware/adc.h"
#include "pico/bootrom.h"

#include "hardware/i2c.h"
#include "inc/ssd1306.h"
#include "inc/font.h"
#define I2C_PORT i2c1
#define I2C_SDA 14
#define I2C_SCL 15
#define endereco 0x3C

#include "C4proj.pio.h"

#define NUM_PIXELS 25

#define BUZZ 21

#define OUT_PIN 7
#define GREEN_PIN 11
#define BLUE_PIN 12
#define RED_PIN 13

const uint button_A = 5;
const uint button_B = 6;
const uint button_C = 22;

// Vetores para criar os numeros na matriz de LED
double animacao[13][25] = {

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
     1.0, 1.0, 1.0, 1.0, 1.0},  // 9

     {1.0, 1.0, 1.0, 1.0, 1.0,
      1.0, 1.0, 1.0, 1.0, 1.0,
      1.0, 1.0, 1.0, 1.0, 1.0,
      1.0, 1.0, 1.0, 1.0, 1.0,
      1.0, 1.0, 1.0, 1.0, 1.0}, // cabum

     {1.0, 1.0, 1.0, 1.0, 1.0,
        1.0, 1.0, 1.0, 1.0, 1.0,
        1.0, 1.0, 1.0, 1.0, 1.0,
        1.0, 1.0, 1.0, 1.0, 1.0,
        1.0, 1.0, 1.0, 1.0, 1.0}, // clear

     {1.0, 1.0, 1.0, 1.0, 1.0,
        1.0, 1.0, 1.0, 1.0, 1.0,
        1.0, 1.0, 1.0, 1.0, 1.0,
        1.0, 1.0, 1.0, 1.0, 1.0,
        1.0, 1.0, 1.0, 1.0, 1.0}, // saved
};

// Cores para cada quadro da animação
double cores[13][3] = {
    {1.0, 0.0, 0.0},
    {1.0, 0.0, 0.0},
    {1.0, 0.0, 0.0},
    {1.0, 0.0, 0.0},
    {0.0, 1.0, 0.0},
    {0.0, 1.0, 0.0},
    {0.0, 1.0, 0.0},
    {0.0, 1.0, 0.0},
    {0.0, 1.0, 0.0},
    {0.0, 1.0, 0.0},
    {1.0, 0.0, 0.0},
    {0.0, 0.0, 0.0},
    {0.0, 1.0, 0.0}
};
int quadro_atual = 0;

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

void buzzer_on_grave(int tempo){
    while (tempo > 0) {
        gpio_put(BUZZ, true);
        sleep_ms(1);
        tempo --;
        gpio_put(BUZZ, false);
        sleep_ms(10);
        tempo -=3;
    }
}

void buzzer_on(int tempo){
    while (tempo > 0) {
        gpio_put(BUZZ, true);
        sleep_ms(1);
        tempo --;
        gpio_put(BUZZ, false);
        sleep_ms(3);
        tempo -=3;
    }
}


//music
void buzzer_on_agudo(int tempo){
    while (tempo > 0) {
        gpio_put(BUZZ, true);
        sleep_ms(1);
        tempo --;
        gpio_put(BUZZ, false);
        sleep_ms(1);
        tempo -=3;
    }
}

void tocar_musica() {
    // Sequência de tempos para criar a melodia
    int tempos[] = {200, 200, 200, 400, 400, 400, 200, 200, 200, 800};
    int pausas[] = {100, 100, 100, 200, 200, 200, 100, 100, 100, 400};

    for (int i = 0; i < 10; i++) {
        buzzer_on_agudo(tempos[i]);
        sleep_ms(pausas[i]);
    }
}
//////////////

// Função principal
int main() {


    stdio_init_all();
    // I2C Initialisation. Using it at 400Khz.
    i2c_init(I2C_PORT, 400 * 1000);

    gpio_set_function(I2C_SDA, GPIO_FUNC_I2C); // Set the GPIO pin function to I2C
    gpio_set_function(I2C_SCL, GPIO_FUNC_I2C); // Set the GPIO pin function to I2C
    gpio_pull_up(I2C_SDA); // Pull up the data line
    gpio_pull_up(I2C_SCL); // Pull up the clock line
    ssd1306_t ssd; // Inicializa a estrutura do display
    ssd1306_init(&ssd, WIDTH, HEIGHT, false, endereco, I2C_PORT); // Inicializa o display
    ssd1306_config(&ssd); // Configura o display
    ssd1306_send_data(&ssd); // Envia os dados para o display

    // Limpa o display. O display inicia com todos os pixels apagados.
    ssd1306_fill(&ssd, false);
    ssd1306_send_data(&ssd);

    
    PIO pio = pio0;
    bool ok;
    uint32_t valor_led;
    double r = 0.0, b = 0.0, g = 0.0;

    ok = set_sys_clock_khz(128000, false);

    

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

    gpio_init(button_C);
    gpio_set_dir(button_C, GPIO_IN);
    gpio_pull_up(button_C);

    //BUZZ
    gpio_init(BUZZ);
    gpio_set_dir(BUZZ, GPIO_OUT);

    // LED RGB
    gpio_init(RED_PIN);
    gpio_set_dir(RED_PIN, GPIO_OUT);
    gpio_init(GREEN_PIN);
    gpio_set_dir(GREEN_PIN, GPIO_OUT);
    gpio_init(BLUE_PIN);
    gpio_set_dir(BLUE_PIN, GPIO_OUT);

    //!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!
    int tempolimit = 10;
    bool cor = true;
    int tempo;
    int desarmar = 0;
    
    while (true) {
        
        
        //limpar Matrix
        quadro_atual = 11;
        desenho_pio(animacao[quadro_atual], valor_led, pio, sm, cores[quadro_atual][0], cores[quadro_atual][1], cores[quadro_atual][2]);
              
        
        cor = !cor;
        ssd1306_fill(&ssd, !cor); // Limpa o display
        ssd1306_rect(&ssd, 3, 3, 122, 58, cor, !cor); // Desenha um retângulo
        ssd1306_draw_string(&ssd, "C4PROJ", 40, 25);   
        ssd1306_send_data(&ssd);


        if(!gpio_get(button_A)){
            tempolimit = tempolimit - 10;
            
            //timer
            ssd1306_fill(&ssd, !cor);
            ssd1306_rect(&ssd, 3, 3, 122, 58, cor, !cor); // Desenha um retângulo
            char buffer[20];
            sprintf(buffer, "TIME %d", tempolimit);  // Formata a string
            ssd1306_draw_string(&ssd, buffer, 35, 25);  
            ssd1306_send_data(&ssd);
            ////

            buzzer_on_agudo(100);
            sleep_ms(100);

        }else if(!gpio_get(button_B)){ 
            tempolimit = tempolimit + 10;

            //timer
            ssd1306_fill(&ssd, !cor);
            ssd1306_rect(&ssd, 3, 3, 122, 58, cor, !cor); // Desenha um retângulo
            char buffer[20];
            sprintf(buffer, "TIME %d", tempolimit);  // Formata a string
            ssd1306_draw_string(&ssd, buffer, 35, 25);  
            ssd1306_send_data(&ssd);
            ////

            buzzer_on(100);
            sleep_ms(100);
        }

        
        //evitar tempos < 10
        if(tempolimit < 10){
            tempolimit = 10;

            ssd1306_fill(&ssd, !cor); // Limpa o display
            ssd1306_rect(&ssd, 3, 3, 122, 58, cor, !cor); // Desenha um retângulo
            ssd1306_draw_string(&ssd, "ERROR", 40, 25);   
            ssd1306_send_data(&ssd);
            
            gpio_put(RED_PIN, true);
            buzzer_on_grave(500);
            gpio_put(RED_PIN, false);
        }
        
        
        if(!gpio_get(button_C)){

            //start
            ssd1306_fill(&ssd, !cor);
            ssd1306_rect(&ssd, 3, 3, 122, 58, cor, !cor); // Desenha um retângulo
            ssd1306_draw_string(&ssd, "C4 HAS BEEN", 20, 25);
            ssd1306_draw_string(&ssd, "PLANTED", 35, 40);
          
            ssd1306_send_data(&ssd);
            buzzer_on_agudo(1000); 

            bool win = false;
            for (int i = tempolimit; i >= 0; i--) {
                printf("i = %d\n", i);
                
                //timer
                ssd1306_fill(&ssd, !cor);
                ssd1306_rect(&ssd, 3, 3, 122, 58, cor, !cor); // Desenha um retângulo
                char buffer[20];
                sprintf(buffer, "TIME %d", i);  // Formata a string
                ssd1306_draw_string(&ssd, buffer, 35, 25);  
                ssd1306_send_data(&ssd);

                //desarmar
                if(!gpio_get(button_A) && !gpio_get(button_B)){
                    win = true;
                    i = 0;
                }
                
                //init cont final
                if(i < 10){
                    quadro_atual = i;

                    desenho_pio(animacao[quadro_atual], valor_led, pio, sm, cores[quadro_atual][0], cores[quadro_atual][1], cores[quadro_atual][2]);
                };
         
                
                if(i < 10 && i > 5){

                    for (int i = 1; i <= 2; i++) {

                        
                    
                    gpio_put(RED_PIN, true);
                    
                    //buzzer
                    buzzer_on(30);
                    //---------------

                    sleep_ms(250);
                    gpio_put(RED_PIN, false);
                    sleep_ms(250);            
                    }

                }else if (i <= 5){

                    for (int i = 1; i <= 5; i++) {
                    
                        gpio_put(RED_PIN, true);
                        
                        //buzzer
                        buzzer_on(10);
                        //---------------
    
                        sleep_ms(100);
                        gpio_put(RED_PIN, false);
                        sleep_ms(100);            
                        }
                    
                }else{
                    
                    // tempo piscada
                    int tempo_ligado = ((float)i / (float)tempolimit) * 1000;
                    int tempo_desligado = (1-((float)i / (float)tempolimit)) * 1000; //t1 + t2 = 1000

                    gpio_put(RED_PIN, true);
                
                    //buzzer
                    buzzer_on(50);
                    //----------------

                    sleep_ms(tempo_ligado);
                    gpio_put(RED_PIN, false);
                    sleep_ms(tempo_desligado);   
            }   
            
            
            
            };

            if(win == true){
                //win
                ssd1306_fill(&ssd, !cor);
                ssd1306_rect(&ssd, 3, 3, 122, 58, cor, !cor); // Desenha um retângulo
                ssd1306_draw_string(&ssd, "C4 HAS BEEN", 20, 25);
                ssd1306_draw_string(&ssd, "DEFUSED", 35, 40);
              
                ssd1306_send_data(&ssd); 

                quadro_atual = 12;
                desenho_pio(animacao[quadro_atual], valor_led, pio, sm, cores[quadro_atual][0], cores[quadro_atual][1], cores[quadro_atual][2]);
                
                tocar_musica();

            }else{
                //game over
                ssd1306_fill(&ssd, !cor);
                ssd1306_rect(&ssd, 3, 3, 122, 58, cor, !cor); // Desenha um retângulo
                ssd1306_draw_string(&ssd, "GAME OVER", 25, 25);
                ssd1306_send_data(&ssd); 

                quadro_atual = 10;
                desenho_pio(animacao[quadro_atual], valor_led, pio, sm, cores[quadro_atual][0], cores[quadro_atual][1], cores[quadro_atual][2]);
                
                buzzer_on_grave(5000);
            }

            ssd1306_fill(&ssd, !cor);
        }
        
    
    }

}