#ifndef PERSIST_H
#define PERSIST_H

#include <Arduino.h>

void saveLiters(int numberOfLiters);
void setupPersist();
void loopPersist();
int getLiters();
int incLiters();
String getLastSavedTs();
String getLastPulseTs();

#endif