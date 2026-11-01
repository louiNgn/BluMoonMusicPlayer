#include <Arduino.h>
#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>
#include <DFRobotDFPlayerMini.h>

namespace Config {

constexpr int DF_RX = 44;  
constexpr int DF_TX = 43;  

constexpr int OLED_SDA = 8;
constexpr int OLED_SCL = 9;
constexpr uint8_t OLED_ADDRESS = 0x3C;  
constexpr int OLED_WIDTH = 128;
constexpr int OLED_HEIGHT = 32;       

constexpr int ENC_A = 5;     
constexpr int ENC_B = 4;      
constexpr int ENC_CLICK = 1;   
constexpr uint8_t ENCODER_EDGES_PER_STEP = 4; 


constexpr int BUTTON_PREV = 2;  
constexpr int BUTTON_PAUSE = 3; 
constexpr int BUTTON_NEXT = 6;  
constexpr int BATTERY_ADC = 7;  


constexpr uint16_t TRACK_COUNT = 10;
constexpr uint8_t INITIAL_VOLUME = 12; // 0..30
constexpr unsigned long DISPLAY_UPDATE_MS = 200;
constexpr unsigned long BATTERY_UPDATE_MS = 3000;
constexpr unsigned long COMMAND_GAP_MS = 220;
} 

HardwareSerial dfSerial(1);
DFRobotDFPlayerMini dfPlayer;
Adafruit_SSD1306 display(Config::OLED_WIDTH, Config::OLED_HEIGHT, &Wire, -1);

bool dfOnline = false;
bool oledOnline = false;
bool playing = false;
uint16_t selectedTrack = 1;
uint16_t playingTrack = 0;
uint8_t volumeLevel = Config::INITIAL_VOLUME;
float batteryVolts = 0.0f;
unsigned long lastDisplayMs = 0;
unsigned long lastBatteryMs = 0;
unsigned long lastDfCommandMs = 0;

class DebouncedButton {
public:
  explicit DebouncedButton(int gpio) : pin(gpio) {}
  void begin() {
    pinMode(pin, INPUT_PULLUP);
    observed = stable = digitalRead(pin);
    changedAt = millis();
  }
  bool justPressed() {
    const bool current = digitalRead(pin);
    if (current != observed) {
      observed = current;
      changedAt = millis();
    }
    if ((millis() - changedAt) >= 25 && observed != stable) {
      stable = observed;
      return stable == LOW;
    }
    return false;
  }
private:
  int pin;
  bool observed = HIGH;
  bool stable = HIGH;
  unsigned long changedAt = 0;
};

DebouncedButton encButton(Config::ENC_CLICK);
DebouncedButton prevButton(Config::BUTTON_PREV);
DebouncedButton pauseButton(Config::BUTTON_PAUSE);
DebouncedButton nextButton(Config::BUTTON_NEXT);

int readEncoder() {
  static bool initialized = false;
  static uint8_t oldAB = 0;
  static int8_t accumulator = 0;
  static constexpr int8_t transitions[16] = {
    0, -1, 1, 0,
    1, 0, 0, -1,
    -1, 0, 0, 1,
    0, 1, -1, 0
  };
  const uint8_t ab = (digitalRead(Config::ENC_A) << 1) |
                      digitalRead(Config::ENC_B);
  if (!initialized) {
    oldAB = ab;
    initialized = true;
    return 0;
  }
  accumulator += transitions[(oldAB << 2) | ab];
  oldAB = ab;
  if (accumulator >= Config::ENCODER_EDGES_PER_STEP) {
    accumulator = 0;
    return 1;
  }
  if (accumulator <= -static_cast<int>(Config::ENCODER_EDGES_PER_STEP)) {
    accumulator = 0;
    return -1;
  }
  return 0;
}

uint16_t wrapTrack(int track) {
  if (track < 1) return Config::TRACK_COUNT;
  if (track > Config::TRACK_COUNT) return 1;
  return static_cast<uint16_t>(track);
}

void dfCommandPause() {
  if (!dfOnline) return;
  dfPlayer.pause();
  playing = false;
  lastDfCommandMs = millis();
}

void dfCommandPlay(uint16_t track) {
  if (!dfOnline) return;
  selectedTrack = wrapTrack(track);
  dfPlayer.playMp3Folder(selectedTrack);
  playingTrack = selectedTrack;
  playing = true;
  lastDfCommandMs = millis();
}

void dfCommandResume() {
  if (!dfOnline) return;
  dfPlayer.start();
  playing = true;
  lastDfCommandMs = millis();
}

void togglePause() {
  if (!dfOnline) return;
  if (playing) dfCommandPause();
  else if (playingTrack != 0) dfCommandResume();
  else dfCommandPlay(selectedTrack);
}

