#include "ModeManual.h"

const char *ModeManual::name()
{
    return "Manual";
}
void ModeManual::initGroupObjects()
{
    KoSHC_CHManualLockActive.value(false, DPT_Switch);
}
bool ModeManual::allowed(const CallContext &callContext)
{
    if (_recalcAllowed || callContext.diagnosticLog)
    {
        _allowed = true;
        if (KoSHC_CHManualLockActive.value(DPT_Switch))
        {
            _allowed = false;
            _waitTime = 0;
            if (callContext.diagnosticLog)
                logInfoP("Lock KO active");
        }
    }
    if (_waitTime == 0)
        return false;
    if (callContext.currentMillis - _waitTime < 10 * 60 * 1000)
        return _allowed;
    _waitTime = 0;
    return _allowed;
}
void ModeManual::start(ModeBase* previous)
{
    KoSHC_CHManuelActiv.value(true, DPT_Switch);
}
void ModeManual::stop(ModeBase* next)
{
    KoSHC_CHManuelActiv.value(false, DPT_Switch);
}
void ModeManual::processInputKo(GroupObject &ko)
{
    auto asap = ko.asap();
    if (asap == SHC_KoCHManualLock)
    {
        KoSHC_CHManualLockActive.value(ko.value(DPT_Switch), DPT_Switch);
        _recalcAllowed = true;
    }
    if (KoSHC_CHManualLockActive.value(DPT_Switch))
        return; // lock active, ignore manual KO's
    switch (asap)
    {
    case SHC_KoCHManualPercent:
        break;
    case SHC_KoCHManualStepStop:
        break;
    case SHC_KoCHManualUpDown:
        break;
    case SHC_KoCHManualSlatPercent:
        break;
    default:
        break;
    }
}