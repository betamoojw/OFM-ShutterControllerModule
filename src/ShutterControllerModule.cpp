#include "ShutterControllerModule.h"
#include "ShutterControllerChannel.h"


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
    openknx.logger.logWithPrefix(logPrefix(), "Shutter Controller");
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
    openknx.console.printHelpLine("sc<CC>", "Show information of channel CC. i.e. sc01");
    openknx.console.printHelpLine("sc<CC> s<0|1>", "Disabled or enable shadow of channel CC. i.e. sc01 s1");
    openknx.console.printHelpLine("sc<CC> w<0|1>", "Close or open window of channel CC. i.e. sc01 w1");
    openknx.console.printHelpLine("sc<CC> wt<0|1>", "Tilt window of channel CC. i.e. sc01 wt1");
    openknx.console.printHelpLine("sc<CC> r<value>", "Room temperature. i.e. sc01 t50");
    openknx.console.printHelpLine("sc<CC> h<value>", "Room heating %. i.e. sc01 h50");
    openknx.console.printHelpLine("sc<CC> m<^|v>", "Manual up down. i.e. sc01 mv");
    openknx.console.printHelpLine("sc<CC> m<-|+>", "Manual step i.e. sc01 m+");
    openknx.console.printHelpLine("sc<CC> m<value>", "Manual position %. i.e. sc01 m50");
    openknx.console.printHelpLine("sc<CC> ms<value>", "Manual slat %. i.e. sc01 m50");
    openknx.console.printHelpLine("sc<CC> sim<0|1|2>", "Shutter simulation 0=off,1=normal,2=fast CC. i.e. sc01 sim1");
}

void ShutterControllerModule::loop()
{
    MeasurementWatchdog::resetMissingValue();
    ShutterControllerChannelOwnerModule::loop();
    _callContext.currentMillis = max(millis(), 1uL); // 0 can be used as special marker -> skip 0

    _measurementTemperature.update(_callContext.currentMillis, _callContext.diagnosticLog);
    _measurementTemperatureForecast.update(_callContext.currentMillis, _callContext.diagnosticLog);
    _measurementBrightness.update(_callContext.currentMillis, _callContext.diagnosticLog);
    _measurementUVIndex.update(_callContext.currentMillis, _callContext.diagnosticLog);
    _measurementRain.update(_callContext.currentMillis, _callContext.diagnosticLog);
    _measurementClouds.update(_callContext.currentMillis, _callContext.diagnosticLog);

    _callContext.timeAndSunValid = openknx.time.isValid() && openknx.sun.isSunCalculatioValid();
    auto utcTime = openknx.time.getUtcTime();
    _callContext.minuteChanged = false;
    if (_callContext.timeAndSunValid &&
        (_lastMinute != utcTime.minute ||
         _lastHour != utcTime.hour ||
         _lastDay != utcTime.day ||
         _lastMonth != utcTime.month ||
         _lastYear != utcTime.year))
    {
        _lastMinute = utcTime.minute;
        _lastHour = utcTime.hour;
        _lastDay = utcTime.day;
        _lastMonth = utcTime.month;
        _lastYear = utcTime.year;

        auto localTime = utcTime.toLocalTime();
        _callContext.minuteChanged = true;
        _callContext.localTime = localTime;
        _callContext.minuteOfDay = localTime.minute + 60 * localTime.hour;
        
        auto localTimeInStandardTime = localTime;
        if (localTime.isDst)
             localTimeInStandardTime.addSeconds(openknx.time.daylightSavingTimeOffset() * -1);
        localTimeInStandardTime.isDst = false;

         _callContext.localTimeInStandardTimeDay = utcTime.day;
         _callContext.localTimeInStandardTime = localTimeInStandardTime;

        _callContext.shadingDailyActivation = _shadingDailyActivation;
        _callContext.azimuth = openknx.sun.azimut();
        _callContext.elevation = openknx.sun.elevation();
    }
    _callContext.diagnosticLog = false;
    auto numberOfChannels = getNumberOfChannels();
    for (uint8_t i = 0; i < numberOfChannels; i++)
    {
        auto channel = (ShutterControllerChannel *)getChannel(i);
        if (channel != nullptr)
        {
            if (_shadingDailyActivation &&
                _callContext.minuteChanged &&
                _callContext.localTime.minute == 0 &&
                _callContext.localTime.hour == 0)
                channel->activateShading();

            channel->execute(_callContext);
        }
    }
    if (_lastMissingValue != MeasurementWatchdog::missingValue())
    {
        _lastMissingValue = MeasurementWatchdog::missingValue();
        logInfoP("Missing value: %d", (int)_lastMissingValue);
        KoSHC_MeasurementError.value(_lastMissingValue, DPT_Switch);
    }
    _measurementTemperature.resetChanged();
    _measurementTemperatureForecast.resetChanged();
    _measurementBrightness.resetChanged();
    _measurementUVIndex.resetChanged();
    _measurementRain.resetChanged();
    _measurementClouds.resetChanged();
}

