#include <ESP8266WiFi.h>
#include "mqtt.h"
#include <WifiClient.h>
#include "config.h"
#include <PubSubClient.h>
#include <ArduinoJson.h>

WiFiClient net;
PubSubClient client(net);
StaticJsonDocument<200> doc;

char buffer[100];
char id[13];

char *discoPayloadPulsesFmt = "{\"name\":\"Water4 Usage Pulses\", \"unique_id\": \"water4_pulse_%s\", \"state_topic\": \"%s\", "
    " \"unit_of_measurement\":\"litres\", \"device\": %s, "
    " \"value_template\": \"{%% if states(entity_id) == 'unknown' %%} {{value_json.value}} {%% else %%} {{ float(states(entity_id)) + value_json.value }} {%% endif %%}\""
    "}";
char *discoPayloadM3Fmt = "{\"name\":\"Water4 Usage M3\", \"unique_id\": \"water4_m3_%s\", \"state_topic\": \"%s\", "
    "\"unit_of_measurement\":\"m³\", \"device\": %s, "
    " \"device_class\": \"water\", "
    " \"state_class\": \"total_increasing\", "
    " \"value_template\": \"{%% if states(entity_id) == 'unknown' %%} {{value_json.value}} {%% else %%} {{ float(states(entity_id)) + value_json.value }} {%% endif %%}\""
    "}";
char *deviceFmt = "{\"name\":\"Water Metter\", \"ids\": [\"water_%s\"], \"cu\": \"http://%s\"}";

char *getId() {
  return id;
}

void connectMqtt() {
  // Loop until we're reconnected
  while (!client.connected()) {
    Serial.print("Attempting MQTT connection...");
    // Attempt to connect
    if (client.connect("watter-metter")) {
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
    mqttLog("Water metter alive");
}

void logState(int value) {
  if (client.connected()) {
    sprintf(buffer, "Water-metter state  %d", value);
    client.publish(logTopic().c_str(),buffer);
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
  u8 val = 0;
  u8 valSet = 0;
  DeserializationError error = deserializeJson(doc, payload);
  if (error) {
    Serial.print(F("deserializeJson() failed: "));
    Serial.println(error.f_str());
    if (strncmp("ON", (char *) payload, 2) == 0) {
      val = 1;
      valSet = 1;
    }
    if (strncmp("OFF", (char *) payload, 3) == 0) {
      val = 0;
      valSet = 1;
    }
    if (!valSet) {
      Serial.println("Non-json content not recognized (expected ON/OFF)");
      return;
    }
  } else {
    val = doc["value"];
    valSet = 1;
  }
  if (valSet) {
      Serial.print("Setting relay to ");
      Serial.println(val);
//      if (relayHandler != NULL) {
//          relayHandler(val);
//      } else {
          Serial.println("WARNING: relay handler not set");
//      }
      if (client.connected()) {
        //client.publish(stateTopic().c_str(), val ? "ON" : "OFF");
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
  String topic1 = MQTT_DISCOVERY_TOPIC_PREFIX + String("pulse_4_") + String(id) + MQTT_DISCOVERY_TOPIC_POSTFIX;
  String topic2 = MQTT_DISCOVERY_TOPIC_PREFIX + String("m3_4_") + String(id) + MQTT_DISCOVERY_TOPIC_POSTFIX;
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

void sendLiterPulse() {
  if (client.connected()) {
    client.publish(stateTopicPulses().c_str(), "{\"value\":1}");
    client.publish(stateTopicM3().c_str(), "{\"value\":0.001}");
  }
}