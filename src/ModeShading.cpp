#include "ModeShading.h"
#include "Timer.h"
#include "PositionController.h"
#include "MeasurementWatchdog.h"

#ifdef SHC_KoCShading2Active
// redefine SHC_ParamCalcIndex to add offset for Shading Mode 2
#undef SHC_ParamCalcIndex
#define SHC_ParamCalcIndex(index) (index + SHC_ParamBlockOffset + _channelIndex * SHC_ParamBlockSize + (SHC_CShading2TempActive - SHC_CShading1TempActive) * (_index - 1))
#endif

ModeShading::ModeShading(uint8_t index)
    : _index(index)
{
    _name = "Shading";
    _name += std::to_string(index);
    logInfoP("ModeShading %s created", _name.c_str());
}

const char *ModeShading::name() const
{
    return _name.c_str();
}

uint8_t ModeShading::sceneNumber() const
{
    return _index; // 1 based index
}

void ModeShading::initGroupObjects()
{
    getKo(SHC_KoCShading1LockActive).value(false, DPT_Switch);
    getKo(SHC_KoCShading1Active).value(false, DPT_Switch);
    if (ParamSHC_CShading1Break != 0)
    {
        getKo(SHC_KoCShading2BreakLockActive).value(false, DPT_Switch);
    }
    updateDiagnosticKos();

    _recalcMeasurmentValues = true;
}

void ModeShading::updateDiagnosticKos()
{
    if (ParamSHC_CShading1DiagnoseBits != 0)
        getKo(SHC_KoCShading1DiagnoseNotAllowedBit).value((uint32_t)_notAllowedReason, DPT_CombinedInfoOnOff);
    
    if (ParamSHC_CShading1DiagnoseReason != 0)
    {
        auto& ko = getKo(SHC_KoCShading1DiagnoseNotAllowedReason);
        if (_notAllowedReason == 0)
        {
            if (ko.valueNoSendCompare((uint8_t)0, DPT_DecimalFactor))   
                ko.objectWritten();
        }
        else
        {
            for (uint8_t i = 0; i < 32; i++)
            {
                if (_notAllowedReason & (1 << i))
                {
                    if (ko.valueNoSendCompare((uint8_t)(i + 1), DPT_DecimalFactor))   
                        ko.objectWritten();
                    break;
                }
            }
        }
    }
}

bool ModeShading::windowOpenAllowed() const
{
    return ParamSHC_CShading1WindowOpenAllowed;
}

bool ModeShading::windowTiltAllowed() const
{
    return ParamSHC_CShading1WindowTiltAllowed;
}

