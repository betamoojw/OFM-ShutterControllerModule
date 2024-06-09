#pragma once

#include "Arduino.h"

class CallContext
{
    public:
        unsigned long currentMillis;
        bool timeAndSunValid;
        bool minuteChanged;
        uint8_t hour;
        uint8_t minute;
        uint8_t minuteOfDay;
        double azimuth;
        double elevation;
};