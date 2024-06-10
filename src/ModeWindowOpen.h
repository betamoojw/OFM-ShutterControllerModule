#pragma once
#include "ModeBase.h"

class ModeWindowOpen : public ModeBase
{
    bool _recalcAllowed = true;
    bool _allowed = false;
protected:
    const char *name() override;
    void initGroupObjects() override;
    bool allowed(const CallContext& callContext) override;
    void start() override;
    void stop() override;
    void processInputKo(GroupObject &ko) override;
};