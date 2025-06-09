#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "hardware/i2c.h"
#include "ssd1306_i2c.h"

void ssd1306_send_cmd(uint8_t cmd) {
    uint8_t buf[2] = {0x80, cmd}; // 0x80 indica modo de comando
    i2c_write_blocking(i2c1, ssd1306_i2c_address, buf, 2, false);
}

void ssd1306_send_data(uint8_t *buf, uint16_t len) {
    uint8_t *temp_buf = malloc(len + 1);
    if (temp_buf == NULL) {
        printf("Falha na alocação de memória\n");
        return;
    }

    temp_buf[0] = 0x40; // 0x40 indica modo de dados
    memcpy(temp_buf + 1, buf, len);
    i2c_write_blocking(i2c1, ssd1306_i2c_address, temp_buf, len + 1, false);
    free(temp_buf);
}

void ssd1306_send_cmd_list(uint8_t *buf, uint16_t len) {
    for (int i = 0; i < len; i++) {
        ssd1306_send_cmd(buf[i]);
    }
}
