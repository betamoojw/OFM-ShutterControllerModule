#include "ModeManual.h"
#include "PositionController.h"
#include "ShutterControllerChannel.h"

ModeManual::ModeManual(ShutterControllerChannel &channel) : _channel(channel)
{
}

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
    _manualControlWithActor = ParamSHC_CManualUpDownType == 0;
    if (!_manualControlWithActor)
        KoSHC_CManualLockActive.value(false, DPT_Switch);
}
bool ModeManual::windowOpenAllowed() const
{
    return true;
}
bool ModeManual::windowTiltAllowed() const
{
    return true;
}

unsigned long ModeManual::getWaitTimeAfterManualUsage() const
{
    // <Enumeration Text="Keine Wartezeit" Value="0" Id="%ENID%" />
    // <Enumeration Text="1 Minute" Value="1" Id="%ENID%" />
    // <Enumeration Text="2 Minuten" Value="2" Id="%ENID%" />
    // <Enumeration Text="5 Minuten" Value="5" Id="%ENID%" />
    // <Enumeration Text="10 Minuten" Value="10" Id="%ENID%" />
    // <Enumeration Text="15 Minuten" Value="15" Id="%ENID%" />
    // <Enumeration Text="30 Minuten" Value="30" Id="%ENID%" />
    // <Enumeration Text="1 Stunde" Value="101" Id="%ENID%" />
    // <Enumeration Text="2 Stunden" Value="102" Id="%ENID%" />
    // <Enumeration Text="3 Stunden" Value="103" Id="%ENID%" />
    // <Enumeration Text="4 Stunden" Value="104" Id="%ENID%" />
    // <Enumeration Text="5 Stunden" Value="105" Id="%ENID%" />
    // <Enumeration Text="6 Stunden" Value="106" Id="%ENID%" />
    // <Enumeration Text="7 Stunden" Value="107" Id="%ENID%" />
    // <Enumeration Text="8 Stunden" Value="108" Id="%ENID%" />
    // <Enumeration Text="9 Stunden" Value="109" Id="%ENID%" />
    // <Enumeration Text="10 Stunden" Value="110" Id="%ENID%" />
    // <Enumeration Text="11 Stunden" Value="111" Id="%ENID%" />
    // <Enumeration Text="12 Stunden" Value="112" Id="%ENID%" />
    auto value = ParamSHC_CManualWaitTime;
    if (value < 100)
        return value * 60 * 1000;
    return (value - 100) * 60 * 60 * 1000;
}

