#include "ModeWindowOpen.h"

const char *ModeWindowOpen::name()
{
    return "WindowOpen";
}
void ModeWindowOpen::initGroupObjects()
{
    KoSHC_CHWindowOpenModeActive.value(false, DPT_Switch);
    KoSHC_CHWindowOpenLockActive.value(false, DPT_Switch);
}
bool ModeWindowOpen::allowed(const CallContext& callContext)
{
    if (_recalcAllowed)
    {
        _allowed = KoSHC_CHWindow.value(DPT_OpenClose) && KoSHC_CHWindowOpenLockActive.value(DPT_Switch);
    }
    return _allowed;
}
void ModeWindowOpen::start()
{
    KoSHC_CHWindowOpenModeActive.value(true, DPT_Switch);
}
void ModeWindowOpen::stop()
{
    KoSHC_CHWindowOpenModeActive.value(false, DPT_Switch);
}
void ModeWindowOpen::processInputKo(GroupObject &ko)
{
    switch (ko.asap())
    {
        case SHC_KoCHWindow:
        case SHC_KoCHWindowOpenLock:
        _recalcAllowed = true;
        break;
    }
}