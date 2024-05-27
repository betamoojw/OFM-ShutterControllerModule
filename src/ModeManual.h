#pragma once
#include "ModeBase.h"

class ModeManual : public ModeBase
{
protected:
    const char *name() override;
    bool allowed() override;
    void start() override;
    void stop() override;
    void processInputKo(GroupObject &ko) override;
};