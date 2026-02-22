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

const uint16_t kRecvPin = 18; 
IRrecv irrecv(kRecvPin);
decode_results results;

#define SCREEN_WIDTH 128
#define SCREEN_HEIGHT 64
#define I2C_SDA 1
#define I2C_SCL 2
Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, -1);

Preferences preferences; 
Audio audio;

int8_t toneLow = 0, toneMid = 0, toneHigh = 0;
int currentMode = 0;
const char* modeNames[] = {"STATION", "BASS", "MIDDLE", "TREBLE"};

bool isMuted = false;
int lastVolume = 12;

#define ENCODER_CLK 41
#define ENCODER_DT 42
#define ENCODER_SW 40

// --- НОВИЙ БЛОК ЕНКОДЕРА ---
volatile int encoderPos = 0;
volatile bool encoderChanged = false;
int lastEncoderPos = 0;
static uint8_t old_AB = 0;
const int8_t encoder_states[] = {0, -1, 1, 0, 1, 0, 0, -1, -1, 0, 0, 1, 0, 1, -1, 0};

unsigned long buttonDownTime = 0;
bool buttonActive = false;
unsigned long lastInteraction = 0; 

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
    {"TuneIn Mix", "https://tunein-live-c.cdnstream1.com/4994_96_2.mp3"},
    {"GB News", "https://tunein-live-c.cdnstream1.com/4994_96_2.mp3"},
    {"ZetCast", "https://cdn1.zetcast.net/stream"},
    {"104.9 The Wolf", "https://rawlco.leanstream.co/CHUPFM"},
    {"Shonan Beach FM", "https://shonanbeachfm.out.airtime.pro:8000/shonanbeachfm_a"},
    {"ROKS Ukraine", "https://online.radioroks.ua/RadioROKS_Ukr"},
    {"Hit FM Best", "https://online.hitfm.ua/HitFM_Best"},
    {"Relax Cafe", "https://online.radiorelax.ua/RadioRelax_Cafe"},
    {"Radio.co", "https://s3.radio.co/sa3e464c40/listen"},
    {"Classic music", "https://s3.radio.co/sa3e464c40/listen"},
    {"Business Radio", "https://cast.brg.ua/business_main_public_mp3_hq"},
    {"Radio NV", "https://online-radio.nv.ua/radionv.mp3"},
    {"UKR Radio", "https://radio2.ukr.radio/ur1-mp3-m"},
    {"Hromadske", "https://hromadske.radio/radio_https_upstream"},
    {"Kultura", "https://radio.ukr.radio/ur3-mp3-m"},
    {"Radiotochka", "https://radio2.ukr.radio/ur5-mp3"},
    {"ReLIFE", "https://resh.news/radio"},
    {"Pryamyi", "https://cast.mediaonline.net.ua/prmfm320"},
    {"Emanuil", "https://online.svitle.org/hls/svitle/aac_hifi.m3u8"}
};

const int numStations = sizeof(stationList) / sizeof(stationList[0]);

int currentStation = 0;
int previewStation = 0; 
String currentTitle = "Ready";

void saveToneSettings() {
  preferences.begin("radio-cfg", false);
  preferences.putChar("bass", toneLow);
  preferences.putChar("mid", toneMid);
  preferences.putChar("treble", toneHigh);
  preferences.end();
}

// --- НОВА ФУНКЦІЯ ОБРОБКИ ---
void IRAM_ATTR handleEncoder() {
  old_AB <<= 2;
  old_AB |= (digitalRead(ENCODER_CLK) << 1) | digitalRead(ENCODER_DT);
  old_AB &= 0x0F;
  int8_t change = encoder_states[old_AB];
  if (change != 0) {
    encoderPos += change;
    encoderChanged = true;
  }
}

void updateDisplay() {
  display.clearDisplay();
  display.setTextColor(WHITE);
  display.setTextSize(1);
  display.setCursor(0, 0);
  
  display.print("MODE: "); display.print(modeNames[currentMode]);
  if (isMuted) { display.setCursor(90, 0); display.print("[M]"); }

  display.drawLine(0, 12, 128, 12, WHITE);
  display.setCursor(0, 20);

  if (currentMode == 0) {
    display.print("ST: "); display.print(previewStation + 1);
    display.print("/"); display.println(numStations);
    display.setCursor(0, 32);
    display.println(stationList[previewStation].name);
    display.setCursor(0, 50);
    display.println(currentTitle);
  } else {
    int8_t val = (currentMode == 1) ? toneLow : (currentMode == 2) ? toneMid : toneHigh;
    display.setTextSize(2);
    display.setCursor(35, 30);
    display.printf("%+d dB", val);
    display.drawRect(10, 55, 108, 6, WHITE);
    int barWidth = map(val, -10, 6, 0, 104);
    display.fillRect(12, 57, barWidth, 2, WHITE);
  }
  display.display();
}

