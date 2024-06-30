#include "ModeIdle.h"

const char *ModeIdle::name()
{
    return "Idle";
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
void ModeIdle::start(ModeBase* previous)
{
}
void ModeIdle::stop(ModeBase* next)
{
}
void ModeIdle::processInputKo(GroupObject &ko)
{
}