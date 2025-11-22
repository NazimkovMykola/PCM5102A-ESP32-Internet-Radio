#include "WiFi.h"
#include "Audio.h"
#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>

#define SCREEN_WIDTH 128
#define SCREEN_HEIGHT 64
#define OLED_RESET -1
#define I2C_SDA 21
#define I2C_SCL 23

Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, OLED_RESET);

const char *ssid = "Xiaomi_ANNA";
const char *password = "23263483";

const char *radioStations[] = {
    "http://direct.fipradio.fr/live/fip-midfi.mp3",
    "http://direct.fipradio.fr/live/fip-lofi.mp3",
    "http://icecast.omroep.nl:80/3fm-bb-mp3",
    "http://stream.live.vc.bbcmedia.co.uk/bbc_world_service",
    "http://icecast.vrtcdn.be/stubru-high.mp3",
    "http://streams.radiobob.de/bob-live/mp3-192",
    "http://online.radioroks.ua/RadioROKS",
    "http://radio.ukr.radio:8000/ur1-mp3",
    "http://radio.ukr.radio:8000/ur2-mp3",
    "https://a9.asurahosting.com:7390/radio.mp3",
    "http://online.radiorelax.ua/RadioRelax",
    "http://online.melodiafm.ua/MelodiaFM",
    "https://icecast.walmradio.com:8443/walm",
    "https://icecast.walmradio.com:8443/classic",
    "https://icecast.walmradio.com:8443/jazz",
    "https://stream02.pcradio.biz/frank_sinatra-med",
    "https://radio2.ukr.radio/ur3-mp3-m",
    "https://radio2.ukr.radio/ur5-mp3", 
    "https://listen-gorgeousfm.sharp-stream.com/45_gorgeous_fm_128_mp3",
    "https://media-ssl.musicradio.com/Heart80s",
    "https://stream.border-radio.it/radio.mp3",
    "https://blimp.streampunk.cc/_stream/blackout.mp3",
    "https://icecast.xtvmedia.pp.ua/radiowandafm_hq.mp3",
    "https://tunein-live-c.cdnstream1.com/4994_96_2.mp3",
    "https://cdn1.zetcast.net/stream",
    "https://rawlco.leanstream.co/CHUPFM",
    "https://shonanbeachfm.out.airtime.pro:8000/shonanbeachfm_a",
    "https://ssl.geckohost.nz/proxy/john1212?mp=/stream",
    "https://online.radioroks.ua/RadioROKS_Ukr"
};

const int numStations = sizeof(radioStations) / sizeof(radioStations[0]);
int currentStation = 0;
String currentTitle = "Ready";

int8_t toneLow = 10; // Bass: від -10 до +6
int8_t toneMid = 0;  // Mid: від -10 до +6
int8_t toneHigh = 0; // Treble: від -10 до +6

int currentMode = 0;
const char *modeNames[] = {"STATION", "BASS (Low)", "MID TONE", "TREBLE (High)"};

// --- Піни ---
#define I2S_DOUT 22
#define I2S_BCLK 26
#define I2S_LRC 25

#define ENCODER_CLK 12
#define ENCODER_DT 14
#define ENCODER_SW 27
Audio audio;

volatile int encoderPos = 0;
volatile bool encoderChanged = false;
int lastEncoderPos = 0;
int lastCLK = HIGH;

unsigned long lastButtonPress = 0;

void IRAM_ATTR handleEncoder()
{
  int clkState = digitalRead(ENCODER_CLK);
  int dtState = digitalRead(ENCODER_DT);

  if (clkState != lastCLK && clkState == LOW)
  {
    if (dtState != clkState)
      encoderPos++;
    else
      encoderPos--;
    encoderChanged = true;
  }
  lastCLK = clkState;
}

void drawBar(int value, int minVal, int maxVal)
{
  display.drawRect(10, 35, 108, 12, SSD1306_WHITE);

  int width = map(value, minVal, maxVal, 0, 104);
  if (width < 0)
    width = 0;
  if (width > 104)
    width = 104;
  display.fillRect(12, 37, width, 8, SSD1306_WHITE);
  display.setCursor(55, 52);
  display.print(value);
  display.print(" dB");
}

