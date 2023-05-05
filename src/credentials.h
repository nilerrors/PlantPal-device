#include <Arduino.h>
#include <Preferences.h>

class Credentials {
private:
  Preferences preferences;
  void begin();
  void end();

public:
  bool setupDone();
  bool wifiNotWritten();
  void readWiFi(String *ssid, String *pass);
  void writeWiFi(String ssid, String pass);

  void readLocalWiFi(String *ssid, String *pass);
  void writeLocalWiFi(String ssid, String pass);
};
