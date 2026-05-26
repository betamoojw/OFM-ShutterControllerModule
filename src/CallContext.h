#pragma once

#include "Arduino.h"
#include "OpenKNX.h"

class ModeIdle;
class ModeManual;
class ModeBase;
class MeasurementSource;
class PositionController;

class CallContext
{
    public:
        bool modeNewStarted = false;
        bool fastSimulationActive = false;
        bool isWindowOpenActive = false;
        bool isWindowTiltActive = false;
        bool hasSlat = false;
        bool diagnosticLog = false;
        bool shadingControlActive = false;
        bool shadingPeriodActive = false;
        bool channelLockActive = false;
        unsigned long currentMillis = 0;
        bool timeAndSunValid = false;
        bool minuteChanged = false;
        bool reactivateShadingWaitTimeRunning = false;
        bool reactivateShadingAfterPeriod = false;
        bool shadingDailyActivation = false;
        float azimuth = 0;
        float elevation = 0;
      
        OpenKNX::TimeOnly localTime = {0};
        uint16_t minuteOfDay = 0;
       
        OpenKNX::TimeOnly localTimeInStandardTime = {0};
        uint16_t localTimeInStandardTimeDay = 0;
     
        const ModeIdle* modeIdle = nullptr;
        const ModeManual* modeManual = nullptr;
        const ModeBase* modeCurrentActive = nullptr;
        MeasurementSource* measurementTemperature = nullptr;
        MeasurementSource* measurementTemperatureForecast = nullptr;
        MeasurementSource* measurementBrightness = nullptr;
        MeasurementSource* measurementUVIndex = nullptr;
        MeasurementSource* measurementRain = nullptr;
        MeasurementSource* measurementClouds = nullptr;
        MeasurementSource* measurementRoomTemperature = nullptr;
        MeasurementSource* measurementHeading = nullptr;
        const PositionController* positionController = nullptr;
};