#include "ShutterControllerModule.h"
#include "ShutterControllerChannel.h"
#include "SunPos.h"
#include "Timer.h"

void sunpos2(cTime udtTime, cLocation udtLocation, cSunCoordinates *udtSunCoordinates);

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
    openknx.console.printHelpLine("sc t<value>", "Set temperature. i.e. sc t28.3");
    openknx.console.printHelpLine("sc f<value>", "Set temperature forecast. i.e. sc f28.3");
    openknx.console.printHelpLine("sc b<value>", "Set brightness. i.e. sb b1000");
    openknx.console.printHelpLine("sc u<value>", "Set UV index. i.e. sb u1.5");
    openknx.console.printHelpLine("sc r<0|1>", "Set rain. i.e. sb r1");
    openknx.console.printHelpLine("sc c<value>", "Set clouds. i.e. sb c20");
    openknx.console.printHelpLine("sc d<YYMMDDhh2mm>", "Set date/time . i.e. d d2304151733");
    openknx.console.printHelpLine("sc<CC>", "Show information of channel CC. i.e. sc01");
    openknx.console.printHelpLine("sc<CC> s<0|1>", "Disabled or enable shadow of channel CC. i.e. sc01 s1");
    openknx.console.printHelpLine("sc<CC> w<0|1>", "Close or open window of channel CC. i.e. sc01 w1");
    openknx.console.printHelpLine("sc<CC> wt<0|1>", "Tilt window of channel CC. i.e. sc01 wt1");
    openknx.console.printHelpLine("sc<CC> t<value>", "Room temperature. i.e. sc01 t50");
    openknx.console.printHelpLine("sc<CC> h<value>", "Room heating %. i.e. sc01 h50");
}

void ShutterControllerModule::loop()
{
    ShutterControllerChannelOwnerModule::loop();

    _measurementTemperature.update(_callContext.currentMillis, _callContext.diagnosticLog);
    _measurementTemperatureForecast.update(_callContext.currentMillis, _callContext.diagnosticLog);
    _measurementBrightness.update(_callContext.currentMillis, _callContext.diagnosticLog);
    _measurementUVIndex.update(_callContext.currentMillis, _callContext.diagnosticLog);
    _measurementRain.update(_callContext.currentMillis, _callContext.diagnosticLog);
    _measurementClouds.update(_callContext.currentMillis, _callContext.diagnosticLog);

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
        _callContext.minuteOfDay = _callContext.minute + 60 * _callContext.hour;
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
        _callContext.UtcMinuteOfDay = _callContext.UtcMinute + 60 * _callContext.UtcHour;

        double latitude = ParamBASE_Latitude;
        double longitude = ParamBASE_Longitude;

        cTime cTime = {0};
        cTime.iYear = _callContext.UtcYear;
        cTime.iMonth = _callContext.UtcMonth;
        cTime.iDay = _callContext.UtcDay;
        cTime.dHours = _callContext.UtcHour;
        cTime.dMinutes = _callContext.UtcMinute;
        cTime.dSeconds = 0;

        cLocation cLocation = {0};
        cLocation.dLatitude = latitude;
        cLocation.dLongitude = longitude;

        cSunCoordinates cSunCoordinates;
        sunpos(cTime, cLocation, &cSunCoordinates);
        _callContext.azimuth = cSunCoordinates.dAzimuth;
        _callContext.elevation = 90 - cSunCoordinates.dZenithAngle;
    }
    auto numberOfChannels = getNumberOfChannels();
    for (uint8_t i = 0; i < numberOfChannels; i++)
    {
        auto channel = (ShutterControllerChannel *)getChannel(i);
        if (channel != nullptr)
        {
            if (_shadingDailyActivation &&
                _callContext.minuteChanged &&
                _callContext.minute == 0 &&
                _callContext.hour == 0)
                channel->activateShading();

            channel->execute(_callContext);
        }
    }
}

