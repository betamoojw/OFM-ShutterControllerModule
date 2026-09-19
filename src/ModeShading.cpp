#include "ModeShading.h"
#include "Timer.h"
#include "PositionController.h"
#include "MeasurementWatchdog.h"
#include <cmath>

#ifdef SHC_KoCShading2Active
// redefine SHC_ParamCalcIndex to add offset for Shading Mode 2
#undef SHC_ParamCalcIndex
#define SHC_ParamCalcIndex(index) (index + SHC_ParamBlockOffset + _channelIndex * SHC_ParamBlockSize + (SHC_CShading2TempActive - SHC_CShading1TempActive) * (_index - 1))

// redefine SHC_KoCalcNumber to add offset for Shading Mode 2
#undef SHC_KoCalcNumber
#define SHC_KoCalcNumber(index) (index + SHC_KoBlockOffset + _channelIndex * SHC_KoBlockSize + (_index - 1) * (SHC_KoCShading2Active - SHC_KoCShading1Active))

// redefine SHC_KoCalcIndex to add offset for Shading Mode 2
#undef SHC_KoCalcIndex
#define SHC_KoCalcIndex(number) ((number >= SHC_KoCalcNumber(0) && number < SHC_KoCalcNumber(SHC_KoBlockSize)) ? (number - SHC_KoBlockOffset - (_index - 1) * (SHC_KoCShading2Active - SHC_KoCShading1Active)) % SHC_KoBlockSize : -1)

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
    KoSHC_CShading1LockActive.value(false, DPT_Switch);
    KoSHC_CShading1Active.value(false, DPT_Switch);
    if (ParamSHC_CShading1Break != 0)
    {
        auto& shadingBreakLock = KoSHC_CShading1BreakLock;
        if (shadingBreakLock.initialized())
            _breakLockActive = shadingBreakLock.value(DPT_Switch);
        else
            shadingBreakLock.requestObjectRead();
        KoSHC_CShading1BreakLockActive.value(_breakLockActive, DPT_Switch);
    }
    updateDiagnosticKos();

    _recalcMeasurmentValues = true;
}

