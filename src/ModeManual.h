#pragma once
#include "ModeBase.h"

class ModeManual : public ModeBase
{
    bool _recalcAllowed = true;
    bool _allowed = false;
    bool _shadingActive = false;
    bool _firstManualCommandWhileShading = false;
    unsigned long _waitTimeStart = 0;
    std::vector<GroupObject*> _changedGroupObjects = std::vector<GroupObject*>();
    bool _requestStart = false;
    unsigned long getWaitTimeAfterManualUsage() const;
    bool _manualControlWithActor = false;
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
    void updatePositionControllerFromKo(GroupObject &ko, PositionController& positionController);
public:
    void stopWaitTime();
};