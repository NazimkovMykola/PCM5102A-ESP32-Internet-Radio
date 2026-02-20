# 24-bit PCM5102A Stereo DAC Digital-to-analog Converter PLL Voice Module pHAT

## Розпіновка DAC модуля

- VCC 5V
- GND GND
- SCK - GND
- BCK 4 
- DIN 6 
- LRCK  5 

Інший бік - 
- FMT GND
- XSMT 3.3V
- DEMP - GND


## Розпіновка дисплея

- SDA 1 
- SCL 2 

## Розпіновка енкодера

- S1 41
- S2 42 
- KEY 40

## Розпіновка ІЧ-приймача LF0038M

(відлік ніжок від скошеного краю лінзи)
- 1 - 18
- 2 - GND
- 3 - VCC +5V

## Склад системи

- ESP32 S3 N16R8 - контролер
- PAM-8403 - підсилювач
- PCM5102A - ЦАП (цифро-аналоговий перетворювач)
- EC11 - енкодер
- ІЧ-приймач LF0038M
