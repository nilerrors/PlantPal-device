#include <WebServer.h>
#include <functional>

typedef std::function<void(String ssid, String pass)> WifiChangeHandler;
typedef std::function<void(String payload)> PlantCreateHandler;

class ConfigServer {
private:
  WebServer *server;

  void response_base(int statusCode, String body, String title);
  void response_base_json(int statusCode, String body);

  WifiChangeHandler changeWifiCallback;
  PlantCreateHandler plantCreateCallback;

public:
  ConfigServer();
  void start();
  void handleClient();
  void onChangeWifi(WifiChangeHandler fn);
  void onPlantCreate(PlantCreateHandler fn);
};
