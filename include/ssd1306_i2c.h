#ifndef SSD1306_I2C_H
#define SSD1306_I2C_H

#include <stdint.h>
#include "hardware/i2c.h"

// Configurações I2C para o display OLED
#define ssd1306_i2c_id i2c1
#define ssd1306_i2c_address 0x3C
#define ssd1306_i2c_clock 400

// Comandos do display
#define SSD1306_SET_MEM_ADDR_MODE 0x20
#define SSD1306_SET_COLUMN_ADDR 0x21
#define SSD1306_SET_PAGE_ADDR 0x22
#define SSD1306_SET_DISP_START_LINE 0x40
#define SSD1306_SET_CONTRAST 0x81
#define SSD1306_SET_SEGMENT_REMAP 0xA0
#define SSD1306_SET_ENTIRE_ON 0xA4
#define SSD1306_SET_NORM_INV 0xA6
#define SSD1306_SET_DISP 0xAE
#define SSD1306_SET_COM_OUT_DIR 0xC0
#define SSD1306_SET_COM_PIN_CFG 0xDA
#define SSD1306_SET_DISP_CLK_DIV 0xD5
#define SSD1306_SET_PRECHARGE 0xD9
#define SSD1306_SET_VCOM_DESEL 0xDB
#define SSD1306_SET_CHARGE_PUMP 0x8D

// Funções para comunicação I2C com o display
void ssd1306_send_cmd(uint8_t cmd);
void ssd1306_send_data(uint8_t *buf, uint16_t len);
void ssd1306_send_cmd_list(uint8_t *buf, uint16_t len);

#endif // SSD1306_I2C_H