bool ShutterControllerModule::processCommand(const std::string cmd, bool diagnoseKo)
{
    if (cmd == "sc")
    {
        // module info
        logInfoP("Used cordinates: %f %f", (double)ParamBASE_Latitude, (double)ParamBASE_Longitude);

        if (!_callContext.timeAndSunValid)
            logInfoP("No valid time");
        else
        {
            logInfoP("Local Time: %04d-%02d-%02d %02d:%02d %s", (int)_callContext.year, (int)_callContext.month, (int)_callContext.day, (int)_callContext.hour, (int)_callContext.minute, _callContext.summerTime ? "Summertime" : "Wintertime");
            logInfoP("UTC: %04d-%02d-%02d %02d:%02d", (int)_callContext.UtcYear, (int)_callContext.UtcMonth, (int)_callContext.UtcDay, (int)_callContext.UtcHour, (int)_callContext.UtcMinute);
            logInfoP("Aizmut: %.2f°", (double)_callContext.azimuth);
            logInfoP("Elevation: %.2f°", (double)_callContext.elevation);
        }
        return true;
    }
    else if (cmd.rfind("sc ") == 0)
    {
        // module commands
        auto moduleCommand = cmd.substr(3);
        if (moduleCommand.rfind("t") == 0)
        {
            logInfoP("Set temperature");
            KoSHC_TempInput.valueNoSend(std::stof(moduleCommand.substr(1)), DPT_Value_Temp);
            processInputKo(KoSHC_TempInput);
            return true;
        }
        else if (moduleCommand.rfind("f") == 0)
        {
            logInfoP("Set temperature forecast");
            KoSHC_TempForecastInput.valueNoSend(std::stof(moduleCommand.substr(1)), DPT_Value_Temp);
            processInputKo(KoSHC_TempForecastInput);
            return true;
        }
        else if (moduleCommand.rfind("b") == 0)
        {
            logInfoP("Set brightness");
            KoSHC_BrightnessInput.valueNoSend(std::stoi(moduleCommand.substr(1)), DPT_Value_Lux);
            processInputKo(KoSHC_BrightnessInput);
            return true;
        }
        else if (moduleCommand.rfind("u") == 0)
        {
            logInfoP("Set UVI");
            KoSHC_UVIInput.valueNoSend(std::stof(moduleCommand.substr(1)), DPT_DecimalFactor);
            processInputKo(KoSHC_UVIInput);
            return true;
        }
        else if (moduleCommand.rfind("r") == 0)
        {
            logInfoP("Set rain");
            KoSHC_RainInput.valueNoSend(std::stoi(moduleCommand.substr(1)), DPT_Switch);
            processInputKo(KoSHC_RainInput);
            return true;
        }
        else if (moduleCommand.rfind("c") == 0)
        {
            logInfoP("Set clouds");
            KoSHC_CloudsInput.valueNoSend(std::stoi(moduleCommand.substr(1)), DPT_Scaling);
            processInputKo(KoSHC_CloudsInput);
            return true;
        }
        else if (moduleCommand.rfind("d") == 0)
        {
            logInfoP("Set date/time");
            tm tm = {0};
            if (moduleCommand.length() == 1)
            {
                tm.tm_year = 2024;
                tm.tm_mon = 7;
                tm.tm_mday = 1;
                tm.tm_hour = 15;
                tm.tm_min = 0;
            }
            else if (moduleCommand.length() == 5)
            {
                tm.tm_year = Timer::instance().isTimerValid() ? Timer::instance().getYear() : 2024;
                tm.tm_mon = Timer::instance().isTimerValid() ? Timer::instance().getMonth() : 1;
                tm.tm_mday = Timer::instance().isTimerValid() ? Timer::instance().getDay() : 7;
                tm.tm_hour = stoi(moduleCommand.substr(1, 2));
                tm.tm_min = stoi(moduleCommand.substr(3, 2));
            }
            else if (moduleCommand.length() == 11)
            {
                tm.tm_year = stoi(moduleCommand.substr(1, 2)) + 2000;
                tm.tm_mon = stoi(moduleCommand.substr(3, 2));
                tm.tm_mday = stoi(moduleCommand.substr(5, 2));
                tm.tm_hour = stoi(moduleCommand.substr(7, 2));
                tm.tm_min = stoi(moduleCommand.substr(9, 2));
            }
            else
            {
                logInfoP("Invalid time format");
                return true;
            }
            logInfoP("%04d-%02d-%02d %02d:%02d", (int)tm.tm_year, (int)tm.tm_mon, (int)tm.tm_mday, (int)tm.tm_hour, (int)tm.tm_min);
            Timer::instance().setDateTimeFromBus(&tm);
            return true;
        }
        else
        {
            logInfoP("Unkown ShutterControllerModule command '%s'", moduleCommand.c_str());
            return true;
        }
    }
    else if (cmd.rfind("sc", 0) == 0)
    {
        // channel commands
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
            auto channel = (ShutterControllerChannel *)getChannel(channelNumber - 1);
            if (channel == nullptr)
            {
                logInfoP("Channel not %d activated", channel);
                return true;
            }
            bool diagnosticLogLoopRequest = false;
            bool handled = channel->processCommand(channelCmd, diagnoseKo, diagnosticLogLoopRequest);
            if (diagnosticLogLoopRequest)
            {
                _callContext.diagnosticLog = true;
                channel->execute(_callContext);
                _callContext.diagnosticLog = false;
            }
            return handled;
        }
    }
    return false;
}