void updateDisplay()
{
  display.clearDisplay();
  display.setTextColor(SSD1306_WHITE);
  display.setCursor(0, 0);
  display.setTextSize(1);
  display.print("MODE: ");
  display.print(modeNames[currentMode]);

  display.drawLine(0, 14, 128, 14, SSD1306_WHITE);
  if (currentMode == 0)
  {
    display.setCursor(0, 18);
    display.print("Station ");
    display.print(currentStation + 1);
    display.print("/");
    display.println(numStations);
    display.setCursor(0, 32);
    if (currentTitle.length() > 21)
    {
      display.println(currentTitle.substring(0, 21));
      display.println(currentTitle.substring(21));
    }
    else
    {
      display.println(currentTitle);
    }
  }
  else if (currentMode == 1)
  {
    drawBar(toneLow, -10, 6);
  }
  else if (currentMode == 2)
  {
    drawBar(toneMid, -10, 6);
  }
  else if (currentMode == 3)
  {
    drawBar(toneHigh, -10, 6);
  }

  display.display();
}

void changeStation(int dir)
{
  int newStation = currentStation + dir;
  if (newStation < 0)
    newStation = numStations - 1;
  if (newStation >= numStations)
    newStation = 0;

  currentStation = newStation;
  currentTitle = "Wait...";
  updateDisplay();

  audio.stopSong();
}

void applyTone()
{
  audio.setTone(toneLow, toneMid, toneHigh);
  updateDisplay();
}

void setup()
{
  Serial.begin(115200);
  Wire.begin(I2C_SDA, I2C_SCL);
  Wire.setClock(400000);

  if (!display.begin(SSD1306_SWITCHCAPVCC, 0x3C))
  {
    Serial.println(F("SSD1306 fail"));
    for (;;)
      ;
  }
  display.clearDisplay();
  display.display();

  pinMode(ENCODER_CLK, INPUT_PULLUP);
  pinMode(ENCODER_DT, INPUT_PULLUP);
  pinMode(ENCODER_SW, INPUT_PULLUP);

  lastCLK = digitalRead(ENCODER_CLK);
  attachInterrupt(digitalPinToInterrupt(ENCODER_CLK), handleEncoder, CHANGE);

  WiFi.mode(WIFI_STA);
  WiFi.begin(ssid, password);

  display.setCursor(0, 0);
  display.setTextSize(1);
  display.setTextColor(SSD1306_WHITE);
  display.println("Connecting WiFi...");
  display.display();

  while (WiFi.status() != WL_CONNECTED)
    delay(500);

  audio.setPinout(I2S_BCLK, I2S_LRC, I2S_DOUT);
  audio.setVolume(21);
  audio.setTone(toneLow, toneMid, toneHigh);
  audio.setConnectionTimeout(10000, 30000);

  audio.connecttohost(radioStations[currentStation]);
  updateDisplay();
}

void loop()
{
  audio.loop();

  if (digitalRead(ENCODER_SW) == LOW)
  {
    if (millis() - lastButtonPress > 300)
    {
      currentMode++;
      if (currentMode > 3)
        currentMode = 0;
      updateDisplay();
      lastButtonPress = millis();
    }
  }

  if (encoderChanged)
  {
    encoderChanged = false;

    int diff = encoderPos - lastEncoderPos;
    int absDiff = abs(diff);
    int threshold = (currentMode == 0) ? 4 : 2;
    if (absDiff >= threshold)
    {
      int direction = (diff > 0) ? 1 : -1;

      if (currentMode == 0)
      {
        changeStation(direction);
        audio.connecttohost(radioStations[currentStation]);
      }
      else if (currentMode == 1)
      {
        toneLow += direction;
        if (toneLow > 6)
          toneLow = 6;
        if (toneLow < -10)
          toneLow = -10;
        applyTone();
      }
      else if (currentMode == 2)
      {
        toneMid += direction;
        if (toneMid > 6)
          toneMid = 6;
        if (toneMid < -10)
          toneMid = -10;
        applyTone();
      }
      else if (currentMode == 3)
      {
        toneHigh += direction;
        if (toneHigh > 6)
          toneHigh = 6;
        if (toneHigh < -10)
          toneHigh = -10;
        applyTone();
      }

      lastEncoderPos = encoderPos;
    }
  }
}

void audio_showstreamtitle(const char *info)
{
  String sInfo = String(info);
  currentTitle = sInfo;
  if (currentMode == 0)
    updateDisplay();
}