bool ModeShading::allowed(const CallContext &callContext)
{
    if (!callContext.shadingControlActive)
    {
        if (callContext.diagnosticLog)
            logInfoP("Shading control KO not active");
        if (callContext.reactivateShadingWaitTimeRunning)
            _notAllowedReason |= ModeShadingNotAllowedReason::ModeShadingNotAllowedReasonSwitchedOffWaitTime;
        else if (callContext.reactivateShadingAfterPeriod)
            _notAllowedReason |= ModeShadingNotAllowedReason::ModeShadingNotAllowedReasonSwitchedOffUntilEndOfPeriod;
        else if (callContext.shadingPeriodActive)
            _notAllowedReason |= ModeShadingNotAllowedReason::ModeShadingNotAllowedReasonSwitchedOff;
        else
            _notAllowedReason |= ModeShadingNotAllowedReason::ModeShadingNotAllowedReasonSwitchedOffNoReactivation;
    }
    else
    {
        _notAllowedReason &= ~ModeShadingNotAllowedReason::ModeShadingNotAllowedReasonSwitchedOffWaitTime;
        _notAllowedReason &= ~ModeShadingNotAllowedReason::ModeShadingNotAllowedReasonSwitchedOffUntilEndOfPeriod;
        _notAllowedReason &= ~ModeShadingNotAllowedReason::ModeShadingNotAllowedReasonSwitchedOff;
        _notAllowedReason &= ~ModeShadingNotAllowedReason::ModeShadingNotAllowedReasonSwitchedOffNoReactivation;
    }
    if (callContext.channelLockActive)
    {
        if (callContext.diagnosticLog)
            logInfoP("Channel lock KO not active");
        _notAllowedReason |= ModeShadingNotAllowedReason::ModeShadingNotAllowedReasonChannelLock;
    }
    else
        _notAllowedReason &= ~ModeShadingNotAllowedReason::ModeShadingNotAllowedReasonChannelLock;
    if (!callContext.timeAndSunValid)
    {
        if (callContext.diagnosticLog)
            logInfoP("Time not valid");
        _notAllowedReason |= ModeShadingNotAllowedReason::ModeShadingNotAllowedReasonTimeNotValid;
    }
    else
        _notAllowedReason &= ~ModeShadingNotAllowedReason::ModeShadingNotAllowedReasonTimeNotValid;

    _recalcMeasurmentValues |=
        callContext.measurementBrightness->isChanged() ||
        callContext.measurementClouds->isChanged() ||
        callContext.measurementHeading->isChanged() ||
        callContext.measurementRain->isChanged() ||
        callContext.measurementRoomTemperature->isChanged() ||
        callContext.measurementTemperature->isChanged() ||
        callContext.measurementTemperatureForecast->isChanged() ||
        callContext.measurementUVIndex->isChanged();

    auto diagnosticLog = callContext.diagnosticLog;
    if (_lockActive)
    {
        if (diagnosticLog)
            logInfoP("Lock KO activ");
        _notAllowedReason |= ModeShadingNotAllowedReason::ModeShadingNotAllowedReasonLock;
    }
    else
    {
        _notAllowedReason &= ~ModeShadingNotAllowedReason::ModeShadingNotAllowedReasonLock;
    }
    bool allowedSun = allowedBySun(callContext);
    if (allowedSun != _lastSunFrameAllowed)
    {
        logDebugP("Allowed by sun: %d", (int)allowedSun);
        _lastSunFrameAllowed = allowedSun;
        _needWaitTime = false; // shading peruiod changed, no wait time needed
    }
    bool logWaitTimeResult = false;
    if (_recalcMeasurmentValues || callContext.diagnosticLog)
    {
        _recalcMeasurmentValues = false;
        _allowedByMeasurementValues = allowedByMeasurmentValues(callContext);
    }
    // Handle heating off wait time
    bool allowedByHeatingOff = true;
    if (_waitTimeAfterHeatingValueChange != 0)
    {
        // <Enumeration Text="ausgeschaltete Heizung" Value="1" Id="%ENID%" />
        // <Enumeration Text="mindestens 1 Stunde ausgeschalten" Value="2" Id="%ENID%" />
        // <Enumeration Text="mindestens 2 Stunde ausgeschalten" Value="3" Id="%ENID%" />
        // <Enumeration Text="mindestens 3 Stunde ausgeschalten" Value="4" Id="%ENID%" />
        // <Enumeration Text="mindestens 4 Stunde ausgeschalten" Value="11" Id="%ENID%" />
        // <Enumeration Text="mindestens 5 Stunde ausgeschalten" Value="12" Id="%ENID%" />
        // <Enumeration Text="mindestens 6 Stunde ausgeschalten" Value="5" Id="%ENID%" />
        // <Enumeration Text="mindestens 7 Stunde ausgeschalten" Value="13" Id="%ENID%" />
        // <Enumeration Text="mindestens 8 Stunde ausgeschalten" Value="6" Id="%ENID%" />
        // <Enumeration Text="mindestens 10 Stunde ausgeschalten" Value="7" Id="%ENID%" />
        // <Enumeration Text="mindestens 12 Stunde ausgeschalten" Value="8" Id="%ENID%" />
        // <Enumeration Text="mindestens 1 Tag ausgeschalten" Value="9" Id="%ENID%" />
        // <Enumeration Text="mindestens 2 Tag ausgeschalten" Value="10" Id="%ENID%" />
        unsigned long waitTimeInMillis = 0;
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
        case 11:
            waitTimeInMillis = 4 * 60 * 60 * 1000;
            break;
        case 12:
            waitTimeInMillis = 5 * 60 * 60 * 1000;
            break;
        case 13:
            waitTimeInMillis = 7 * 60 * 60 * 1000;
            break;
        }
        if (callContext.fastSimulationActive)
            waitTimeInMillis /= 10;

        if (callContext.currentMillis - _waitTimeAfterHeatingValueChange > waitTimeInMillis)
        {
            if (diagnosticLog)
                logInfoP("Heating off wait time %lumin reached", (unsigned long)(waitTimeInMillis / 1000 / 60));

            _notAllowedReason &= ~ModeShadingNotAllowedReason::ModeShadingNotAllowedReasonHeatingInThePast;
            _waitTimeAfterHeatingValueChange = 0;
        }
        else
        {
            if (diagnosticLog)
                logInfoP("Heating off wait time %lumin not reached: %lumin", (unsigned long)(waitTimeInMillis / 1000 / 60), (unsigned long)((callContext.currentMillis - _waitTimeAfterHeatingValueChange) / 1000 / 60));
            _notAllowedReason |= ModeShadingNotAllowedReason::ModeShadingNotAllowedReasonHeatingInThePast;
            allowedByHeatingOff = false;
        }
    }
    // Check if allow changed through measurment values and heating off
    bool allowedByMeasurementValuesAndHeatingOffWaitTime = _allowedByMeasurementValues && allowedByHeatingOff;
    if (_allowedByMeasurementValuesAndHeatingOffWaitTime != allowedByMeasurementValuesAndHeatingOffWaitTime)
    {
        _allowedByMeasurementValuesAndHeatingOffWaitTime = allowedByMeasurementValuesAndHeatingOffWaitTime;

        if (_active && !allowedByMeasurementValuesAndHeatingOffWaitTime)
            _needWaitTime = true; // not longer allowed and active, active wait time for deactivation and reactivation

        if (_active)
        {
            if (_needWaitTime && !allowedByMeasurementValuesAndHeatingOffWaitTime)
            {
                logDebugP("Start stopping wait time");
                _waitTimeAfterMeasurmentValueChange = callContext.currentMillis; // acivate stop wait time
            }
            else
            {
                logDebugP("Stop stopping wait time");
                _waitTimeAfterMeasurmentValueChange = 0; // stop wait time, because allowed again
            }
        }
    }
    // Handle start and stop wait time
    bool stopWaitTimeActive = false;
    bool startWaitTimeActive = false;
    if (_waitTimeAfterMeasurmentValueChange != 0)
    {
        if (_active)
        {
            // Check end wait time
            auto waitTimeInSeconds = (unsigned long)ParamSHC_CShading1WaitTimeEnd * 60;
            if (callContext.fastSimulationActive)
                waitTimeInSeconds /= 10;
            if (callContext.currentMillis - _waitTimeAfterMeasurmentValueChange < waitTimeInSeconds * 1000)
            {
                if (callContext.diagnosticLog)
                    logInfoP("Stop wait time %ds not reached: %ds", (int)waitTimeInSeconds, (int)((callContext.currentMillis - _waitTimeAfterMeasurmentValueChange) / 1000));
                stopWaitTimeActive = true;
            }
            else
            {
                if (callContext.diagnosticLog)
                    logInfoP("Stop wait time %ds reached", (int)waitTimeInSeconds);
                _waitTimeAfterMeasurmentValueChange = 0;
                stopWaitTimeActive = false;
            }
            if (logWaitTimeResult)
                logDebugP("Stop wait time active: %d", (int)stopWaitTimeActive);
        }
        else
        {
            // Check start wait time
            auto waitTimeInSeconds = (unsigned long)ParamSHC_CShading1WaitTimeStart * 60;
            if (callContext.fastSimulationActive)
                waitTimeInSeconds /= 10;

            if (logWaitTimeResult)
                logDebugP("Wait time: %ds", (int)(waitTimeInSeconds));

            if (callContext.currentMillis - _waitTimeAfterMeasurmentValueChange < waitTimeInSeconds * 1000)
            {
                if (callContext.diagnosticLog || logWaitTimeResult)
                    logInfoP("Start wait time %ds not reached: %ds", (int)waitTimeInSeconds, (int)((callContext.currentMillis - _waitTimeAfterMeasurmentValueChange) / 1000));
                startWaitTimeActive = true;
            }
            else
            {
                if (callContext.diagnosticLog || logWaitTimeResult)
                    logInfoP("Start wait time %ds reached", (int)waitTimeInSeconds);
                _waitTimeAfterMeasurmentValueChange = 0;
                startWaitTimeActive = false;
            }
            if (logWaitTimeResult)
                logDebugP("Start wait time active: %d", (int)startWaitTimeActive);
        }
    }

    // Handle not allowed reason
    if (startWaitTimeActive)
        _notAllowedReason |= ModeShadingNotAllowedReason::ModeShadingNotAllowedReasonStartWaitTime;
    else
        _notAllowedReason &= ~ModeShadingNotAllowedReason::ModeShadingNotAllowedReasonStartWaitTime;

    if (callContext.isWindowOpenActive)
        _notAllowedReason |= ModeShadingNotAllowedReason::ModeShadingNotAllowedReasonWindowOpen;
    else
        _notAllowedReason &= ~ModeShadingNotAllowedReason::ModeShadingNotAllowedReasonWindowOpen;

    if (callContext.modeCurrentActive == (const ModeBase *)callContext.modeManual)
        _notAllowedReason |= ModeShadingNotAllowedReason::ModeShadingNotAllowedReasonManualUsage;
    else
        _notAllowedReason &= ~ModeShadingNotAllowedReason::ModeShadingNotAllowedReasonManualUsage;

    if (diagnosticLog)
        logInfoP("Not allowed reason: %lu", (unsigned long)_notAllowedReason);

    if (_lastNotAllowedReason != _notAllowedReason)
    {
        logDebugP("Not allowed reason changed: %lu", (unsigned long)_notAllowedReason);
        _lastNotAllowedReason = _notAllowedReason;
        updateDiagnosticKos();
    }
    // Return result
    if (!_lastSunFrameAllowed)
        return false;
    if (startWaitTimeActive)
        return false;
    if (stopWaitTimeActive)
        return true;
    if (!allowedByHeatingOff)
        return false;
    return _allowedByMeasurementValues;
}

