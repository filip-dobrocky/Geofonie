#include <ArduinoOSCWiFi.h>

const char* ssid     = "TrychtyrLOM";
const char* password = "LomLomLom";

const IPAddress ip(192, 168, 0, 210);
const IPAddress gateway(192, 168, 10, 1);
const IPAddress subnet(255, 255, 255, 0);

const int recv_port = 54350;

int led_brightness = 0;

void setup() {
  Serial.begin(115200);
  delay(1000);

  WiFi.disconnect(true, true);
  delay(1000);
  WiFi.mode(WIFI_STA);

  WiFi.begin(ssid, password);
  WiFi.config(ip, gateway, subnet);

  while (WiFi.status() != WL_CONNECTED) {
        Serial.print(".");
        delay(500);
        static int count = 0;
        if (count++ > 20) {
            Serial.println("WiFi connection timeout, retry");
            WiFi.begin(ssid, password);
            count = 0;
        }
  }

  Serial.print("WiFi connected, IP = ");
  Serial.println(WiFi.localIP());

  OscWiFi.subscribe(recv_port, "/1/brightness", led_brightness);

  pinMode(LED_BUILTIN, OUTPUT);
}

void loop() {
  OscWiFi.update();

  analogWrite(LED_BUILTIN, led_brightness);
}
