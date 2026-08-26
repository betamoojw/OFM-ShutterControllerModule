#pragma once

#include "OpenKNX.h"
#include "CallContext.h"

class ShutterControlBase
{
public:
    virtual ~ShutterControlBase() = default;
    virtual void processInputKo(GroupObject& ko) = 0;
    virtual void update(const CallContext& callContext) = 0;
    virtual void loop() = 0;
};
