#pragma once
#include "ModeBase.h"

class ModeManual : public ModeBase
{
    bool _recalcAllowed = true;
    bool _allowed = false;
    unsigned long _waitTime = 0;
protected:
    const char *name() override;
    void initGroupObjects() override;
    bool modeWindowOpenAllowed() const override;
    bool allowed(const CallContext& callContext) override;
    void start(ModeBase* previous) override;
    void stop(ModeBase* next) override;
    void processInputKo(GroupObject &ko) override;
};