void setup() {
  Wire.begin(I2C_SDA, I2C_SCL, 100000);
  if(!display.begin(SSD1306_SWITCHCAPVCC, 0x3C)) for(;;);

  irrecv.enableIRIn(); 

  pinMode(ENCODER_CLK, INPUT_PULLUP);
  pinMode(ENCODER_DT, INPUT_PULLUP);
  pinMode(ENCODER_SW, INPUT_PULLUP);
  
  // Переривання на обидва піни для точності
  attachInterrupt(digitalPinToInterrupt(ENCODER_CLK), handleEncoder, CHANGE);
  attachInterrupt(digitalPinToInterrupt(ENCODER_DT), handleEncoder, CHANGE);

  preferences.begin("radio-cfg", true);
  toneLow = preferences.getChar("bass", 0);
  toneMid = preferences.getChar("mid", 0);
  toneHigh = preferences.getChar("treble", 0);
  currentStation = preferences.getInt("station_idx", 0);
  preferences.end();

  WiFi.begin("Xiaomi_ANNA", "23263483");
  WiFi.setSleep(false);
  while (WiFi.status() != WL_CONNECTED) delay(500);

  audio.setPinout(4, 5, 6);
  audio.setVolume(lastVolume);
  audio.setBufsize(1500000, 500000);
  audio.setTone(toneLow, toneMid, toneHigh);
  
  previewStation = currentStation;
  audio.connecttohost(stationList[currentStation].url);
  updateDisplay();
}

void loop() {
  audio.loop();

  if (irrecv.decode(&results)) {
    uint32_t code = (uint32_t)results.value;
    lastInteraction = millis(); 

    if (code == 0x11EE08F7) { 
       currentMode = (currentMode + 1) % 4;
    }
    else if (code == 0x11EECC33 || code == 0x11EEA857) { 
       if (currentMode == 0) { 
         previewStation = (previewStation + 1) % numStations;
         currentStation = previewStation;
         audio.connecttohost(stationList[currentStation].url);
       } else {
         if (currentMode == 1) toneLow = constrain(toneLow + 1, -10, 6);
         else if (currentMode == 2) toneMid = constrain(toneMid + 1, -10, 6);
         else if (currentMode == 3) toneHigh = constrain(toneHigh + 1, -10, 6);
         audio.setTone(toneLow, toneMid, toneHigh);
         saveToneSettings();
       }
    } 
    else if (code == 0x11EE2CD3 || code == 0x11EE6897) {
       if (currentMode == 0) { 
         previewStation = (previewStation - 1 + numStations) % numStations;
         currentStation = previewStation;
         audio.connecttohost(stationList[currentStation].url);
       } else {
         if (currentMode == 1) toneLow = constrain(toneLow - 1, -10, 6);
         else if (currentMode == 2) toneMid = constrain(toneMid - 1, -10, 6);
         else if (currentMode == 3) toneHigh = constrain(toneHigh - 1, -10, 6);
         audio.setTone(toneLow, toneMid, toneHigh);
         saveToneSettings();
       }
    }
    else if (code == 0x20DF906F || code == 0x11EE9867) { 
       isMuted = !isMuted;
       audio.setVolume(isMuted ? 0 : lastVolume);
    }
    updateDisplay();
    irrecv.resume(); 
  }

  int btnState = digitalRead(ENCODER_SW);
  if (btnState == LOW && !buttonActive) {
    buttonActive = true;
    buttonDownTime = millis();
  }
  if (btnState == HIGH && buttonActive) {
    buttonActive = false;
    lastInteraction = millis();
    if (millis() - buttonDownTime > 500) { 
       currentMode = (currentMode + 1) % 4;
    } else { 
       if (currentMode == 0) {
         currentStation = previewStation;
         audio.connecttohost(stationList[currentStation].url);
       }
    }
    updateDisplay();
  }

  if (encoderChanged) {
    encoderChanged = false;
    lastInteraction = millis();
    int diff = encoderPos - lastEncoderPos;
    // Один фізичний клік зазвичай видає 4 зміни стану
    if (abs(diff) >= 4) {
      int dir = (diff > 0) ? 1 : -1;
      if (currentMode == 0) {
        previewStation = (previewStation + dir + numStations) % numStations;
      } else {
        if (currentMode == 1) toneLow = constrain(toneLow + dir, -10, 6);
        else if (currentMode == 2) toneMid = constrain(toneMid + dir, -10, 6);
        else if (currentMode == 3) toneHigh = constrain(toneHigh + dir, -10, 6);
        audio.setTone(toneLow, toneMid, toneHigh);
        saveToneSettings();
      }
      lastEncoderPos = encoderPos;
      updateDisplay();
    }
  }

  if (currentMode != 0 && millis() - lastInteraction > 10000) {
    currentMode = 0;
    updateDisplay();
  }
}

void audio_showstreamtitle(const char *i) { 
  String s = String(i); s.trim();
  if (s.length() > 0) { currentTitle = s; updateDisplay(); }
}
void audio_error(const char *i) { currentTitle = "Stream Error"; updateDisplay(); }