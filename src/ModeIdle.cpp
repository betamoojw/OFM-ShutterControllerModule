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
void ModeIdle::start(const CallContext& callContext, const ModeBase* previous, PositionController& positionController)
{
    // For the first start of idle, previous will be null
}
void ModeIdle::control(const CallContext &callContext, PositionController& positionController)
{
}
void ModeIdle::stop(const CallContext& callContext, const ModeBase* next, PositionController& positionController)
{
}
void ModeIdle::processInputKo(GroupObject &ko, PositionController& positionController)
{
}