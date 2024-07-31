#include "ModeShading.h"
#include "Timer.h"
#include "PositionController.h"
#include "MeasurementWatchdog.h"

#ifdef SHC_KoCShading2Active
// redefine SHC_ParamCalcIndex to add offset for Shading Mode 2
#undef SHC_ParamCalcIndex
#define SHC_ParamCalcIndex(index) (index + SHC_ParamBlockOffset + _channelIndex * SHC_ParamBlockSize + (SHC_CShading2 - SHC_CShading1) * (_index - 1))
#endif

ModeShading::ModeShading(uint8_t index)
    : _index(index)
{
    _name = "Shading";
    _name += std::to_string(index);
}

const char *ModeShading::name() const
{
    return _name.c_str();
}

uint8_t ModeShading::sceneNumber() const
{
    return _index;
}

void ModeShading::initGroupObjects()
{
    getKo(SHC_KoCShading1LockActive).value(false, DPT_Switch);
    getKo(SHC_KoCShading1Active).value(false, DPT_Switch);
    _recalcMeasurmentValues = true;
}
bool ModeShading::modeWindowOpenAllowed() const
{
    return ParamSHC_CShading1WindowOpenAllowed;
}
bool ModeShading::allowed(const CallContext &callContext)
{
    auto diagnosticLog = callContext.diagnosticLog;
    if (_lockActive)
    {
        if (diagnosticLog)
            logInfoP("Lock KO activ");
        else
            return false;
    }
    bool allowedTimeAndSun = allowedByTimeAndSun(callContext);
    if (allowedTimeAndSun != _lastTimeAndSunFrameAllowed)
    {
        logDebugP("Allowed by time and sun: %d", (int)allowedTimeAndSun);
        _lastTimeAndSunFrameAllowed = allowedTimeAndSun;
        _needWaitTime = false;
    }
    if (!diagnosticLog && !_lastTimeAndSunFrameAllowed)
        return false;

    if (_recalcMeasurmentValues || callContext.diagnosticLog)
    {
        _recalcMeasurmentValues = false;
        auto allowedByMeasurementValues = allowedByMeasurmentValues(callContext);
        if (_allowedByMeasurementValues != allowedByMeasurementValues)
        {
            logDebugP("Allowed by measurement values: %d", (int)allowedByMeasurementValues);
            if (_needWaitTime)
                _waitTimeAfterMeasurmentValueChange = callContext.currentMillis;
            else
                _waitTimeAfterMeasurmentValueChange = 0;
            _allowedByMeasurementValues = allowedByMeasurementValues;
            if (allowedByMeasurementValues)
                _needWaitTime = true;

            if (allowedByMeasurementValues && _active)
                _waitTimeAfterMeasurmentValueChange = 0; // Currently active and allowed, not need to wait

            logDebugP("Need wait time: %d", (int)_needWaitTime);
            logDebugP("Wait time start set: %d", (int)(_waitTimeAfterMeasurmentValueChange != 0));
            logDebugP("Allowed by sun: %d", (int)_lastTimeAndSunFrameAllowed);
        }
    }
    if (_waitTimeAfterMeasurmentValueChange != 0)
    {
        bool lastState = !_allowedByMeasurementValues;
        if (lastState)
        {
            // Check end wait time
            auto waitTimeInMinutes = (unsigned long)ParamSHC_CShading1WaitTimeEnd;
            if (callContext.currentMillis - _waitTimeAfterMeasurmentValueChange < waitTimeInMinutes * 60000)
            {
                if (callContext.diagnosticLog)
                    logInfoP("Stop wait time %ds not reached: %ds", (int)waitTimeInMinutes * 60, (int)((callContext.currentMillis - _waitTimeAfterMeasurmentValueChange) / 1000));
                return lastState;
            }
        }
        else
        {
            // Check start wait time
            auto waitTimeInMinutes = (unsigned long)ParamSHC_CShading1WaitTimeStart;
            if (callContext.currentMillis - _waitTimeAfterMeasurmentValueChange < waitTimeInMinutes * 60000)
            {
                if (callContext.diagnosticLog)
                    logInfoP("Start wait time %ds not reached: %ds", (int)waitTimeInMinutes * 60, (int)((callContext.currentMillis - _waitTimeAfterMeasurmentValueChange) / 1000));
                return lastState;
            }
        }
        _waitTimeAfterMeasurmentValueChange = 0;
    }
    return _allowedByMeasurementValues && _lastTimeAndSunFrameAllowed;
}

