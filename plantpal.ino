#include "src/consts.h"
#include "src/credentials.h"
#include "src/flowmeter.h"
#include "src/plant.h"
#include "src/server.h"
#include <ArduinoJson.h>
#include <WiFi.h>

FlowMeter flow_meter = {FLOW_METER_PIN, 0, 0, false, 0};

void IRAM_ATTR pulseCounter() {
  flow_meter.pulses_count++;
  flow_meter.running = true;
}

unsigned long last_time = 0;
unsigned long last_time_pump_check = 0;
unsigned long last_time_plant_fetch = 0;

IPAddress local_IP(8, 8, 8, 8);
IPAddress gateway = local_IP;
IPAddress subnet(255, 255, 255, 0);

ConfigServer server;
Credentials credentials;
Plant plant;

void setup() {
  Serial.begin(115200);

  WiFi.mode(WIFI_AP_STA);
  WiFi.config(INADDR_NONE, INADDR_NONE, INADDR_NONE, INADDR_NONE);
  WiFi.setHostname(HOSTNAME);
  WiFi.softAPConfig(local_IP, gateway, subnet);
  String localSSID, localPass;
  credentials.readLocalWiFi(&localSSID, &localPass);
  WiFi.softAP(localSSID.c_str(), localPass.c_str());

  Serial.print("IP Address = ");
  Serial.println(WiFi.softAPIP());

  Serial.print("Setting up Web Server ... ");
  server.onChangeWifi([&](String ssid, String pass) {
    credentials.writeWiFi(ssid, pass);
    Serial.println("Wifi is written");
  });
  server.onPlantCreate([&](String payload) {
    StaticJsonDocument<512> doc;
    deserializeJson(doc, payload.c_str());

    String plant_id = doc["id"].as<String>();

    plant.create(plant_id);
  });
  server.start();

  if (!credentials.wifiNotWritten()) {
    String ssid, pass;
    credentials.readWiFi(&ssid, &pass);
    WiFi.begin(ssid.c_str(), pass.c_str());
    Serial.print("Connecting to WiFi ");
    int times = 0;
    while (WiFi.status() != WL_CONNECTED) {
      times++;
      if (times > 20) { // 60000 -> 60sec
        Serial.println(" Could not connect");
        credentials.writeWiFi("", "");
        Serial.println("Wifi network credentials need to be updated.");
        Serial.println("Restarting ESP");
        ESP.restart();
      }
      Serial.print(".");
      delay(1000);
    }
    Serial.println(" Connected");
  }

  pinMode(WATER_PUMP_PIN, OUTPUT);
  pinMode(FLOW_METER_PIN, INPUT_PULLUP);
  attachInterrupt(digitalPinToInterrupt(flow_meter.PIN), pulseCounter, FALLING);
}

void loop() {
  server.handleClient();

  if (credentials.wifiNotWritten()) {
    return;
  }
  if (WiFi.status() != WL_CONNECTED) {
    // Add functionality to ensure
    // that if the issue precedes,
    // the plant still gets irrigated

    Serial.println("WiFi not connected\nTrying to reconnect");
    WiFi.reconnect();
    int times = 0;
    while (WiFi.status() != WL_CONNECTED) {
      times++;
      if (times > 30) {
        Serial.println(" Could not connect");
        credentials.writeWiFi("", "");
        Serial.println("Wifi network credentials need to be updated.");
        Serial.println("Restarting ESP");
        ESP.restart();
      }
      Serial.print(".");
      delay(1000);
    }
  }

  if (!plant.isCreated()) {
    Serial.println("Plant is not created");
    delay(500);
    return;
  }

  // Get plant configuration data
  // every 5 minutes or custom

  if ((millis() - last_time_plant_fetch) > 30000) {
    plant.fetch();
  }

  // Get data about irrigation
  // every 30 seconds
  if ((millis() - last_time_pump_check) > 30000) {
    if (plant.shouldIrrigate()) {
      // start pomp
      // Serial.println("Start pomp");
      // delay(1000);
      digitalWrite(WATER_PUMP_PIN, HIGH);
    } else {
      // start pomp
      // Serial.println("Start pomp");
      // delay(1000);
      digitalWrite(WATER_PUMP_PIN, HIGH);
    }
  }

  if ((millis() - last_time) > INTERVAL) {
    /*
     * moisture_sensor
     */
    uint16_t moisture_analog = analogRead(MOISTURE_SENSOR_PIN);
    Serial.println(moisture_analog);
    uint8_t moisture_percentage =
        map(moisture_analog, MOISTURE_NONE, MOISTURE_WET, 0, 100);
    if (moisture_percentage > 100)
      moisture_percentage = 100;
    if (moisture_percentage < 0)
      moisture_percentage = 0;

    // Serial.print("Moisture = ");
    // Serial.print(moisture_percentage);
    // Serial.println("%");
    // delay(1000);

    // Check if plant should be irrigated
    // based on the data from irrigation
    if (plant.shouldIrrigate(moisture_percentage)) {
      // start pomp
      // Serial.println("Start pomp");
      // delay(1000);
      digitalWrite(WATER_PUMP_PIN, HIGH);
    }

    /*
     * flow meter
     */
    if (flow_meter.running) {
      uint8_t pulse_count = flow_meter.pulses_count;
      float flow_rate =
          ((1000.0 / (millis() - flow_meter.last_running)) * pulse_count) /
          CALIBRATION_FACTOR;
      // Serial.print("Flow Rate: ");
      // Serial.println(flow_rate);
      flow_meter.last_running = millis();
      flow_meter.running = false;

      float flow_milli_litres = (flow_rate / 60) * 1000;
      // Serial.print("Flow Milli Litres: ");
      // Serial.println(flow_milli_litres);
      // delay(1000);
      flow_meter.total_milli_litres += flow_milli_litres;

      // Serial.print("Total milli Litres: ");
      // Serial.println(flow_meter.total_milli_litres);
      // delay(1000);

      if (flow_meter.total_milli_litres >= plant.waterAmount()) {
        flow_meter.total_milli_litres = 0;
        // stop pomp
        // Serial.println("Stop pomp");
        // delay(1000);
        digitalWrite(WATER_PUMP_PIN, LOW);
      }
    }

    last_time = millis();
  }
}
