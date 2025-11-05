# 24-bit PCM5102A Stereo DAC Digital-to-analog Converter PLL Voice Module pHAT
DAC	ESP32
VCC	5V
GND	GND
BCK	BCK (GPIO26)
DIN	OUT (GPIO22)
LCK	WS (GPIO25)
FMT	GND
XMT	3V
SCK (GPIO13)

## Склад системи 
* ESP32 - контролер
* PAM-8403 - підсилювач
* PCM5102A - ЦАП (цифро-аналоговий перетворювач)
* EC11 - енкодер

Не вистачає потужного блоку живлення, на 0,5А не система не працює. 