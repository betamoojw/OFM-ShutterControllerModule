#pragma once
#include "ModeBase.h"

class ModeNight : public ModeBase
{
    bool _allowed = false;
    bool _recalcAllowed = true;
    uint8_t _lastDayUTC = 0;
    bool sunRise = false;
    bool startTime = false;
    bool sunSet = false;
    bool stopTime = false;
    double getElevationFromSunSetParameter();
    double getElevationFromSunRiseParameter();

protected:
    const char *name() const override;
    uint8_t sceneNumber() const override;
    void initGroupObjects() override;
    bool modeWindowOpenAllowed() const override;
    bool allowed(const CallContext& callContext) override;
    void start(const CallContext& callContext, const ModeBase* previous, PositionController& positionController) override;
    void control(const CallContext& callContext, PositionController& positionController) override;
    void stop(const CallContext& callContext, const ModeBase* next, PositionController& positionController) override;
    void processInputKo(GroupObject &ko) override;
  
};