bool ModeShading::allowedByTimeAndSun(const CallContext &callContext)
{
    bool diagnosticLog = callContext.diagnosticLog;
    bool allowed = true;
    if (callContext.azimuth < ParamSHC_CShading1AzimutMin || callContext.azimuth > ParamSHC_CShading1AzimutMax)
    {
        allowed = false;
        if (diagnosticLog)
            logInfoP("Azimut %.2f not between %d and %d", callContext.azimuth, (int)ParamSHC_CShading1AzimutMin, (int)ParamSHC_CShading1AzimutMax);
        else
            return false;
    }
    if (callContext.elevation < ParamSHC_CShading1ElevationMin || callContext.elevation > ParamSHC_CShading1ElevationMax)
    {
        allowed = false;
        if (diagnosticLog)
            logInfoP("Elevantion %.2f not between %d and %d", callContext.elevation, (int)ParamSHC_CShading1ElevationMin, (int)ParamSHC_CShading1ElevationMax);
        else
            return false;
    }

    auto shadingBreak = ParamSHC_CShading1Break;

    // <Enumeration Text="Deaktiviert" Value="0" Id="%ENID%" />
    // <Enumeration Text="Azimut" Value="1" Id="%ENID%" />
    // <Enumeration Text="Höhenwinkel" Value="2" Id="%ENID%" />
    // <Enumeration Text="Azimut UND Höhenwinkel" Value="3" Id="%ENID%" />
    // <Enumeration Text="Azimut ODER Höhenwinkel" Value="4" Id="%ENID%" />
    switch (shadingBreak)
    {
    case 1:
        if (callContext.azimuth >= ParamSHC_CShading1BreakAzimutMin && callContext.azimuth <= ParamSHC_CShading1BreakAzimutMax)
        {
            allowed = false;
            if (diagnosticLog)
                logInfoP("Shading break azimut in range");
            else
                return false;
        }
        break;
    case 2:
        if (callContext.elevation >= ParamSHC_CShading1BreakElevationMin && callContext.elevation <= ParamSHC_CShading1BreakElevationMax)
        {
            allowed = false;
            if (diagnosticLog)
                logInfoP("Shading break elevation in range");
            else
                return false;
        }
        break;
    case 3:
        if ((callContext.azimuth >= ParamSHC_CShading1BreakAzimutMin && callContext.azimuth <= ParamSHC_CShading1BreakAzimutMax) &&
            (callContext.elevation >= ParamSHC_CShading1BreakElevationMin && callContext.elevation <= ParamSHC_CShading1BreakElevationMax))
        {
            allowed = false;
            if (diagnosticLog)
                logInfoP("Shading break azimut and elevation in range");
            else
                return false;
        }
        break;
    case 4:
        if ((callContext.azimuth >= ParamSHC_CShading1BreakAzimutMin && callContext.azimuth <= ParamSHC_CShading1BreakAzimutMax) ||
            (callContext.elevation >= ParamSHC_CShading1BreakElevationMin && callContext.elevation <= ParamSHC_CShading1BreakElevationMax))
            return false;
        {
            allowed = false;
            if (diagnosticLog)
                logInfoP("Shading break azimut or elevation in range");
            else
                return false;
        }
        break;
    }
    return allowed;
}

