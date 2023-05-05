#include "credentials.h"
#include "consts.h"

void Credentials::end() { preferences.end(); }

void Credentials::readWiFi(String *ssid, String *pass) {
  if (!preferences.begin("plantpal_wifi")) {
    Serial.println("Preferences could not begin");
    return;
  }
  *ssid = preferences.getString("ssid");
  *pass = preferences.getString("pass");
  end();
}

void Credentials::writeWiFi(String ssid, String pass) {
  if (!preferences.begin("plantpal_wifi")) {
    Serial.println("Preferences could not begin");
    return;
  }
  preferences.putString("ssid", ssid);
  preferences.putString("pass", pass);
  end();
}

bool Credentials::wifiNotWritten() {
  String ssid, pass;
  readWiFi(&ssid, &pass);
  return ssid == "" || pass == "";
}

void Credentials::readLocalWiFi(String *ssid, String *pass) {
  if (!preferences.begin("plantpal_wifi")) {
    Serial.println("Preferences could not begin");
    return;
  }
  *ssid = preferences.getString("localssid", PROTO_SSID);
  *pass = preferences.getString("localpass", PROTO_PASSWORD);
  end();
}

void Credentials::writeLocalWiFi(String ssid, String pass) {
  if (!preferences.begin("plantpal_wifi")) {
    Serial.println("Preferences could not begin");
    return;
  }
  preferences.putString("localssid", ssid);
  preferences.putString("localpass", pass);
  end();
}
