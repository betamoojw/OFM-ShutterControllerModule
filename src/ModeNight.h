#pragma once
#include "ModeBase.h"

class ModeNight : public ModeBase
{
public:
    enum Stage : uint8_t
    {
        StageNone = 0,
        StageEvening = 1, // Vorstufe Abend
        StageNight = 2,   // Nacht
        StageMorning = 3, // Vorstufe Morgen
        StageDay = 4      // Tag
    };

private:
    static constexpr uint8_t NumberOfSwitchPoints = 8;

    struct SwitchPoint
    {
        uint8_t days = 0; // bit 0 = Sunday ... bit 6 = Saturday
        uint8_t stage = StageNone;
        uint8_t trigger = 0;
        uint8_t brightnessLink = 0; // 0 = none, 1 = and, 2 = or
        uint8_t condition = 0;      // 0 = none, 1 = not before, 2 = at the latest
        uint16_t time = 0;
        uint16_t timeOffset = 0;
        uint8_t elevation = 0;
        uint16_t lux = 0;
        uint16_t linkLux = 0;
        uint16_t conditionTime = 0;
        unsigned long luxSince = 0;
        unsigned long linkLuxSince = 0;
    };

    SwitchPoint _switchPoints[NumberOfSwitchPoints];
    bool _allowed = false;
    bool _fired[StageDay + 1] = {};
    uint8_t _stage = StageNone;
    uint8_t _pendingStage = StageNone;
    bool _cycleInitialized = false;
    bool _yieldedToShading = false;
    // stage or night KO waiting for the channel specific delay
    uint8_t _delayedStage = StageNone;
    bool _delayedSilent = false;
    int8_t _delayedNightKo = -1;
    unsigned long _delayStart = 0;
    uint8_t _stageStatus = 0xFF;
    uint16_t _lastMinuteOfDay = 0;

    void readSwitchPoints();
    void readLegacySwitchPoint(SwitchPoint& switchPoint, uint8_t stage, uint8_t behavior, uint16_t time, uint8_t sun, uint8_t elevation);
    void evaluate(const CallContext& callContext, bool reconstruct);
    bool isStageReached(const CallContext& callContext, uint8_t stage);
    bool isSwitchPointReached(const CallContext& callContext, SwitchPoint& switchPoint);
    bool isTriggerReached(const CallContext& callContext, SwitchPoint& switchPoint, bool evening);
    bool isBrightnessReached(const CallContext& callContext, uint16_t lux, bool evening, unsigned long& since, bool& valid);
    bool readBrightness(const CallContext& callContext, float& lux);
    bool isTimeReached(const CallContext& callContext, int16_t minuteOfDay, bool evening);
    void fireStage(uint8_t stage, bool silent);
    void scheduleStage(uint8_t stage, bool silent);
    void applyNightKo(bool night);
    void handleDelayed();
    void updateStageStatus();
    void applyStage(uint8_t stage, PositionController& positionController);
    uint8_t stageAction(uint8_t stage);
    uint8_t stagePosition(uint8_t stage);
    uint8_t stageSlat(uint8_t stage);
    static const char* stageName(uint8_t stage);

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

public:
    bool isNight() const;
};
