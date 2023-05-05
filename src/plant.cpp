#include "plant.h"
#include "consts.h"
#include <ArduinoJson.h>

char *chip_id() {
  uint64_t chipid = ESP.getEfuseMac();
  char chipid_str[17];
  sprintf(chipid_str, "%016llX", chipid);
  return chipid_str;
}

String Plant::credentials() {
  return String(String("{\"plant_id\":\"") + this->ID() + "\",\"chip_id\":\"" +
                chip_id() + "\"");
}

bool Plant::create(String id) {
  if (!_prefs.begin("plantpal_plant")) {
    Serial.println("Preferences could not begin");
    return false;
  }
  _prefs.putString("id", id);
  _prefs.end();

  return this->fetch();
}

bool Plant::fetch() {
  if (this->ID() == "") {
    Serial.println('Plant not created');
    return false;
  }
  String url = String(SERVER_URL) + "/plants/espget";
  _http.begin(url.c_str());

  int status_code = _http.POST(this->credentials());
  if (status_code != 200) {
    return false;
  }

  // parse json
  String payload = _http.getString();
  StaticJsonDocument<512> doc;
  deserializeJson(doc, payload.c_str());

  String plant_id = doc["id"].as<String>();
  uint16_t water_amount = doc["water_amount"].as<unsigned int>();
  bool auto_irrigation = doc["auto_irrigation"].as<bool>();
  uint8_t moisture_pecentage_threshold =
      doc["moisture_percentage_treshold"].as<unsigned int>();

  if (!_prefs.begin("plantpal_plant")) {
    Serial.println("Preferences could not begin");
    return false;
  }

  _prefs.putString("id", plant_id);
  _prefs.putUShort("water_amount", water_amount);
  _prefs.putBool("auto_irrigation", auto_irrigation);
  _prefs.putUChar("moisture_percentage_threshold",
                  moisture_pecentage_threshold);
  _prefs.end();

  return true;
};

bool Plant::shouldIrrigate(uint8_t moisture_percentage) {
  // if value is 255 then check server
  // else compare with moisture_pecentage_threshold
  if (moisture_percentage == 255) {
    if (this->ID() == "") {
      Serial.println('Plant not created');
      return false;
    }
    String url =
        String(SERVER_URL) + "/plants/should_irrigate_now";
    _http.begin(url.c_str());

    int status_code = _http.POST(this->credentials());
    if (status_code == 404) {
      return false;
    }

    // parse json
    String payload = _http.getString();
    StaticJsonDocument<512> doc;
    deserializeJson(doc, payload.c_str());

    bool should_irrigate_now = doc["irrigate"].as<bool>();
    return should_irrigate_now;
  }

  return moisture_percentage < this->moisturePercentageThreshold();
}

String Plant::ID() {
  if (!_prefs.begin("plantpal_plant")) {
    Serial.println("Preferences could not begin");
    return "";
  }
  String id = _prefs.getString("id");
  _prefs.end();

  return id;
}

uint16_t Plant::waterAmount() {
  if (!_prefs.begin("plantpal_plant")) {
    Serial.println("Preferences could not begin");
    return 65536;
  }
  uint16_t water_amount = _prefs.getUShort("water_amount");
  _prefs.end();

  return water_amount;
}

bool Plant::autoIrrigation() {
  if (!_prefs.begin("plantpal_plant")) {
    Serial.println("Preferences could not begin");
    return false;
  }
  bool auto_irrigation = _prefs.getBool("auto_irrigation");
  _prefs.end();

  return auto_irrigation;
}

uint8_t Plant::moisturePercentageThreshold() {
  if (!_prefs.begin("plantpal_plant")) {
    Serial.println("Preferences could not begin");
    return 255;
  }
  uint8_t moisture_percentage_threshold =
      _prefs.getUChar("moisture_percentage_threshold");
  _prefs.end();

  return moisture_percentage_threshold;
}
