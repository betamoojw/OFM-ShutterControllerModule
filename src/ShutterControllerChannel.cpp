#include "ShutterControllerChannel.h"
#include "ModeManual.h"
#include "ModeWindowOpen.h"
#include "ModeNight.h"
#include "ModeShading.h"
#include "ModeIdle.h"
#include "ShutterSimulation.h"

ShutterControllerChannel::ShutterControllerChannel(uint8_t channelIndex)
    : _modes(), _positionController(channelIndex)
{
    _name = "SC";
}

const std::string ShutterControllerChannel::name()
{
    return _name;
}

void ShutterControllerChannel::setup()
{
    _measurementRoomTemperature.init(
        "RoomTemperature",
        ParamSHC_CRoomTemp ? &KoSHC_CRoomTemp : nullptr,
        ParamSHC_CRoomTempWatchdog,
        KNXValue(ParamSHC_CRoomTempFallback),
        DPT_Value_Temp,
        (MeasurementWatchdogFallbackBehavior)ParamSHC_CRoomTempWatchdogBehavior);

    // <Enumeration Text="Nein" Value="0" Id="%ENID%" />
    // <Enumeration Text="Stellwert %" Value="1" Id="%ENID%" />
    // <Enumeration Text="Heizungsanforderung (EIN/AUS)" Value="2" Id="%ENID%" />
    _measurementHeading.init(
        "Heading",
        ParamSHC_CHeatingInput > 0 ? &KoSHC_CHeading : nullptr,
        ParamSHC_CHeatingWatchdog,
        ParamSHC_CHeatingInput == 1 ? KNXValue((uint8_t)ParamSHC_CHeadingFallbackPercent) : KNXValue(ParamSHC_CHeadingFallback),
        ParamSHC_CHeatingInput == 1 ? DPT_Scaling : DPT_Switch,
        (MeasurementWatchdogFallbackBehavior)ParamSHC_CHeatingWatchdogBehavior);

    KoSHC_CShutterPercentInput.requestObjectRead();
    if (ParamSHC_CType == 1)
        KoSHC_CShutterSlatInput.requestObjectRead();

    KoSHC_CShadingControl.value(shadingControlActive(), DPT_Switch);
    KoSHC_CShadingControlActive.value(shadingControlActive(), DPT_Switch);
    KoSHC_CShadingActive.value(false, DPT_Switch);

    KoSHC_CLock.value(_channelLockActive, DPT_Switch);
    KoSHC_CLockActive.value(_channelLockActive, DPT_Switch);

    // add modes, highest priority first
    _modeManual = new ModeManual();
    logErrorP("Slat1: %d", ParamSHC_CWindowOpenSlatPositionControl1);
    logErrorP("Slat2: %d", ParamSHC_CWindowOpenSlatPositionControl2);
    for (uint8_t i = 1; i <= ParamSHC_CWindowOpenCount; i++)
    {
        _modes.push_back(new ModeWindowOpen(i));
    }

    _modes.push_back(_modeManual);

    if (ParamSHC_CNight)
        _modes.push_back(new ModeNight());

    auto numberOfShadings = ParamSHC_CShadingCount;
    for (size_t i = numberOfShadings; i > 0; i--)
    {
        _modes.push_back(new ModeShading(i));
    }

    _modeIdle = new ModeIdle();
    _modes.push_back(_modeIdle);
    for (auto mode : _modes)
    {
        mode->setup(_channelIndex);
    }
#ifdef SIMULATION
    _positionController.startSimulation(false);
#endif
}

void ShutterControllerChannel::processInputKo(GroupObject &ko)
{
    _measurementHeading.processIputKo(ko);
    _measurementRoomTemperature.processIputKo(ko);

    // channel ko
    auto index = SHC_KoCalcIndex(ko.asap());
    switch (index)
    {
    case SHC_KoCLock:
        _channelLockActive = ko.value(DPT_Switch);
        KoSHC_CLockActive.value(_channelLockActive, DPT_Switch);
        break;
    case SHC_KoCShadingControl:
        shadingControlActive(ko.value(DPT_Switch));
        if (shadingControlActive() && _currentMode == _modeManual)
        {
            _modeManual->stopWaitTime();
        }
        break;
    }
    _positionController.processInputKo(ko);
    for (auto mode : _modes)
    {
        mode->processInputKo(ko, _positionController);
    }
}

