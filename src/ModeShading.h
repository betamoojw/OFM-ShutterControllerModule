#pragma once
#include "ModeBase.h"

class ModeShading : public ModeBase
{
    std::string _name;
    uint8_t _index;
    bool _recalcMeasurmentValues = true;
    bool _allowedByMeasurementValues = false;
    bool _lockActive = false;
    bool _active = false;
    unsigned long _waitTimeAfterMeasurmentValueChange = 0;
    bool _lastTimeAndSunFrameAllowed = false;
    bool _needWaitTime = false;
    unsigned long _lastHeadingTimeStamp = 0;
    bool _heatingOff = true;
    int16_t koChannelOffset();
    GroupObject& getKo(uint8_t ko);
    bool allowedByMeasurmentValues(const CallContext& callContext);
    bool handleMeasurmentValue(bool& allowed, bool enabled, const MeasurementWatchdog *measurementWatchdog, const CallContext &callContext, bool (*predicate)(const MeasurementWatchdog *, uint8_t _channelIndex, uint8_t _index));

public:
    ModeShading(uint8_t index);
    bool allowedByTimeAndSun(const CallContext& callContext);
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
    bool isModeShading() const override;
};