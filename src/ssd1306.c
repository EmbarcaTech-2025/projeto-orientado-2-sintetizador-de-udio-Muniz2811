#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "ssd1306.h"
#include "ssd1306_i2c.h"
#include "ssd1306_font.h"

void ssd1306_init() {
    // Sequência de inicialização do display
    uint8_t cmds[] = {
        SSD1306_SET_DISP | 0x00,         // Display off
        SSD1306_SET_MEM_ADDR_MODE,       // Set memory address mode
        0x00,                            // Horizontal addressing mode
        SSD1306_SET_DISP_START_LINE | 0x00, // Set start line address
        SSD1306_SET_SEGMENT_REMAP | 0x01,   // Set segment re-map
        SSD1306_SET_COM_OUT_DIR | 0x08,     // Set COM output scan direction
        SSD1306_SET_COM_PIN_CFG,         // Set COM pins hardware configuration
        0x12,
        SSD1306_SET_CONTRAST,            // Set contrast control
        0x7F,
        SSD1306_SET_ENTIRE_ON,           // Disable entire display on
        SSD1306_SET_NORM_INV,            // Set normal display
        SSD1306_SET_DISP_CLK_DIV,        // Set oscillator frequency
        0x80,
        SSD1306_SET_PRECHARGE,           // Set pre-charge period
        0xF1,
        SSD1306_SET_VCOM_DESEL,          // Set VCOMH deselect level
        0x30,
        SSD1306_SET_CHARGE_PUMP,         // Set charge pump
        0x14,
        SSD1306_SET_DISP | 0x01          // Display on
    };
    
    ssd1306_send_cmd_list(cmds, sizeof(cmds));
}

void calculate_render_area_buffer_length(struct render_area *area) {
    area->buffer_length = (area->end_column - area->start_column + 1) * (area->end_page - area->start_page + 1);
}

void render_on_display(uint8_t *buf, struct render_area *area) {
    uint8_t cmds[] = {
        SSD1306_SET_COLUMN_ADDR,
        area->start_column,
        area->end_column,
        SSD1306_SET_PAGE_ADDR,
        area->start_page,
        area->end_page
    };
    
    ssd1306_send_cmd_list(cmds, sizeof(cmds));
    ssd1306_send_data(buf, area->buffer_length);
}

void ssd1306_clear_display(uint8_t *buf) {
    memset(buf, 0, ssd1306_buffer_length);
}

void ssd1306_draw_pixel(uint8_t *buf, int16_t x, int16_t y, bool color) {
    if (x < 0 || x >= ssd1306_width || y < 0 || y >= ssd1306_height) {
        return;
    }
    
    uint16_t byte_idx = x + (y / 8) * ssd1306_width;
    uint8_t bit_position = y % 8;
    
    if (color) {
        buf[byte_idx] |= (1 << bit_position);
    } else {
        buf[byte_idx] &= ~(1 << bit_position);
    }
}

void ssd1306_draw_line(uint8_t *buf, int16_t x0, int16_t y0, int16_t x1, int16_t y1, bool color) {
    int16_t steep = abs(y1 - y0) > abs(x1 - x0);
    
    if (steep) {
        int16_t temp = x0;
        x0 = y0;
        y0 = temp;
        
        temp = x1;
        x1 = y1;
        y1 = temp;
    }
    
    if (x0 > x1) {
        int16_t temp = x0;
        x0 = x1;
        x1 = temp;
        
        temp = y0;
        y0 = y1;
        y1 = temp;
    }
    
    int16_t dx = x1 - x0;
    int16_t dy = abs(y1 - y0);
    int16_t err = dx / 2;
    int16_t ystep = (y0 < y1) ? 1 : -1;
    
    for (; x0 <= x1; x0++) {
        if (steep) {
            ssd1306_draw_pixel(buf, y0, x0, color);
        } else {
            ssd1306_draw_pixel(buf, x0, y0, color);
        }
        
        err -= dy;
        if (err < 0) {
            y0 += ystep;
            err += dx;
        }
    }
}

void ssd1306_draw_string(uint8_t *buf, int16_t x, int16_t y, const char *text) {
    int16_t cursor_x = x;
    int16_t cursor_y = y;
    
    while (*text) {
        uint8_t c = *text - 32; // A tabela começa no espaço (ASCII 32)
        
        for (int i = 0; i < FONT_WIDTH; i++) {
            uint8_t line = font[c * FONT_WIDTH + i];
            
            for (int j = 0; j < FONT_HEIGHT; j++) {
                if (line & (1 << j)) {
                    ssd1306_draw_pixel(buf, cursor_x + i, cursor_y + j, true);
                }
            }
        }
        
        cursor_x += FONT_WIDTH + CHAR_SPACING;
        
        if (cursor_x > ssd1306_width - FONT_WIDTH) {
            cursor_x = x;
            cursor_y += FONT_HEIGHT + 1;
        }
        
        text++;
    }
}

void ssd1306_draw_vertical_bar(uint8_t *buf, int16_t x, int16_t height, bool color) {
    if (x < 0 || x >= ssd1306_width) {
        return;
    }

    if (height > ssd1306_height) {
        height = ssd1306_height;
    }

    for (int16_t y = ssd1306_height - 1; y >= ssd1306_height - height; y--) {
        ssd1306_draw_pixel(buf, x, y, color);
    }
}
