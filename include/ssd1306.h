#ifndef SSD1306_H
#define SSD1306_H

#include <stdint.h>
#include <stdbool.h>

// Dimensões do display
#define ssd1306_width 128
#define ssd1306_height 64
#define ssd1306_n_pages 8
#define ssd1306_buffer_length ((ssd1306_width * ssd1306_height) / 8)

// Estrutura para definir a área de renderização
struct render_area {
    uint8_t start_column;
    uint8_t end_column;
    uint8_t start_page;
    uint8_t end_page;
    int buffer_length;  // Tamanho do buffer necessário para esta área
};

// Funções para o display
void ssd1306_init();
void calculate_render_area_buffer_length(struct render_area *area);
void render_on_display(uint8_t *buf, struct render_area *area);
void ssd1306_clear_display(uint8_t *buf);
void ssd1306_draw_pixel(uint8_t *buf, int16_t x, int16_t y, bool color);
void ssd1306_draw_line(uint8_t *buf, int16_t x0, int16_t y0, int16_t x1, int16_t y1, bool color);
void ssd1306_draw_string(uint8_t *buf, int16_t x, int16_t y, const char *text);
void ssd1306_draw_vertical_bar(uint8_t *buf, int16_t x, int16_t height, bool color);

#endif // SSD1306_H
