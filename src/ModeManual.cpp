#include "ModeManual.h"

const char *ModeManual::name()
{
    return "Manual";
}
void ModeManual::initGroupObjects()
{

}
bool ModeManual::allowed()
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