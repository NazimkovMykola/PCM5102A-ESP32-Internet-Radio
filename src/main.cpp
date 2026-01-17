#include <Arduino.h>
#include "WiFi.h"
#include "Audio.h"
#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>
#include <Preferences.h> 

#include <IRremoteESP8266.h>
#include <IRrecv.h>
#include <IRutils.h>

const uint16_t kRecvPin = 33; 
IRrecv irrecv(kRecvPin);
decode_results results;

#define SCREEN_WIDTH 128
#define SCREEN_HEIGHT 64
#define I2C_SDA 21
#define I2C_SCL 23
Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, -1);

Preferences preferences; 
Audio audio;

bool isMuted = false;
int lastVolume = 12;

struct Station { const char* name; const char* url; };
Station stationList[] = {
    {"FIP Radio", "http://direct.fipradio.fr/live/fip-midfi.mp3"},
    {"3FM BB", "http://icecast.omroep.nl:80/3fm-bb-mp3"},
    {"BBC World Service", "http://stream.live.vc.bbcmedia.co.uk/bbc_world_service"},
    {"Studio Brussel", "http://icecast.vrtcdn.be/stubru-high.mp3"},
    {"RADIO BOB!", "http://streams.radiobob.de/bob-live/mp3-192"},
    {"Radio ROKS", "http://online.radioroks.ua/RadioROKS"},
    {"Asura Radio", "https://a9.asurahosting.com:7390/radio.mp3"},
    {"Radio Relax", "http://online.radiorelax.ua/RadioRelax"},
    {"Melodia FM", "http://online.melodiafm.ua/MelodiaFM"},
    {"Walm Jazz", "https://icecast.walmradio.com:8443/jazz"},
    {"Frank Sinatra", "https://stream02.pcradio.biz/frank_sinatra-med"},
    {"Gorgeous FM", "https://listen-gorgeousfm.sharp-stream.com/45_gorgeous_fm_128_mp3"},
    {"Heart 80s", "https://media-ssl.musicradio.com/Heart80s"},
    {"Blackout", "https://blimp.streampunk.cc/_stream/blackout.mp3"},
    {"Wanda FM", "https://icecast.xtvmedia.pp.ua/radiowandafm_hq.mp3"},
    {"GB News", "https://tunein-live-c.cdnstream1.com/4994_96_2.mp3"},
    {"ZetCast", "https://cdn1.zetcast.net/stream"},
    {"104.9 The Wolf", "https://rawlco.leanstream.co/CHUPFM"},
    {"Shonan Beach FM", "https://shonanbeachfm.out.airtime.pro:8000/shonanbeachfm_a"},
    {"ROKS Ukraine", "https://online.radioroks.ua/RadioROKS_Ukr"},
    {"Hit FM Best", "https://online.hitfm.ua/HitFM_Best"},
    {"Relax Cafe", "https://online.radiorelax.ua/RadioRelax_Cafe"},
    {"Classic music", "https://s3.radio.co/sa3e464c40/listen"},
    {"Business Radio", "https://cast.brg.ua/business_main_public_mp3_hq"}
};
const int numStations = sizeof(stationList) / sizeof(stationList[0]);

int currentStation = 0;
int previewStation = 0; 
String currentTitle = "Ready";

void updateDisplay() {
  display.clearDisplay();
  display.setTextColor(WHITE);
  display.setTextSize(1);
  display.setCursor(0, 0);
  
  if (isMuted) {
    display.print("ST: "); display.print(previewStation + 1);
    display.setCursor(80, 0);
    display.print("[MUTE]");
  } else {
    display.print("STATION: "); display.print(previewStation + 1);
    display.print("/"); display.print(numStations);
  }

  display.drawLine(0, 12, 128, 12, WHITE);
  display.setCursor(0, 20);
  display.println(stationList[previewStation].name);
  display.setCursor(0, 40);
  display.println(currentTitle);
  
  display.display();
}

void setup() {
  Serial.begin(115200);
  
  Wire.begin(I2C_SDA, I2C_SCL);
  if(!display.begin(SSD1306_SWITCHCAPVCC, 0x3C)) for(;;);

  irrecv.enableIRIn(); 

  WiFi.begin("Xiaomi_ANNA", "23263483");
  while (WiFi.status() != WL_CONNECTED) delay(500);

  audio.setPinout(26, 25, 22);
  audio.setVolume(lastVolume);
  audio.connecttohost(stationList[currentStation].url);
  
  updateDisplay();
}

void loop() {
  audio.loop();

  if (irrecv.decode(&results)) {
    uint32_t code = (uint32_t)results.value;
    
    if (code == 0x11EECC33 || code == 0x11EEA857) { // Next
       previewStation = (previewStation + 1) % numStations;
       currentStation = previewStation;
       audio.connecttohost(stationList[currentStation].url);
    } 
    else if (code == 0x11EE2CD3 || code == 0x11EE6897) { // Prev
       previewStation = (previewStation - 1 + numStations) % numStations;
       currentStation = previewStation;
       audio.connecttohost(stationList[currentStation].url);
    }
    else if (code == 0x20EF906F) { 
       isMuted = !isMuted;
       if (isMuted) {
         audio.setVolume(0);
       } else {
         audio.setVolume(lastVolume);
       }
    }

    updateDisplay();
    irrecv.resume(); 
  }

  static unsigned long lastUpdate = 0;
  if (millis() - lastUpdate > 2000) {
    lastUpdate = millis();
    updateDisplay();
  }
}

void audio_showstreamtitle(const char *i) { 
  String s = String(i);
  s.trim();
  if (s.length() > 0) {
    currentTitle = s;
    updateDisplay(); 
  }
}
void audio_info(const char *i) {}
void audio_error(const char *i) { currentTitle = "Stream Error"; updateDisplay(); }