#include "ModeNight.h"
#include "PositionController.h"

const char *ModeNight::name() const
{
    return "Night";
}

uint8_t ModeNight::sceneNumber() const 
{
    return 12;
}

void ModeNight::initGroupObjects()
{
    KoSHC_CNightActive.value(false, DPT_Switch);
    KoSHC_CNightLockActive.value(false, DPT_Switch);
}
bool ModeNight::windowOpenAllowed() const
{
    return ParamSHC_CNightWindowOpenAllowed;
}
bool ModeNight::windowTiltAllowed() const
{
    return ParamSHC_CNightWindowTiltAllowed;
}
bool ModeNight::allowed(const CallContext &callContext)
{
    if (callContext.localTimeInStandardTimeDay != _lastDayInStandardTime)
    {
        _lastDayInStandardTime = callContext.localTimeInStandardTimeDay;
        // new day
        _sunRise = false;
        _startTime = false;
        _sunSet = false;
        _stopTime = false;
    }
    if (_startPositionDeativatedForPeriod)
    {
        if (callContext.diagnosticLog)
            logInfoP("Start position deactivated for period");
    }
    if (callContext.minuteChanged || callContext.diagnosticLog)
    {
        bool trigger = false;
        // 	<Enumeration Text="Kein Autstart" Value="0" Id="%ENID%" />
        // 	<Enumeration Text="Uhrzeit" Value="1" Id="%ENID%" />
        // 	<Enumeration Text="Sonne" Value="2" Id="%ENID%" />
        // 	<Enumeration Text="Uhrzeit, Sonne (früheres Ergeignis)" Value="3" Id="%ENID%" />
        // 	<Enumeration Text="Uhrzeit, Sonne (späteres Ergeignis)" Value="4" Id="%ENID%" />
        switch (ParamSHC_CNightStartBehavior)
        {
        case 1:
            if (!_startTime && callContext.minuteOfDay == knx.paramWord(SHC_ParamCalcIndex(SHC_CNightFromTime)))
            {
                logInfoP("Start time");
                _startTime = true;
                trigger = true;
            }
            if (callContext.diagnosticLog)
                logInfoP("Start Time: %s", _startTime ? "true" : "false");
            break;
        case 2:
            if (!_sunSet && callContext.localTime.hour > 12 && callContext.elevation <= getElevationFromSunSetParameter())
            {
                logInfoP("Sun set");
                _sunSet = true;
                trigger = true;
            }
            if (callContext.diagnosticLog)
                logInfoP("Sun Set: %s", _sunSet ? "true" : "false");
            break;
        case 3:
            if (!_startTime && callContext.minuteOfDay == knx.paramWord(SHC_ParamCalcIndex(SHC_CNightFromTime)))
            {
                logInfoP("Start time");
                _startTime = true;
                trigger = true;
            }
            if (!_sunSet && callContext.localTime.hour > 12 && callContext.elevation <= getElevationFromSunSetParameter())
            {
                logInfoP("Sun set");
                _sunSet = true;
                trigger = true;
            }
            break;
        case 4:
            if (!_startTime && callContext.minuteOfDay == knx.paramWord(SHC_ParamCalcIndex(SHC_CNightFromTime)))
            {
                logInfoP("Start time");
                _startTime = true;
                trigger = _sunSet;
            }
            if (!_sunSet && callContext.localTime.hour > 12 && callContext.elevation <= getElevationFromSunSetParameter())
            {
                logInfoP("Sun set");
                _sunSet = true;
                trigger = _startTime;
            }
            if (callContext.diagnosticLog)
                logInfoP("Start Time: %s AND Sun Set: %s", _startTime ? "true" : "false", _sunSet ? "true" : "false");
            break;
        }
        if (trigger)
        {
            logInfoP("Night start triggered");
            _allowed = true;
        }
        trigger = false;
        // 	<Enumeration Text="Kein Autstart" Value="0" Id="%ENID%" />
        // 	<Enumeration Text="Uhrzeit" Value="1" Id="%ENID%" />
        // 	<Enumeration Text="Sonne" Value="2" Id="%ENID%" />
        // 	<Enumeration Text="Uhrzeit, Sonne (früheres Ergeignis)" Value="3" Id="%ENID%" />
        // 	<Enumeration Text="Uhrzeit, Sonne (späteres Ergeignis)" Value="4" Id="%ENID%" />
        switch (ParamSHC_CNightEndBehavior)
        {
        case 1:
            if (!_stopTime && callContext.minuteOfDay == knx.paramWord(SHC_ParamCalcIndex(SHC_CNightToTime)))
            {
                logInfoP("Stop time");
                _stopTime = true;
                trigger = true;
            }
            if (callContext.diagnosticLog)
                logInfoP("Stop Time: %s", _stopTime ? "true" : "false");
            break;
        case 2:
            if (callContext.diagnosticLog)
                logInfoP("Sun rise %d %lf %lf", callContext.localTime.hour, callContext.elevation, getElevationFromSunRiseParameter());
            if (!_sunRise && callContext.localTime.hour < 12 && callContext.elevation >= getElevationFromSunRiseParameter())
            {
                logInfoP("Sun rise");
                _sunRise = true;
                trigger = true;
            }
            if (callContext.diagnosticLog)
                logInfoP("Sun Rise: %s", _sunRise ? "true" : "false");
            break;
        case 3:
            if (!_stopTime && callContext.minuteOfDay == knx.paramWord(SHC_ParamCalcIndex(SHC_CNightToTime)))
            {
                logInfoP("Stop time");
                _stopTime = true;
                trigger = true;
            }
            if (!_sunRise && callContext.localTime.hour < 12 && callContext.elevation >= getElevationFromSunRiseParameter())
            {
                logInfoP("Sun rise");
                _sunRise = true;
                trigger = true;
            }
            if (callContext.diagnosticLog)
                logInfoP("Stop Time: %s OR Sun Rise: %s", _stopTime ? "true" : "false", _sunRise ? "true" : "false");
            break;
        case 4:
            if (!_stopTime && callContext.minuteOfDay == knx.paramWord(SHC_ParamCalcIndex(SHC_CNightToTime)))
            {
                logInfoP("Stop time");
                _stopTime = true;
                trigger = _sunRise;
            }
            if (!_sunRise && callContext.localTime.hour < 12 && callContext.elevation >= getElevationFromSunRiseParameter())
            {
                logInfoP("Sun rise");
                _sunRise = true;
                trigger = _stopTime;
            }
            if (callContext.diagnosticLog)
                logInfoP("Stop Time: %s AND Sun Rise: %s", _stopTime ? "true" : "false", _sunRise ? "true" : "false");
            break;
        }
        if (trigger)
        {
            logInfoP("Night stop triggered");
            _allowed = false;
            _startPositionDeativatedForPeriod = false;
        }
    }
    if (KoSHC_CNightLockActive.value(DPT_Switch))
    {
        if (callContext.diagnosticLog)
            logInfoP("Lock KO active");
        return false;
    }
    return _allowed;
}