bool ModeShading::allowedBySun(const CallContext &callContext)
{
    bool diagnosticLog = callContext.diagnosticLog;
    bool allowed = true;
    if (callContext.azimuth < ParamSHC_CShading1AzimutMin || callContext.azimuth > ParamSHC_CShading1AzimutMax)
    {
        allowed = false;
        if (diagnosticLog)
            logInfoP("Azimut %.2f not between %d and %d", callContext.azimuth, (int)ParamSHC_CShading1AzimutMin, (int)ParamSHC_CShading1AzimutMax);
        _notAllowedReason |= ModeShadingNotAllowedReasonSunAzimut;
    }
    else
    {
        _notAllowedReason &= ~ModeShadingNotAllowedReasonSunAzimut;
    }
    if (callContext.elevation < ParamSHC_CShading1ElevationMin || callContext.elevation > ParamSHC_CShading1ElevationMax)
    {
        allowed = false;
        if (diagnosticLog)
            logInfoP("Elevation %.2f not between %d and %d", callContext.elevation, (int)ParamSHC_CShading1ElevationMin, (int)ParamSHC_CShading1ElevationMax);
        _notAllowedReason |= ModeShadingNotAllowedReasonSunElevation;
    }
    else
    {
        _notAllowedReason &= ~ModeShadingNotAllowedReasonSunElevation;
    }

    bool shadingBreakActive = false;
    if (_breakLockActive)
    {
        if (callContext.diagnosticLog)
            logInfoP("Shading break locked");
    }
    else
    {
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
                shadingBreakActive = true;
            }
            break;
        case 2:
            if (callContext.elevation >= ParamSHC_CShading1BreakElevationMin && callContext.elevation <= ParamSHC_CShading1BreakElevationMax)
            {
                allowed = false;
                if (diagnosticLog)
                    logInfoP("Shading break elevation in range");
                shadingBreakActive = true;
            }
            break;
        case 3:
            if ((callContext.azimuth >= ParamSHC_CShading1BreakAzimutMin && callContext.azimuth <= ParamSHC_CShading1BreakAzimutMax) &&
                (callContext.elevation >= ParamSHC_CShading1BreakElevationMin && callContext.elevation <= ParamSHC_CShading1BreakElevationMax))
            {
                allowed = false;
                if (diagnosticLog)
                    logInfoP("Shading break azimut and elevation in range");
                shadingBreakActive = true;
            }
            break;
        case 4:
            if ((callContext.azimuth >= ParamSHC_CShading1BreakAzimutMin && callContext.azimuth <= ParamSHC_CShading1BreakAzimutMax) ||
                (callContext.elevation >= ParamSHC_CShading1BreakElevationMin && callContext.elevation <= ParamSHC_CShading1BreakElevationMax))
            {
                allowed = false;
                if (diagnosticLog)
                    logInfoP("Shading break azimut or elevation in range");
                shadingBreakActive = true;
            }
            break;
        }
    }
    if (shadingBreakActive)
        _notAllowedReason |= ModeShadingNotAllowedReasonSunBreak;
    else
        _notAllowedReason &= ~ModeShadingNotAllowedReasonSunBreak;
    return allowed;
}

