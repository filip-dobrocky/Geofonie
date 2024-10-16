#include <ArduinoOSCWiFi.h>

#include "../NetworkConfig.h"

const IPAddress ip(NetworkConfig::ap_ip);
const IPAddress gateway(NetworkConfig::gateway);
const IPAddress subnet(NetworkConfig::subnet);


// OSC input parameters
float slider_value = 0;

// OSC output parameters
int brightness = 0;  // how bright the LED_BUILTIN is
int fadeAmount = 1;


// void on_toggle(const OscMessage& m) {
//   Serial.printf("%s %d %d %s ", m.remoteIP(), m.remotePort(), m.size(), m.address());
//   int toggle = m.arg<int>(0);
//   Serial.print(toggle);
//   Serial.println();

//   digitalWrite(LED_BUILTIN, !toggle);
// }

void setup() {
  Serial.begin(115200);
  delay(1000);
  Serial.println("\n[*] Creating AP");
  WiFi.mode(WIFI_AP);
  WiFi.softAPConfig(ip, gateway, subnet);
  WiFi.softAP(NetworkConfig::ssid, NetworkConfig::password, 1, 0, 20);
  Serial.print("[+] AP Created with IP Gateway ");
  Serial.println(WiFi.softAPIP());

  OscWiFi.subscribe(NetworkConfig::osc_from_ctl, "/slider1", slider_value);

  pinMode(LED_BUILTIN, OUTPUT);
}

void loop() {
  OscWiFi.update();

  brightness = brightness + fadeAmount;

  if (brightness <= 0 || brightness >= 255) {
    fadeAmount = -fadeAmount;
  }

  OscWiFi.send(GET_OBJ_IP(0), NetworkConfig::osc_from_ap, "/1/brightness", brightness);

  analogWrite(LED_BUILTIN, (int)(slider_value*255));

  delay(10);
}