bool ModeManual::allowed(const CallContext &callContext)
{
    if (_shadingActive != callContext.modeCurrentActive->isModeShading())
    {
        _shadingActive = callContext.modeCurrentActive->isModeShading();
        if (_shadingActive)
            _ignoreFirstUpWhileShading = ParamSHC_CIgnoreFirstManualCommandIfShadingActiv;
        else
            _ignoreFirstUpWhileShading = false;
    }
    if (callContext.diagnosticLog && _ignoreFirstUpWhileShading)
        logInfoP("First UP might be ignored");
    if (_recalcAllowed || callContext.diagnosticLog)
    {
        _allowed = true;
        if (!_manualControlWithActor && KoSHC_CManualLockActive.value(DPT_Switch))
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
    _forceClose = false;
    _requestStart = false;
    _changedGroupObjects.clear();
    if (_waitTimeStart != 0)
    {
        auto timeout = getWaitTimeAfterManualUsage();
        if (callContext.fastSimulationActive)
            timeout /= 10;
        if (callContext.diagnosticLog)
            logInfoP("Wait time %lus acitve. Waiting since: %lus", (unsigned long)timeout / 1000, (unsigned long)((callContext.currentMillis - _waitTimeStart) / 1000));

        if (callContext.currentMillis - _waitTimeStart < timeout)
            return _allowed; // In wait time
        _waitTimeStart = 0;
    }
    if (callContext.diagnosticLog)
        logInfoP("No manual change detected");
    return false;
}
void ModeManual::stopWaitTime()
{
    if (_waitTimeStart == 0)
        return;
    logInfoP("Stopping wait time");
    _waitTimeStart = 0;
}

void ModeManual::start(const CallContext &callContext, const ModeBase *previous, PositionController &positionController)
{
    KoSHC_CManuelActiv.value(true, DPT_Switch);
    positionController.resetBlockedPositionAndLimits();
}
void ModeManual::control(const CallContext &callContext, PositionController &positionController)
{
    if (_forceClose)
    {
        positionController.setAutomaticPositionAndStoreForRestore(100);
        positionController.setAutomaticSlatAndStoreForRestore(100);
        _forceClose = false;
    }
    if (!_changedGroupObjects.empty())
    {
        for (auto ko : _changedGroupObjects)
        {
            updatePositionControllerFromKo(*ko, positionController);
        }
        _changedGroupObjects.clear();
    }
}
void ModeManual::updatePositionControllerFromKo(GroupObject &ko, PositionController &positionController)
{
    _waitTimeStart = max(millis(), 1uL); // Start wait time

    switch (ko.asap())
    {
    case SHC_KoCManualPercent:
        positionController.setManualPosition(ko.value(DPT_Scaling));
        break;
    case SHC_KoCManualStepStop:
        positionController.setManualStep(ko.value(DPT_Step));
        break;
    case SHC_KoCManualUpDown:
        positionController.setManualUpDown(ko.value(DPT_UpDown));
        break;
    case SHC_KoCManualSlatPercent:
        positionController.setManualSlat(ko.value(DPT_Scaling));
        break;
    default:
        break;
    }
}
void ModeManual::stop(const CallContext &callContext, const ModeBase *next, PositionController &positionController)
{
    _waitTimeStart = 0;
    KoSHC_CManuelActiv.value(false, DPT_Switch);
}
void ModeManual::processInputKo(GroupObject &ko, PositionController &positionController)
{
    auto koIndex = SHC_KoCalcIndex(ko.asap());

    // special key handling if opened
    if (positionController.state() == PositionControllerState::Idle && positionController.position() == 0)
    {
        uint8_t specialHandling = 0;
        switch (koIndex)
        {
        case SHC_KoCManualStepStop:
            if ((bool)ko.value(DPT_Step) == false)
                specialHandling = ParamSHC_CShortKeyPressUpIfOpen;
            break;
        case SHC_KoCManualUpDown:
            if ((bool)ko.value(DPT_Step) == false)
                specialHandling = ParamSHC_CLongKeyPressUpIfOpen;
            break;
        }
        // <Enumeration Text="Deaktiviert" Value="0" Id="%ENID%" />
        // <Enumeration Text="Beschattungsautomatik EIN/AUS" Value="1" Id="%ENID%" />
        // <Enumeration Text="Jalousie schließen" Value="2" Id="%ENID%" />
        switch (specialHandling)
        {
        case 1:
        {

            auto newValue = !_channel.shadingControlActive();
            logInfoP("Special key handling: Toggle shading to %s", newValue ? "ON" : "OFF");
            _channel.activateShadingControl(newValue);
            return;
        }
        case 2:
        {
            logInfoP("Special key handling: Close shading");
            _forceClose = true;
            _requestStart = true;
            return;
        }
        }
    }

    if (koIndex == SHC_KoCManualLock && !_manualControlWithActor)
    {
        KoSHC_CManualLockActive.value(ko.value(DPT_Switch), DPT_Switch);
        _recalcAllowed = true;
    }
    switch (koIndex)
    {
    case SHC_KoCManuelStopStart:
        if (!ko.value(DPT_Switch))
        {
            logDebugP("Manual stop");
            _waitTimeStart = 0; // Stop manual mode immeditaly
            return;
        }
        logDebugP("Manual start");
        break;
    case SHC_KoCManualStepStop:
        logDebugP("Manual change: %s", (bool)ko.value(DPT_Switch) ? "increase" : "decrease");
        break;
    case SHC_KoCManualUpDown:
        if (_ignoreFirstUpWhileShading)
        {
            _ignoreFirstUpWhileShading = false;
            bool up = !ko.value(DPT_Switch);
            if (up && positionController.getRestorePosition() >= positionController.position())
            {
                logInfoP("Ignore first manual uo while shading active");
                positionController.setRestorePosition(0);
                positionController.setRestoreSlat(0);
                // <Enumeration Text="Manuelle Bedienung über Aktor" Value="0" Id="%ENID%" />
                if (_manualControlWithActor)
                    KoSHC_CShutterStopStepOutput.value(true, DPT_Switch);
                }
            return;
        }
        logDebugP("Manual change: %s", (bool)ko.value(DPT_Switch) ? "down" : "up");
        break;
    case SHC_KoCManualPercent:
        logDebugP("Manual position: %d%%", (uint8_t)ko.value(DPT_Scaling));
        break;
    case SHC_KoCManualSlatPercent:
        logDebugP("Manual slat: %d%%", (uint8_t)ko.value(DPT_Scaling));
        break;
    default:
        return;
    }
    if (_manualControlWithActor)
    {
        updatePositionControllerFromKo(ko, positionController);
    }
    else
    {
        _changedGroupObjects.push_back(&ko);
    }
    _requestStart = true;
}
