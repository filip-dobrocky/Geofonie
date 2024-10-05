#include <ArduinoOSCWiFi.h>

const char* ssid     = "TrychtyrLOM";
const char* password = "LomLomLom";

const char* host1 = "192.168.0.210";

const IPAddress ip(192, 168, 0, 201);
const IPAddress gateway(192, 168, 10, 1);
const IPAddress subnet(255, 255, 255, 0);

const int recv_port = 54345;
const int send_port = 54350;

int brightness = 0;  // how bright the LED_BUILTIN is
int fadeAmount = 5;


void on_toggle(const OscMessage& m) {
  Serial.printf("%s %d %d %s ", m.remoteIP(), m.remotePort(), m.size(), m.address());
  int toggle = m.arg<int>(0);
  Serial.print(toggle);
  Serial.println();

  digitalWrite(LED_BUILTIN, !toggle);
}

void setup() {
  Serial.begin(115200);
  delay(1000);
  Serial.println("\n[*] Creating AP");
  WiFi.mode(WIFI_AP);
  WiFi.softAPConfig(ip, gateway, subnet);
  WiFi.softAP(ssid, password);
  Serial.print("[+] AP Created with IP Gateway ");
  Serial.println(WiFi.softAPIP());

  OscWiFi.subscribe(recv_port, "/1/toggle1", on_toggle);

  pinMode(LED_BUILTIN, OUTPUT);
}

void loop() {
  OscWiFi.update();

  brightness = brightness + fadeAmount;

  if (brightness <= 0 || brightness >= 255) {
    fadeAmount = -fadeAmount;
  }

  delay(10);
  OscWiFi.send(host1, send_port, "/1/brightness", brightness);
}
