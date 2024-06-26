#pragma once
#include "OpenKNX.h"
#include "ChannelOwnerModule.h"
#include "CallContext.h"

class ShutterControllerModule : public ShutterControllerChannelOwnerModule
{
  protected:
    OpenKNX::Channel* createChannel(uint8_t _channelIndex /* this parameter is used in macros, do not rename */) override; 
    uint8_t _lastMinute = 61;
    CallContext _callContext = CallContext();
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
};

extern ShutterControllerModule openknxShutterControllerModule;
