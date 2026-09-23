#pragma once
#include "ModeBase.h"
class ShutterControllerChannel;

class ModeScene : public ModeBase
{
public:
    struct LearnedValue
    {
        bool valid = false;
        uint8_t height = 0;
        uint8_t slat = 0;
    };

private:
    ShutterControllerChannel& _channel;

    // 1-based slot index (1..16), 0 = no request/no active scene
    uint8_t _pendingSlot = 0;
    unsigned long _pendingUntil = 0;
    uint8_t _activeSlot = 0;
    bool _requestStart = false;
    // Set when the applied scene hands control back to the automatics right away.
    bool _releaseAfterApply = false;

    // Snapshot of the lock states before entering scene mode, and the value
    // a scene last set them to (to detect whether a KO changed them since).
    bool _shadingBefore = false;
    bool _shadingSetByScene = false;
    bool _shadingTouched = false;
    bool _nightBefore = false;
    bool _nightSetByScene = false;
    bool _nightTouched = false;
    bool _manualBefore = false;
    bool _manualSetByScene = false;
    bool _manualTouched = false;

    LearnedValue _learned[17]; // index 1..16, index 0 unused

    struct SceneRow
    {
        bool active;
        uint8_t number;
        bool storable;
        bool heightEnabled;
        uint8_t height;
        bool slatEnabled;
        uint8_t slat;
        uint8_t lock;
        uint16_t delaySeconds;
        bool hold;
    };
    SceneRow readSceneRow(uint8_t slot) const;
    uint8_t findSlotForSceneNumber(uint8_t knxSceneNumber) const;
    void applyScene(uint8_t slot, PositionController& positionController);
    void applyLock(uint8_t lockCode, bool armRestore);
    void restoreLocks();

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
    ModeScene(ShutterControllerChannel& channel);

    // Used by ShutterControllerModule to persist/restore learned positions.
    bool hasLearnedValue(uint8_t slot) const;
    uint8_t learnedHeight(uint8_t slot) const;
    uint8_t learnedSlat(uint8_t slot) const;
    void loadLearnedValue(uint8_t slot, uint8_t height, uint8_t slat);
};
