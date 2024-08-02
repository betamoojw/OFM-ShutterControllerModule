#include "ModeManual.h"
#include "PositionController.h"

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
    KoSHC_CManualLockActive.value(false, DPT_Switch);
}
bool ModeManual::modeWindowOpenAllowed() const
{
    return true;
}
bool ModeManual::allowed(const CallContext &callContext)
{
    if (_shadingActive != callContext.modeCurrentActive->isModeShading())
    {
        _shadingActive = callContext.modeCurrentActive->isModeShading();
        if (_shadingActive)
            _firstManualCommandWhileShading = true;
        else
            _firstManualCommandWhileShading = false;
    }
    if (_recalcAllowed || callContext.diagnosticLog)
    {
        _allowed = true;
        if (KoSHC_CManualLockActive.value(DPT_Switch))
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
        if (callContext.diagnosticLog)
                logInfoP("Wait time acitve: %d", (int)(callContext.currentMillis - _waitTimeStart));

        if (callContext.currentMillis - _waitTimeStart < 10 * 60 * 1000)
            return _allowed; // In wait time
        _waitTimeStart = 0;
    }
    if (callContext.diagnosticLog)
        logInfoP("No manual change detected");
    return false;
}
void ModeManual::start(const CallContext& callContext, const ModeBase *previous, PositionController& positionController)
{
    KoSHC_CManuelActiv.value(true, DPT_Switch);
}
void ModeManual::control(const CallContext &callContext, PositionController& positionController)
{
    if (_changedGroupObjects.empty())
        return;

    _waitTimeStart = callContext.currentMillis; // Start wait time
    auto mode = ParamSHC_CManualUpDownType;
    // <Enumeration Text="Manuelle Bedienung über Aktor" Value="0" Id="%ENID%" />
    if (mode != 0)
    {
        for (auto ko : _changedGroupObjects)
        {
            switch (ko->asap())
            {
            case SHC_KoCManualPercent:
                positionController.setManualPosition(ko->value(DPT_Scaling));
                break;
            case SHC_KoCManualStepStop:
                positionController.setManualStep(ko->value(DPT_Step));
                break;
            case SHC_KoCManualUpDown:
                positionController.setManualUpDown(ko->value(DPT_UpDown));
                break;
            case SHC_KoCManualSlatPercent:
                positionController.setManualSlat(ko->value(DPT_Scaling));
                break;
            default:
                break;
            }
        }
    }
    _changedGroupObjects.clear();
}
void ModeManual::stop(const CallContext& callContext, const ModeBase *next, PositionController& positionController)
{
    KoSHC_CManuelActiv.value(false, DPT_Switch);
}
void ModeManual::processInputKo(GroupObject &ko)
{
    auto asap = ko.asap();
    if (asap == SHC_KoCManualLock)
    {
        KoSHC_CManualLockActive.value(ko.value(DPT_Switch), DPT_Switch);
        _recalcAllowed = true;
    }
    if (KoSHC_CManualLockActive.value(DPT_Switch))
        return; // lock active, ignore manual KO's

    if (_firstManualCommandWhileShading)
    {
        _firstManualCommandWhileShading = false;
        if (ParamSHC_CIgnoreFirstManualCommandIfShadingActiv)
            return;
    }
    switch (SHC_KoCalcIndex(asap))
    {
    case SHC_KoCManuelStopStart:
        if (!ko.value(DPT_Switch))
        {
            logDebugP("Manual stop");
            _waitTimeStart = 0; // Stop manual mode immeditaly
            return;
        }
        logDebugP("Manual start");
         _changedGroupObjects.push_back(&ko);
        _requestStart = true;
        break;
    case SHC_KoCManualStepStop:
        logDebugP("Manual change: %s",(bool) ko.value(DPT_Switch) ? "increase" : "decrease");
        _changedGroupObjects.push_back(&ko);
        _requestStart = true;
        break;
    case SHC_KoCManualUpDown:
        logDebugP("Manual change: %s",(bool) ko.value(DPT_Switch) ? "down" : "up");
        _changedGroupObjects.push_back(&ko);
        _requestStart = true;
        break;
    case SHC_KoCManualPercent:
        logDebugP("Manual position: %d%%", (uint8_t) ko.value(DPT_Scaling));
        break;
    case SHC_KoCManualSlatPercent:
        logDebugP("Manual slat: %d%%", (uint8_t) ko.value(DPT_Scaling));
        break;
    default:
        return;
    }
    _changedGroupObjects.push_back(&ko);
    _requestStart = true;

}