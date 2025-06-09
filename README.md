# Projetos de Sistemas Embarcados - EmbarcaTech 2025

Autor: **Pedro Henrique Muniz de Oliveira**

Curso: Residência Tecnológica em Sistemas Embarcados

Instituição: EmbarcaTech - HBr

Brasilia, 06 de 2025


# Sintetizador de Áudio para BitDogLab

Este projeto implementa um sintetizador de áudio para a placa BitDogLab (Raspberry Pi Pico), capaz de gravar áudio através do microfone e reproduzi-lo através do buzzer.

## Funcionalidades

- Gravação de áudio através do microfone da BitDogLab
- Armazenamento temporário do áudio em um buffer na RAM
- Reprodução do áudio gravado através do buzzer
- Visualização da forma de onda no display OLED
- Feedback visual através do LED RGB
- Controle através dos botões A e B

## Hardware Utilizado

- BitDogLab (Raspberry Pi Pico)
- Microfone (conectado ao ADC no pino 28)
- Buzzer (conectado ao pino 21)
- Display OLED SSD1306 (conectado via I2C nos pinos 14 e 15)
- LED RGB (pinos 11, 12 e 13)
- Botões A e B (pinos 5 e 6)

## Especificações Técnicas

- Taxa de amostragem: 8 kHz
- Tempo máximo de gravação: 5 segundos
- Resolução do ADC: 12 bits
- Modulação de saída: PWM

## Como Usar

1. Compile o projeto e carregue o arquivo UF2 na BitDogLab
2. Pressione o botão A para iniciar a gravação (LED vermelho acende)
3. Fale ou produza sons próximo ao microfone
4. A gravação termina automaticamente após 5 segundos ou ao pressionar o botão A novamente
5. Pressione o botão B para reproduzir o áudio gravado (LED verde acende)
6. Durante a reprodução, a forma de onda do áudio é exibida no display OLED
7. Pressione o botão B novamente para interromper a reprodução


Um vídeo de demonstração do sintetizador pode ser visto no seguinte link: https://youtube.com/shorts/9HnHQZs3BQg
## Compilação

Para compilar o projeto:

```bash
mkdir build
cd build
cmake ..
make
```

O arquivo UF2 será gerado em `build/audio_synthesizer.uf2`. Copie este arquivo para a BitDogLab quando ela estiver no modo de programação (pressione o botão BOOTSEL enquanto conecta a placa).

## Estrutura do Projeto

- `src/audio_synthesizer.c`: Implementação principal do sintetizador
- `src/ssd1306.c`: Implementação das funções do display OLED
- `src/ssd1306_i2c.c`: Implementação da comunicação I2C com o display
- `include/audio_synthesizer.h`: Definições e protótipos para o sintetizador
- `include/ssd1306.h`: Definições e protótipos para o display
- `include/ssd1306_i2c.h`: Definições para a comunicação I2C
- `include/ssd1306_font.h`: Definição da fonte para o display

## Conceitos Aplicados

- Conversão analógico-digital (ADC)
- Modulação por largura de pulso (PWM)
- Taxa de amostragem e processamento de sinais
- Armazenamento e manipulação de dados em buffer
- Interface de usuário com feedback visual

---

## 📜 Licença
MIT License - MIT GPL-3.0.