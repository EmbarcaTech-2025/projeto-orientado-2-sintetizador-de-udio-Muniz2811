#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "pico/stdlib.h"
#include "hardware/adc.h"
#include "hardware/pwm.h"
#include "hardware/dma.h"
#include "hardware/i2c.h"
#include "audio_synthesizer.h"
#include "ssd1306.h"
#include "ssd1306_i2c.h"


static uint16_t audio_buffer[BUFFER_SIZE];
static uint32_t buffer_index = 0;
static uint32_t buffer_length = 0;


static system_state_t current_state = STATE_IDLE;


static absolute_time_t last_sample_time;
static absolute_time_t recording_start_time;


static uint8_t display_buffer[ssd1306_buffer_length];

static struct render_area full_screen = {
    .start_column = 0,
    .end_column = ssd1306_width - 1,
    .start_page = 0,
    .end_page = ssd1306_n_pages - 1
};

// Inicializa os pinos GPIO
void init_gpio() {
    // Inicializa os botões como entrada
    gpio_init(BUTTON_A_PIN);
    gpio_init(BUTTON_B_PIN);
    gpio_set_dir(BUTTON_A_PIN, GPIO_IN);
    gpio_set_dir(BUTTON_B_PIN, GPIO_IN);
    gpio_pull_up(BUTTON_A_PIN);
    gpio_pull_up(BUTTON_B_PIN);

    // Inicializa os LEDs RGB como saída
    gpio_init(RED_LED_PIN);
    gpio_init(GREEN_LED_PIN);
    gpio_init(BLUE_LED_PIN);
    gpio_set_dir(RED_LED_PIN, GPIO_OUT);
    gpio_set_dir(GREEN_LED_PIN, GPIO_OUT);
    gpio_set_dir(BLUE_LED_PIN, GPIO_OUT);

    // Desliga todos os LEDs inicialmente
    gpio_put(RED_LED_PIN, 0);
    gpio_put(GREEN_LED_PIN, 0);
    gpio_put(BLUE_LED_PIN, 0);
}


void init_adc() {
    // Inicializa o ADC
    adc_init();
    
    // Configura o pino do microfone para o ADC
    adc_gpio_init(MIC_PIN);
    
    // Seleciona o canal do microfone
    adc_select_input(MIC_CHANNEL);
    
    adc_set_clkdiv(4354);
    
    adc_fifo_setup(
        true,    // Ativa o FIFO
        true,    // Ativa o threshold de FIFO
        1,       // Threshold de FIFO (1 amostra)
        false,   // Shift right desativado
        false    // Não acumula amostras
    );
    
    adc_run(true);
}

void init_pwm() {
    gpio_set_function(BUZZER_PIN, GPIO_FUNC_PWM);
    
    uint slice_num = pwm_gpio_to_slice_num(BUZZER_PIN);
    
    pwm_config config = pwm_get_default_config();
    
    pwm_config_set_clkdiv_int_frac(&config, 1, 13);
    
    pwm_config_set_wrap(&config, 4095);
    
    pwm_init(slice_num, &config, true);
    
    pwm_set_gpio_level(BUZZER_PIN, 0);
}

void init_i2c_display() {
    i2c_init(i2c1, ssd1306_i2c_clock * 1000);
    
    gpio_set_function(I2C_SDA_PIN, GPIO_FUNC_I2C);
    gpio_set_function(I2C_SCL_PIN, GPIO_FUNC_I2C);
    gpio_pull_up(I2C_SDA_PIN);
    gpio_pull_up(I2C_SCL_PIN);
    
    ssd1306_init();
    
    calculate_render_area_buffer_length(&full_screen);
    
    ssd1306_clear_display(display_buffer);
    
    // Mostra mensagem inicial
    ssd1306_draw_string(display_buffer, 5, 5, "Sintetizador de Audio");
    ssd1306_draw_string(display_buffer, 5, 20, "Pressione A: Gravar");
    ssd1306_draw_string(display_buffer, 5, 35, "Pressione B: Tocar");
    
    render_on_display(display_buffer, &full_screen);
}

// Define a cor do LED RGB
void set_led_color(bool red, bool green, bool blue) {
    gpio_put(RED_LED_PIN, red);
    gpio_put(GREEN_LED_PIN, green);
    gpio_put(BLUE_LED_PIN, blue);
}