bool ModeShading::handleMeasurmentValue(bool& allowed, bool enabled, const MeasurementWatchdog *measurementWatchdog, const CallContext &callContext, bool (*predicate)(const MeasurementWatchdog *, uint8_t _channelIndex, uint8_t _index))
{
    if (!enabled)
        return true;
    if (measurementWatchdog->ignoreValue())
    {
        if (callContext.diagnosticLog)
            logInfoP("%s: value ignore", measurementWatchdog->logPrefix().c_str());
        return true;
    }

    if (measurementWatchdog->waitForValue())
    {
        if (callContext.diagnosticLog)
            logInfoP("%s: wait for value", measurementWatchdog->logPrefix().c_str());

        allowed = false;
        return false;
    }
    if (!predicate(measurementWatchdog, _channelIndex, _index))
    {
        if (callContext.diagnosticLog)
              logInfoP("%s: value not allowed", measurementWatchdog->logPrefix().c_str());
         allowed = false;
         return false;
    }
    return true;
}

bool ModeShading::allowedByMeasurmentValues(const CallContext &callContext)
{
    bool diagnosticLog = callContext.diagnosticLog;
    bool allowed = true;
   
    handleMeasurmentValue(
        allowed, 
        ParamSHC_CShading1TempActive,
        callContext.measurementTemperature, 
        callContext, 
        [](const MeasurementWatchdog* m, auto _channelIndex, uint8_t _index) 
        { return (float) m->getValue() >= ParamSHC_CShading1TempMin;});

    handleMeasurmentValue(
        allowed, 
        ParamSHC_CShading1TempForecastActive,
        callContext.measurementTemperatureForecast, 
        callContext, 
        [](const MeasurementWatchdog* m, auto _channelIndex, uint8_t _index) 
        { return (float) m->getValue() >= ParamSHC_CShading1TempForecastMin;});

    handleMeasurmentValue(
        allowed, 
        ParamSHC_CShading1BrightnessActive,
        callContext.measurementBrightness, 
        callContext, 
        [](const MeasurementWatchdog* m, auto _channelIndex, uint8_t _index) 
        { return (double) m->getValue() >= 1000 * ParamSHC_CShading1BrightnessMin;});

    
    handleMeasurmentValue(
        allowed, 
        ParamSHC_CShading1UVIActive,
        callContext.measurementUVIndex, 
        callContext, 
        [](const MeasurementWatchdog* m, auto _channelIndex, uint8_t _index) 
        { return (float) m->getValue() >= ParamSHC_CShading1UVIMin;});

   
    handleMeasurmentValue(
        allowed, 
        ParamSHC_CShading1RainActive,
        callContext.measurementRain, 
        callContext, 
        [](const MeasurementWatchdog* m, auto _channelIndex, uint8_t _index) 
        { return !(bool) m->getValue();});


    handleMeasurmentValue(
        allowed, 
        ParamSHC_CShading1Clouds != 101,
        callContext.measurementClouds, 
        callContext, 
        [](const MeasurementWatchdog* m, auto _channelIndex, uint8_t _index) 
        { return (uint8_t) m->getValue() <= ParamSHC_CShading1Clouds;});


    handleMeasurmentValue(
        allowed, 
        ParamSHC_CShading1RoomTemperaturActive,
        callContext.measurementRoomTemperature, 
        callContext, 
        [](const MeasurementWatchdog* m, auto _channelIndex, uint8_t _index) 
        { return (float) m->getValue() >= ParamSHC_CRoomTemp;});    


    // <Enumeration Text="Nein" Value="0" Id="%ENID%" />
    // <Enumeration Text="Stellwert %" Value="1" Id="%ENID%" />
    // <Enumeration Text="Heizungsanforderung (EIN/AUS)" Value="2" Id="%ENID%" />   
    bool heatingOff;
    if (ParamSHC_CHeatingInput == 1)
    {
        heatingOff = handleMeasurmentValue(
            allowed, 
            ParamSHC_CShading1HeatingActive != 0,  // <Enumeration Text="Deaktiviert" Value="0" Id="%ENID%" />
            callContext.measurementHeading, 
            callContext, 
            [](const MeasurementWatchdog* m, auto _channelIndex, uint8_t _index) 
            { return (uint8_t) m->getValue() <= ParamSHC_CShading1MaxHeatingValue;});
    }
    else
    {
        heatingOff = handleMeasurmentValue(
            allowed, 
            ParamSHC_CShading1HeatingActive != 0,  // <Enumeration Text="Deaktiviert" Value="0" Id="%ENID%" />
            callContext.measurementHeading, 
            callContext, 
            [](const MeasurementWatchdog* m, auto _channelIndex, uint8_t _index) 
            { return !(bool) m->getValue();});
    }
    if (_heatingOff != heatingOff)
    {
        _heatingOff = heatingOff;
        if (heatingOff)
            _waitTimeAfterMeasurmentValueChange = callContext.currentMillis;
        else
            _waitTimeAfterMeasurmentValueChange = 0;
    }
    if (_waitTimeAfterMeasurmentValueChange != 0)
    {

        // <Enumeration Text="Deaktiviert" Value="0" Id="%ENID%" />
        // <Enumeration Text="ausgeschaltete Heizung" Value="1" Id="%ENID%" />
        // <Enumeration Text="mindestens 1 Stunde ausgeschalten" Value="2" Id="%ENID%" />
        // <Enumeration Text="mindestens 2 Stunde ausgeschalten" Value="3" Id="%ENID%" />
        // <Enumeration Text="mindestens 3 Stunde ausgeschalten" Value="4" Id="%ENID%" />
        // <Enumeration Text="mindestens 6 Stunde ausgeschalten" Value="5" Id="%ENID%" />
        // <Enumeration Text="mindestens 8 Stunde ausgeschalten" Value="6" Id="%ENID%" />
        // <Enumeration Text="mindestens 10 Stunde ausgeschalten" Value="7" Id="%ENID%" />
        // <Enumeration Text="mindestens 12 Stunde ausgeschalten" Value="8" Id="%ENID%" />
        // <Enumeration Text="mindestens 1 Tag ausgeschalten" Value="9" Id="%ENID%" />
        // <Enumeration Text="mindestens 2 Tag ausgeschalten" Value="10" Id="%ENID%" />
        unsigned long waitTimeInMillis= 0;
        switch (ParamSHC_CShading1HeatingActive)
        {
        case 0:
            waitTimeInMillis = 0;
            break;
        case 1:
            waitTimeInMillis = 0;
            break;  
        case 2:
            waitTimeInMillis = 1 * 60 * 60 * 1000;
            break;
        case 3:       
            waitTimeInMillis = 2 * 60 * 60 * 1000;
            break;
        case 4:
            waitTimeInMillis = 3 * 60 * 60 * 1000;
            break;
        case 5:
            waitTimeInMillis = 6 * 60 * 60 * 1000;
            break;
        case 6:      
            waitTimeInMillis = 8 * 60 * 60 * 1000;
            break;
        case 7:
            waitTimeInMillis = 10 * 60 * 60 * 1000;
            break;
        case 8:
            waitTimeInMillis = 12 * 60 * 60 * 1000;
            break;
        case 9:
            waitTimeInMillis = 24 * 60 * 60 * 1000;
            break;
        case 10:    
            waitTimeInMillis = 48 * 60 * 60 * 1000;
            break;
        }

        if (callContext.currentMillis - _waitTimeAfterMeasurmentValueChange > waitTimeInMillis)
        {
            if (diagnosticLog)
                logInfoP("Heating off wait time not reached");
            allowed = false;
        }
        else
        {
            _waitTimeAfterMeasurmentValueChange = 0;
        }
    }
    if (!callContext.modeCurrentActive->isModeShading() && (uint8_t)KoSHC_CShutterPercentInput.value(DPT_Scaling) > ParamSHC_CShading1OnlyIfLessThan)
    {
        if (diagnosticLog)
            logInfoP("Shutter %d more than %d", (int)(uint8_t) KoSHC_CShutterPercentInput.value(DPT_Scaling), (int)ParamSHC_CShading1OnlyIfLessThan);
        allowed = false;
    }
    return allowed;
}

