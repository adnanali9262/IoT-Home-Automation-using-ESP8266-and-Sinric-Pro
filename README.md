# ESP8266 Smart Home Controller

An IoT-based smart home automation system using ESP8266, Sinric Pro, WiFiManager, EEPROM, and OTA updates.

## Features

- Dual relay control
- Toggle and impulse relay modes
- Manual push button switching
- Wi-Fi provisioning using WiFiManager
- Wi-Fi credential reset button
- EEPROM relay state memory
- OTA firmware updates
- Contact sensor support
- LED Wi-Fi status indication
- Sinric Pro cloud integration

## Hardware Used

- ESP8266 NodeMCU
- Relay Module
- Push Buttons
- Contact Sensor
- Power Supply

## Pin Configuration

| Component | Pin |
|-----------|-----|
| Relay 1 | D1 |
| Relay 2 | D2 |
| Button 1 | D5 |
| Button 2 | D6 |
| Contact Sensor | D7 |
| WiFi Reset | D3 |
| LED Status | D4 |

## Libraries Used

- ESP8266WiFi
- WiFiManager
- EEPROM
- SinricPro
- ArduinoOTA

## Setup

1. Install required libraries.
2. Replace placeholders with Sinric credentials.
3. Upload code to ESP8266.
4. Configure Wi-Fi using WiFiManager portal.
5. Control switches through Sinric Pro.

## Future Improvements

- Web dashboard
- MQTT support
- Energy monitoring
- Mobile PWA dashboard