// Inicia a gravação 
void start_recording() {
    if (current_state != STATE_IDLE) {
        return;
    }
    
    printf("Iniciando gravação...\n");
    
    // Limpa o buffer de áudio
    memset(audio_buffer, 0, sizeof(audio_buffer));
    buffer_index = 0;
    
    // Garante que o ADC esteja configurado corretamente
    adc_fifo_drain(); // Limpa qualquer dado residual no FIFO
    adc_run(false);   // Para o ADC temporariamente
    
    // Reconfigura o FIFO do ADC
    adc_fifo_setup(
        true,    // Ativa o FIFO
        true,    // Ativa o threshold de FIFO
        1,       // Threshold de FIFO (1 amostra)
        false,   // Shift right desativado
        false    // Não acumula amostras
    );
    
    // Reinicia o ADC
    adc_run(true);
    
    current_state = STATE_RECORDING;
    
    set_led_color(true, false, false);
    
    update_display(STATE_RECORDING);
    
    recording_start_time = get_absolute_time();
    last_sample_time = recording_start_time;
}

// Para a gravação de áudio
void stop_recording() {
    if (current_state != STATE_RECORDING) {
        return;
    }
    
    printf("Gravação finalizada. %lu amostras capturadas.\n", buffer_index);
    
    // Salva o tamanho do buffer
    buffer_length = buffer_index;
    
    // Para o ADC para economizar energia
    adc_run(false);
    adc_fifo_drain();
    
    current_state = STATE_IDLE;
    
    set_led_color(false, false, false);
    
    update_display(STATE_IDLE);
}

// Inicia a reprodução de áudio
void start_playback() {
    if (current_state != STATE_IDLE || buffer_length == 0) {
        return;
    }
    
    printf("Iniciando reprodução...\n");
    
    current_state = STATE_PLAYBACK;
    
    // Acende o LED verde
    set_led_color(false, true, false);
    
    update_display(STATE_PLAYBACK);
    
    buffer_index = 0;
    
    last_sample_time = get_absolute_time();
}

// Para a reprodução de áudio
void stop_playback() {
    if (current_state != STATE_PLAYBACK) {
        return;
    }
    
    printf("Reprodução finalizada.\n");
    
    // Desliga o buzzer completamente
    uint slice_num = pwm_gpio_to_slice_num(BUZZER_PIN);
    pwm_set_gpio_level(BUZZER_PIN, 0);
    
    // Desativa o PWM temporariamente para garantir que não haja som residual
    pwm_set_enabled(slice_num, false);
    sleep_ms(10);
    pwm_set_enabled(slice_num, true);
    
    current_state = STATE_IDLE;
    
    set_led_color(false, false, false);
    
    update_display(STATE_IDLE);
}

// Atualiza o display OLED com base no estado atual
void update_display(system_state_t state) {
    ssd1306_clear_display(display_buffer);
    
    switch (state) {
        case STATE_IDLE:
            ssd1306_draw_string(display_buffer, 5, 5, "Sintetizador de Audio");
            ssd1306_draw_string(display_buffer, 5, 20, "Pressione A: Gravar");
            ssd1306_draw_string(display_buffer, 5, 35, "Pressione B: Tocar");
            break;
            
        case STATE_RECORDING:
            ssd1306_draw_string(display_buffer, 5, 5, "Gravando...");
            ssd1306_draw_string(display_buffer, 5, 20, "Tempo restante:");
            
            int64_t elapsed_us = absolute_time_diff_us(recording_start_time, get_absolute_time());
            int remaining_sec = MAX_RECORD_TIME_SEC - (elapsed_us / 1000000);
            if (remaining_sec < 0) remaining_sec = 0;
            
            char time_str[16];
            sprintf(time_str, "%d segundos", remaining_sec);
            ssd1306_draw_string(display_buffer, 5, 35, time_str);
            break;
            
        case STATE_PLAYBACK:
            ssd1306_draw_string(display_buffer, 5, 5, "Reproduzindo...");
            
            if (buffer_length > 0) {
                draw_waveform(audio_buffer, buffer_length);
            }
            break;
    }
    
    render_on_display(display_buffer, &full_screen);
}