#ifndef SHC_KoCShading2Active
#define SHC_KoCShading2Active SHC_KoCShading1Active
#endif

int16_t ModeShading::koChannelOffset()
{
    return (_index - 1) * (SHC_KoCShading2Active - SHC_KoCShading1Active);
}

GroupObject &ModeShading::getKo(uint8_t ko)
{
    return knx.getGroupObject(ko + koChannelOffset());
}

void ModeShading::start(const CallContext &callContext, const ModeBase *previous, PositionController &positionController)
{
    _active = true;
    getKo(SHC_KoCShading1Active).value(true, DPT_Switch);

    positionController.setAutomaticPosition(ParamSHC_CShading1ShadingPosition);

    // <Enumeration Text="Kanal deaktiviert" Value="0" Id="%ENID%" />
    // <Enumeration Text="Jalousie" Value="1" Id="%ENID%" />
    // <Enumeration Text="Rollo" Value="2" Id="%ENID%" />
    if (!ParamSHC_CShading1SlatElevationDepending)
        positionController.setAutomaticSlat(ParamSHC_CShading1SlatShadingPosition);
}

void ModeShading::control(const CallContext &callContext, PositionController &positionController)
{
    if (!callContext.modeNewStarted && !callContext.minuteChanged && !callContext.diagnosticLog)
        return;

    // <Enumeration Text="Kanal deaktiviert" Value="0" Id="%ENID%" />
    // <Enumeration Text="Jalousie" Value="1" Id="%ENID%" />
    // <Enumeration Text="Rollo" Value="2" Id="%ENID%" />
    if (positionController.hasSlat())
        return;

    // Jalousie
    if (ParamSHC_CShading1SlatElevationDepending)
    {
        auto targetSlatPosition = (90 - callContext.elevation) / 90 * 50 + 50 + (double)ParamSHC_CShading1OffsetSlatPosition;
        if (targetSlatPosition < 0)
            targetSlatPosition = 0;
        else if (targetSlatPosition > 100)
            targetSlatPosition = 100;
        auto slatPosition = (uint8_t)targetSlatPosition;
        if (callContext.diagnosticLog)
            logInfoP("Calculated slat position %d", (int)slatPosition);

        if (!callContext.modeNewStarted && abs((uint8_t) KoSHC_CShutterSlatOutput.value(DPT_Scaling) - slatPosition) < ParamSHC_CShading1MinChangeForSlatAdaption)
        {
            if (callContext.diagnosticLog)
                logInfoP("Slat position %d difference is less then %d", (int)abs((uint8_t) KoSHC_CShutterSlatOutput.value(DPT_Scaling) - slatPosition), (int)ParamSHC_CShading1MinChangeForSlatAdaption);

            return; // Do not change, to less difference
        }
        positionController.setAutomaticSlat(slatPosition);
    }
}

void ModeShading::stop(const CallContext &callContext, const ModeBase *next, PositionController &positionController)
{
    _active = false;
    _waitTimeAfterMeasurmentValueChange = 0;
    getKo(SHC_KoCShading1Active).value(false, DPT_Switch);
}
void ModeShading::processInputKo(GroupObject &ko)
{
    // channel ko
    switch (ko.asap() - koChannelOffset())
    {
    case SHC_KoCShading1Lock:
        _lockActive = ko.value(DPT_Switch);
        getKo(SHC_KoCShading1LockActive).value(_lockActive, DPT_Switch);
        _waitTimeAfterMeasurmentValueChange = 0; // lock does not use wait time
        _recalcMeasurmentValues = true;
        return;
    }
    // global ko
    switch (ko.asap())
    {
    case SHC_KoCShadingControl:
        // Manual activativation / deativiation stop wait time
        _waitTimeAfterMeasurmentValueChange = 0;
        return;
    }
    _recalcMeasurmentValues = true;
}

bool ModeShading::isModeShading() const
{
    return true;
}