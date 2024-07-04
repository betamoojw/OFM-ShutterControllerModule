#include "ModeIdle.h"

const char *ModeIdle::name() const
{
    return "Idle";
}

uint8_t ModeIdle::sceneNumber() const 
{
    return 10;
}

void ModeIdle::initGroupObjects()
{
}
bool ModeIdle::modeWindowOpenAllowed() const
{
    return true;
}
bool ModeIdle::allowed(const CallContext &callContext)
{
   return true;
}
void ModeIdle::start(const CallContext& callContext, const ModeBase* previous)
{
}
void ModeIdle::control(const CallContext &callContext)
{
}
void ModeIdle::stop(const CallContext& callContext, const ModeBase* next)
{
}
void ModeIdle::processInputKo(GroupObject &ko)
{
}