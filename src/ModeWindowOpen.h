#pragma once
#include "ModeBase.h"

class ModeWindowOpen : public ModeBase
{
    bool _recalcAllowed = true;
    bool _allowed = false;
protected:
    const char *name() const override;
    uint8_t sceneNumber() const override;
    void initGroupObjects() override;
    bool modeWindowOpenAllowed() const override;
    bool allowed(const CallContext& callContext) override;
    void start(const CallContext& callContext, const ModeBase* previous) override;
    void control(const CallContext& callContext) override;
    void stop(const CallContext& callContext, const ModeBase* next) override;
    void processInputKo(GroupObject &ko) override;
};