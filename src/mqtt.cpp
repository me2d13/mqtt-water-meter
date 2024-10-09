#include <ESP8266WiFi.h>
#include "mqtt.h"
#include <WifiClient.h>
#include "config.h"
#include <PubSubClient.h>
#include <ArduinoJson.h>
#include <persist.h>
#include "time_utils.h"

WiFiClient net;
PubSubClient client(net);

char buffer[100];
char id[13];

String lastHeartBeatTs = String("?");

char *discoPayloadPulsesFmt = "{\"name\":\"Water" SENSOR_VERSION " Usage Pulses\", \"unique_id\": \"water" SENSOR_VERSION 
    "_pulse_%s\", \"state_topic\": \"%s\", "
    " \"unit_of_measurement\":\"litres\", \"device\": %s, "
    " \"value_template\": \"{{value_json.value}}\""
    "}";
char *discoPayloadM3Fmt = "{\"name\":\"Water" SENSOR_VERSION " Usage M3\", \"unique_id\": \"water" SENSOR_VERSION
    "_m3_%s\", \"state_topic\": \"%s\", "
    "\"unit_of_measurement\":\"m³\", \"device\": %s, "
    " \"device_class\": \"water\", "
    " \"state_class\": \"total_increasing\", "
    " \"value_template\": \"{{value_json.value}}\""
    "}";
char *deviceFmt = "{\"name\":\"Water Metter " SENSOR_VERSION "\", \"ids\": [\"water_%s\"], \"cu\": \"http://%s\"}";

char *getId() {
  return id;
}

void connectMqtt() {
  // Loop until we're reconnected
  while (!client.connected()) {
    Serial.print("Attempting MQTT connection...");
    // Attempt to connect
    if (client.connect("watter-metter-" SENSOR_VERSION)) {
      Serial.println("connected");
      // Once connected, publish an announcement...
      sprintf(buffer, "Water metter connected to MQTT with IP  %s, dedicated log %s", WiFi.localIP().toString().c_str(), logTopic().c_str());
      client.publish(MQTT_GLOBAL_LOG_TOPIC,buffer);
      mqttLog(buffer);
      // ... and resubscribe
      client.subscribe(controlTopic().c_str());
    } else {
      Serial.print("Mqtt connect failed, rc=");
      Serial.print(client.state());
      Serial.println(" try again in 5 seconds");
      // Wait 5 seconds before retrying
      delay(5000);
    }
  }
}

void mqttHeartBeat() {
  if (client.connected()) {
    mqttLog("Water metter alive");
    lastHeartBeatTs = String(getTime());
  }
}

void logState(char* message_d, int value) {
  if (client.connected()) {
    sprintf(buffer, message_d, value);
    Serial.print("MQTT log: ");
    Serial.println(buffer);
    mqttLog(buffer);
  }
}

void mqttLog(char *message) {
  if (client.connected()) {
    client.publish(logTopic().c_str(), message);
  }
}

void callback(char* topic, byte* payload, unsigned int length) {
  Serial.print("Message arrived [");
  Serial.print(topic);
  Serial.print("] ");
  for (unsigned int i=0;i<length;i++) {
    Serial.print((char)payload[i]);
  }
  Serial.println();
  int val = 0;
  JsonDocument doc;
  DeserializationError error = deserializeJson(doc, payload);
  if (error) {
    Serial.print(F("deserializeJson() failed: "));
    Serial.println(error.f_str());
  } else {
    val = doc["liters"];
    Serial.print("Setting liters to ");
    Serial.println(val);
    if (client.connected()) {
      sprintf(buffer, "Received liters %d", val);
      client.publish(logTopic().c_str(),buffer);
      saveLiters(val);
      sendStateMessages();
    }
  }
}

void setupMqtt() {
  client.setBufferSize(1024);
  client.setServer(MQTT_SERVER, 1883);
  client.setCallback(callback);
  String mac = WiFi.macAddress();
  size_t j = 0;
  for (size_t i = 0; i < mac.length(); i++)
  {
    if (isAlphaNumeric(mac[i])) {
      id[j++] = mac[i];
    }
  }
  id[j] = 0;
}

String stateTopicPulses() {
  return MQTT_TOPIC_PREFIX + String(id) + MQTT_PULSES_POSTFIX;
}

String stateTopicM3() {
  return MQTT_TOPIC_PREFIX + String(id) + MQTT_M3_POSTFIX;
}

String controlTopic() {
  return MQTT_TOPIC_PREFIX + String(id) + MQTT_SET_POSTFIX;
}
String logTopic() {
  return MQTT_TOPIC_PREFIX + String(id) + MQTT_LOG_POSTFIX;
}

void mqttLoop() {
  if (!client.connected()) {
    connectMqtt();
  }
  client.loop();
}

uint8_t sendDiscovery(bool on) {
  char deviceBuff[200];
  sprintf(deviceBuff, deviceFmt, id, WiFi.localIP().toString().c_str());
  Serial.print("Device data: ");
  Serial.println(deviceBuff);
  String topic1 = MQTT_DISCOVERY_TOPIC_PREFIX + String("pulse_" SENSOR_VERSION "_") + String(id) + MQTT_DISCOVERY_TOPIC_POSTFIX;
  String topic2 = MQTT_DISCOVERY_TOPIC_PREFIX + String("m3_" SENSOR_VERSION "_") + String(id) + MQTT_DISCOVERY_TOPIC_POSTFIX;
  char topicBuff[700];
  if (client.connected()) {
    if (on) {
      sprintf(topicBuff, discoPayloadPulsesFmt, id, stateTopicPulses().c_str(), deviceBuff);
      Serial.print("Discovery topic: ");
      Serial.println(topic1.c_str());
      Serial.print("Discovery data: ");
      Serial.println(topicBuff);
      client.publish(topic1.c_str(), topicBuff);
      sprintf(topicBuff, discoPayloadM3Fmt, id, stateTopicM3().c_str(), deviceBuff);
      client.publish(topic2.c_str(), topicBuff);
    } else {
      client.publish(topic1.c_str(), "");
      client.publish(topic2.c_str(), "");
    }
    return 1;
  }
  return 0;
}

void sendStateMessages() {
  if (client.connected()) {
    sprintf(buffer, "{\"value\":%d, \"diff\": 1}", getLiters());
    client.publish(stateTopicPulses().c_str(), buffer);
    sprintf(buffer, "{\"value\":%f, \"diff\": 0.001}", ((float)getLiters() / 1000));
    client.publish(stateTopicM3().c_str(), buffer);
  }
}

String getLastHeartBeatTs() {
  return lastHeartBeatTs;
}