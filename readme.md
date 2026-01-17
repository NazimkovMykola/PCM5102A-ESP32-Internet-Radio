# 24-bit PCM5102A Stereo DAC Digital-to-analog Converter PLL Voice Module pHAT

## Розпіновка DAC модуля

- VCC 5V
- GND GND
- BCK BCK (GPIO26)
- DIN OUT (GPIO22)
- LCK WS (GPIO25)
- FMT GND
- XMT 3.3V
- SCK (GPIO13)

## Розпіновка дисплея

- SDA 21
- SCL 23

## Розпіновка енкодера

- S1 12
- S2 14
- KEY 27

## Розпіновка ІЧ-приймача LF0038M

(відлік ніжок від скошеного краю лінзи)

- 1 - GPIO33
- 2 - GND
- 3 - VCC +5V

## Склад системи

- ESP32 - контролер
- PAM-8403 - підсилювач
- PCM5102A - ЦАП (цифро-аналоговий перетворювач)
- EC11 - енкодер
- ІЧ-приймач LF0038M