// Desenha a forma de onda das amostras no display
void draw_waveform(uint16_t *samples, uint32_t num_samples) {
    uint32_t display_samples = (num_samples < ssd1306_width) ? num_samples : ssd1306_width;
    
    uint32_t sample_step = num_samples / display_samples;
    if (sample_step == 0) sample_step = 1;
    
    uint16_t max_height = ssd1306_height / 2;
    
    // Linha central (eixo Y)
    uint16_t center_y = ssd1306_height / 2;
    
    // Desenha a linha central
    ssd1306_draw_line(display_buffer, 0, center_y, ssd1306_width - 1, center_y, true);
    
    int16_t prev_y = center_y; // Posição Y inicial (linha central)
    
    for (uint32_t i = 0; i < display_samples; i++) {
        uint32_t sample_idx = i * sample_step;
        if (sample_idx >= num_samples) sample_idx = num_samples - 1;
        
        uint16_t sample_value = samples[sample_idx];
        int16_t y_offset = ((int32_t)sample_value - ADC_MAX_VALUE/2) * max_height / (ADC_MAX_VALUE/2);
        
        if (y_offset > max_height) y_offset = max_height;
        if (y_offset < -max_height) y_offset = -max_height;
        
        int16_t y = center_y - y_offset;
        
        if (i > 0) {
            ssd1306_draw_line(display_buffer, i-1, prev_y, i, y, true);
        } else {
            ssd1306_draw_pixel(display_buffer, i, y, true);
        }
        
        prev_y = y;
    }
}

// Verifica se um botão foi pressionado (com debounce simples)
bool button_pressed(uint button_pin) {
    static uint32_t last_press_time[2] = {0, 0};
    static bool last_state[2] = {true, true};
    
    uint button_idx = (button_pin == BUTTON_A_PIN) ? 0 : 1;
    
    bool current_state = gpio_get(button_pin);
    
    if (current_state != last_state[button_idx]) {
        last_state[button_idx] = current_state;
        
        if (current_state == 0) {
            uint32_t current_time = to_ms_since_boot(get_absolute_time());
            
            if (current_time - last_press_time[button_idx] > 200) {
                last_press_time[button_idx] = current_time;
                return true;
            }
        }
    }
    
    return false;
}

