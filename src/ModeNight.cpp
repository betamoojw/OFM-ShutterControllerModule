#include "ModeNight.h"

const char *ModeNight::name()
{
    return "Night";
}
void ModeNight::initGroupObjects()
{

}
bool ModeNight::allowed(unsigned long currentMillis)
{
    return false;
}
void ModeNight::start()
{
}
void ModeNight::stop()
{
}
void ModeNight::processInputKo(GroupObject &ko)
{
}