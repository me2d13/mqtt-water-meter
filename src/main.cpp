#include <Arduino.h>
#include "config.h"
#include "ota.h"
#include "mqtt.h"
#include <Ticker.h>
#include "sensor.h"
#include <ESP8266WiFi.h>
#include "webserver.h"


const char* ssid = WIFI_SSID;
const char* password = WIFI_PASSWORD;


void blick();

Ticker heartBeatTimer(mqttHeartBeat, HEART_BEAT_S*1000);
Ticker blickTimer(blick, BLICK_S*1000);

void handler(u8 value) {
  if (value) {
    //digitalWrite(PIN_RELAY, HIGH);
    logState(1);
  } else {
    //digitalWrite(PIN_RELAY, LOW);
    logState(0);
  }
}

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
}

void loop() {
  ArduinoOTA.handle();
  mqttLoop();
  heartBeatTimer.update();
  blickTimer.update();
  loopSensor();
}

void blick() {
  digitalWrite(LED_BUILTIN, LOW);
  delay(5);
  digitalWrite(LED_BUILTIN, HIGH);
}