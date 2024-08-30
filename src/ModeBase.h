#pragma once
#include "Arduino.h"
#include "OpenKNX.h"
#include "CallContext.h"
#include <vector>

class PositionController;

class ModeBase
{
protected:
    uint8_t _channelIndex = 0;
    std::string _logPrefix = "";

    const std::string& logPrefix() const;
public:
    void setup(uint8_t _channelIndex);
    virtual bool windowOpenAllowed() const = 0;
    virtual bool windowTiltAllowed() const = 0;
    virtual const char* name() const = 0;
    virtual uint8_t sceneNumber() const = 0;
    virtual void initGroupObjects() = 0;
    virtual bool allowed(const CallContext& callContext) = 0;
    virtual void start(const CallContext& callContext, const ModeBase* previous, PositionController& positionController) = 0;
    virtual void control(const CallContext& callContext, PositionController& positionController) = 0;
    virtual void stop(const CallContext& callContext, const ModeBase* next, PositionController& positionController) = 0;
    virtual void processInputKo(GroupObject &ko, PositionController& positionController) = 0;
    virtual bool isModeShading() const;
};