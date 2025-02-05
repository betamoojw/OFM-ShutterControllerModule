#pragma once
#include "ModeBase.h"

class ModeNight : public ModeBase
{
    bool _allowed = false;
    bool _recalcAllowed = true;
    uint8_t _lastDayInStandardTime = 0;
    bool _sunRise = false;
    bool _startTime = false;
    bool _sunSet = false;
    bool _stopTime = false;
    bool _startPositionDeativatedForPeriod = false;
    double getElevationFromSunSetParameter();
    double getElevationFromSunRiseParameter();

protected:
    const char *name() const override;
    uint8_t sceneNumber() const override;
    void initGroupObjects() override;
    bool windowOpenAllowed() const override;
    bool windowTiltAllowed() const override;
    bool allowed(const CallContext& callContext) override;
    void start(const CallContext& callContext, const ModeBase* previous, PositionController& positionController) override;
    void control(const CallContext& callContext, PositionController& positionController) override;
    void stop(const CallContext& callContext, const ModeBase* next, PositionController& positionController) override;
    void processInputKo(GroupObject &ko, PositionController& positionController) override;
  
};