void ModeShading::updateDiagnosticKos()
{
    if (ParamSHC_CShading1DiagnoseBits != 0)
        KoSHC_CShading1DiagnoseNotAllowedBit.value((uint32_t)_notAllowedReason, DPT_CombinedInfoOnOff);
    
    if (ParamSHC_CShading1DiagnoseReason != 0)
    {
        auto& ko = KoSHC_CShading1DiagnoseNotAllowedReason;
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
        _needWaitTime = false; // shading period changed, no wait time needed
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
        // <Enumeration Text="Heating switched off" Value="1" Id="%ENID%" />
        // <Enumeration Text="At least 1 hour switched off" Value="2" Id="%ENID%" />
        // <Enumeration Text="At least 2 hours switched off" Value="3" Id="%ENID%" />
        // <Enumeration Text="At least 3 hours switched off" Value="4" Id="%ENID%" />
        // <Enumeration Text="At least 4 hours switched off" Value="11" Id="%ENID%" />
        // <Enumeration Text="At least 5 hours switched off" Value="12" Id="%ENID%" />
        // <Enumeration Text="At least 6 hours switched off" Value="5" Id="%ENID%" />
        // <Enumeration Text="At least 7 hours switched off" Value="13" Id="%ENID%" />
        // <Enumeration Text="At least 8 hours switched off" Value="6" Id="%ENID%" />
        // <Enumeration Text="At least 10 hours switched off" Value="7" Id="%ENID%" />
        // <Enumeration Text="At least 12 hours switched off" Value="8" Id="%ENID%" />
        // <Enumeration Text="At least 1 day switched off" Value="9" Id="%ENID%" />
        // <Enumeration Text="At least 2 days switched off" Value="10" Id="%ENID%" />
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
    // Check if allowance changed through measurement values and heating off
    bool allowedByMeasurementValuesAndHeatingOffWaitTime = _allowedByMeasurementValues && allowedByHeatingOff;
    if (_allowedByMeasurementValuesAndHeatingOffWaitTime != allowedByMeasurementValuesAndHeatingOffWaitTime)
    {
        _allowedByMeasurementValuesAndHeatingOffWaitTime = allowedByMeasurementValuesAndHeatingOffWaitTime;

        if (_active && !allowedByMeasurementValuesAndHeatingOffWaitTime)
            _needWaitTime = true; // no longer allowed and active, start wait time for deactivation and reactivation

        if (_active)
        {
            if (_needWaitTime && !allowedByMeasurementValuesAndHeatingOffWaitTime)
            {
                logDebugP("Start stopping wait time");
                _waitTimeAfterMeasurmentValueChange = callContext.currentMillis; // activate stop wait time
            }
            else
            {
                logDebugP("Stop stopping wait time");
                _waitTimeAfterMeasurmentValueChange = 0; // stop wait time, because allowed again
            }
        }
        else
        {
            // Mode is currently inactive (shading not running):
            // When the measurement values become favorable again after a previous under-shoot
            // within the same shading period (_needWaitTime == true), (re)start the start wait time
            // so a short favorable spell does not immediately reactivate shading.
            // Previously the start wait time was only armed in stop(); once that timer had elapsed
            // while the values stayed unfavorable, the next favorable change reactivated shading
            // instantly, so the configured "Beschattungsstart" wait time was effectively ignored.
            if (_needWaitTime && allowedByMeasurementValuesAndHeatingOffWaitTime)
            {
                logDebugP("Restart starting wait time");
                _waitTimeAfterMeasurmentValueChange = callContext.currentMillis; // (re)activate start wait time
            }
            else if (!allowedByMeasurementValuesAndHeatingOffWaitTime)
            {
                _waitTimeAfterMeasurmentValueChange = 0; // values unfavorable again, cancel pending start wait time
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
#ifdef KoSHC_CShading1Ready
    bool readiness = _notAllowedReason == 0;
    if (KoSHC_CShading1Ready.valueNoSendCompare(readiness, DPT_Switch))
        KoSHC_CShading1Ready.objectWritten();
#endif
    // Return result
    if (_lockActive)
        return false;
    if (!_lastSunFrameAllowed)
        return false;
    if (startWaitTimeActive)
        return false;
    if (callContext.isWindowTiltActive && !windowTiltAllowed())
        return false;
    if (callContext.isWindowOpenActive && !callContext.isWindowTiltActive && !windowOpenAllowed())
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
        // <Enumeration Text="Disabled" Value="0" Id="%ENID%" />
        // <Enumeration Text="Azimuth" Value="1" Id="%ENID%" />
        // <Enumeration Text="Elevation" Value="2" Id="%ENID%" />
        // <Enumeration Text="Azimuth AND elevation" Value="3" Id="%ENID%" />
        // <Enumeration Text="Azimuth OR elevation" Value="4" Id="%ENID%" />
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

    if (ParamSHC_CShading1RequireFacadeHit == 1)
    {
        bool isGeoTracking = (ParamSHC_CType == 2)
            ? (ParamSHC_CShading1RolloPositionMode >= 1)
            : (ParamSHC_CShading1SlatElevationDepending >= 3);

        const uint8_t orientation = ParamSHC_CWindowOrientation;
        if (isGeoTracking && orientation != 5 && orientation != 6)
        {
            float beta = calculateProfileAngle(
                (float)callContext.elevation,
                (float)callContext.azimuth,
                orientation,
                (float)(int8_t)ParamSHC_CFacadeInclination);
            if (beta <= 0.0f)
            {
                if (diagnosticLog)
                    logInfoP("RequireFacadeHit: Profilwinkel %.2f <= 0, nicht erlaubt", beta);
                allowed = false;
                _notAllowedReason |= ModeShadingNotAllowedReasonProfileAngleSentinel;
            }
            else
                _notAllowedReason &= ~ModeShadingNotAllowedReasonProfileAngleSentinel;
        }
    }

    return allowed;
}

bool ModeShading::handleMeasurmentValue(bool &allowed, bool enabled, const MeasurementSource *measurementSource, const CallContext &callContext, bool (*predicate)(const MeasurementSource *, uint8_t _channelIndex, uint8_t _index, bool previousAllowed), ModeShadingNotAllowedReason reasonBit)
{
    if (!enabled)
    {
        _notAllowedReason &= ~reasonBit;
        return true;
    }
    if (measurementSource->ignoreValue())
    {
        if (callContext.diagnosticLog)
            logInfoP("%s: value ignore", measurementSource->logPrefix().c_str());
        _notAllowedReason &= ~reasonBit;
        return true;
    }

    if (measurementSource->waitForValue())
    {
        if (callContext.diagnosticLog)
            logInfoP("%s: wait for value", measurementSource->logPrefix().c_str());
        _notAllowedReason |= reasonBit;
        allowed = false;
        return true;
    }
    bool previousAllowed = !measurementSource->useFallback() && !(_notAllowedReason & reasonBit);
  
    if (!predicate(measurementSource, _channelIndex, _index, previousAllowed))
    {
        if (callContext.diagnosticLog)
            logInfoP("%s: value not allowed", measurementSource->logPrefix().c_str());
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
        [](const MeasurementSource *m, uint8_t _channelIndex, uint8_t _index, bool previousAllowed)
        { return (float)m->getValue() >= ParamSHC_CShading1TempMin; },
        ModeShadingNotAllowedReason::ModeShadingNotAllowedReasonTemperature);

    handleMeasurmentValue(
        allowed,
        ParamSHC_CShading1TempForecastActive,
        callContext.measurementTemperatureForecast,
        callContext,
        [](const MeasurementSource *m, uint8_t _channelIndex, uint8_t _index, bool previousAllowed)
        { return (float)m->getValue() >= ParamSHC_CShading1TempForecastMin; },
        ModeShadingNotAllowedReason::ModeShadingNotAllowedReasonTemperatureForecase);

    handleMeasurmentValue(
        allowed,
        ParamSHC_CShading1BrightnessActive,
        callContext.measurementBrightness,
        callContext,
        [](const MeasurementSource *m, uint8_t _channelIndex, uint8_t _index, bool previousAllowed)
        { return (double)m->getValue() >= max(0., 1000. * ((double) ParamSHC_CShading1BrightnessMin) - (previousAllowed ? ((double) ParamSHC_CShading1BrightnessHyst) * 1000. : 0)); },
        ModeShadingNotAllowedReason::ModeShadingNotAllowedReasonBrightness);

    handleMeasurmentValue(
        allowed,
        ParamSHC_CShading1UVIActive,
        callContext.measurementUVIndex,
        callContext,
        [](const MeasurementSource *m, uint8_t _channelIndex, uint8_t _index, bool previousAllowed)
        { return (float)m->getValue() >= ParamSHC_CShading1UVIMin; },
        ModeShadingNotAllowedReason::ModeShadingNotAllowedReasonUVI);

    handleMeasurmentValue(
        allowed,
        ParamSHC_CShading1RainActive,
        callContext.measurementRain,
        callContext,
        [](const MeasurementSource *m, uint8_t _channelIndex, uint8_t _index, bool previousAllowed)
        { return !(bool)m->getValue(); },
        ModeShadingNotAllowedReason::ModeShadingNotAllowedReasonRain);

    handleMeasurmentValue(
        allowed,
        ParamSHC_CShading1Clouds != 101,
        callContext.measurementClouds,
        callContext,
        [](const MeasurementSource *m, uint8_t _channelIndex, uint8_t _index, bool previousAllowed)
        { return (uint8_t)m->getValue() <= ParamSHC_CShading1Clouds; },
        ModeShadingNotAllowedReason::ModeShadingNotAllowedReasonClouds);

    handleMeasurmentValue(
        allowed,
        ParamSHC_CShading1RoomTemperaturActive,
        callContext.measurementRoomTemperature,
        callContext,
        [](const MeasurementSource *m, uint8_t _channelIndex, uint8_t _index, bool previousAllowed)
        { return (float)m->getValue() >= ParamSHC_CRoomTemp; },
        ModeShadingNotAllowedReason::ModeShadingNotAllowedReasonRoomTemperature);

    // <Enumeration Text="No" Value="0" Id="%ENID%" />
    // <Enumeration Text="Setpoint %" Value="1" Id="%ENID%" />
    // <Enumeration Text="Heating request (ON/OFF)" Value="2" Id="%ENID%" />
    bool heatingOff;
    if (ParamSHC_CHeatingInput == 1)
    {
        heatingOff = handleMeasurmentValue(
            allowed,
            ParamSHC_CShading1HeatingActive != 0, // <Enumeration Text="Disabled" Value="0" Id="%ENID%" />
            callContext.measurementHeading,
            callContext,
            [](const MeasurementSource *m, uint8_t _channelIndex, uint8_t _index, bool previousAllowed)
            { return (uint8_t)m->getValue() <= ParamSHC_CShading1MaxHeatingValue; },
            ModeShadingNotAllowedReason::ModeShadingNotAllowedReasonHeating);
    }
    else
    {
        heatingOff = handleMeasurmentValue(
            allowed,
            ParamSHC_CShading1HeatingActive != 0, // <Enumeration Text="Disabled" Value="0" Id="%ENID%" />
            callContext.measurementHeading,
            callContext,
            [](const MeasurementSource *m, uint8_t _channelIndex, uint8_t _index, bool previousAllowed)
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

// Berechnet den Profilwinkel [°] der Sonne bezogen auf die Fassadenebene.
// Rückgabe: Winkel in Grad [>0], oder -1.0f als Sentinel (Sonne trifft Fassade nicht / nicht berechenbar).
// Argumente:
//   elevationDeg         - Sonnenhöhe in Grad [0..90]
//   azimuthDeg           - Sonnenazimut in Grad [0..360, 0=Nord, 90=Ost, 180=Süd, 270=West]
//   orientationEnum      - WindowOrientation enum: 0=Ost, 1=Südost, 2=Süd, 3=Südwest, 4=West, 5=Dachfläche, 6=Keine
//   facadeInclinationDeg - Fassadenneigung/Fensterneigung in Grad (0=senkrecht/Wand, positiv=nach vorne geneigt)
float ModeShading::calculateProfileAngle(float elevationDeg, float azimuthDeg, uint8_t orientationEnum, float facadeInclinationDeg) const
{
    static const float DEG2RAD = (float)M_PI / 180.0f;
    static const float RAD2DEG = 180.0f / (float)M_PI;
    static const float COS_EPSILON = 0.0175f;  // cos(89°) ≈ 0.0175

    if (orientationEnum == 6)
        return -1.0f;  // "Keine Himmelsrichtungsauswertung" → Modi 3-5 inaktiv

    if (orientationEnum == 5)
    {
        // Dachfläche: Profilwinkel = Höhenwinkel minus Dachneigung (von Horizontal)
        float beta_deg = elevationDeg - facadeInclinationDeg;
        return (beta_deg <= 0.0f) ? -1.0f : beta_deg;
    }

    // Normalfall: Fassade mit Himmelsrichtung
    // Mapping: Ost=90°, Südost=135°, Süd=180°, Südwest=225°, West=270°
    static const float orientationAzimuth[] = {90.0f, 135.0f, 180.0f, 225.0f, 270.0f};
    float facadeAzimuth = orientationAzimuth[orientationEnum];

    // Relativwinkel Sonne–Fassade, normiert auf [-180°, 180°]
    float delta_phi = fmodf(azimuthDeg - facadeAzimuth + 180.0f, 360.0f) - 180.0f;

    // Guard 1: Sonne hinter der Fassade
    if (fabsf(delta_phi) >= 90.0f)
        return -1.0f;

    float cos_delta = cosf(delta_phi * DEG2RAD);

    // Guard 2: Sonne nahezu parallel zur Fassade (cos → 0)
    if (fabsf(cos_delta) < COS_EPSILON)
        return 89.9f;

    float elevation_rad = elevationDeg * DEG2RAD;
    float facadeInclination_rad = facadeInclinationDeg * DEG2RAD;

    // Profilwinkel: Standardformel. facadeInclination korrigiert senkrechte Fassade → geneigte Fassade
    float beta_rad = atanf(tanf(elevation_rad) / cos_delta) - facadeInclination_rad;

    // Guard 3: Vorneigung der Fassade übersteigt Sonnenhöhe → Sonne unterhalb Fassadenebene
    if (beta_rad <= 0.0f)
        return -1.0f;

    float beta_deg = beta_rad * RAD2DEG;

    // Guard 4: Cap bei 85° (tan(>85°) numerisch instabil)
    if (beta_deg > 85.0f)
        beta_deg = 85.0f;

    return beta_deg;
}

void ModeShading::start(const CallContext &callContext, const ModeBase *previous, PositionController &positionController)
{
    _active = true;
    _lastSentShadowPos = -1.0f;
    KoSHC_CShading1Active.value(true, DPT_Switch);
    // Restore-Position nur beim ersten Shading-Start speichern (nicht bei Shading->Shading-Wechsel),
    // damit "Position vor Beschattungsstart" die tatsächliche Position vor der gesamten Beschattungsphase ist.
    if (previous == nullptr || !previous->isModeShading())
    {
        positionController.storeCurrentPositionForRestore();
        positionController.setRestoreSlat(positionController.slat());
    }
    positionController.setAutomaticPosition(ParamSHC_CShading1ShadingPosition);

    // <Enumeration Text="Channel disabled" Value="0" Id="%ENID%" />
    // <Enumeration Text="Venetian blind" Value="1" Id="%ENID%" />
    // <Enumeration Text="Roller shutter" Value="2" Id="%ENID%" />
    if (ParamSHC_CShading1SlatElevationDepending == 0)
        positionController.setAutomaticSlat(ParamSHC_CShading1SlatShadingPosition);
}

void ModeShading::control(const CallContext &callContext, PositionController &positionController)
{
    if (!callContext.modeNewStarted && !callContext.minuteChanged && !callContext.diagnosticLog)
        return;

    // <Enumeration Text="Channel disabled" Value="0" Id="%ENID%" />
    // <Enumeration Text="Venetian blind" Value="1" Id="%ENID%" />
    // <Enumeration Text="Roller shutter" Value="2" Id="%ENID%" />

    // ── Rollo-Pfad: Geo. Positionsnachführung für Rollo (kein hasSlat!) ──
    if (ParamSHC_CType == 2 && ParamSHC_CShading1RolloPositionMode >= 1)
    {
        static const float DEG2RAD = (float)M_PI / 180.0f;
        _notAllowedReason &= ~(ModeShadingNotAllowedReasonProfileAngleSentinel | ModeShadingNotAllowedReasonFlatRoofGuard);

        float gamma_rad;
        float sin_alpha;
        const uint8_t orientation = ParamSHC_CWindowOrientation;
        const float facadeInclination = (float)(int8_t)ParamSHC_CFacadeInclination;

        if (orientation == 5)
        {
            // Dachfläche: gamma = Höhenwinkel, sin_alpha = |sin(Dachneigung)|
            float elev = (float)callContext.elevation;
            if (elev <= 0.0f)
                return;
            sin_alpha = fabsf(sinf(facadeInclination * DEG2RAD));
            if (sin_alpha < 0.05f)
            {
                _notAllowedReason |= ModeShadingNotAllowedReasonFlatRoofGuard;
                return;  // Flachdach — kein sinnvolles Positionstracking
            }
            gamma_rad = elev * DEG2RAD;
        }
        else
        {
            float beta_deg = calculateProfileAngle(
                (float)callContext.elevation,
                (float)callContext.azimuth,
                orientation,
                facadeInclination);
            if (beta_deg <= 0.0f)
            {
                _notAllowedReason |= ModeShadingNotAllowedReasonProfileAngleSentinel;
                return;  // Sonne trifft Fassade nicht
            }
            gamma_rad = beta_deg * DEG2RAD;
            sin_alpha = 1.0f;  // senkrechte Fassade: sin(90°) = 1
        }

        float windowHeight = (float)ParamSHC_CShading1WindowHeight;
        float maxPenetration = (float)ParamSHC_CShading1MaxPenetrationDepth;
        float windowSillHeight = (float)ParamSHC_CShading1WindowSillHeight;
        float minShadowEdgeChange = (float)ParamSHC_CShading1MinShadowEdgeChange;

        float s_crit = (maxPenetration * tanf(gamma_rad) - windowSillHeight) / sin_alpha;

        float targetPos;
        if (s_crit <= 0.0f)
            targetPos = 100.0f;
        else if (s_crit >= windowHeight)
            targetPos = (float)ParamSHC_CShading1ShadingPosition;
        else
            targetPos = (1.0f - s_crit / windowHeight) * 100.0f;

        if (targetPos < 0.0f) targetPos = 0.0f;
        if (targetPos > 100.0f) targetPos = 100.0f;

        // Hysterese in cm
        if (_lastSentShadowPos >= 0.0f && s_crit < windowHeight && s_crit > 0.0f)
        {
            float delta_cm = fabsf(targetPos - _lastSentShadowPos) * windowHeight / 100.0f;
            if (delta_cm < minShadowEdgeChange)
                return;
        }

        if (callContext.diagnosticLog)
            logInfoP("Rollo geo position: gamma=%.1f° s_crit=%.1f targetPos=%.1f", gamma_rad * (180.0f / (float)M_PI), s_crit, targetPos);

        _lastSentShadowPos = targetPos;  // ungeclippt für korrekte Hysterese

        float sendPos = targetPos;
        if (ParamSHC_CShading1RolloPositionMode == 2)
        {
            uint8_t minClipPos = ParamSHC_CShading1MinClipPosition;
            uint8_t maxClipPos = ParamSHC_CShading1MaxClipPosition;
            if (minClipPos <= maxClipPos)
            {
                if (sendPos < (float)minClipPos) sendPos = (float)minClipPos;
                if (sendPos > (float)maxClipPos) sendPos = (float)maxClipPos;
            }
        }

        positionController.setAutomaticPosition((uint8_t)sendPos);
        return;
    }

    // Jalousie
    if (!positionController.hasSlat())
        return;

    switch (ParamSHC_CShading1SlatElevationDepending)
    {
    case 0:
        // Nein: fixed slat position set in start(), nothing to do here
        return;
    case 1:
    {
        // Standard: elevation-based formula
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

            return; // Do not change, too little difference
        }
        positionController.setAutomaticSlat(slatPosition);
        break;
    }
    case 2:
    {
        // Lamellennachführung Min/Max: linear interpolation between posLow (sun low) and posHigh (sun high)
        if (callContext.diagnosticLog)
            logInfoP("ModeShading control Lamellennachführung Min/Max");
        double elevMin = (double)ParamSHC_CShading1ElevationMin;
        double elevMax = (double)ParamSHC_CShading1ElevationMax;
        double t = (elevMax > elevMin) ? (callContext.elevation - elevMin) / (elevMax - elevMin) : 0.5;
        if (t < 0.0) t = 0.0;
        if (t > 1.0) t = 1.0;
        double posLow = (double)ParamSHC_CShading1SlatLowSunPosition;
        double posHigh = (double)ParamSHC_CShading1SlatHighSunPosition;
        double targetSlatPosition = posLow + (posHigh - posLow) * t
            + (double)ParamSHC_CShading1OffsetSlatPosition;
        if (targetSlatPosition < 0.0) targetSlatPosition = 0.0;
        if (targetSlatPosition > 100.0) targetSlatPosition = 100.0;
        auto slatPosition = (uint8_t)targetSlatPosition;
        if (callContext.diagnosticLog)
            logInfoP("Calculated slat position %d for elevation %lf (t=%.2f, low=%d, high=%d)", (int)slatPosition, callContext.elevation, t, (int)posLow, (int)posHigh);
        if (!callContext.modeNewStarted && abs((uint8_t)KoSHC_CShutterSlatOutput.value(DPT_Scaling) - slatPosition) < ParamSHC_CShading1MinChangeForSlatAdaption)
            break;
        positionController.setAutomaticSlat(slatPosition);
        break;
    }
    case 3:
    {
        // Geo. Positionsnachführung (Jalousie): Position + feste Lamellenstellung
        if (ParamSHC_CType == 2) break;  // Schutzguard
        static const float DEG2RAD = (float)M_PI / 180.0f;
        _notAllowedReason &= ~(ModeShadingNotAllowedReasonProfileAngleSentinel | ModeShadingNotAllowedReasonFlatRoofGuard);

        float gamma_rad;
        float sin_alpha;
        const uint8_t orientation = ParamSHC_CWindowOrientation;
        const float facadeInclination = (float)(int8_t)ParamSHC_CFacadeInclination;

        if (orientation == 5)
        {
            // Dachfläche: gamma = Höhenwinkel, sin_alpha = |sin(Dachneigung)|
            float elev = (float)callContext.elevation;
            if (elev <= 0.0f)
                return;
            sin_alpha = fabsf(sinf(facadeInclination * DEG2RAD));
            if (sin_alpha < 0.05f)
            {
                _notAllowedReason |= ModeShadingNotAllowedReasonFlatRoofGuard;
                return;
            }
            gamma_rad = elev * DEG2RAD;
        }
        else
        {
            float beta_deg = calculateProfileAngle(
                (float)callContext.elevation,
                (float)callContext.azimuth,
                orientation,
                facadeInclination);
            if (beta_deg <= 0.0f)
            {
                _notAllowedReason |= ModeShadingNotAllowedReasonProfileAngleSentinel;
                return;
            }
            gamma_rad = beta_deg * DEG2RAD;
            sin_alpha = 1.0f;
        }

        float windowHeight = (float)ParamSHC_CShading1WindowHeight;
        float maxPenetration = (float)ParamSHC_CShading1MaxPenetrationDepth;
        float windowSillHeight = (float)ParamSHC_CShading1WindowSillHeight;
        float minShadowEdgeChange = (float)ParamSHC_CShading1MinShadowEdgeChange;

        float s_crit = (maxPenetration * tanf(gamma_rad) - windowSillHeight) / sin_alpha;

        float targetPos;
        if (s_crit <= 0.0f)
            targetPos = 100.0f;
        else if (s_crit >= windowHeight)
            targetPos = (float)ParamSHC_CShading1ShadingPosition;
        else
            targetPos = (1.0f - s_crit / windowHeight) * 100.0f;

        if (targetPos < 0.0f) targetPos = 0.0f;
        if (targetPos > 100.0f) targetPos = 100.0f;

        if (_lastSentShadowPos >= 0.0f && s_crit < windowHeight && s_crit > 0.0f)
        {
            float delta_cm = fabsf(targetPos - _lastSentShadowPos) * windowHeight / 100.0f;
            if (delta_cm < minShadowEdgeChange)
            {
                positionController.setAutomaticSlat(ParamSHC_CShading1SlatShadingPosition);
                return;
            }
        }

        if (callContext.diagnosticLog)
            logInfoP("Case3 geo position: gamma=%.1f° s_crit=%.1f targetPos=%.1f", gamma_rad * (180.0f / (float)M_PI), s_crit, targetPos);

        _lastSentShadowPos = targetPos;
        positionController.setAutomaticPosition((uint8_t)targetPos);
        positionController.setAutomaticSlat(ParamSHC_CShading1SlatShadingPosition);
        break;
    }
    case 4:
    {
        // Geo. Positionsnachführung (Min/Max) (Jalousie): wie case 3 + Position-Clipping
        if (ParamSHC_CType == 2) break;  // Schutzguard
        static const float DEG2RAD = (float)M_PI / 180.0f;
        _notAllowedReason &= ~(ModeShadingNotAllowedReasonProfileAngleSentinel | ModeShadingNotAllowedReasonFlatRoofGuard);

        float gamma_rad;
        float sin_alpha;
        const uint8_t orientation = ParamSHC_CWindowOrientation;
        const float facadeInclination = (float)(int8_t)ParamSHC_CFacadeInclination;

        if (orientation == 5)
        {
            float elev = (float)callContext.elevation;
            if (elev <= 0.0f)
                return;
            sin_alpha = fabsf(sinf(facadeInclination * DEG2RAD));
            if (sin_alpha < 0.05f)
            {
                _notAllowedReason |= ModeShadingNotAllowedReasonFlatRoofGuard;
                return;
            }
            gamma_rad = elev * DEG2RAD;
        }
        else
        {
            float beta_deg = calculateProfileAngle(
                (float)callContext.elevation,
                (float)callContext.azimuth,
                orientation,
                facadeInclination);
            if (beta_deg <= 0.0f)
            {
                _notAllowedReason |= ModeShadingNotAllowedReasonProfileAngleSentinel;
                return;
            }
            gamma_rad = beta_deg * DEG2RAD;
            sin_alpha = 1.0f;
        }

        float windowHeight = (float)ParamSHC_CShading1WindowHeight;
        float maxPenetration = (float)ParamSHC_CShading1MaxPenetrationDepth;
        float windowSillHeight = (float)ParamSHC_CShading1WindowSillHeight;
        float minShadowEdgeChange = (float)ParamSHC_CShading1MinShadowEdgeChange;

        float s_crit = (maxPenetration * tanf(gamma_rad) - windowSillHeight) / sin_alpha;

        float targetPos;
        if (s_crit <= 0.0f)
            targetPos = 100.0f;
        else if (s_crit >= windowHeight)
            targetPos = (float)ParamSHC_CShading1ShadingPosition;
        else
            targetPos = (1.0f - s_crit / windowHeight) * 100.0f;

        if (targetPos < 0.0f) targetPos = 0.0f;
        if (targetPos > 100.0f) targetPos = 100.0f;

        if (_lastSentShadowPos >= 0.0f && s_crit < windowHeight && s_crit > 0.0f)
        {
            float delta_cm = fabsf(targetPos - _lastSentShadowPos) * windowHeight / 100.0f;
            if (delta_cm < minShadowEdgeChange)
            {
                positionController.setAutomaticSlat(ParamSHC_CShading1SlatShadingPosition);
                return;
            }
        }

        _lastSentShadowPos = targetPos;  // ungeclippt für korrekte Hysterese

        float sendPos = targetPos;
        uint8_t minClipPos = ParamSHC_CShading1MinClipPosition;
        uint8_t maxClipPos = ParamSHC_CShading1MaxClipPosition;
        if (minClipPos <= maxClipPos)
        {
            if (sendPos < (float)minClipPos) sendPos = (float)minClipPos;
            if (sendPos > (float)maxClipPos) sendPos = (float)maxClipPos;
        }

        if (callContext.diagnosticLog)
            logInfoP("Case4 geo pos min/max: gamma=%.1f° s_crit=%.1f targetPos=%.1f sendPos=%.1f", gamma_rad * (180.0f / (float)M_PI), s_crit, targetPos, sendPos);

        positionController.setAutomaticPosition((uint8_t)sendPos);
        positionController.setAutomaticSlat(ParamSHC_CShading1SlatShadingPosition);
        break;
    }
    case 5:
    {
        // Schattenkanten- und Lamellennachführung: Position (Case 4) + Lamellenwinkel (Case 3)
        if (ParamSHC_CType == 2) break;  // Schutzguard
        static const float DEG2RAD = (float)M_PI / 180.0f;
        _notAllowedReason &= ~(ModeShadingNotAllowedReasonProfileAngleSentinel | ModeShadingNotAllowedReasonFlatRoofGuard);

        float gamma_rad;
        float sin_alpha;
        bool slatTrackingPossible;
        const uint8_t orientation = ParamSHC_CWindowOrientation;
        const float facadeInclination = (float)(int8_t)ParamSHC_CFacadeInclination;

        if (orientation == 5)
        {
            // Dachfläche: gamma = Höhenwinkel, sin_alpha = |sin(Dachneigung)|
            float elev = (float)callContext.elevation;
            if (elev <= 0.0f)
                return;
            sin_alpha = fabsf(sinf(facadeInclination * DEG2RAD));
            if (sin_alpha < 0.05f)
            {
                _notAllowedReason |= ModeShadingNotAllowedReasonFlatRoofGuard;
                return;  // Flachdach — kein sinnvolles Positionstracking
            }
            gamma_rad = elev * DEG2RAD;
            // Für Dachfläche: calculateProfileAngle() prüfen ob Lamellennachführung möglich
            float beta_slat = (float)callContext.elevation - facadeInclination;
            slatTrackingPossible = (beta_slat > 0.0f);
            if (!slatTrackingPossible)
                _notAllowedReason |= ModeShadingNotAllowedReasonProfileAngleSentinel;
        }
        else
        {
            float beta_deg = calculateProfileAngle(
                (float)callContext.elevation,
                (float)callContext.azimuth,
                orientation,
                facadeInclination);
            if (beta_deg <= 0.0f)
            {
                _notAllowedReason |= ModeShadingNotAllowedReasonProfileAngleSentinel;
                return;
            }
            gamma_rad = beta_deg * DEG2RAD;
            sin_alpha = 1.0f;  // senkrechte Fassade: sin(90°) = 1
            slatTrackingPossible = true;
        }

        float windowHeight = (float)ParamSHC_CShading1WindowHeight;
        float maxPenetration = (float)ParamSHC_CShading1MaxPenetrationDepth;
        float windowSillHeight = (float)ParamSHC_CShading1WindowSillHeight;
        float minShadowEdgeChange = (float)ParamSHC_CShading1MinShadowEdgeChange;

        float s_crit = (maxPenetration * tanf(gamma_rad) - windowSillHeight) / sin_alpha;

        float targetPos;
        if (s_crit <= 0.0f)
            targetPos = 100.0f;
        else if (s_crit >= windowHeight)
            targetPos = (float)ParamSHC_CShading1ShadingPosition;
        else
            targetPos = (1.0f - s_crit / windowHeight) * 100.0f;

        if (targetPos < 0.0f) targetPos = 0.0f;
        if (targetPos > 100.0f) targetPos = 100.0f;

        bool positionChanged = true;
        if (_lastSentShadowPos >= 0.0f && s_crit < windowHeight && s_crit > 0.0f)
        {
            float delta_cm = fabsf(targetPos - _lastSentShadowPos) * windowHeight / 100.0f;
            if (delta_cm < minShadowEdgeChange)
                positionChanged = false;
        }

        if (positionChanged)
        {
            _lastSentShadowPos = targetPos;
            positionController.setAutomaticPosition((uint8_t)targetPos);
        }

        // Lamellenwinkel — Betriebsart: 0=Tageslicht, 1=Blendschutz, 2=Tabelle
        uint8_t betriebsart = ParamSHC_CShading1SlatTrackingBetriebsart;
        if (slatTrackingPossible)
        {
        float gamma_deg = gamma_rad * (180.0f / (float)M_PI);
        if (betriebsart == 2)
        {
            float startPos = (float)ParamSHC_CShading1SlatTableStartPos;
            float minElevation = (float)ParamSHC_CShading1SlatTableMinElevation;
            float e1 = (float)ParamSHC_CShading1SlatTable1Elevation;
            float p1 = (float)ParamSHC_CShading1SlatTable1Position;
            float e2 = (float)ParamSHC_CShading1SlatTable2Elevation;
            float p2 = (float)ParamSHC_CShading1SlatTable2Position;
            float e3 = (float)ParamSHC_CShading1SlatTable3Elevation;
            float p3 = (float)ParamSHC_CShading1SlatTable3Position;
            float e4 = (float)ParamSHC_CShading1SlatTable4Elevation;
            float p4 = (float)ParamSHC_CShading1SlatTable4Position;
            float e5 = (float)ParamSHC_CShading1SlatTable5Elevation;
            float p5 = (float)ParamSHC_CShading1SlatTable5Position;
            float e6 = (float)ParamSHC_CShading1SlatTable6Elevation;
            float p6 = (float)ParamSHC_CShading1SlatTable6Position;
            float slatPercent;
            if (gamma_deg <= minElevation)
                slatPercent = startPos;
            else if (gamma_deg >= e6)
                slatPercent = p6;
            else
            {
                float lo_e, lo_p, hi_e, hi_p;
                if (gamma_deg < e2)      { lo_e = e1; lo_p = p1; hi_e = e2; hi_p = p2; }
                else if (gamma_deg < e3) { lo_e = e2; lo_p = p2; hi_e = e3; hi_p = p3; }
                else if (gamma_deg < e4) { lo_e = e3; lo_p = p3; hi_e = e4; hi_p = p4; }
                else if (gamma_deg < e5) { lo_e = e4; lo_p = p4; hi_e = e5; hi_p = p5; }
                else                     { lo_e = e5; lo_p = p5; hi_e = e6; hi_p = p6; }
                slatPercent = (hi_e > lo_e) ? lo_p + (hi_p - lo_p) * (gamma_deg - lo_e) / (hi_e - lo_e) : lo_p;
            }
            slatPercent += (float)(int8_t)ParamSHC_CShading1OffsetSlatPosition;
            if (slatPercent < 0.0f) slatPercent = 0.0f;
            if (slatPercent > 100.0f) slatPercent = 100.0f;
            auto slatPosition = (uint8_t)slatPercent;
            if (callContext.modeNewStarted || abs((uint8_t)KoSHC_CShutterSlatOutput.value(DPT_Scaling) - slatPosition) >= ParamSHC_CShading1MinChangeForSlatAdaption)
                positionController.setAutomaticSlat(slatPosition);
        }
        else
        {
        float a = (float)ParamSHC_CShading1SlatSpacing;
        float b = (float)ParamSHC_CShading1SlatWidth;
        uint8_t angleAtMin = ParamSHC_CShading1SlatAngleAtMin;
        uint8_t angleAtMax = ParamSHC_CShading1SlatAngleAtMax;

        if (angleAtMin != angleAtMax)
        {
            float theta_krit = (betriebsart == 1)
                ? gamma_deg
                : atan2f(a * sinf(gamma_rad), b - a * cosf(gamma_rad)) * (180.0f / (float)M_PI);

            if (theta_krit < 0.0f)
            {
                positionController.setAutomaticSlat(100);
            }
            else
            {
                float slatPercent = ((float)theta_krit - (float)angleAtMin) / ((float)angleAtMax - (float)angleAtMin) * 100.0f;
                slatPercent += (float)(int8_t)ParamSHC_CShading1OffsetSlatPosition;
                if (slatPercent < 0.0f) slatPercent = 0.0f;
                if (slatPercent > 100.0f) slatPercent = 100.0f;
                auto slatPosition = (uint8_t)slatPercent;

                if (callContext.modeNewStarted || abs((uint8_t)KoSHC_CShutterSlatOutput.value(DPT_Scaling) - slatPosition) >= ParamSHC_CShading1MinChangeForSlatAdaption)
                    positionController.setAutomaticSlat(slatPosition);
            }
        }
        }
        }

        if (callContext.diagnosticLog)
            logInfoP("Case5 combined: gamma=%.1f° s_crit=%.1f targetPos=%.1f slatPossible=%d betriebsart=%d", gamma_rad * (180.0f / (float)M_PI), s_crit, targetPos, (int)slatTrackingPossible, (int)betriebsart);

        break;
    }
    case 6:
    {
        // Geo. Positions- und Lamellennachführung mit Begrenzung
        // Identisch zu Case 5, jedoch mit konfigurierbarem Clipping für Position und Lamelle
        if (ParamSHC_CType == 2) break;
        static const float DEG2RAD = (float)M_PI / 180.0f;
        _notAllowedReason &= ~(ModeShadingNotAllowedReasonProfileAngleSentinel | ModeShadingNotAllowedReasonFlatRoofGuard);

        float gamma_rad;
        float sin_alpha;
        bool slatTrackingPossible;
        const uint8_t orientation = ParamSHC_CWindowOrientation;
        const float facadeInclination = (float)(int8_t)ParamSHC_CFacadeInclination;

        if (orientation == 5)
        {
            float elev = (float)callContext.elevation;
            if (elev <= 0.0f)
                return;
            sin_alpha = fabsf(sinf(facadeInclination * DEG2RAD));
            if (sin_alpha < 0.05f)
            {
                _notAllowedReason |= ModeShadingNotAllowedReasonFlatRoofGuard;
                return;
            }
            gamma_rad = elev * DEG2RAD;
            float beta_slat = (float)callContext.elevation - facadeInclination;
            slatTrackingPossible = (beta_slat > 0.0f);
            if (!slatTrackingPossible)
                _notAllowedReason |= ModeShadingNotAllowedReasonProfileAngleSentinel;
        }
        else
        {
            float beta_deg = calculateProfileAngle(
                (float)callContext.elevation,
                (float)callContext.azimuth,
                orientation,
                facadeInclination);
            if (beta_deg <= 0.0f)
            {
                _notAllowedReason |= ModeShadingNotAllowedReasonProfileAngleSentinel;
                return;
            }
            gamma_rad = beta_deg * DEG2RAD;
            sin_alpha = 1.0f;
            slatTrackingPossible = true;
        }

        float windowHeight = (float)ParamSHC_CShading1WindowHeight;
        float maxPenetration = (float)ParamSHC_CShading1MaxPenetrationDepth;
        float windowSillHeight = (float)ParamSHC_CShading1WindowSillHeight;
        float minShadowEdgeChange = (float)ParamSHC_CShading1MinShadowEdgeChange;

        float s_crit = (maxPenetration * tanf(gamma_rad) - windowSillHeight) / sin_alpha;

        float targetPos;
        if (s_crit <= 0.0f)
            targetPos = 100.0f;
        else if (s_crit >= windowHeight)
            targetPos = (float)ParamSHC_CShading1ShadingPosition;
        else
            targetPos = (1.0f - s_crit / windowHeight) * 100.0f;

        if (targetPos < 0.0f) targetPos = 0.0f;
        if (targetPos > 100.0f) targetPos = 100.0f;

        bool positionChanged = true;
        if (_lastSentShadowPos >= 0.0f && s_crit < windowHeight && s_crit > 0.0f)
        {
            float delta_cm = fabsf(targetPos - _lastSentShadowPos) * windowHeight / 100.0f;
            if (delta_cm < minShadowEdgeChange)
                positionChanged = false;
        }

        if (positionChanged)
        {
            _lastSentShadowPos = targetPos; // Ungeclippten Wert für korrekte Hysterese speichern
            float clippedPos = targetPos;
            uint8_t minClipPos = ParamSHC_CShading1MinClipPosition;
            uint8_t maxClipPos = ParamSHC_CShading1MaxClipPosition;
            if (minClipPos <= maxClipPos)
            {
                if (clippedPos < (float)minClipPos) clippedPos = (float)minClipPos;
                if (clippedPos > (float)maxClipPos) clippedPos = (float)maxClipPos;
            }
            positionController.setAutomaticPosition((uint8_t)clippedPos);
        }

        uint8_t betriebsart = ParamSHC_CShading1SlatTrackingBetriebsart;
        if (slatTrackingPossible)
        {
            float gamma_deg = gamma_rad * (180.0f / (float)M_PI);
            if (betriebsart == 2)
            {
                float startPos = (float)ParamSHC_CShading1SlatTableStartPos;
                float minElevation = (float)ParamSHC_CShading1SlatTableMinElevation;
                float e1 = (float)ParamSHC_CShading1SlatTable1Elevation;
                float p1 = (float)ParamSHC_CShading1SlatTable1Position;
                float e2 = (float)ParamSHC_CShading1SlatTable2Elevation;
                float p2 = (float)ParamSHC_CShading1SlatTable2Position;
                float e3 = (float)ParamSHC_CShading1SlatTable3Elevation;
                float p3 = (float)ParamSHC_CShading1SlatTable3Position;
                float e4 = (float)ParamSHC_CShading1SlatTable4Elevation;
                float p4 = (float)ParamSHC_CShading1SlatTable4Position;
                float e5 = (float)ParamSHC_CShading1SlatTable5Elevation;
                float p5 = (float)ParamSHC_CShading1SlatTable5Position;
                float e6 = (float)ParamSHC_CShading1SlatTable6Elevation;
                float p6 = (float)ParamSHC_CShading1SlatTable6Position;
                float slatPercent;
                if (gamma_deg <= minElevation)
                    slatPercent = startPos;
                else if (gamma_deg >= e6)
                    slatPercent = p6;
                else
                {
                    float lo_e, lo_p, hi_e, hi_p;
                    if (gamma_deg < e2)      { lo_e = e1; lo_p = p1; hi_e = e2; hi_p = p2; }
                    else if (gamma_deg < e3) { lo_e = e2; lo_p = p2; hi_e = e3; hi_p = p3; }
                    else if (gamma_deg < e4) { lo_e = e3; lo_p = p3; hi_e = e4; hi_p = p4; }
                    else if (gamma_deg < e5) { lo_e = e4; lo_p = p4; hi_e = e5; hi_p = p5; }
                    else                     { lo_e = e5; lo_p = p5; hi_e = e6; hi_p = p6; }
                    slatPercent = (hi_e > lo_e) ? lo_p + (hi_p - lo_p) * (gamma_deg - lo_e) / (hi_e - lo_e) : lo_p;
                }
                slatPercent += (float)(int8_t)ParamSHC_CShading1OffsetSlatPosition;
                if (slatPercent < 0.0f) slatPercent = 0.0f;
                if (slatPercent > 100.0f) slatPercent = 100.0f;
                auto slatPosition = (uint8_t)slatPercent;
                if (callContext.modeNewStarted || abs((uint8_t)KoSHC_CShutterSlatOutput.value(DPT_Scaling) - slatPosition) >= ParamSHC_CShading1MinChangeForSlatAdaption)
                    positionController.setAutomaticSlat(slatPosition);
            }
            else
            {
                float a = (float)ParamSHC_CShading1SlatSpacing;
                float b = (float)ParamSHC_CShading1SlatWidth;
                uint8_t angleAtMin = ParamSHC_CShading1SlatAngleAtMin;
                uint8_t angleAtMax = ParamSHC_CShading1SlatAngleAtMax;

                if (angleAtMin != angleAtMax)
                {
                    float theta_krit = (betriebsart == 1)
                        ? gamma_deg
                        : atan2f(a * sinf(gamma_rad), b - a * cosf(gamma_rad)) * (180.0f / (float)M_PI);

                    if (theta_krit < 0.0f)
                    {
                        positionController.setAutomaticSlat(100);
                    }
                    else
                    {
                        float slatPercent = ((float)theta_krit - (float)angleAtMin) / ((float)angleAtMax - (float)angleAtMin) * 100.0f;
                        slatPercent += (float)(int8_t)ParamSHC_CShading1OffsetSlatPosition;
                        if (slatPercent < 0.0f) slatPercent = 0.0f;
                        if (slatPercent > 100.0f) slatPercent = 100.0f;

                        // Lamellen-Clipping: min(PPP+51, PPP+52) bis max(PPP+51, PPP+52)
                        float slatLow = (float)ParamSHC_CShading1SlatLowSunPosition;
                        float slatHigh = (float)ParamSHC_CShading1SlatHighSunPosition;
                        float slatClipMin = (slatLow < slatHigh) ? slatLow : slatHigh;
                        float slatClipMax = (slatLow > slatHigh) ? slatLow : slatHigh;
                        if (slatPercent < slatClipMin) slatPercent = slatClipMin;
                        if (slatPercent > slatClipMax) slatPercent = slatClipMax;

                        auto slatPosition = (uint8_t)slatPercent;
                        if (callContext.modeNewStarted || abs((uint8_t)KoSHC_CShutterSlatOutput.value(DPT_Scaling) - slatPosition) >= ParamSHC_CShading1MinChangeForSlatAdaption)
                            positionController.setAutomaticSlat(slatPosition);
                    }
                }
            }
        }

        if (callContext.diagnosticLog)
            logInfoP("Case6 clip: gamma=%.1f° s_crit=%.1f targetPos=%.1f slatPossible=%d betriebsart=%d", gamma_rad * (180.0f / (float)M_PI), s_crit, targetPos, (int)slatTrackingPossible, (int)betriebsart);

        break;
    }
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
    KoSHC_CShading1Active.value(false, DPT_Switch);
}
void ModeShading::processInputKo(GroupObject &ko, PositionController &positionController)
{
    switch (SHC_KoCalcIndex(ko.asap()))
    {
    case SHC_KoCShading1Lock:
        _lockActive = ko.value(DPT_Switch);
        KoSHC_CShading1LockActive.value(_lockActive, DPT_Switch);
        _waitTimeAfterMeasurmentValueChange = 0; // lock does not use wait time
        _recalcMeasurmentValues = true;
        if (_lockActive)
            _notAllowedReason |= ModeShadingNotAllowedReason::ModeShadingNotAllowedReasonLock;
        else
            _notAllowedReason &= ~ModeShadingNotAllowedReason::ModeShadingNotAllowedReasonLock;
        return;
    case SHC_KoCShading1BreakLock:
        _breakLockActive = ko.value(DPT_Switch);
        KoSHC_CShading1BreakLockActive.value(_breakLockActive, DPT_Switch); 
        return;
    }
    // global KO
    switch (ko.asap())
    {
    case SHC_KoCShadingControl:
        // Manual activation / deactivation stops wait time
        _waitTimeAfterMeasurmentValueChange = 0;
        return;
    }
    _recalcMeasurmentValues = true;
}

bool ModeShading::isPositionAllowed(const CallContext& callContext) const
{
    return callContext.positionController->targetPosition() <= ParamSHC_CShading1OnlyIfLessThan;
}

bool ModeShading::isModeShading() const
{
    return true;
}