void ShutterControllerModule::setDailyShadingActivation(bool active)
{
    _shadingDailyActivation = active;
    // <Enumeration Text="Aus" Value="0" Id="%ENID%" />
    // <Enumeration Text="Ein" Value="1" Id="%ENID%" />
    // <Enumeration Text="Über KO, Standard AUS" Value="2" Id="%ENID%" />
    // <Enumeration Text="Über KO, Standard EIN" Value="3" Id="%ENID%" />
    // <Enumeration Text="Über KO, Standard AUS, Initial vom Bus lesen" Value="4" Id="%ENID%" />
    // <Enumeration Text="Über KO, Standard EIN, Initial vom Bus lesen" Value="5" Id="%ENID%" />
    if (ParamSHC_ShadingActivation >= 2)
    {
        KoSHC_ShadingControlDailyActivationStatus.value(active, DPT_Switch);
    }
    if (active)
    {
        auto numberOfChannels = getNumberOfChannels();
        for (uint8_t i = 0; i < numberOfChannels; i++)
        {
            auto channel = (ShutterControllerChannel *)getChannel(i);
            if (channel != nullptr)
                channel->activateShading();
        }
    }
}
void ShutterControllerModule::processInputKo(GroupObject &ko)
{
    if (ko.asap() == SHC_KoShadingControlDailyActivation)
    {
        setDailyShadingActivation(ko.value(DPT_Switch));
        return;
    }
    _measurementTemperature.processIputKo(ko);
    _measurementTemperatureForecast.processIputKo(ko);
    _measurementBrightness.processIputKo(ko);
    _measurementUVIndex.processIputKo(ko);
    _measurementRain.processIputKo(ko);
    _measurementClouds.processIputKo(ko);
    
    ShutterControllerChannelOwnerModule::processInputKo(ko);
}

