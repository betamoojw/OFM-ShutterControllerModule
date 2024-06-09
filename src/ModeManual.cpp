#include "ModeManual.h"

const char *ModeManual::name()
{
    return "Manual";
}
void ModeManual::initGroupObjects()
{

}
bool ModeManual::allowed(const CallContext& callContext)
{
    return true;
}
void ModeManual::start()
{
}
void ModeManual::stop()
{
}
void ModeManual::processInputKo(GroupObject &ko)
{
}