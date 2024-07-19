#pragma once
#include "ModeBase.h"

class ModeWindowOpen : public ModeBase
{
    bool _recalcAllowed = true;
    bool _allowed = false;
    int8_t _slatToRestore = -1;
    int8_t _positionToRestore = -1;
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