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
    openknx.console.printHelpLine("sc", "Show Shutter Controller Information");
    openknx.console.printHelpLine("sc<CC>", "Show information of channel CC. i.e. sc01");  
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
        _callContext.minuteOfDay = _callContext.minute + 60 *  _callContext.hour;
        _callContext.year = timer.getYear();
        _callContext.month = timer.getMonth();
        _callContext.day = timer.getDay();
        _callContext.summerTime = timer.IsSummertime();

        tm localTime = {0};
        localTime.tm_isdst = -1;
        localTime.tm_year = timer.getYear() - 1900;
        localTime.tm_mon = timer.getMonth() - 1;
        localTime.tm_mday = timer.getDay();
        localTime.tm_hour = timer.getHour();
        localTime.tm_min = timer.getMinute();
        localTime.tm_sec = timer.getSecond();
     
        std::time_t timet = std::mktime(&localTime);
        timet -= ParamBASE_Timezone * 60 * 60;
        if (timer.IsSummertime())
        {
            timet -= 60 * 60;
        }

        tm utc;
        localtime_r(&timet, &utc);

        _callContext.UtcYear = utc.tm_year + 1900;
        _callContext.UtcMonth = utc.tm_mon + 1;
        _callContext.UtcDay = utc.tm_mday;
        _callContext.UtcHour = utc.tm_hour;
        _callContext.UtcMinute = utc.tm_min;
        _callContext.UtcMinuteOfDay = _callContext.UtcMinute + 60 *  _callContext.UtcHour;
      

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
    if (cmd == "sc")
    {
        if (!_callContext.timeAndSunValid)
            logInfoP("No valid time");
        else
        {
            logInfoP("UTC: %04d-%02d-%02d %02d-%02d", (int) _callContext.UtcYear, (int) _callContext.UtcMonth, (int) _callContext.UtcDay, (int) _callContext.UtcHour, (int) _callContext.UtcMinute);
            
            logInfoP("Local Time: %04d-%02d-%02d %02d-%02d %s", (int) _callContext.year, (int) _callContext.month, (int) _callContext.day, (int) _callContext.hour, (int) _callContext.minute, _callContext.summerTime ? "Summertime" : "Wintertime");
            logInfoP("Aizmut: %.1f°", (double) _callContext.azimuth);
            logInfoP("Elevation: %.1f°", (double) _callContext.elevation);
        }
        return true;
    }
    else if (cmd.rfind("sc", 0) == 0)
    {
        auto channelString = cmd.substr(2);
        if (channelString.length() > 0)
        {
            auto pos = channelString.find_first_of(' ');
            std::string channelNumberString;
            std::string channelCmd;
            if (pos > 0 && pos != std::string::npos)
            {
                channelNumberString = channelString.substr(0, pos);
                channelCmd = channelString.substr(pos + 1);
            }
            else
            {
                channelNumberString = channelString;
                channelCmd = "";
            }
            auto channelNumber = atoi(channelNumberString.c_str());
            if (channelNumber < 1 || channelNumber > getNumberOfChannels())
            {
                logInfoP("Channel %d not found", channelNumber);
                return true;
            }
            auto channel = (ShutterControllerChannel*) getChannel(channelNumber - 1);
            if (channel == nullptr)
            {
                logInfoP("Channel not %d activated", channel);
                return true;
            }
            return channel->processCommand(channelCmd, diagnoseKo);
        }
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
