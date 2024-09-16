#include "persist.h"
#include "config.h"
#include <config2eeprom.hpp>
#include "mqtt.h"

// https://github.com/geekbrother/config2eeprom

// Config class and map hash
config2eeprom c2e;
staticConfigDoc configHash;

int liters = 0;
int lastLitersSaved = 0;
unsigned long lastSavedMilis = 0;

void saveLiters(int numberOfLiters) {
    liters = numberOfLiters;
    logState("Persisting liters: %d", liters);
    staticConfigDoc c2econfig;
    c2econfig["liters"] = liters;
    c2e.save( c2econfig );
    lastSavedMilis = millis();
    lastLitersSaved = liters;
}

void setupPersist() {
  // Load config file and check is configured
  if (!c2e.load(configHash)) {
    Serial.println("No config in EEPROM.");
  } else {
    Serial.println("Config found in EEPROM.");
    const int lastLiters = configHash["liters"].as<int>();
    Serial.print("Read liters ");
    Serial.println(lastLiters);
    liters = lastLiters;
    lastLitersSaved = liters;
  }
}

void loopPersist() {
  unsigned long elapsed = millis() - lastSavedMilis;
  if (elapsed > PERSIST_AFTER_MINUTES * 60 * 1000) {
    if (liters != lastLitersSaved) {
      saveLiters(liters);
    }
  }
}

int getLiters() {
    return liters;
}

int incLiters() {
  ++liters;
  if (liters - lastLitersSaved > PERSIST_AFTER_LITERS) {
    saveLiters(liters);
    lastLitersSaved = liters;
  }
  return liters;
}
