#include "WiFi.h"
#include "Audio.h"

const char* ssid = "Xiaomi_ANNA";
const char* password = "23263483";

// const char* radioUrl = "http://direct.fipradio.fr/live/fip-lofi.mp3"; 
// const char* radioUrl = "http://icecast.omroep.nl:80/3fm-bb-mp3";
const char* radioUrl = "http://direct.fipradio.fr/live/fip-midfi.mp3";


#define I2S_DOUT      22  // DIN на PCM5102A
#define I2S_BCLK      26  // BCK на PCM5102A  
#define I2S_LRC       25  // LRCK на PCM5102A

Audio audio;

void setup() {
  Serial.begin(115200);
  
  WiFi.mode(WIFI_STA);
  WiFi.setSleep(false); 
  
  WiFi.begin(ssid, password);
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  Serial.println("\nWiFi подключен!");
  Serial.print("IP адрес: ");
  Serial.println(WiFi.localIP());
  Serial.printf("Сила сигнала: %d dBm\n", WiFi.RSSI());
  
  audio.setPinout(I2S_BCLK, I2S_LRC, I2S_DOUT);
  
  audio.setVolume(21); 
  audio.setTone(10, 0, 0); 
  
  audio.setConnectionTimeout(10000, 30000); 
  audio.setBufsize(-1, 12288);
  
  delay(2000);
  Serial.println("Подключение к радиостанции...");
  if(audio.connecttohost(radioUrl)) {
    Serial.println("Успешно подключились к радио!");
    Serial.println("Звук выводится через PCM5102A DAC");
  } else {
    Serial.println("Ошибка подключения к радио");
  }
}

void loop() {
  audio.loop(); 
}

void audio_info(const char *info){
  Serial.printf("INFO: %s\n", info);
}

void audio_id3data(const char *info){
  Serial.printf("ID3: %s\n", info);
}

void audio_eof_mp3(const char *info){
  Serial.printf("EOF: %s\n", info);
}

void audio_showstation(const char *info){
  Serial.printf("STATION: %s\n", info);
}

void audio_showstreamtitle(const char *info){
  Serial.printf("STREAM TITLE: %s\n", info);
}

void audio_bitrate(const char *info){
  Serial.printf("BITRATE: %s\n", info);
}

void audio_commercial(const char *info){
  Serial.printf("COMMERCIAL: %s\n", info);
}

void audio_icyurl(const char *info){
  Serial.printf("ICY URL: %s\n", info);
}

void audio_lasthost(const char *info){
  Serial.printf("LAST HOST: %s\n", info);
}