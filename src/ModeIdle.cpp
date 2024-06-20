#include "ModeIdle.h"

const char *ModeIdle::name()
{
    return "Idle";
}
void ModeIdle::initGroupObjects()
{
}
bool ModeIdle::allowed(const CallContext &callContext)
{
   return true;
}
void ModeIdle::start()
{
}
void ModeIdle::stop()
{
}
void ModeIdle::processInputKo(GroupObject &ko)
{
}