bool ModeShading::handleMeasurmentValue(bool &allowed, bool enabled, const MeasurementWatchdog *measurementWatchdog, const CallContext &callContext, bool (*predicate)(const MeasurementWatchdog *, uint8_t _channelIndex, uint8_t _index, bool previousAllowed), ModeShadingNotAllowedReason reasonBit)
{
    if (!enabled)
    {
        _notAllowedReason &= ~reasonBit;
        return true;
    }
    if (measurementWatchdog->ignoreValue())
    {
        if (callContext.diagnosticLog)
            logInfoP("%s: value ignore", measurementWatchdog->logPrefix().c_str());
        _notAllowedReason &= ~reasonBit;
        return true;
    }

    if (measurementWatchdog->waitForValue())
    {
        if (callContext.diagnosticLog)
            logInfoP("%s: wait for value", measurementWatchdog->logPrefix().c_str());
        _notAllowedReason |= reasonBit;
        allowed = false;
        return true;
    }
    bool previousAllowed = !measurementWatchdog->useFallback() && !(_notAllowedReason & reasonBit);
  
    if (!predicate(measurementWatchdog, _channelIndex, _index, previousAllowed))
    {
        if (callContext.diagnosticLog)
            logInfoP("%s: value not allowed", measurementWatchdog->logPrefix().c_str());
        allowed = false;
        _notAllowedReason |= reasonBit;
        return false;
    }
    // Value allowed
    _notAllowedReason &= ~reasonBit;
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
        [](const MeasurementWatchdog *m, uint8_t _channelIndex, uint8_t _index, bool previousAllowed)
        { return (float)m->getValue() >= ParamSHC_CShading1TempMin; },
        ModeShadingNotAllowedReason::ModeShadingNotAllowedReasonTemperature);

    handleMeasurmentValue(
        allowed,
        ParamSHC_CShading1TempForecastActive,
        callContext.measurementTemperatureForecast,
        callContext,
        [](const MeasurementWatchdog *m, uint8_t _channelIndex, uint8_t _index, bool previousAllowed)
        { return (float)m->getValue() >= ParamSHC_CShading1TempForecastMin; },
        ModeShadingNotAllowedReason::ModeShadingNotAllowedReasonTemperatureForecase);

    handleMeasurmentValue(
        allowed,
        ParamSHC_CShading1BrightnessActive,
        callContext.measurementBrightness,
        callContext,
        [](const MeasurementWatchdog *m, uint8_t _channelIndex, uint8_t _index, bool previousAllowed)
        { return (double)m->getValue() >= max(0., 1000. * ((double) ParamSHC_CShading1BrightnessMin) - (previousAllowed ? ((double) ParamSHC_CShading1BrightnessHyst) * 1000. : 0)); },
        ModeShadingNotAllowedReason::ModeShadingNotAllowedReasonBrightness);

    handleMeasurmentValue(
        allowed,
        ParamSHC_CShading1UVIActive,
        callContext.measurementUVIndex,
        callContext,
        [](const MeasurementWatchdog *m, uint8_t _channelIndex, uint8_t _index, bool previousAllowed)
        { return (float)m->getValue() >= ParamSHC_CShading1UVIMin; },
        ModeShadingNotAllowedReason::ModeShadingNotAllowedReasonUVI);

    handleMeasurmentValue(
        allowed,
        ParamSHC_CShading1RainActive,
        callContext.measurementRain,
        callContext,
        [](const MeasurementWatchdog *m, uint8_t _channelIndex, uint8_t _index, bool previousAllowed)
        { return !(bool)m->getValue(); },
        ModeShadingNotAllowedReason::ModeShadingNotAllowedReasonRain);

    handleMeasurmentValue(
        allowed,
        ParamSHC_CShading1Clouds != 101,
        callContext.measurementClouds,
        callContext,
        [](const MeasurementWatchdog *m, uint8_t _channelIndex, uint8_t _index, bool previousAllowed)
        { return (uint8_t)m->getValue() <= ParamSHC_CShading1Clouds; },
        ModeShadingNotAllowedReason::ModeShadingNotAllowedReasonClouds);

    handleMeasurmentValue(
        allowed,
        ParamSHC_CShading1RoomTemperaturActive,
        callContext.measurementRoomTemperature,
        callContext,
        [](const MeasurementWatchdog *m, uint8_t _channelIndex, uint8_t _index, bool previousAllowed)
        { return (float)m->getValue() >= ParamSHC_CRoomTemp; },
        ModeShadingNotAllowedReason::ModeShadingNotAllowedReasonRoomTemperature);

    // <Enumeration Text="Nein" Value="0" Id="%ENID%" />
    // <Enumeration Text="Stellwert %" Value="1" Id="%ENID%" />
    // <Enumeration Text="Heizungsanforderung (EIN/AUS)" Value="2" Id="%ENID%" />
    bool heatingOff;
    if (ParamSHC_CHeatingInput == 1)
    {
        heatingOff = handleMeasurmentValue(
            allowed,
            ParamSHC_CShading1HeatingActive != 0, // <Enumeration Text="Deaktiviert" Value="0" Id="%ENID%" />
            callContext.measurementHeading,
            callContext,
            [](const MeasurementWatchdog *m, uint8_t _channelIndex, uint8_t _index, bool previousAllowed)
            { return (uint8_t)m->getValue() <= ParamSHC_CShading1MaxHeatingValue; },
            ModeShadingNotAllowedReason::ModeShadingNotAllowedReasonHeating);
    }
    else
    {
        heatingOff = handleMeasurmentValue(
            allowed,
            ParamSHC_CShading1HeatingActive != 0, // <Enumeration Text="Deaktiviert" Value="0" Id="%ENID%" />
            callContext.measurementHeading,
            callContext,
            [](const MeasurementWatchdog *m, uint8_t _channelIndex, uint8_t _index, bool previousAllowed)
            { return !(bool)m->getValue(); },
            ModeShadingNotAllowedReason::ModeShadingNotAllowedReasonHeating);
        if (callContext.measurementHeading->waitForValue())
            heatingOff = true;
    }
    if (_heatingOff != heatingOff)
    {
        _heatingOff = heatingOff;
        if (heatingOff && !callContext.measurementHeading->ignoreValue())
        {
            // start heating off wait time
            _waitTimeAfterHeatingValueChange = callContext.currentMillis;
        }
        else
            _waitTimeAfterHeatingValueChange = 0;
    }
    if (!callContext.modeCurrentActive->isModeShading() && callContext.positionController->targetPosition() > ParamSHC_CShading1OnlyIfLessThan)
    {
        if (diagnosticLog)
            logInfoP("Shutter %d more than %d", (int)callContext.positionController->targetPosition(), (int)ParamSHC_CShading1OnlyIfLessThan);
        _notAllowedReason |= ModeShadingNotAllowedReason::ModeShadingNotAllowedReasonShutterPosition;
        allowed = false;
    }
    else
    {
        _notAllowedReason &= ~ModeShadingNotAllowedReason::ModeShadingNotAllowedReasonShutterPosition;
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
    return knx.getGroupObject(SHC_KoCalcNumber(ko) + koChannelOffset());
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
    if (!positionController.hasSlat())
        return;

    // Jalousie
    if (ParamSHC_CShading1SlatElevationDepending)
    {
        if (callContext.diagnosticLog)
            logInfoP("ModeShading control 4");
        auto targetSlatPosition = max((-1.131d * callContext.elevation + 101.41d) + (double)ParamSHC_CShading1OffsetSlatPosition, 50.d);

        // auto targetSlatPosition = (90 - callContext.elevation) / 90 * 50 + 50 + (double)ParamSHC_CShading1OffsetSlatPosition;
        if (targetSlatPosition < 0)
            targetSlatPosition = 0;
        else if (targetSlatPosition > 100)
            targetSlatPosition = 100;
        auto slatPosition = (uint8_t)targetSlatPosition;
        if (callContext.diagnosticLog)
            logInfoP("Calculated slat position %d for %lf", (int)slatPosition, callContext.elevation);

        if (!callContext.modeNewStarted && abs((uint8_t)KoSHC_CShutterSlatOutput.value(DPT_Scaling) - slatPosition) < ParamSHC_CShading1MinChangeForSlatAdaption)
        {
            if (callContext.diagnosticLog)
                logInfoP("Slat position %d difference is less then %d", (int)abs((uint8_t)KoSHC_CShutterSlatOutput.value(DPT_Scaling) - slatPosition), (int)ParamSHC_CShading1MinChangeForSlatAdaption);

            return; // Do not change, to less difference
        }
        positionController.setAutomaticSlat(slatPosition);
    }
}