// Função principal
int main() {
    // Inicializa a comunicação serial
    stdio_init_all();
    sleep_ms(2000); // Aguarda a inicialização do terminal
    
    printf("Inicializando Sintetizador de Audio...\n");
    
    // Inicializa os componentes
    init_gpio();
    init_adc();
    init_pwm();
    init_i2c_display();
    
    printf("Sintetizador de Audio inicializado!\n");
    printf("Pressione o botão A para iniciar a gravação.\n");
    printf("Pressione o botão B para reproduzir o áudio gravado.\n");
    
    // Loop principal
    while (true) {
        // Verifica os botões
        if (button_pressed(BUTTON_A_PIN)) {
            if (current_state == STATE_IDLE) {
                start_recording();
            } else if (current_state == STATE_RECORDING) {
                stop_recording();
            }
        }
        
        if (button_pressed(BUTTON_B_PIN)) {
            if (current_state == STATE_IDLE && buffer_length > 0) {
                start_playback();
            } else if (current_state == STATE_PLAYBACK) {
                stop_playback();
            }
        }
        
        switch (current_state) {
            case STATE_RECORDING:
                if (absolute_time_diff_us(last_sample_time, get_absolute_time()) >= SAMPLE_INTERVAL_US) {
                    // Atualiza o tempo da última amostra
                    last_sample_time = get_absolute_time();
                    
                    // Verifica se o ADC tem dados disponíveis (sem travar em loop)
                    if (!adc_fifo_is_empty()) {
                        uint16_t sample = adc_fifo_get();
                        
                        static const float pre_amp_gain = 1.5f;
                        
                        int32_t centered = ((int32_t)sample - (ADC_MAX_VALUE / 2));
                        centered = (int32_t)(centered * pre_amp_gain);
                        
                        if (centered > (ADC_MAX_VALUE / 2) - 100) centered = (ADC_MAX_VALUE / 2) - 100;
                        if (centered < -(ADC_MAX_VALUE / 2) + 100) centered = -(ADC_MAX_VALUE / 2) + 100;
                        
                        sample = (uint16_t)(centered + (ADC_MAX_VALUE / 2));
                        
                        static uint16_t prev_samples[6] = {0};
                        static int filter_idx = 0;
                        
                        prev_samples[filter_idx] = sample;
                        filter_idx = (filter_idx + 1) % 6;
                        
                        uint32_t filtered_sample = 
                            (prev_samples[filter_idx] * 8 + 
                             prev_samples[(filter_idx + 1) % 6] * 6 + 
                             prev_samples[(filter_idx + 2) % 6] * 4 + 
                             prev_samples[(filter_idx + 3) % 6] * 3 + 
                             prev_samples[(filter_idx + 4) % 6] * 2 + 
                             prev_samples[(filter_idx + 5) % 6] * 1) / 24;
                        
                        static uint16_t noise_floor = 2048; // Valor inicial
                        static const float noise_decay = 0.995f;
                        
                        int32_t signal_level = abs((int32_t)filtered_sample - (ADC_MAX_VALUE / 2));
                        if (signal_level < noise_floor) {
                            noise_floor = (uint16_t)(noise_floor * noise_decay + signal_level * (1.0f - noise_decay));
                        }
                        
                        if (signal_level > noise_floor * 1.5) { // Está acima do ruído
                            if (signal_level < 500) {
                                int32_t centered_sample = (int32_t)filtered_sample - (ADC_MAX_VALUE / 2);
                                centered_sample = centered_sample * 2;
                                filtered_sample = (uint16_t)(centered_sample + (ADC_MAX_VALUE / 2));
                            }
                        } else {
                            filtered_sample = (ADC_MAX_VALUE / 2); // Silêncio
                        }
                        
                        if (buffer_index < BUFFER_SIZE) {
                            audio_buffer[buffer_index++] = (uint16_t)filtered_sample;
                        } else {
                            stop_recording();
                        }
                    }
                    
                    if (buffer_index % 100 == 0) {
                        update_display(STATE_RECORDING);
                    }
                }
                
                if (absolute_time_diff_us(recording_start_time, get_absolute_time()) >= MAX_RECORD_TIME_SEC * 1000000) {
                    stop_recording();
                }
                break;
                
            case STATE_PLAYBACK:
                if (absolute_time_diff_us(last_sample_time, get_absolute_time()) >= SAMPLE_INTERVAL_US) {
                    last_sample_time = get_absolute_time();
                    
                    if (buffer_index < buffer_length) {
                        // Amostra atual
                        uint16_t current_sample = audio_buffer[buffer_index];
                        
                        uint16_t next_sample = current_sample; // Valor padrão = amostra atual
                        if (buffer_index + 1 < buffer_length) {
                            next_sample = audio_buffer[buffer_index + 1];
                        }
                        
                        buffer_index++;
                        
                        uint slice_num = pwm_gpio_to_slice_num(BUZZER_PIN);
                        
                        static uint16_t prev_raw_samples[8] = {0}; // Histórico de amostras para filtro
                        static int filter_index = 0;
                        
                        for (int subpos = 0; subpos < 4; subpos++) {
                            int32_t interpolated_value = current_sample + 
                                ((int32_t)next_sample - (int32_t)current_sample) * subpos / 4;
                            
                            prev_raw_samples[filter_index] = interpolated_value;
                            filter_index = (filter_index + 1) % 8;
                        }
                        
                        int32_t centered_sample = (int32_t)prev_raw_samples[filter_index] - (ADC_MAX_VALUE / 2);
                        
                        int32_t filtered_sample = 0;
                        const int weights[8] = {16, 14, 12, 10, 8, 6, 4, 2};
                        int total_weight = 0;
                        
                        for (int i = 0; i < 8; i++) {
                            int idx = (filter_index - i + 8) % 8;
                            int32_t s = (int32_t)prev_raw_samples[idx] - (ADC_MAX_VALUE / 2);
                            filtered_sample += s * weights[i];
                            total_weight += weights[i];
                        }
                        filtered_sample = filtered_sample / total_weight;
                        
                        float gain = 2.0;
                        if (abs(filtered_sample) > 1000) gain = 1.5;
                        filtered_sample = filtered_sample * gain;
                        
                        if (filtered_sample > 2047) filtered_sample = 2047;
                        if (filtered_sample < -2047) filtered_sample = -2047;
                        
                        uint16_t pwm_value = (uint16_t)(filtered_sample + 2047);
                        
                        // Configura o PWM
                        pwm_set_gpio_level(BUZZER_PIN, pwm_value);
                        
                        if (buffer_index % 100 == 0) {
                            update_display(STATE_PLAYBACK);
                        }
                    } else {
                        // Reprodução completa
                        stop_playback();
                    }
                }
                break;
                
            case STATE_IDLE:
                sleep_ms(10);
                break;
        }
    }
    
    return 0;
}
