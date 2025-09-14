#include <ArduinoOSCWiFi.h>
#include <string.h>

#include "../NetworkConfig.h"

#define MAX_PERIOD 1000

const IPAddress ip(NetworkConfig::ap_ip);
const IPAddress gateway(NetworkConfig::gateway);
const IPAddress subnet(NetworkConfig::subnet);


// OSC input parameters
bool auto_mode = true;
float auto_density = 0.1;

// OSC output parameters
int brightness = 0;  // how bright the LED_BUILTIN is
int fadeAmount = 1;

bool events[5] = {false};


void to_acid_osc(const OscMessage& m) {
  String adr = m.address();
  int id = (int)m.arg<float>(0);
  float param = m.arg<float>(1);
  
  OscWiFi.send(GET_ACID_IP(id), NetworkConfig::osc_from_ap, adr, param);

  Serial.print(adr);
  Serial.printf(" id %d param %f\n", id, param);
}

void to_roto_osc(const OscMessage& m) {
  String adr = m.address();
  int id = (int)m.arg<float>(0);
  float param = m.arg<float>(1);
  
  OscWiFi.send(GET_ROTO_IP(id), NetworkConfig::osc_from_ap, adr, param);

  Serial.print(adr);
  Serial.printf(" id %d param %f\n", id, param);
}

void global_osc(const OscMessage& m) {
  String adr = m.address();
  float param = m.arg<float>(0);

  for (int i = 0; i < 5; i++) {
    const char* ip = GET_ROTO_IP(i);
    if (adr.endsWith("/rotSpeed")) {
      OscWiFi.send(ip, NetworkConfig::osc_from_ap, "/toRoto/rotation/speed", param);
    }
  }

  if (adr.endsWith("/autoDensity")) {
    auto_density = m.arg<float>(0);
  }
  else if (adr.endsWith("/auto")) {
    auto_mode = m.arg<float>(0) > 0;
  }
  
  // for (int i = 0; i < 5; i++) {
  //   const char* ip = GET_ACID_IP(i);

  //   if (adr.endsWith("/acidVolume")) {
  //     OscWiFi.send(ip, NetworkConfig::osc_from_ap, "/toRoto/rotSpeed", param);
  //   }
  // }

}

void auto_routine() {
  uint32_t event_period = (uint32_t)floor(MAX_PERIOD / auto_density);
  static uint32_t elapsed = millis();

  if (millis() - elapsed >= event_period) {
    for (int i = 0; i < 5; i++) {
      const char* ip = GET_ACID_IP(i);
      int trigger = random(100);

      if (trigger > 40) {
        OscWiFi.send(ip, NetworkConfig::osc_from_ap, "/toAcid/param4", (float)random(100) / 100);
        OscWiFi.send(ip, NetworkConfig::osc_from_ap, "/toAcid/param5", (float)random(20, 100) / 100);
        OscWiFi.send(ip, NetworkConfig::osc_from_ap, "/toAcid/param6", (float)random((int)floor(auto_density * 100)) / 100);
        events[i] = true;
      }
    }

    elapsed = millis();
  }
}

void kill_events() {
  static uint32_t kill_period = 2000;
  static uint32_t elapsed = millis();

  if (millis() - elapsed >= kill_period) {
    for (int i = 0; i < 5; i++) {
      const char* ip = GET_ACID_IP(i);
      if (events[i]) {
        const char* ip = GET_ACID_IP(i);
        OscWiFi.send(ip, NetworkConfig::osc_from_ap, "/toAcid/param4", 0.0f);
        OscWiFi.send(ip, NetworkConfig::osc_from_ap, "/toAcid/param5", 0.0f);
        OscWiFi.send(ip, NetworkConfig::osc_from_ap, "/toAcid/param6", 0.0f);
        events[i] = false;
      }
    }

    elapsed = millis();
  }
  kill_period = random(1, 5) * 2000;
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

  OscWiFi.subscribe(NetworkConfig::osc_from_ctl, "/toAcid/*", to_acid_osc);
  OscWiFi.subscribe(NetworkConfig::osc_from_ctl, "/toRoto/*/*", to_roto_osc);
  OscWiFi.subscribe(NetworkConfig::osc_from_ctl, "/global/*", global_osc);


  pinMode(LED_BUILTIN, OUTPUT);
}

void loop() {
  OscWiFi.update();

  if (auto_mode) {
    auto_routine();
    kill_events();
  }

  delay(10);
}
