# mqtt-water-meter
This is code for ESP8266 device which takes ON/OFF pulses from water meter as input and sends mqtt messages to home assistant to show water usage in energy dasboard. It covers entity creation in HA via MQTT discovery. It supports debouncing on the sensor input as my water meter has some noisy contact. At the beginning I was using [ESPhome](https://esphome.io) but could not get debouncing working properly. So I used own solution.

## Requirements version 1
My requirements were (aka covered features)
* connection via wifi
* OTA (upload via wifi)
* debouncing on input sensor
* every OFF->ON pulse sends mqtt message
* device is stateless - total consumption is held in home assistant because it's persisted out of the box there
* automatic MQTT autodiscovery - device subscribes to Home Assistant status topic and automatically sends discovery messages when HA restarts

Technically ESP8266 could act as sensor with absolute value and send total consumption as number to HA. This would bring easier entity configuration in HA. But in that case the value had to be persisted in ESP8266 to have correct value after power lost. To have precise results the consumption would have to be persisted after every pulse which could ruin the EEPROM chip after some time. The soultion would be persist the number only after some number of pulses (or time) or keep total value in HA and send only increments (which is what is implemented here).

## Version 2
Keeping the state in HA appeared to be quite fragile - every lost of the value (e.g. problems with Recorder) destroys water monitoring as it creates step to 0 and back.

So keeping the state in ESP seems better approach. The total value is persisted to flash after given amount of pulses or after given time interval (see config.h). Let's see if this is more stable.