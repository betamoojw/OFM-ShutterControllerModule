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
    int16_t koChannelOffset();
    GroupObject& getKo(uint8_t ko);
    bool allowedByMeasurmentValues();
    bool allowedByTimeAndSun();
public:
    ModeShading(uint8_t index);
protected:
    const char *name() override;
    void initGroupObjects() override;
    bool allowed(unsigned long currentMillis) override;
    void start() override;
    void stop() override;
    void processInputKo(GroupObject &ko) override;
};