void ShutterControllerModule::setup()
{
    _measurementTemperature.init(
        "Temperature",
        ParamSHC_HasTemperaturInput ? &KoSHC_TempInput : nullptr,
        ParamSHC_TempWatchdog,
        KNXValue(ParamSHC_TempFallback),
        DPT_Value_Temp,
        (MeasurementWatchdogFallbackBehavior) ParamSHC_TempFallbackMode);
    _callContext.measurementTemperature = &_measurementTemperature;

    _measurementTemperatureForecast.init(
        "Temperature Forecast",
        ParamSHC_HasTemperaturForecastInput ? &KoSHC_TempForecastInput : nullptr,
        ParamSHC_TempForecastWatchdog,
        KNXValue(ParamSHC_TempForecastFallback),
        DPT_Value_Temp,
        (MeasurementWatchdogFallbackBehavior) ParamSHC_TempForecastFallbackMode);
    _callContext.measurementTemperatureForecast = &_measurementTemperatureForecast;

    _measurementBrightness.init(
        "Brightness",
        ParamSHC_HasBrightnessInput ? &KoSHC_BrightnessInput : nullptr,
        ParamSHC_BrightnessWatchdog,
        KNXValue(ParamSHC_BrightnessFallback),
        DPT_Value_Lux,
        (MeasurementWatchdogFallbackBehavior) ParamSHC_BrightnessFallbackMode);
    _callContext.measurementBrightness = &_measurementBrightness;    

    _measurementUVIndex.init(
        "UV Index",
        ParamSHC_HasUVIInput ? &KoSHC_UVIInput : nullptr,
        ParamSHC_UVIWatchdog,
        KNXValue(ParamSHC_UVIFallback),
        DPT_DecimalFactor,
        (MeasurementWatchdogFallbackBehavior) ParamSHC_UVIFallbackMode);
    _callContext.measurementUVIndex = &_measurementUVIndex;

    _measurementRain.init(  
        "Rain",
        ParamSHC_HasRainInput ? &KoSHC_RainInput : nullptr,
        ParamSHC_RainWatchdog,
        KNXValue(ParamSHC_RainFallback),
        DPT_Switch,
        (MeasurementWatchdogFallbackBehavior) ParamSHC_RainFallbackMode);
    _callContext.measurementRain = &_measurementRain;

    _measurementClouds.init(
        "Clouds",
        ParamSHC_HasCloudsInput ? &KoSHC_CloudsInput : nullptr,
        ParamSHC_CloudsWatchdog,
        KNXValue(ParamSHC_CloudsFallback),
        DPT_Scaling,
        (MeasurementWatchdogFallbackBehavior) ParamSHC_CloudsFallbackMode);
    _callContext.measurementClouds = &_measurementClouds;

    // <Enumeration Text="Aus" Value="0" Id="%ENID%" />
    // <Enumeration Text="Ein" Value="1" Id="%ENID%" />
    // <Enumeration Text="Über KO, Standard AUS" Value="2" Id="%ENID%" />
    // <Enumeration Text="Über KO, Standard EIN" Value="3" Id="%ENID%" />
    // <Enumeration Text="Über KO, Standard AUS, Initial vom Bus lesen" Value="4" Id="%ENID%" />
    // <Enumeration Text="Über KO, Standard EIN, Initial vom Bus lesen" Value="5" Id="%ENID%" />
    switch (ParamSHC_ShadingActivation)
    {
    case 0:
        _shadingDailyActivation = false;
        break;
    case 1:
        _shadingDailyActivation = true;
        break;
    case 2:
        setDailyShadingActivation(false);
        KoSHC_ShadingControlDailyActivation.value(false, DPT_Switch);
        break;
    case 3:
        setDailyShadingActivation(true);
        KoSHC_ShadingControlDailyActivation.value(true, DPT_Switch);
        break;
    case 4:
        setDailyShadingActivation(false);
        KoSHC_ShadingControlDailyActivation.value(false, DPT_Switch);
        KoSHC_ShadingControlDailyActivation.requestObjectRead();
        break;
    case 5:
        setDailyShadingActivation(true);
        KoSHC_ShadingControlDailyActivation.value(true, DPT_Switch);
        KoSHC_ShadingControlDailyActivation.requestObjectRead();
        break;
    }
    setupChannels(ParamSHC_VisibleChannels);
}

OpenKNX::Channel *ShutterControllerModule::createChannel(uint8_t _channelIndex /* this parameter is used in macros, do not rename */)
{
    if (ParamSHC_CDeactivated)
        return nullptr;
    return new ShutterControllerChannel(_channelIndex);
}

ShutterControllerModule openknxShutterControllerModule;
