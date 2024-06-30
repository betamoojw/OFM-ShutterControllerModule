#pragma once
#include "ModeBase.h"

class ModeIdle : public ModeBase
{
protected:
    const char *name() override;
    void initGroupObjects() override;
    bool modeWindowOpenAllowed() const override;
    bool allowed(const CallContext& callContext) override;
    void start(ModeBase* previous) override;
    void stop(ModeBase* next) override;
    void processInputKo(GroupObject &ko) override;
};