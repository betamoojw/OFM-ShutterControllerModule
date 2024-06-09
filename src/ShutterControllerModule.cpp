#include "ShutterControllerModule.h"
#include "ShutterControllerChannel.h"
#include "Helios.h"
#include "Timer.h"

ShutterControllerModule::ShutterControllerModule()
    : ShutterControllerChannelOwnerModule()
{
}

const std::string ShutterControllerModule::name()
{
    return "SHC";
}

const std::string ShutterControllerModule::version()
{
#ifdef MODULE_ShutterControllerModule_Version
    return MODULE_ShutterControllerModule_Version;
#else
    // hides the module in the version output on the console, because the firmware version is sufficient.
    return "";
#endif
}

void ShutterControllerModule::showInformations()
{
    openknx.logger.logWithPrefix(logPrefix(), "Shutter Controller Module");
}

void ShutterControllerModule::showHelp()
{
}

void ShutterControllerModule::loop()
{
    ShutterControllerChannelOwnerModule::loop();

    _callContext.currentMillis = millis();
    if (_callContext.currentMillis == 0)
        _callContext.currentMillis = 1; // 0 can be used as special marker -> skip 0
    Timer &timer = Timer::instance();
    _callContext.timeAndSunValid = timer.isTimerValid();
    _callContext.minuteChanged = false;
    if (_callContext.timeAndSunValid && _lastMinute != timer.getMinute())
    {
        _lastMinute = timer.getMinute();
         _callContext.minuteChanged = true;
        _callContext.hour = timer.getHour();
        _callContext.minute = _lastMinute;
        _callContext.minuteOfDay = _lastMinute + 60 *  _callContext.hour;

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
        _callContext.azimuth = helios.dAzimuth;
        _callContext.elevation = helios.dElevation;
    }
    auto numberOfChannels = getNumberOfChannels();
    for (uint8_t i = 0; i < numberOfChannels; i++)
    {
        auto channel = (ShutterControllerChannel*) getChannel(i);
        if (channel != nullptr)
            channel->execute(_callContext);
    }
}

bool ShutterControllerModule::processCommand(const std::string cmd, bool diagnoseKo)
{
    if (cmd == "CalcSun")
    {

        Helios helios;
        helios.calcSunPos(2024, 5, 28, 20, 00, 00, 15.40664048676037, 47.071934457426266);
        logInfoP("Sun azimut: %d elevation: %d", helios.dAzimuth, helios.dElevation);
        return true;
    }
    return false;
}

void ShutterControllerModule::setup()
{
    setupChannels(ParamSHC_VisibleChannels);
}

OpenKNX::Channel *ShutterControllerModule::createChannel(uint8_t _channelIndex /* this parameter is used in macros, do not rename */)
{
    if (ParamSHC_ChannelDeactivated)
        return nullptr;
    return new ShutterControllerChannel(_channelIndex);
}

ShutterControllerModule openknxShutterControllerModule;
