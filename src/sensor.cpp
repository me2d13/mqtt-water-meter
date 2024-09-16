#include "sensor.h"
#include "config.h"
#include <Arduino.h>
#include <Debounce.h>
#include "mqtt.h"
#include "persist.h"

int lastValue = 1;

void setupSensor() {
    pinMode(PIN_PULSE, INPUT_PULLUP);
    debouncePins(PIN_PULSE, PIN_PULSE);
}

void loopSensor() {
    int inputValue = debouncedDigitalRead(PIN_PULSE);
    if (lastValue == 1 && inputValue == 0) {
        lastValue = inputValue;
        int liters = incLiters();
        Serial.print("Pulse ");
        Serial.println(liters);
        sendStateMessages();
    } else {
        lastValue = inputValue;
    }
}
