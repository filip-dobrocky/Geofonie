#include <ArduinoOSCWiFi.h>
#include <string.h>

#include "../NetworkConfig.h"

const IPAddress ip(NetworkConfig::ap_ip);
const IPAddress gateway(NetworkConfig::gateway);
const IPAddress subnet(NetworkConfig::subnet);


// OSC input parameters
float slider_value = 0;

// OSC output parameters
int brightness = 0;  // how bright the LED_BUILTIN is
int fadeAmount = 1;


void to_acid_osc(const OscMessage& m) {
  String adr = m.address();
  int id = (int)m.arg<float>(0);
  float param = m.arg<float>(1);
  Serial.println(adr);
  Serial.printf("id %d param %f\n", id, param);
  OscWiFi.send(GET_ACID_IP(id), NetworkConfig::osc_from_ap, adr, param);
}

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
  OscWiFi.subscribe(NetworkConfig::osc_from_ctl, "/toAcid/*", to_acid_osc);

  pinMode(LED_BUILTIN, OUTPUT);
}

void loop() {
  OscWiFi.update();

  brightness = brightness + fadeAmount;

  if (brightness <= 0 || brightness >= 255) {
    fadeAmount = -fadeAmount;
  }

  OscWiFi.send(GET_ACID_IP(0), NetworkConfig::osc_from_ap, "/1/brightness", brightness);

  analogWrite(LED_BUILTIN, (int)(slider_value*255));

  delay(10);
}
