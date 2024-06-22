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
    const char *name() override;
    void initGroupObjects() override;
    bool allowed(const CallContext& callContext) override;
    void start(ModeBase* previous) override;
    void stop(ModeBase* next) override;
    void processInputKo(GroupObject &ko) override;
  
};
