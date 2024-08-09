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
bool ModeNight::modeWindowOpenAllowed() const
{
    return ParamSHC_CNightWindowOpenAllowed;
}
bool ModeNight::allowed(const CallContext &callContext)
{
    if (callContext.UtcDay != _lastDayUTC)
    {
        _lastDayUTC = callContext.UtcDay;
        // new day
        _sunRise = false;
        _startTime = false;
        _sunSet = false;
        _stopTime = false;
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
                logDebugP("Start time");
                _startTime = true;
                trigger = true;
            }
            if (callContext.diagnosticLog)
                logDebugP("Start Time: %s", _startTime ? "true" : "false");
            break;
        case 2:
            if (!_sunSet && callContext.UtcHour > 12 && callContext.elevation <= getElevationFromSunSetParameter())
            {
                logDebugP("Sun set");
                _sunSet;
                trigger = true;
            }
            if (callContext.diagnosticLog)
                logDebugP("Sun Set: %s", _sunSet ? "true" : "false");
            break;
        case 3:
            if (!_startTime && callContext.minuteOfDay == knx.paramWord(SHC_ParamCalcIndex(SHC_CNightFromTime)))
            {
                logDebugP("Start time");
                _startTime = true;
                trigger = true;
            }
            if (!_sunSet && callContext.UtcHour > 12 && callContext.elevation <= getElevationFromSunSetParameter())
            {
                logDebugP("Sun set");
                _sunSet = true;
                trigger = true;
            }
            break;
        case 4:
            if (!_startTime && callContext.minuteOfDay == knx.paramWord(SHC_ParamCalcIndex(SHC_CNightFromTime)))
            {
                logDebugP("Start time");
                _startTime = true;
                trigger = _sunSet;
            }
            if (!_sunSet && callContext.UtcHour > 12 && callContext.elevation <= getElevationFromSunSetParameter())
            {
                logDebugP("Sun set");
                _sunSet = true;
                trigger = _startTime;
            }
            if (callContext.diagnosticLog)
                logDebugP("Start Time: %s AND Sun Set: %s", _startTime ? "true" : "false", _sunSet ? "true" : "false");
            break;
        }
        if (trigger)
        {
            logDebugP("Night start triggered");
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
                logDebugP("Stop time");
                _stopTime = true;
                trigger = true;
            }
            if (callContext.diagnosticLog)
                logDebugP("Stop Time: %s", _stopTime ? "true" : "false");
            break;
        case 2:
            if (callContext.diagnosticLog)
                logDebugP("Sun rise %d %lf %lf", callContext.UtcHour, callContext.elevation, getElevationFromSunRiseParameter());
            if (!_sunRise && callContext.UtcHour < 12 && callContext.elevation >= getElevationFromSunRiseParameter())
            {
                logDebugP("Sun rise");
                _sunRise = true;
                trigger = true;
            }
            if (callContext.diagnosticLog)
                logDebugP("Sun Rise: %s", _sunRise ? "true" : "false");
            break;
        case 3:
            if (!_stopTime && callContext.minuteOfDay == knx.paramWord(SHC_ParamCalcIndex(SHC_CNightToTime)))
            {
                logDebugP("Stop time");
                _stopTime = true;
                trigger = true;
            }
            if (!_sunRise && callContext.UtcHour < 12 && callContext.elevation >= getElevationFromSunRiseParameter())
            {
                logDebugP("Sun rise");
                _sunRise = true;
                trigger = true;
            }
            if (callContext.diagnosticLog)
                logDebugP("Stop Time: %s OR Sun Rise: %s", _stopTime ? "true" : "false", _sunRise ? "true" : "false");
            break;
        case 4:
            if (!_stopTime && callContext.minuteOfDay == knx.paramWord(SHC_ParamCalcIndex(SHC_CNightToTime)))
            {
                logDebugP("Stop time");
                _stopTime = true;
                trigger = _sunRise;
            }
            if (!_sunRise && callContext.UtcHour < 12 && callContext.elevation >= getElevationFromSunRiseParameter())
            {
                logDebugP("Sun rise");
                _sunRise = true;
                trigger = _stopTime;
            }
            if (callContext.diagnosticLog)
                logDebugP("Stop Time: %s AND Sun Rise: %s", _stopTime ? "true" : "false", _sunRise ? "true" : "false");
            break;
        }
        if (trigger)
        {
            logInfoP("Night stop triggered");
            _allowed = false;
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
        // Use manual modes, because the position should be stored as last manual position
        logDebugP("Set night start position %d slat %d", (int) ParamSHC_CNightStartPosition, (int) ParamSHC_CNightStartSlatPosition);
        positionController.setManualPosition(ParamSHC_CNightStartPosition, true);
        positionController.setManualSlat(ParamSHC_CNightStartSlatPosition, true);
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
        // Use manual modes, because the position should be stored as last manual position
        positionController.setManualPosition(ParamSHC_CNightStopPosition, true); 
        positionController.setManualSlat(ParamSHC_CNightStopSlatPosition, true);
    }
}

void ModeNight::processInputKo(GroupObject &ko, PositionController& positionController)
{
    switch (ko.asap())
    {
    case SHC_KoCNight:
        _allowed = ko.value(DPT_Switch);
        break;
    case SHC_KoCNightLock:
        KoSHC_CNightLockActive.value(ko.value(DPT_Switch), DPT_Switch);
        break;
    default:
        break;
    }
}