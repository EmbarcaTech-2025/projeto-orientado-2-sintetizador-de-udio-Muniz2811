#ifndef AUDIO_SYNTHESIZER_H
#define AUDIO_SYNTHESIZER_H

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "pico/stdlib.h"
#include "hardware/adc.h"
#include "hardware/pwm.h"
#include "hardware/dma.h"
#include "hardware/i2c.h"

// Pinos do BitDogLab
#define MIC_CHANNEL 2                
#define MIC_PIN (26 + MIC_CHANNEL)   
#define BUZZER_PIN 21                
#define BUTTON_A_PIN 5               
#define BUTTON_B_PIN 6              
#define RED_LED_PIN 13              
#define GREEN_LED_PIN 11             
#define BLUE_LED_PIN 12              
#define I2C_SDA_PIN 14              
#define I2C_SCL_PIN 15              
// Configurações do ADC
#define ADC_CLOCK_DIV 96.0f         
#define ADC_BITS 12                 
#define ADC_MAX_VALUE (1 << ADC_BITS) 

// Configurações de amostragem de áudio
#define SAMPLE_RATE 11025           
#define SAMPLE_INTERVAL_US (1000000 / SAMPLE_RATE) 
#define MAX_RECORD_TIME_SEC 5        
#define BUFFER_SIZE (SAMPLE_RATE * MAX_RECORD_TIME_SEC) 

// Estados do sistema
typedef enum {
    STATE_IDLE,         // Estado inicial, aguardando comando
    STATE_RECORDING,    // Gravando áudio
    STATE_PLAYBACK,     // Reproduzindo áudio
} system_state_t;

void init_gpio();
void init_adc();
void init_pwm();
void init_i2c_display();

void set_led_color(bool red, bool green, bool blue);

void start_recording();
void stop_recording();
void start_playback();
void stop_playback();

void update_display(system_state_t state);
void draw_waveform(uint16_t *samples, uint32_t num_samples);

bool button_pressed(uint button_pin);

#endif // AUDIO_SYNTHESIZER_H
