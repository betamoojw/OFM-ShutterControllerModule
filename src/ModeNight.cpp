#include "ModeNight.h"

const char *ModeNight::name()
{
    return "Night";
}
void ModeNight::initGroupObjects()
{
    KoSHC_CHModeNightActive.value(false, DPT_Switch);
    KoSHC_CHModeNightLockActive.value(false, DPT_Switch);
}
bool ModeNight::modeWindowOpenAllowed() const
{
    return ParamSHC_ChannelNightModeWindowOpenAllowed;
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
        switch (ParamSHC_ChannelNightModeStartBehavior)
        {
        case 1:
            if (!startTime && callContext.minuteOfDay == knx.paramWord(SHC_ParamCalcIndex(SHC_ChannelModeNightFromTime)))
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
            if (!startTime && callContext.minuteOfDay == knx.paramWord(SHC_ParamCalcIndex(SHC_ChannelModeNightFromTime)))
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
            if (!startTime && callContext.minuteOfDay == knx.paramWord(SHC_ParamCalcIndex(SHC_ChannelModeNightFromTime)))
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
        switch (ParamSHC_ChannelNightModeEndBehavior)
        {
        case 1:
            if (!stopTime && callContext.minuteOfDay == knx.paramWord(SHC_ParamCalcIndex(SHC_ChannelModeNightToTime)))
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
            if (!stopTime && callContext.minuteOfDay == knx.paramWord(SHC_ParamCalcIndex(SHC_ChannelModeNightToTime)))
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
            if (!stopTime && callContext.minuteOfDay == knx.paramWord(SHC_ParamCalcIndex(SHC_ChannelModeNightToTime)))
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
    if (KoSHC_CHModeNightLockActive.value(DPT_Switch))
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
    switch (ParamSHC_ChannelModeNightSunSet)
    {
    case 0:
        return ParamSHC_ChannelModeNightSunSetElevationOffset;
    case 2:
        return ((double)ParamSHC_ChannelModeNightSunSetElevationOffset) * -1;
    default:
        return 0;
    }
}

// <Enumeration Text="vor Sonnenaufgang" Value="0" Id="%ENID%" />
// <Enumeration Text="bei Sonnenaufgang" Value="1" Id="%ENID%" />
// <Enumeration Text="nach Sonnenaufgang" Value="2" Id="%ENID%" />
double ModeNight::getElevationFromSunRiseParameter()
{
    switch (ParamSHC_ChannelModeNightSunRise)
    {
    case 0:
        return ((double)ParamSHC_ChannelModeNightSunRiseElevationOffset) * -1;
    case 2:
        return ParamSHC_ChannelModeNightSunRiseElevationOffset;
    default:
        return 0;
    }
}

void ModeNight::start(const CallContext& callContext, const ModeBase* previous)
{
    KoSHC_CHModeNightActive.value(true, DPT_Switch);
    if (ParamSHC_ChannelModeNightStartPositionEnabled)
    {
        KoSHC_CHShutterPercentOutput.value(ParamSHC_ChannelModeNightStartPositionEnabled, DPT_Scaling);
        KoSHC_CHShutterSlatOutput.value(ParamSHC_ChannelNightModeStartSlatPosition, DPT_Scaling);
    }
}

void ModeNight::control(const CallContext &callContext)
{
}

void ModeNight::stop(const CallContext& callContext, const ModeBase* next)
{
    KoSHC_CHModeNightActive.value(false, DPT_Switch);
    if (next != (const ModeBase*) callContext.modeManual && ParamSHC_ChannelModeNightStartPositionEnabled)
    {
        KoSHC_CHShutterPercentOutput.value(ParamSHC_ChannelModeNightStartPositionEnabled, DPT_Scaling);
        KoSHC_CHShutterSlatOutput.value(ParamSHC_ChannelNightModeStartSlatPosition, DPT_Scaling);
    }
}

void ModeNight::processInputKo(GroupObject &ko)
{
    switch (ko.asap())
    {
    case SHC_KoCHModeNight:
        _allowed = ko.value(DPT_Switch);
        break;
    case SHC_KoCHModeNightLock:
        KoSHC_CHModeNightLockActive.value(ko.value(DPT_Switch), DPT_Switch);
        break;
    default:
        break;
    }
}