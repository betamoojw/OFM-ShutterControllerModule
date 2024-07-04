#include "ModeManual.h"

const char *ModeManual::name() const
{
    return "Manual";
}

uint8_t ModeManual::sceneNumber() const 
{
    return 11;
}

void ModeManual::initGroupObjects()
{
    KoSHC_CHManualLockActive.value(false, DPT_Switch);
}
bool ModeManual::modeWindowOpenAllowed() const
{
    return true;
}
bool ModeManual::allowed(const CallContext &callContext)
{
    if (_recalcAllowed || callContext.diagnosticLog)
    {
        _allowed = true;
        if (KoSHC_CHManualLockActive.value(DPT_Switch))
        {
            _allowed = false;
            _waitTimeStart = 0;
            if (callContext.diagnosticLog)
                logInfoP("Lock KO active");
        }
    }
    if (_requestStart)
    {
        // Manual change was dectected between last check
        _requestStart = false;
        return _allowed;
    }
    _requestStart = false;
    _changedGroupObjects.clear();
    if (_waitTimeStart != 0)
    {
        if (callContext.currentMillis - _waitTimeStart < 10 * 60 * 1000)
            return _allowed; // In wait time
        _waitTimeStart = 0;
    }
    return false;
}
void ModeManual::start(const CallContext& callContext, const ModeBase *previous)
{
    KoSHC_CHManuelActiv.value(true, DPT_Switch);
}
void ModeManual::control(const CallContext &callContext)
{
    if (_changedGroupObjects.empty())
        return;

    _waitTimeStart = callContext.currentMillis; // Start wait time
    auto mode = ParamSHC_ChannelUpDownTypeForManualControl;
    // <Enumeration Text="Manuelle Bedienung über Aktor" Value="0" Id="%ENID%" />
    if (mode != 0)
    {
        for (auto ko : _changedGroupObjects)
        {
            switch (ko->asap())
            {
            case SHC_KoCHManualPercent:
                KoSHC_CHShutterPercentOutput.value(ko->value(DPT_Scaling), DPT_Scaling);
                break;
            case SHC_KoCHManualStepStop:
                KoSHC_CHShutterStopStepOutput.value(ko->value(DPT_Step), DPT_Step);
                break;
            case SHC_KoCHManualUpDown:
                // <Enumeration Text="Modul sendet Auf/Ab zum Aktor" Value="1" Id="%ENID%" />
                // <Enumeration Text="Modul sendet 0/100% zum Aktor " Value="2" Id="%ENID%" />
                if (mode == 1)
                {
                    KoSHC_CHShutterUpDownOutput.value(ko->value(DPT_UpDown), DPT_UpDown);
                }
                else
                {
                    if (ko->value(DPT_UpDown))
                    {
                        // Up
                        KoSHC_CHShutterPercentOutput.value(0, DPT_Scaling);
                        KoSHC_CHShutterSlatOutput.value(0, DPT_Scaling);
                    }
                    else
                    {
                        // Down
                        KoSHC_CHShutterPercentOutput.value(100, DPT_Scaling);
                        KoSHC_CHShutterSlatOutput.value(100, DPT_Scaling);
                    }
                }
                break;
            case SHC_KoCHManualSlatPercent:
                KoSHC_CHShutterSlatOutput.value(ko->value(DPT_Scaling), DPT_Scaling);
                break;
            default:
                break;
            }
        }
    }
    _changedGroupObjects.clear();
}
void ModeManual::stop(const CallContext& callContext, const ModeBase *next)
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

    switch (ko.asap())
    {
    case SHC_KoCHManualPercent:
    case SHC_KoCHManualStepStop:
    case SHC_KoCHManualUpDown:
    case SHC_KoCHManualSlatPercent:
        _changedGroupObjects.push_back(&ko);
        _requestStart = true;
        break;
    }
}