#include <Arduino.h>
#include "config.h"
#include "ota.h"
#include "mqtt.h"
#include <Ticker.h>
#include "sensor.h"
#include <ESP8266WiFi.h>
#include "webserver.h"
#include "persist.h"
#include "time_utils.h"

// mosquitto_sub -t /devices/water-metter-5CCF7F802F8D/log
// mosquitto_sub -t /devices/water-metter-5CCF7F802F8D/m3
// mosquitto_pub -t /devices/water-metter-5CCF7F802F8D/set -m '{"liters":1123}'

const char* ssid = WIFI_SSID;
const char* password = WIFI_PASSWORD;


void blick();

Ticker heartBeatTimer(mqttHeartBeat, HEART_BEAT_S*1000);
Ticker blickTimer(blick, BLICK_S*1000);

void setupAndWaitForWifi() {
  WiFi.mode(WIFI_STA);
  WiFi.begin(ssid, password);
  while (WiFi.waitForConnectResult() != WL_CONNECTED) {
    Serial.println("Connection Failed! Rebooting...");
    delay(5000);
    ESP.restart();
  }
  Serial.println('\n');
  Serial.println("Connection established!");
  WiFi.setAutoReconnect(true);
}

void setup() {
  pinMode(LED_BUILTIN, OUTPUT);  // initialize onboard LED as output
  //pinMode(PIN_RELAY, OUTPUT);  
  configTime(MY_TZ, "pool.ntp.org");
  Serial.begin(115200);
  Serial.println("Booting");
  setupAndWaitForWifi();
  setupOTA();
  setupMqtt();
  connectMqtt();
  heartBeatTimer.start();
  blickTimer.start();
  digitalWrite(LED_BUILTIN, HIGH);
  //setRelayHandler(handler);
  setupSensor();
  setupWebServer();
  setupPersist();
}

void loop() {
  ArduinoOTA.handle();
  mqttLoop();
  heartBeatTimer.update();
  blickTimer.update();
  loopSensor();
  loopPersist();
}

void blick() {
  digitalWrite(LED_BUILTIN, LOW);
  delay(5);
  digitalWrite(LED_BUILTIN, HIGH);
  //Serial.println(getTime());
}