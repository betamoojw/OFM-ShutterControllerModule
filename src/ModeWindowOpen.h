#pragma once
#include "ModeBase.h"

class ModeWindowOpen : public ModeBase
{
    bool _recalcAllowed = true;
    bool _allowed = false;
protected:
    const char *name() override;
    void initGroupObjects() override;
    bool modeWindowOpenAllowed() const override;
    bool allowed(const CallContext& callContext) override;
    void start(ModeBase* previous) override;
    void control(const CallContext& callContext) override;
    void stop(ModeBase* next) override;
    void processInputKo(GroupObject &ko) override;
};