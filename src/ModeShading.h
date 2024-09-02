#pragma once
#include "ModeBase.h"


enum ModeShadingNotAllowedReason : uint32_t
{
    ModeShadingNotAllowedReasonNone = 0,
    ModeShadingNotAllowedReasonTimeNotValid = 1,
    ModeShadingNotAllowedReasonSwitchedOffNoReactivation = 2,
    ModeShadingNotAllowedReasonSwitchedOff = 4,
    ModeShadingNotAllowedReasonSwitchedOffUntilEndOfPeriod = 8,
    ModeShadingNotAllowedReasonSwitchedOffWaitTime = 16,
    ModeShadingNotAllowedReasonChannelLock = 32,
    ModeShadingNotAllowedReasonLock = 64,
    ModeShadingNotAllowedReasonSunElevation = 128,
    ModeShadingNotAllowedReasonSunAzimut = 256,
    ModeShadingNotAllowedReasonSunBreak = 512,
    ModeShadingNotAllowedReasonShutterPosition = 1024,
    ModeShadingNotAllowedReasonStartWaitTime = 2048,
    ModeShadingNotAllowedReasonManualUsage = 4096,  
    ModeShadingNotAllowedReasonWindowOpen = 8192,
    ModeShadingNotAllowedReasonRoomTemperature = 16384,
    ModeShadingNotAllowedReasonHeating = 32768,
    ModeShadingNotAllowedReasonHeatingInThePast = 65536,
    ModeShadingNotAllowedReasonRain = 131072,
    ModeShadingNotAllowedReasonBrightness = 262144,
    ModeShadingNotAllowedReasonTemperature =  524288,
    ModeShadingNotAllowedReasonTemperatureForecase = 1048576,
    ModeShadingNotAllowedReasonClouds = 2097152,
    ModeShadingNotAllowedReasonUVI = 4194304,
  
};

class ModeShading : public ModeBase
{
    std::string _name;
    uint8_t _index;
    uint32_t _notAllowedReason = ModeShadingNotAllowedReason::ModeShadingNotAllowedReasonNone;
    uint32_t _lastNotAllowedReason = 1;
    bool _recalcMeasurmentValues = true;
    bool _allowedByMeasurementValues = false;
    bool _allowedByMeasurementValuesAndHeatingOffWaitTime = false;
    bool _lockActive = false;
    bool _active = false;
    unsigned long _waitTimeAfterMeasurmentValueChange = 0;
    unsigned long _waitTimeAfterHeatingValueChange = 0;
    bool _lastSunFrameAllowed = false;
    bool _needWaitTime = false;
    unsigned long _lastHeadingTimeStamp = 0;
    bool _heatingOff = true;
    int16_t koChannelOffset();
    GroupObject& getKo(uint8_t ko);
    bool allowedByMeasurmentValues(const CallContext& callContext);
    bool handleMeasurmentValue(bool& allowed, bool enabled, const MeasurementWatchdog *measurementWatchdog, const CallContext &callContext, bool (*predicate)(const MeasurementWatchdog *, uint8_t _channelIndex, uint8_t _index, bool previousAllowed), ModeShadingNotAllowedReason reasonBit);
    void updateDiagnosticKos();
public:
    ModeShading(uint8_t index);
    bool allowedBySun(const CallContext& callContext);
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
    bool isModeShading() const override;
};