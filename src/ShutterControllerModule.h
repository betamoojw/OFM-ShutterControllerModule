#pragma once
#include "OpenKNX.h"
#include "ChannelOwnerModule.h"
#include "CallContext.h"
#include "MeasurementWatchdog.h"

class ShutterControllerModule : public ShutterControllerChannelOwnerModule
{
  private: 
    bool _shadingDailyActivation = false;
    void setDailyShadingActivation(bool active);
  protected:
    OpenKNX::Channel* createChannel(uint8_t _channelIndex /* this parameter is used in macros, do not rename */) override; 
    uint8_t _lastMinute = 61;
    CallContext _callContext = CallContext();
    MeasurementWatchdog _measurementTemperature = MeasurementWatchdog();
    MeasurementWatchdog _measurementTemperatureForecast = MeasurementWatchdog();
    MeasurementWatchdog _measurementBrightness = MeasurementWatchdog();
    MeasurementWatchdog _measurementUVIndex = MeasurementWatchdog();
    MeasurementWatchdog _measurementRain = MeasurementWatchdog();
    MeasurementWatchdog _measurementClouds = MeasurementWatchdog();
  public:
    ShutterControllerModule();
    void loop() override;
    void setup() override;
    const std::string name() override;
    const std::string version() override;
    void showInformations() override;
    void showHelp() override;
    bool connected();
    bool processCommand(const std::string cmd, bool diagnoseKo) override;
    void processInputKo(GroupObject &ko) override;
};

extern ShutterControllerModule openknxShutterControllerModule;
