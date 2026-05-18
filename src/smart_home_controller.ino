#include <ESP8266WiFi.h>
#include <WiFiManager.h>
#include <EEPROM.h>
#include <SinricPro.h>
#include <SinricProSwitch.h>
#include <SinricProContactsensor.h>
#include <ArduinoOTA.h>

#define RELAY1_PIN        D1
#define RELAY2_PIN        D2
#define BUTTON1_PIN       D5
#define BUTTON2_PIN       D6
#define CONTACT_PIN       D7
#define WIFI_RESET_PIN    D3
#define LED_BUILTIN_PIN   D4

#define DEVICE_ID_1   "YOUR_DEVICE_ID_1"
#define DEVICE_ID_2   "YOUR_DEVICE_ID_2"
#define DEVICE_ID_3   "YOUR_DEVICE_ID_3"

#define APP_KEY       "YOUR_APP_KEY"
#define APP_SECRET    "YOUR_APP_SECRET"

#define EEPROM_ADDR       0
#define IMPULSE_DURATION  300  // ms

bool relay1State = false;
bool relay2State = false;
bool lastContactState = false;

unsigned long impulseStartTime = 0;
bool impulseActive = false;

void setRelayState(uint8_t pin, bool state) {
  digitalWrite(pin, state ? LOW : HIGH);  // LOW = ON, HIGH = OFF
}

void saveRelayStates() {
  EEPROM.write(EEPROM_ADDR, relay1State);
  EEPROM.write(EEPROM_ADDR + 1, relay2State);
  EEPROM.commit();
}

void loadRelayStates() {
  relay1State = EEPROM.read(EEPROM_ADDR);
  relay2State = EEPROM.read(EEPROM_ADDR + 1);
  setRelayState(RELAY1_PIN, relay1State);
  setRelayState(RELAY2_PIN, false); // Impulse is off by default
}

bool onPowerState1(const String &deviceId, bool state) {
  if (deviceId == DEVICE_ID_1) {
    relay1State = state;
    setRelayState(RELAY1_PIN, relay1State);
    saveRelayStates();
    return true;
  }
  return false;
}

bool onPowerState2(const String &deviceId, bool state) {
  if (deviceId == DEVICE_ID_2) {
    if (state) {
      relay2State = true;
      setRelayState(RELAY2_PIN, true);
      impulseStartTime = millis();
      impulseActive = true;
    }
    return true;
  }
  return false;
}

void IRAM_ATTR handleButton1() {
  relay1State = !relay1State;
  setRelayState(RELAY1_PIN, relay1State);
  SinricProSwitch& sw = SinricPro[DEVICE_ID_1];
  sw.sendPowerStateEvent(relay1State);
  saveRelayStates();
}

void IRAM_ATTR handleButton2() {
  relay2State = true;
  setRelayState(RELAY2_PIN, true);
  impulseStartTime = millis();
  impulseActive = true;
  SinricProSwitch& sw = SinricPro[DEVICE_ID_2];
  sw.sendPowerStateEvent(true);
}

void IRAM_ATTR handleWifiReset() {
  WiFi.disconnect(true);
  ESP.eraseConfig();
  delay(1000);
  ESP.reset();
}

void setupSinric() {
  SinricProSwitch& switch1 = SinricPro[DEVICE_ID_1];
  switch1.onPowerState(onPowerState1);

  SinricProSwitch& switch2 = SinricPro[DEVICE_ID_2];
  switch2.onPowerState(onPowerState2);

  SinricProContactsensor& contact = SinricPro[DEVICE_ID_3];
  SinricPro.onConnected([]() { Serial.println("SinricPro connected"); });
  SinricPro.onDisconnected([]() { Serial.println("SinricPro disconnected"); });
  SinricPro.begin(APP_KEY, APP_SECRET);
}

void setupPins() {
  pinMode(RELAY1_PIN, OUTPUT);
  pinMode(RELAY2_PIN, OUTPUT);
  pinMode(BUTTON1_PIN, INPUT_PULLUP);
  pinMode(BUTTON2_PIN, INPUT_PULLUP);
  pinMode(CONTACT_PIN, INPUT_PULLUP);
  pinMode(WIFI_RESET_PIN, INPUT_PULLUP);
  pinMode(LED_BUILTIN_PIN, OUTPUT);

  attachInterrupt(digitalPinToInterrupt(BUTTON1_PIN), handleButton1, FALLING);
  attachInterrupt(digitalPinToInterrupt(BUTTON2_PIN), handleButton2, FALLING);
  attachInterrupt(digitalPinToInterrupt(WIFI_RESET_PIN), handleWifiReset, FALLING);
}

void reportContactState(SinricProContactsensor &sensor, bool currentState) {
  sensor.sendContactEvent(currentState ? "closed" : "open");
}

void setupOTA() {
  ArduinoOTA.begin();
}

void setup() {
  Serial.begin(115200);
  EEPROM.begin(512);

  setupPins();

  WiFiManager wm;
  wm.setConfigPortalTimeout(180);
  if (!wm.autoConnect("SmartHomeSetup")) {
    ESP.restart();
    delay(5000);
  }

  setupOTA();
  setupSinric();
  loadRelayStates();
}

void loop() {
  SinricPro.handle();
  ArduinoOTA.handle();

  // LED WiFi Status
  digitalWrite(LED_BUILTIN_PIN, WiFi.isConnected() ? LOW : HIGH);  // LOW = ON

  // Impulse logic
  if (impulseActive && millis() - impulseStartTime >= IMPULSE_DURATION) {
    setRelayState(RELAY2_PIN, false);
    relay2State = false;
    impulseActive = false;
    SinricProSwitch& sw = SinricPro[DEVICE_ID_2];
    sw.sendPowerStateEvent(false);
  }

  // Contact sensor
  bool contact = digitalRead(CONTACT_PIN) == LOW;
  if (contact != lastContactState) {
    lastContactState = contact;
    SinricProContactsensor& sensor = SinricPro[DEVICE_ID_3];
    reportContactState(sensor, contact);
  }

  delay(10); // Let CPU breathe
}
