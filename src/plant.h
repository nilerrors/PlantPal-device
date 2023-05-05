#include <Arduino.h>
#include <HTTPClient.h>
#include <Preferences.h>
#include <WiFi.h>

class Plant {
private:
  Preferences _prefs;
  HTTPClient _http;

  String credentials();

public:
  String ID();
  uint16_t waterAmount();
  bool autoIrrigation();
  uint8_t moisturePercentageThreshold();

  bool create(String id);
  bool shouldIrrigate(uint8_t moisture_percentage = 255);
  bool fetch();
};
