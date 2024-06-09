#include "ModeManual.h"

const char *ModeManual::name()
{
    return "Manual";
}
void ModeManual::initGroupObjects()
{

}
bool ModeManual::allowed(unsigned long currentMillis)
{
    return false;
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