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
        sunRise = false;
        startTime = false;
        sunSet = false;
        stopTime = false;
    }
    if (callContext.minuteChanged)
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
            if (!startTime && callContext.minuteOfDay == knx.paramWord(SHC_ParamCalcIndex(SHC_CNightFromTime)))
            {
                startTime = true;
                trigger = true;
            }
            break;
        case 2:
            if (!sunRise && callContext.UtcHour < 720 && callContext.elevation >= getElevationFromSunRiseParameter())
            {
                sunRise = true;
                trigger = true;
            }
            break;
        case 3:
            if (!startTime && callContext.minuteOfDay == knx.paramWord(SHC_ParamCalcIndex(SHC_CNightFromTime)))
            {
                startTime = true;
                trigger = true;
            }
            if (!sunRise && callContext.UtcHour < 720 && callContext.elevation >= getElevationFromSunRiseParameter())
            {
                sunRise = true;
                trigger = true;
            }
            break;
        case 4:
            if (!startTime && callContext.minuteOfDay == knx.paramWord(SHC_ParamCalcIndex(SHC_CNightFromTime)))
            {
                startTime = true;
                trigger = sunRise;
            }
            if (!sunRise && callContext.UtcHour < 720 && callContext.elevation >= getElevationFromSunRiseParameter())
            {
                sunRise = true;
                trigger = startTime;
            }
            break;
        }
        if (trigger)
            _allowed = true;
        // 	<Enumeration Text="Kein Autstart" Value="0" Id="%ENID%" />
        // 	<Enumeration Text="Uhrzeit" Value="1" Id="%ENID%" />
        // 	<Enumeration Text="Sonne" Value="2" Id="%ENID%" />
        // 	<Enumeration Text="Uhrzeit, Sonne (früheres Ergeignis)" Value="3" Id="%ENID%" />
        // 	<Enumeration Text="Uhrzeit, Sonne (späteres Ergeignis)" Value="4" Id="%ENID%" />
        switch (ParamSHC_CNightEndBehavior)
        {
        case 1:
            if (!stopTime && callContext.minuteOfDay == knx.paramWord(SHC_ParamCalcIndex(SHC_CNightToTime)))
            {
                stopTime = true;
                trigger = true;
            }
            break;
        case 2:
            if (!sunSet && callContext.UtcHour > 720 && callContext.elevation <= getElevationFromSunSetParameter())
            {
                sunSet = true;
                trigger = true;
            }
            break;
        case 3:
            if (!stopTime && callContext.minuteOfDay == knx.paramWord(SHC_ParamCalcIndex(SHC_CNightToTime)))
            {
                stopTime = true;
                trigger = true;
            }
            if (!sunRise && callContext.UtcHour > 720 && callContext.elevation <= getElevationFromSunSetParameter())
            {
                sunSet = true;
                trigger = true;
            }
            break;
        case 4:
            if (!stopTime && callContext.minuteOfDay == knx.paramWord(SHC_ParamCalcIndex(SHC_CNightToTime)))
            {
                stopTime = true;
                trigger = sunRise;
            }
            if (!sunRise && callContext.UtcHour > 720 && callContext.elevation <= getElevationFromSunSetParameter())
            {
                sunSet = true;
                trigger = stopTime;
            }
            break;
        }
        if (trigger)
            _allowed = false;
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
        positionController.setManualPosition(ParamSHC_CNightStartPosition);
        positionController.setManualSlat(ParamSHC_CNightStartSlatPosition);
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
        // Use manual modes, because the position should be stored as last manual position
        positionController.setManualPosition(ParamSHC_CNightStopPosition); 
        positionController.setManualSlat(ParamSHC_CNightStopSlatPosition);
    }
}

void ModeNight::processInputKo(GroupObject &ko)
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