// <Enumeration Text="vor Sonnenuntergang" Value="0" Id="%ENID%" />
// <Enumeration Text="bei Sonnenuntergang" Value="1" Id="%ENID%" />
// <Enumeration Text="nach Sonnenuntergang" Value="2" Id="%ENID%" />
double ModeNight::getElevationFromSunSetParameter()
{
    switch (ParamSHC_CNightSunSet)
    {
    case 0:
        return ParamSHC_CNightSunSetElevationOffset;
    case 2:
        return ((double)ParamSHC_CNightSunSetElevationOffset) * -1;
    default:
        return 0;
    }
}

// <Enumeration Text="vor Sonnenaufgang" Value="0" Id="%ENID%" />
// <Enumeration Text="bei Sonnenaufgang" Value="1" Id="%ENID%" />
// <Enumeration Text="nach Sonnenaufgang" Value="2" Id="%ENID%" />
double ModeNight::getElevationFromSunRiseParameter()
{
    switch (ParamSHC_CNightSunRise)
    {
    case 0:
        return ((double)ParamSHC_CNightSunRiseElevationOffset) * -1;
    case 2:
        return ParamSHC_CNightSunRiseElevationOffset;
    default:
        return 0;
    }
}

void ModeNight::start(const CallContext &callContext, const ModeBase *previous, PositionController& positionController)
{
    KoSHC_CNightActive.value(true, DPT_Switch);
    if (ParamSHC_CNightStartPositionEnabled)
    {
        if (_startPositionDeativatedForPeriod)
        {
            logDebugP("Start position deactivated for period");  
        }
        else
        {
            logDebugP("Set night start position %d slat %d", (int) ParamSHC_CNightStartPosition, (int) ParamSHC_CNightStartSlatPosition);
            positionController.setAutomaticPositionAndStoreForRestore(ParamSHC_CNightStartPosition);
            positionController.setAutomaticSlatAndStoreForRestore(ParamSHC_CNightStartSlatPosition);
        }
    }
}

void ModeNight::control(const CallContext &callContext, PositionController& positionController)
{
}

void ModeNight::stop(const CallContext &callContext, const ModeBase *next, PositionController& positionController)
{
    KoSHC_CNightActive.value(false, DPT_Switch);
    if (next != (const ModeBase *)callContext.modeManual && ParamSHC_CNightStopPositionEnabled)
    {
        logDebugP("Set night stop position %d slat %d", (int) ParamSHC_CNightStartPosition, (int) ParamSHC_CNightStartSlatPosition);  
        positionController.setAutomaticPositionAndStoreForRestore(ParamSHC_CNightStopPosition); 
        positionController.setAutomaticSlatAndStoreForRestore(ParamSHC_CNightStopSlatPosition);
    }
    if (_allowed)
    {
        // starting again should not trigger the position
        _startPositionDeativatedForPeriod = true;
    }
}

void ModeNight::processInputKo(GroupObject &ko, PositionController& positionController)
{
    switch (SHC_KoCalcIndex(ko.asap()))
    {
    case SHC_KoCNight:
        _allowed = ko.value(DPT_Switch);
        if (_allowed)
            _startPositionDeativatedForPeriod = false;
        break;
    case SHC_KoCNightLock:
        KoSHC_CNightLockActive.value(ko.value(DPT_Switch), DPT_Switch);
        break;
    default:
        break;
    }
}