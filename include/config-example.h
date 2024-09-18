// rename this file to config.h and update values below

#ifndef CONFIG_H
#define CONFIG_H

// prefix
#define MQTT_TOPIC_PREFIX "/devices/water-metter-"
// status topic
#define MQTT_PULSES_POSTFIX "/pulses"
#define MQTT_M3_POSTFIX "/m3"
// control topic
#define MQTT_SET_POSTFIX "/set"
// logging topic
#define MQTT_LOG_POSTFIX "/log"

#define MQTT_DISCOVERY_TOPIC_PREFIX "homeassistant/sensor/"
#define MQTT_DISCOVERY_TOPIC_POSTFIX "/config"
// logging topic to send IP once
#define MQTT_GLOBAL_LOG_TOPIC "log"
#define WIFI_SSID "mysid"
#define WIFI_PASSWORD "mypwd"
#define MQTT_SERVER "192.168.1.1"
// interval in seconds to send heartbeat message
#define HEART_BEAT_S 60*60
// interval in seconds to blink built-in LED
#define BLICK_S 30

// Node MCU
#define PIN_PULSE D1 

// Persist thresholds - after what difference state should be persisted to eeprom (if value differs)
#define PERSIST_AFTER_LITERS 50
#define PERSIST_AFTER_MINUTES 120

#define MY_TZ "CET-1CEST,M3.5.0/02,M10.5.0/03" 

#endif