bool ShutterControllerChannel::processCommand(const std::string cmd, bool diagnoseKo, bool &diagnosticLogLoopRequest)
{
    if (cmd == "")
    {
        if (_currentMode == nullptr)
            logInfoP("Not started");
        else
        {
            logInfoP("Current active mode: %s", _currentMode->name());
            diagnosticLogLoopRequest = true;
        }
        return true;
    }
    else if (cmd.rfind("sim") == 0)
    {
        if (cmd.length() > 3)
        {
            switch (std::stoi(cmd.substr(3)))
            {
            case 0:
                _positionController.stopSimulation();
                break;
            case 1:
                _positionController.startSimulation(false);
                break;
            case 2:
                _positionController.startSimulation(true);
                break;
            }
        }
        switch (_positionController.simulationMode())
        {
        case 0:
            logInfoP("Simulation not started");
            break;
        case 1:
            logInfoP("Simulation started");
            break;
        case 2:
            logInfoP("Fast simulation started");
            break;
        }
        return true;
    }
    else if (cmd.rfind("s") == 0)
    {
        if (cmd.length() == 1)
        {
            logErrorP("Missing value 0 or 1");
            return true;
        }
        KoSHC_CShadingControl.value((uint8_t)std::stoi(cmd.substr(1)), DPT_Switch);
        processInputKo(KoSHC_CShadingControl);
        return true;
    }
    else if (cmd.rfind("t") == 0)
    {
        if (cmd.length() == 1)
        {
            logErrorP("Missing value 0 or 1");
            return true;
        }
        KoSHC_CRoomTemp.valueNoSend((uint8_t)std::stoi(cmd.substr(1)), DPT_Value_Temp);
        processInputKo(KoSHC_CloudsInput);
        return true;
    }
    else if (cmd.rfind("h") == 0)
    {
        if (cmd.length() == 1)
        {
            logErrorP("Missing value");
            return true;
        }
        KoSHC_CRoomTemp.valueNoSend((float)std::stof(cmd.substr(1)), ParamSHC_CHeatingInput == 1 ? DPT_Scaling : DPT_Switch);
        processInputKo(KoSHC_CRoomTemp);
        return true;
    }
    else if (cmd == "m^")
    {
        KoSHC_CManualUpDown.valueNoSend(0, DPT_UpDown);
        processInputKo(KoSHC_CManualUpDown);
        return true;
    }
    else if (cmd == "mv")
    {
        KoSHC_CManualUpDown.valueNoSend(1, DPT_UpDown);
        processInputKo(KoSHC_CManualUpDown);
        return true;
    }
    else if (cmd == "m-")
    {
        KoSHC_CManualStepStop.valueNoSend(0, DPT_UpDown);
        processInputKo(KoSHC_CManualStepStop);
        return true;
    }
    else if (cmd == "m+")
    {
        KoSHC_CManualStepStop.valueNoSend(1, DPT_UpDown);
        processInputKo(KoSHC_CManualStepStop);
        return true;
    }
    else if (cmd.rfind("ms") == 0 && _positionController.hasSlat())
    {
        if (cmd.length() == 2)
        {
            logErrorP("Missing value");
            return true;
        }
        KoSHC_CManualSlatPercent.valueNoSend((uint8_t)std::stoi(cmd.substr(2)), DPT_Scaling);
        processInputKo(KoSHC_CManualSlatPercent);
        return true;
    }
    else if (cmd.rfind("m") == 0)
    {
        if (cmd.length() == 1)
        {
            logErrorP("Missing value");
            return true;
        }
        KoSHC_CManualPercent.valueNoSend((uint8_t)std::stoi(cmd.substr(1)), DPT_Scaling);
        processInputKo(KoSHC_CManualPercent);
        return true;
    }
#ifdef KoSHC_CWindowOpenOpened2
    else if (cmd.rfind("wt") == 0)
    {
        if (cmd.length() == 2)
        {
            logErrorP("Missing value 0 or 1");
            return true;
        }
        KoSHC_CWindowOpenOpened2.valueNoSend((uint8_t)std::stoi(cmd.substr(2)), DPT_OpenClose);
        processInputKo(KoSHC_CWindowOpenOpened2);
        return true;
    }
#endif
    else if (cmd.rfind("w") == 0)
    {
        if (cmd.length() == 1)
        {
            logErrorP("Missing value 0 or 1");
            return true;
        }
        KoSHC_CWindowOpenOpened1.valueNoSend((uint8_t)std::stoi(cmd.substr(1)), DPT_OpenClose);
        processInputKo(KoSHC_CWindowOpenOpened1);
        return true;
    }

    return false;
}
void ShutterControllerChannel::activateShading()
{
    KoSHC_CShadingControl.value(true, DPT_Switch);
    processInputKo(KoSHC_CShadingControl);
}
void ShutterControllerChannel::execute(CallContext &callContext)
{
    _measurementHeading.update(callContext.currentMillis, callContext.diagnosticLog);
    _measurementRoomTemperature.update(callContext.currentMillis, callContext.diagnosticLog);

    ModeBase *nextMode = nullptr;
    callContext.positionController = &_positionController;
    callContext.fastSimulationActive = _positionController.simulationMode() == 2;
    callContext.hasSlat = _positionController.hasSlat();
    callContext.modeNewStarted = false;
    callContext.modeIdle = _modeIdle;
    callContext.modeManual = _modeManual;
    callContext.measurementHeading = &_measurementHeading;
    callContext.measurementRoomTemperature = &_measurementRoomTemperature;
    callContext.channelLockActive = _channelLockActive;

    if (_currentMode == nullptr)
    {
        _currentMode = _modeIdle;
        _currentMode->start(callContext, callContext.modeCurrentActive, _positionController);
        KoSHC_CActiveMode.value((uint8_t)(_currentMode->sceneNumber() - 1), DPT_SceneNumber);
    }
    callContext.modeCurrentActive = _currentMode;

    // Handle reacticvate of shading after manual usage
    if (_waitTimeForReactivateShadingAfterManualStarted != 0)
    {
        if (callContext.currentMillis - _waitTimeForReactivateShadingAfterManualStarted > callContext.fastSimulationActive ? ParamSHC_CManualShadingWaitTime / 10 : ParamSHC_CManualShadingWaitTime)
        {
            // reactivate
            _waitTimeForReactivateShadingAfterManualStarted = 0;
            shadingControlActive(KoSHC_CShadingControl.value(DPT_Switch));
        }
    }
    else if (_waitForShadingPeriodEnd)
    {
        bool allShadingPeriodsEnd = true;
        for (auto mode : _modes)
        {
            if (mode->isModeWindowOpen() && _handleWindowOpenAsShading != nullptr)
                mode = _handleWindowOpenAsShading;
            if (mode->isModeShading())
            {
                auto modeShading = (ModeShading *)mode;
                if (modeShading->allowedBySun(callContext))
                {
                    allShadingPeriodsEnd = false;
                    break;
                }
            }
        }
        if (allShadingPeriodsEnd)
        {
            _waitForShadingPeriodEnd = false;
            shadingControlActive(KoSHC_CShadingControl.value(DPT_Switch));
        }
    }
    callContext.shadingControlActive = shadingControlActive();
    // State machine handling for mode activateion
    bool currentModeAllowed = false;
    for (auto mode : _modes)
    {
        if (callContext.diagnosticLog)
            logInfoP("Mode %s", mode->name());

        logIndentUp();
        if (mode->allowed(callContext))
        {
            if (mode == _currentMode)
                currentModeAllowed = true;
            if (_channelLockActive && (mode != _modeManual || !ParamSHC_CManualIgnoreChannelLock))
            {
                if (callContext.diagnosticLog)
                    logInfoP("-> global channel lock active, not allowed");
            }
            else
            {
                if (callContext.diagnosticLog)
                    logInfoP("-> allowed");
                if (nextMode == nullptr) // check if this is the first allowed mode
                {
                    if (!shadingControlActive() && mode->isModeShading())
                    {
                        if (callContext.diagnosticLog)
                            logInfoP("-> but ignored, because global shadow not alloewd");
                    }
                    else
                        nextMode = mode;
                    // Do not break because allowed should be called for all modes
                    // because it is a replacment for the loop function
                }
                else
                {
                    if (callContext.diagnosticLog)
                        logInfoP("-> but ignored because other mode has higher priority");
                }
            }
        }
        else
        {
            if (callContext.diagnosticLog)
                logInfoP("-> not allowed");
        }
        logIndentDown();
    }
    if (_currentMode != nextMode)
    {
        if (nextMode == _modeManual)
        {
            // If manual mode stops shading, temporary disable Shadingcontrol
            if (_currentMode->isModeShading() || (_currentMode->isModeWindowOpen() && _handleWindowOpenAsShading != nullptr))
            {
                shadingControlActive(false);
                if (ParamSHC_CManualShadingWaitTime != 0)
                    _waitTimeForReactivateShadingAfterManualStarted = callContext.currentMillis;
                else
                    _waitForShadingPeriodEnd = true;
            }
        }
        bool nextModeIsAutoMode = nextMode != _modeIdle && nextMode != _modeManual;
        if (nextModeIsAutoMode != _anyAutoModeActive)
        {
            _anyAutoModeActive = nextModeIsAutoMode;
        }

        if (_currentMode != nullptr)
        {
            logInfoP("Changing mode from %s to %s", _currentMode->name(), nextMode->name());
            _handleWindowOpenAsShading = nullptr;
            _currentMode->stop(callContext, nextMode, _positionController);
            if (!nextMode->isModeShading() && anyShadingModeActive())
            {
                if (nextMode->isModeWindowOpen() && currentModeAllowed)
                {
                    // shading still allowed, handle windowOpen as shading mode
                    _handleWindowOpenAsShading = (ModeShading *)_currentMode;
                }
                else
                {
                    anyShadingModeActive(false);
                }
            }
        }
        else
        {
            logDebugP("Start mode %s", nextMode->name());
        }
        auto previousMode = _currentMode;
        _currentMode = nextMode;
        if (_currentMode->isModeShading() && !anyShadingModeActive())
        {
            anyShadingModeActive(true);
        }
        _currentMode->start(callContext, previousMode, _positionController);
        KoSHC_CActiveMode.value((uint8_t)(_currentMode->sceneNumber() - 1), DPT_SceneNumber);
        callContext.modeNewStarted = true;
    }
    _currentMode->control(callContext, _positionController);
    _positionController.control(callContext);

    callContext.modeIdle = nullptr;
    callContext.modeManual = nullptr;
    callContext.modeCurrentActive = nullptr;
    _measurementHeading.resetChanged();
    _measurementRoomTemperature.resetChanged();
}

