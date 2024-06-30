#pragma once
#include "ModeBase.h"

class ModeShading : public ModeBase
{
    std::string _name;
    uint8_t _index;
    bool _recalcMeasurmentValues = true;
    bool _allowedByMeasurementValues = false;
    bool _active = false;
    unsigned long _waitTimeAfterMeasurmentValueChange = 0;
    bool _lastTimeAndSunFrameAllowed = false;
    bool _needWaitTime = false;
    int16_t koChannelOffset();
    GroupObject& getKo(uint8_t ko);
    bool allowedByMeasurmentValues(bool diagnosticLog);
    bool allowedByTimeAndSun(const CallContext& callContext);
public:
    ModeShading(uint8_t index);
protected:
    const char *name() override;
    void initGroupObjects() override;
    bool modeWindowOpenAllowed() const override;
    bool allowed(const CallContext& callContext) override;
    void start(ModeBase* previous) override;
    void stop(ModeBase* next) override;
    void processInputKo(GroupObject &ko) override;
    bool isShading() override;
};