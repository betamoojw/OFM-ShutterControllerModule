#include "ModeShading.h"
#include "Timer.h"
#include "Helios.h"

// redefine SHC_ParamCalcIndex to add offset for Shading Mode 2
#undef SHC_ParamCalcIndex
#define SHC_ParamCalcIndex(index) (index + SHC_ParamBlockOffset + _channelIndex * SHC_ParamBlockSize + (SHC_ChannelModeShading2 - SHC_ChannelModeShading1) * (_index - 1))

ModeShading::ModeShading(uint8_t index)
    : _index(index)
{
    _name = "Shading";
    _name += std::to_string(index);
}

const char *ModeShading::name()
{
    return _name.c_str();
}
void ModeShading::initGroupObjects()
{
    getKo(SHC_KoCHModeShading1LockActive).value(false, DPT_Switch);
}
bool ModeShading::allowed()
{
    Timer& timer = Timer::instance();
    if (timer.isTimerValid())
        return false;
    tm time;
    time.tm_isdst = -1;
    time.tm_year = timer.getYear();
    time.tm_mon = timer.getMonth();
    time.tm_mday = timer.getDay();
    time.tm_hour = timer.getHour();
    time.tm_min = timer.getMinute();
    time.tm_sec = timer.getSecond();

    std::time_t timet = std::mktime(&time);
    timet -= ParamBASE_Timezone * 60 * 60;
    if (timer.IsSummertime())
        timet += 60 * 60;

    tm utc;
    localtime_r(&timet, &utc);
    Helios helios;
    helios.calcSunPos(utc.tm_year, utc.tm_mon, utc.tm_mday, utc.tm_hour, utc.tm_min, utc.tm_sec, ParamBASE_Longitude, ParamBASE_Latitude);

    if (helios.dAzimuth < ParamSHC_ChannelModeShading1AzimutMin || helios.dAzimuth > ParamSHC_ChannelModeShading1AzimutMax)
        return false;
    if (helios.dElevation < ParamSHC_ChannelModeShading1ElevationMin || helios.dElevation > ParamSHC_ChannelModeShading1ElevationMax)
        return false;

    auto shadingBreak = ParamSHC_ChannelModeShading1ShadingBreak;

    // <Enumeration Text="Deaktiviert" Value="0" Id="%ENID%" />
    // <Enumeration Text="Azimut" Value="1" Id="%ENID%" />
    // <Enumeration Text="Höhenwinkel" Value="2" Id="%ENID%" />
    // <Enumeration Text="Azimut UND Höhenwinkel" Value="3" Id="%ENID%" />
    // <Enumeration Text="Azimut ODER Höhenwinkel" Value="4" Id="%ENID%" />

    switch (shadingBreak)
    {
    case 1:
        if (helios.dAzimuth >= ParamSHC_ChannelModeShading1ShadingBreakAzimutMin && helios.dAzimuth <= ParamSHC_ChannelModeShading1ShadingBreakAzimutMax)
            return false;
    case 2:
        if (helios.dElevation >= ParamSHC_ChannelModeShading1ShadingBreakElevationMin && helios.dElevation <= ParamSHC_ChannelModeShading1ShadingBreakElevationMax)
            return false;
    case 3:
        if ((helios.dAzimuth >= ParamSHC_ChannelModeShading1ShadingBreakAzimutMin && helios.dAzimuth <= ParamSHC_ChannelModeShading1ShadingBreakAzimutMax) &&
            (helios.dElevation >= ParamSHC_ChannelModeShading1ShadingBreakElevationMin && helios.dElevation <= ParamSHC_ChannelModeShading1ShadingBreakElevationMax))
            return false;
        break;
        case 4:
        if ((helios.dAzimuth >= ParamSHC_ChannelModeShading1ShadingBreakAzimutMin && helios.dAzimuth <= ParamSHC_ChannelModeShading1ShadingBreakAzimutMax) ||
            (helios.dElevation >= ParamSHC_ChannelModeShading1ShadingBreakElevationMin && helios.dElevation <= ParamSHC_ChannelModeShading1ShadingBreakElevationMax))
            return false;
        break;
    }

    if (!getKo(SHC_KoCHModeShading1LockActive).value(DPT_Switch))
        return false;
    if (ParamSHC_ChannelModeShading1TemperatureActive && (float)KoSHC_TemperatureInput.value(DPT_Value_Temp) < ParamSHC_ChannelModeShading1TemperatureMin)
        return false;
    if (ParamSHC_ChannelModeShading1TemperatureForecast && (float)KoSHC_TemperatureForecastInput.value(DPT_Value_Temp) < ParamSHC_ChannelModeShading1TemperatureForecastMin)
        return false;
    if (ParamSHC_ChannelModeShading1BrightnessActiv && (float)KoSHC_BrightnessInput.value(DPT_Value_Lux) < 1000 * ParamSHC_ChannelModeShading1BrightnessMin)
        return false;
    if (ParamSHC_ChannelModeShading1UVIActiv && (float)KoSHC_UVIInput.value(DPT_DecimalFactor) < ParamSHC_ChannelModeShading1UVIMin)
        return false;
    if (ParamSHC_ChannelModeShading1RainActiv && KoSHC_RainInput.value(DPT_Switch))
        return false;
    if ((uint8_t)KoSHC_CloudsInput.value(DPT_Percent_U8) > ParamSHC_ChannelModeShading1Clouds)
        return false;

    return false;
}

#ifdef SHC_KoCHModeShading2Active
#error SHC_KoCHModeShading2Active is now defined in knxprod.h - preliminary code must be removed
#else
#define SHC_KoCHModeShading2Active SHC_KoCHModeShading1Active
#endif

int16_t ModeShading::koChannelOffset()
{
    return (_index - 1) * (SHC_KoCHModeShading2Active - SHC_KoCHModeShading1Active);
}

GroupObject &ModeShading::getKo(uint8_t ko)
{
    return knx.getGroupObject(ko + koChannelOffset());
}

void ModeShading::start()
{
}
void ModeShading::stop()
{
}
void ModeShading::processInputKo(GroupObject &ko)
{
    switch (ko.asap() - koChannelOffset())
    {
    case SHC_KoCHModeShading1Lock:
        getKo(SHC_KoCHModeShading1LockActive).value(ko.value(DPT_Switch), DPT_Switch);
        _recalcAllowed = true;
        break;
    }
}