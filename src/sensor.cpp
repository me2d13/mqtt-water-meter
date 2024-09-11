#include "sensor.h"
#include "config.h"
#include <Arduino.h>
#include <Debounce.h>
#include "mqtt.h"

int pulses = 0;
int lastValue = 1;

int getPulses() {
    return pulses;
}

void setPulses(int numberOfPulses) {
    pulses = numberOfPulses;
}

void setupSensor() {
    pinMode(PIN_PULSE, INPUT_PULLUP);
    debouncePins(PIN_PULSE, PIN_PULSE);
}

void loopSensor() {
    int inputValue = debouncedDigitalRead(PIN_PULSE);
    if (lastValue == 1 && inputValue == 0) {
        ++pulses;
        lastValue = inputValue;
        Serial.print("Pulse ");
        Serial.println(pulses);
        sendLiterPulse();
    } else {
        lastValue = inputValue;
    }
}