void setVolume(int newVolume) {
  volumeLevel = static_cast<uint8_t>(constrain(newVolume, 0, 30));
  if (dfOnline) {
    dfPlayer.volume(volumeLevel);
    lastDfCommandMs = millis();
  }
}

void readBattery() {
  uint32_t totalMv = 0;
  for (int i = 0; i < 8; ++i) totalMv += analogReadMilliVolts(Config::BATTERY_ADC);
  const float adcV = (totalMv / 8.0f) / 1000.0f;
  batteryVolts = adcV * 2.0f;
}

void updateDisplay() {
  if (!oledOnline) return;
  display.clearDisplay();
  display.setTextSize(1);
  display.setTextColor(SSD1306_WHITE);
  display.setCursor(0, 0);
  display.print(F("BluMoon  "));
  display.print(dfOnline ? (playing ? F("PLAY") : F("PAUSE")) : F("NO DF"));

  display.setCursor(0, 11);
  display.print(F("Track "));
  if (selectedTrack < 1000) display.print('0');
  if (selectedTrack < 100) display.print('0');
  if (selectedTrack < 10) display.print('0');
  display.print(selectedTrack);
  display.print('/');
  display.print(Config::TRACK_COUNT);

  display.setCursor(0, 22);
  display.print(F("V:"));
  display.print(volumeLevel);
  display.print(F("  Bat:"));
  display.print(batteryVolts, 2);
  display.print(F("V"));
  display.display();
}

void handlePlayerEvents() {
  if (!dfOnline || !dfPlayer.available()) return;
  const uint8_t type = dfPlayer.readType();
  const int value = dfPlayer.read();
  if (type == DFPlayerPlayFinished) {
    Serial.printf("Track finished: %d\n", value);
    selectedTrack = wrapTrack(static_cast<int>(playingTrack) + 1);
    dfCommandPlay(selectedTrack);
  } else if (type == DFPlayerError) {
    Serial.printf("DFPlayer error: %d\n", value);
  }
}

void setup() {
  Serial.begin(115200);
  delay(150);
  Serial.println(F("BluMoon firmware v0.1"));

  pinMode(Config::ENC_A, INPUT_PULLUP);
  pinMode(Config::ENC_B, INPUT_PULLUP);
  encButton.begin();
  prevButton.begin();
  pauseButton.begin();
  nextButton.begin();

  analogReadResolution(12);
  analogSetPinAttenuation(Config::BATTERY_ADC, ADC_11db);
  readBattery();

  Wire.begin(Config::OLED_SDA, Config::OLED_SCL);
  Wire.setClock(400000);
  oledOnline = display.begin(SSD1306_SWITCHCAPVCC, Config::OLED_ADDRESS);
  if (!oledOnline) Serial.println(F("OLED not detected at 0x3C"));
  else updateDisplay();

  dfSerial.begin(9600, SERIAL_8N1, Config::DF_RX, Config::DF_TX);
  dfOnline = dfPlayer.begin(dfSerial, /*ACK=*/false, /*reset=*/true);
  if (dfOnline) {
    dfPlayer.setTimeOut(500);
    dfPlayer.volume(volumeLevel);
    Serial.println(F("DFPlayer online. Rotate to choose; click to play."));
  } else {
    Serial.println(F("DFPlayer not responding. Check power, SD and UART."));
  }
  lastDfCommandMs = millis();
  updateDisplay();
}

void loop() {
  const int delta = readEncoder();
  if (delta != 0) {
    selectedTrack = wrapTrack(static_cast<int>(selectedTrack) + delta);
    Serial.printf("Selected track %u\n", selectedTrack);
  }
  const bool encClick = encButton.justPressed();
  const bool prevClick = prevButton.justPressed();
  const bool pauseClick = pauseButton.justPressed();
  const bool nextClick = nextButton.justPressed();

  if (millis() - lastDfCommandMs >= Config::COMMAND_GAP_MS) {
    if (encClick) {
      dfCommandPlay(selectedTrack);
    } else if (prevClick) {
      dfCommandPlay(wrapTrack(static_cast<int>(selectedTrack) - 1));
    } else if (nextClick) {
      dfCommandPlay(wrapTrack(static_cast<int>(selectedTrack) + 1));
    } else if (pauseClick) {
      togglePause();
    }
  }
  handlePlayerEvents();

  const unsigned long now = millis();
  if (now - lastBatteryMs >= Config::BATTERY_UPDATE_MS) {
    lastBatteryMs = now;
    readBattery();
  }
  if (now - lastDisplayMs >= Config::DISPLAY_UPDATE_MS) {
    lastDisplayMs = now;
    updateDisplay();
  }
  delay(1);
}