bool ShutterControllerChannel::shadingControlActive()
{
    return __shadingControlActive;
}

void ShutterControllerChannel::shadingControlActive(bool active)
{
    if (active)
    {
        logInfoP("Shading control started");
        __shadingControlActive = true;
        KoSHC_CShadingControlActive.value(true, DPT_Switch);
        _waitTimeForReactivateShadingAfterManualStarted = 0;
        _waitForShadingPeriodEnd = false;
    }
    else
    {
        logInfoP("Shading control stopped");
        __shadingControlActive = false;
        KoSHC_CShadingControlActive.value(false, DPT_Switch);
    }
}

bool ShutterControllerChannel::anyShadingModeActive()
{
    return __anyShadingModeActive;
}

void ShutterControllerChannel::anyShadingModeActive(bool active)
{
    if (active)
    {
        logInfoP("Shading started");
        __anyShadingModeActive = true;
        KoSHC_CShadingActive.value(true, DPT_Switch);
    }
    else
    {
        logInfoP("Shading stopped");
        __anyShadingModeActive = false;
        KoSHC_CShadingActive.value(false, DPT_Switch);
        // <Enumeration Text="Keine Änderung" Value="0" Id="%ENID%" />
        // <Enumeration Text="Position vor Beschattungsstart" Value="1" Id="%ENID%" />
        // <Enumeration Text="Fährt Auf" Value="2" Id="%ENID%" />
        // <Enumeration Text="Lamelle Waagrecht" Value="3" Id="%ENID%" />
        switch (ParamSHC_CAfterShading)
        {
        case 1:
            _positionController.restoreLastManualPosition();
            break;
        case 2:                                             // Fährt auf
            _positionController.setManualPosition(0, true); // Handled as manual operation because the value should be stored
            _positionController.setManualSlat(0, true);     // Handled as manual operation because the value should be stored
            break;
        case 3:
            _positionController.setManualSlat(50, true); // Handled as manual operation because the value should be stored
            break;
        }
    }
}
