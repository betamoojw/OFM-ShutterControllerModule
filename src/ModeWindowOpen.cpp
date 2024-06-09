#include "ModeWindowOpen.h"

const char *ModeWindowOpen::name()
{
    return "WindowOpen";
}
void ModeWindowOpen::initGroupObjects()
{

}
bool ModeWindowOpen::allowed(const CallContext& callContext)
{
    return false;
}
void ModeWindowOpen::start()
{
}
void ModeWindowOpen::stop()
{
}
void ModeWindowOpen::processInputKo(GroupObject &ko)
{
}