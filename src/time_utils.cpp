#include "time_utils.h"
#include <Arduino.h>
#include <time.h>

// from https://werner.rothschopf.net/202011_arduino_esp8266_ntp_en.htm

time_t now;                         // this are the seconds since Epoch (1970) - UTC
tm tm;                              // the structure tm holds time information in a more convenient way

char timeBuffer[50];

char *getTime() {
    time(&now);                       // read the current time
    localtime_r(&now, &tm);           // update the structure tm with the current time
    sprintf(timeBuffer, "%04d-%02d-%02d %2d:%02d:%02d", 
        tm.tm_year + 1900, 
        tm.tm_mon + 1,      // January = 0 (!)
        tm.tm_mday,         // day of month
        tm.tm_hour,         // hours since midnight  0-23
        tm.tm_min,
        tm.tm_sec);          // seconds after the minute  0-61*
    return timeBuffer;
}