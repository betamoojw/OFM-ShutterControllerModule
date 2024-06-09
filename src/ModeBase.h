#pragma once
#include "Arduino.h"
#include "OpenKNX.h"
#include "CallContext.h"

class ModeBase
{
protected:
    uint8_t _channelIndex = 0;
    std::string _logPrefix = "";

    const std::string logPrefix();

public:
    void setup(uint8_t _channelIndex);
    virtual const char* name() = 0;
    virtual void initGroupObjects() = 0;
    virtual bool allowed(const CallContext& callContext) = 0;
    virtual void start() = 0;
    virtual void stop() = 0;
    virtual void processInputKo(GroupObject &ko) = 0;
};