bool ShutterControllerModule::processCommand(const std::string cmd, bool diagnoseKo)
{
    if (cmd == "sc")
    {
        _callContext.diagnosticLog = true;
        loop();
        return true;
    }
    else if (cmd.rfind("sc ") == 0)
    {
        // module commands
        auto moduleCommand = cmd.substr(3);
        if (moduleCommand.rfind("t") == 0)
        {
            if (moduleCommand.length() == 1)
            {
                _measurementTemperature.logState(true);
                return true;
            }
            logInfoP("Set temperature");
            KoSHC_TempInput.valueNoSend(std::stof(moduleCommand.substr(1)), DPT_Value_Temp);
            processInputKo(KoSHC_TempInput);
            return true;
        }
        else if (moduleCommand.rfind("f") == 0)
        {
            if (moduleCommand.length() == 1)
            {
                _measurementTemperatureForecast.logState(true);
                return true;
            }
            logInfoP("Set temperature forecast");
            KoSHC_TempForecastInput.valueNoSend(std::stof(moduleCommand.substr(1)), DPT_Value_Temp);
            processInputKo(KoSHC_TempForecastInput);
            return true;
        }
        else if (moduleCommand.rfind("b") == 0)
        {
            if (moduleCommand.length() == 1)
            {
                _measurementBrightness.logState(true);
                return true;
            }
            logInfoP("Set brightness");
            KoSHC_BrightnessInput.valueNoSend((uint8_t)std::stoi(moduleCommand.substr(1)), DPT_Value_Lux);
            processInputKo(KoSHC_BrightnessInput);
            return true;
        }
        else if (moduleCommand.rfind("u") == 0)
        {
            if (moduleCommand.length() == 1)
            {
                _measurementUVIndex.logState(true);
                return true;
            }
            logInfoP("Set UVI");
            KoSHC_UVIInput.valueNoSend(std::stof(moduleCommand.substr(1)), DPT_DecimalFactor);
            processInputKo(KoSHC_UVIInput);
            return true;
        }
        else if (moduleCommand.rfind("r") == 0)
        {
            if (moduleCommand.length() == 1)
            {
                _measurementRain.logState(true);
                return true;
            }
            logInfoP("Set rain");
            KoSHC_RainInput.valueNoSend(1 == std::stoi(moduleCommand.substr(1)), DPT_Switch);
            processInputKo(KoSHC_RainInput);
            return true;
        }
        else if (moduleCommand.rfind("c") == 0)
        {
            if (moduleCommand.length() == 1)
            {
                _measurementClouds.logState(true);
                return true;
            }
            logInfoP("Set clouds");
            KoSHC_CloudsInput.valueNoSend((uint8_t)std::stoi(moduleCommand.substr(1)), DPT_Scaling);
            processInputKo(KoSHC_CloudsInput);
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

    //ShutterControllerChannelOwnerModule::processInputKo(ko);

    auto numberOfChannels = getNumberOfChannels();
    for (uint8_t i = 0; i < numberOfChannels; i++)
    {
        auto channel = (ShutterControllerChannel *)getChannel(i);
        if (channel != nullptr)
            channel->processInputKo(ko, &_callContext);
    }
}

void ShutterControllerModule::setup()
{
    _measurementTemperature.init(
        "Temperature",
        ParamSHC_HasTemperaturInput ? &KoSHC_TempInput : nullptr,
        ParamSHC_TempWatchdog,
        KNXValue(ParamSHC_TempFallback),
        DPT_Value_Temp,
        (MeasurementWatchdogFallbackBehavior)ParamSHC_TempFallbackMode);
    _callContext.measurementTemperature = &_measurementTemperature;

    _measurementTemperatureForecast.init(
        "Temperature Forecast",
        ParamSHC_HasTemperaturForecastInput ? &KoSHC_TempForecastInput : nullptr,
        ParamSHC_TempForecastWatchdog,
        KNXValue(ParamSHC_TempForecastFallback),
        DPT_Value_Temp,
        (MeasurementWatchdogFallbackBehavior)ParamSHC_TempForecastFallbackMode);
    _callContext.measurementTemperatureForecast = &_measurementTemperatureForecast;

    _measurementBrightness.init(
        "Brightness",
        ParamSHC_HasBrightnessInput ? &KoSHC_BrightnessInput : nullptr,
        ParamSHC_BrightnessWatchdog,
        KNXValue((float)ParamSHC_BrightnessFallback * 1000.F),
        DPT_Value_Lux,
        (MeasurementWatchdogFallbackBehavior)ParamSHC_BrightnessFallbackMode);
    _callContext.measurementBrightness = &_measurementBrightness;

    _measurementUVIndex.init(
        "UV Index",
        ParamSHC_HasUVIInput ? &KoSHC_UVIInput : nullptr,
        ParamSHC_UVIWatchdog,
        KNXValue(ParamSHC_UVIFallback),
        DPT_DecimalFactor,
        (MeasurementWatchdogFallbackBehavior)ParamSHC_UVIFallbackMode);
    _callContext.measurementUVIndex = &_measurementUVIndex;

    _measurementRain.init(
        "Rain",
        ParamSHC_HasRainInput ? &KoSHC_RainInput : nullptr,
        ParamSHC_RainWatchdog,
        KNXValue(ParamSHC_RainFallback),
        DPT_Switch,
        (MeasurementWatchdogFallbackBehavior)ParamSHC_RainFallbackMode);
    _callContext.measurementRain = &_measurementRain;

    _measurementClouds.init(
        "Clouds",
        ParamSHC_HasCloudsInput ? &KoSHC_CloudsInput : nullptr,
        ParamSHC_CloudsWatchdog,
        KNXValue((uint8_t)ParamSHC_CloudsFallback),
        DPT_Scaling,
        (MeasurementWatchdogFallbackBehavior)ParamSHC_CloudsFallbackMode);
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
    if (ParamSHC_CType == 0)
    {
        logDebugP("Channel %d not used", _channelIndex);
        return nullptr;
    }
    if (ParamSHC_CDeactivated)
    {
        logDebugP("Channel %d deactivated", _channelIndex);
        return nullptr;
    }
    return new ShutterControllerChannel(_channelIndex);
}

ShutterControllerModule openknxShutterControllerModule;