void ModeShading::stop(const CallContext &callContext, const ModeBase *next, PositionController &positionController)
{
    _active = false;
    if (_needWaitTime && !_allowedByMeasurementValuesAndHeatingOffWaitTime && _lastSunFrameAllowed)
    {
        logDebugP("Start starting wait time");
        _waitTimeAfterMeasurmentValueChange = callContext.currentMillis; // start wait time for reactivation
    }
    else
        _waitTimeAfterMeasurmentValueChange = 0;
    getKo(SHC_KoCShading1Active).value(false, DPT_Switch);
}
void ModeShading::processInputKo(GroupObject &ko, PositionController &positionController)
{
    // channel ko
    switch (ko.asap() - SHC_KoBlockOffset - koChannelOffset())
    {
    case SHC_KoCShading1Lock:
        _lockActive = ko.value(DPT_Switch);
        getKo(SHC_KoCShading1LockActive).value(_lockActive, DPT_Switch);
        _waitTimeAfterMeasurmentValueChange = 0; // lock does not use wait time
        _recalcMeasurmentValues = true;
        if (_lockActive)
            _notAllowedReason |= ModeShadingNotAllowedReason::ModeShadingNotAllowedReasonLock;
        else
            _notAllowedReason &= ~ModeShadingNotAllowedReason::ModeShadingNotAllowedReasonLock;
        return;
    case SHC_KoCShading1BreakLock:
        _breakLockActive = ko.value(DPT_Switch);
        getKo(SHC_KoCShading2BreakLockActive).value(_breakLockActive, DPT_Switch); 
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