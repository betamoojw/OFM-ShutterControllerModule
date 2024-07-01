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
bool ModeWindowOpen::modeWindowOpenAllowed() const
{
    return true;
}
bool ModeWindowOpen::allowed(const CallContext& callContext)
{
    if (!callContext.modeCurrentActive->modeWindowOpenAllowed())
    {
        if (callContext.diagnosticLog)
            logInfoP("Window open mode not allowed by current active mode");
        return false;
    }
    if (_recalcAllowed || callContext.diagnosticLog)
    {
        _allowed = true;
        if (!KoSHC_CHWindow.value(DPT_OpenClose))
        {
            _allowed = false;
            if (callContext.diagnosticLog)
                logInfoP("Window closed");
        }
        if (KoSHC_CHWindowOpenLockActive.value(DPT_Switch))
        {
            _allowed = false;
            if (callContext.diagnosticLog)
                logInfoP("Lock KO active");
        }
    }
    return _allowed;
}
void ModeWindowOpen::start(ModeBase* previous)
{
    KoSHC_CHWindowOpenModeActive.value(true, DPT_Switch);
}
void ModeWindowOpen::control(const CallContext &callContext)
{
}
void ModeWindowOpen::stop(ModeBase* next)
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