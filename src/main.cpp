#include "WiFi.h"
#include "Audio.h"

const char* ssid = "Xiaomi_ANNA";
const char* password = "23263483";

const char* radioStations[] = {
  "http://direct.fipradio.fr/live/fip-midfi.mp3",
  "http://direct.fipradio.fr/live/fip-lofi.mp3",
  "http://icecast.omroep.nl:80/3fm-bb-mp3",
  "http://stream.nbcnews.com/data/news/newsradiolive.mp3"
  "http://direct.fipradio.fr/live/fip-midfi.mp3",
"http://direct.fipradio.fr/live/fip-lofi.mp3",
"http://icecast.omroep.nl:80/3fm-bb-mp3",
"http://stream.live.vc.bbcmedia.co.uk/bbc_world_service",
"http://icecast.vrtcdn.be/stubru-high.mp3",
"http://ice1.somafm.com/groovesalad-256-mp3",
"http://ice1.somafm.com/defcon-256-mp3",
"http://ice1.somafm.com/dronezone-256-mp3",
"http://ice1.somafm.com/spacestation-128-mp3",
"http://streams.radiobob.de/bob-live/mp3-192",
"http://icecast.omroep.nl:80/radio1-bb-mp3",
"http://icecast.omroep.nl:80/radio2-bb-mp3",
"http://radio.ukr.radio:8000/ur1-mp3",
"http://radio.ukr.radio:8000/ur2-mp3",
"http://radio.ukr.radio:8000/ur3-mp3",
"http://online.radioroks.ua/RadioROKS"
};

const int numStations = sizeof(radioStations) / sizeof(radioStations[0]);
int currentStation = 0;

#define I2S_DOUT      22  
#define I2S_BCLK      26  
#define I2S_LRC       25  

#define ENCODER_CLK   12  
#define ENCODER_DT    14  

Audio audio;

volatile int encoderPos = 0;
volatile bool encoderChanged = false;
int lastEncoderPos = 0;
int lastCLK = HIGH;

void IRAM_ATTR handleEncoder() {
  int clkState = digitalRead(ENCODER_CLK);
  int dtState = digitalRead(ENCODER_DT);
  
  if (clkState != lastCLK && clkState == LOW) {
    if (dtState != clkState) {
      encoderPos++;
    } else {
      encoderPos--;
    }
    encoderChanged = true;
  }
  lastCLK = clkState;
}

void changeStation(int newStation) {
  if (newStation < 0) newStation = numStations - 1;
  if (newStation >= numStations) newStation = 0;
  
  currentStation = newStation;
  
  audio.stopSong();
  delay(100);
  audio.connecttohost(radioStations[currentStation]);
}

void setup() {
  pinMode(ENCODER_CLK, INPUT_PULLUP);
  pinMode(ENCODER_DT, INPUT_PULLUP);
  
  lastCLK = digitalRead(ENCODER_CLK);
  attachInterrupt(digitalPinToInterrupt(ENCODER_CLK), handleEncoder, CHANGE);
  
  WiFi.mode(WIFI_STA);
  WiFi.setSleep(false); 
  
  WiFi.begin(ssid, password);
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
  }
  
  audio.setPinout(I2S_BCLK, I2S_LRC, I2S_DOUT);
  audio.setVolume(21); 
  audio.setTone(8, 0, 0); 
  audio.setConnectionTimeout(10000, 30000); 
  audio.setBufsize(-1, 12288);
  
  delay(1000);
  changeStation(currentStation);
}

void loop() {
  audio.loop();
  
  if (encoderChanged) {
    encoderChanged = false;
    
    int diff = encoderPos - lastEncoderPos;
    
    if (abs(diff) >= 4) {
      if (diff > 0) {
        changeStation(currentStation + 1);
      } else {
        changeStation(currentStation - 1);
      }
      lastEncoderPos = encoderPos;
    }
  }
}