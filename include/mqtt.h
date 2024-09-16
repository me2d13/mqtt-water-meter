#ifndef MQTT_H
#define MQTT_H

#include <Arduino.h>

void mqttHeartBeat();
void setupMqtt();
void connectMqtt();
void mqttLoop();
void mqttLog(char *message);
void logState(char* message_d, int value);
void sendStateMessages();
char *getId();
String stateTopicPulses();
String stateTopicM3();
String controlTopic();
String logTopic();
uint8_t sendDiscovery(bool on);

#endif