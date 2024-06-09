#pragma once
#include "ModeBase.h"

class ModeNight : public ModeBase
{
    bool _allowed = false;
    bool _recalcAllowed = true;
protected:
    const char *name() override;
    void initGroupObjects() override;
    bool allowed(const CallContext& callContext) override;
    void start() override;
    void stop() override;
    void processInputKo(GroupObject &ko) override;
  
};
