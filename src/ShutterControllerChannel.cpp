#include "ShutterControllerChannel.h"
#include "ModeManual.h"
#include "ModeWindowOpen.h"
#include "ModeNight.h"
#include "ModeShading.h"
#include "ModeIdle.h"

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
    KoSHC_CHShutterPercentInput.requestObjectRead();
    if (ParamSHC_ChannelType == 1)
        KoSHC_CHShutterSlatInput.requestObjectRead();

    KoSHC_CHShadingControl.value(_shadingControlActive, DPT_Switch);
    KoSHC_CHShadingControlActive.value(_shadingControlActive, DPT_Switch);
    KoSHC_CHShadingActive.value(false, DPT_Switch);

    KoSHC_CHLock.value(_channelLockActive, DPT_Switch);
    KoSHC_CHLockActive.value(_channelLockActive, DPT_Switch);

    // add modes, highest priority first
    _modeManual = new ModeManual();
    logErrorP("Slat1: %d", ParamSHC_ChannelModeWindowOpenSlatPositionControl1);
    logErrorP("Slat2: %d", ParamSHC_ChannelModeWindowOpenSlatPositionControl2);
    for (uint8_t i = 1; i <= ParamSHC_ChannelModeWindowOpenCount; i++)
    {
        _modes.push_back(new ModeWindowOpen(i));
    }
  
    _modes.push_back(_modeManual);

    if (ParamSHC_ChannelModeNight)
        _modes.push_back(new ModeNight());

// up to 4 instances can be defined of ModeShading in ShutterControllerModule.templ.xml
#ifdef ParamSHC_ChannelModeShading5
#error Not more the 4 instances allowed for ModeShading in ShutterControllerModule.templ.xml
#endif
#ifdef ParamSHC_ChannelModeShading4
    if (ParamSHC_ChannelModeShading4)
        _modes.push_back(new ModeShading(4));
#endif
#ifdef ParamSHC_ChannelModeShading3
    if (ParamSHC_ChannelModeShading3)
        _modes.push_back(new ModeShading(3));
#endif
#ifdef ParamSHC_ChannelModeShading2
    if (ParamSHC_ChannelModeShading2)
        _modes.push_back(new ModeShading(2));
#endif
    if (ParamSHC_ChannelModeShading1)
        _modes.push_back(new ModeShading(1));

    _modeIdle = new ModeIdle();
    _modes.push_back(_modeIdle);
    for (auto mode : _modes)
    {
        mode->setup(_channelIndex);
    }
}

void ShutterControllerChannel::processInputKo(GroupObject &ko)
{
    // channel ko
    auto index = SHC_KoCalcIndex(ko.asap());
    switch (index)
    {
    case SHC_KoCHLock:
        _channelLockActive = ko.value(DPT_Switch);
        KoSHC_CHLockActive.value(_channelLockActive, DPT_Switch);
        break;
     case SHC_KoCHShadingControl:
        _shadingControlActive = ko.value(DPT_Switch);
        _waitTimeForReactivateShadingAfterManualStarted = 0;
        _waitForShadingPeriodEnd = false;
        KoSHC_CHShadingControlActive.value(_shadingControlActive, DPT_Switch);
        break;
    }
    _positionController.processInputKo(ko);
    for (auto mode : _modes)
    {
        mode->processInputKo(ko);
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
    else if (cmd == "s1")
    {
        KoSHC_CHShadingControl.value(1, DPT_Switch);
        processInputKo(KoSHC_CHShadingControl);
        return true;
    }
    else if (cmd == "s0")
    {
        KoSHC_CHShadingControl.value(0, DPT_Switch);
        processInputKo(KoSHC_CHShadingControl);
        return true;
    }
    else if (cmd == "w1")
    {
        KoSHC_CHModeWindowOpenOpened1.value(1, DPT_OpenClose);
        processInputKo(KoSHC_CHModeWindowOpenOpened1);
        return true;
    }
    else if (cmd == "w0")
    {
        KoSHC_CHModeWindowOpenOpened1.value(0, DPT_OpenClose);
        processInputKo(KoSHC_CHModeWindowOpenOpened1);
        return true;
    }
#ifdef KoSHC_CHModeWindowOpenOpened2
    else if (cmd == "wt1")
    {
        KoSHC_CHModeWindowOpenOpened2.value(1, DPT_OpenClose);
        processInputKo(KoSHC_CHModeWindowOpenOpened2);
        return true;
    }
    else if (cmd == "wt0")
    {
        KoSHC_CHModeWindowOpenOpened2.value(0, DPT_OpenClose);
        processInputKo(KoSHC_CHModeWindowOpenOpened2);
        return true;
    }
#endif
    return false;
}
void ShutterControllerChannel::activateShading()
{
    KoSHC_CHShadingControl.value(true, DPT_Switch);
    processInputKo(KoSHC_CHShadingControl);
}
void ShutterControllerChannel::execute(CallContext &callContext)
{
    ModeBase *nextMode = nullptr;
    callContext.hasSlat = _positionController.hasSlat();
    callContext.modeNewStarted = false;
    callContext.modeIdle = _modeIdle;
    callContext.modeManual = _modeManual;
    if (_currentMode == nullptr)
    {
        _currentMode = _modeIdle;
        _currentMode->start(callContext, callContext.modeCurrentActive, _positionController);
        KoSHC_CHActiveMode.value(_currentMode->sceneNumber() - 1, DPT_SceneNumber);
    }
    callContext.modeCurrentActive = _currentMode;
    
    // Handle reacticvate of shading after manual usage
    if (_waitTimeForReactivateShadingAfterManualStarted != 0)
    {
        if (callContext.currentMillis - _waitTimeForReactivateShadingAfterManualStarted > ParamSHC_ChannelWaitTimeAfterManualUsageForShading)
        {
            // reactivate
            _waitTimeForReactivateShadingAfterManualStarted = 0;
            _shadingControlActive = KoSHC_CHShadingControl.value(DPT_Switch);
            KoSHC_CHShadingControlActive.value(_shadingControlActive, DPT_Switch);
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
                if (modeShading->allowedByTimeAndSun(callContext))
                {
                    allShadingPeriodsEnd = false;
                    break;
                }
            }
        }
        if (allShadingPeriodsEnd)
        {
            _waitForShadingPeriodEnd = false;
            _shadingControlActive = KoSHC_CHShadingControl.value(DPT_Switch);
            KoSHC_CHShadingControlActive.value(_shadingControlActive, DPT_Switch);
        }
    }
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
            if (_channelLockActive && (mode != _modeManual || !ParamSHC_ChannelModeManualIgnoreChannelLock))
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
                    if (!_shadingControlActive && mode->isModeShading())
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
                _shadingControlActive = false;
                if (ParamSHC_ChannelWaitTimeAfterManualUsageForShading != 0)
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
            logDebugP("Changing mode from %s to %s", _currentMode->name(), nextMode->name());
            _handleWindowOpenAsShading = nullptr;
            _currentMode->stop(callContext, nextMode, _positionController);
            if (!nextMode->isModeShading() && _anyShadingModeActive)
            {
                if (nextMode->isModeWindowOpen() && currentModeAllowed)
                {
                    // shading still allowed, handle windowOpen as shading mode
                    _handleWindowOpenAsShading = (ModeShading*) _currentMode;
                }
                else
                {
                    shadingStopped();
                }
            }
        }
        else
        {
            logDebugP("Start mode %s", nextMode->name());
        }
        auto previousMode = _currentMode;
        _currentMode = nextMode;
        if (_currentMode->isModeShading() && !_anyShadingModeActive)
        {
            shadingStarted();
        }
        _currentMode->start(callContext, previousMode, _positionController);
        KoSHC_CHActiveMode.value(_currentMode->sceneNumber() - 1, DPT_SceneNumber);
        callContext.modeNewStarted = true;
    }
    _currentMode->control(callContext, _positionController);
    _positionController.control(callContext);

    callContext.modeIdle = nullptr;
    callContext.modeManual = nullptr;
    callContext.modeCurrentActive = nullptr;
}

void ShutterControllerChannel::shadingStarted()
{
    logInfoP("Shading started");
    _anyShadingModeActive = true;
    KoSHC_CHShadingActive.value(true, DPT_Switch);
}

void ShutterControllerChannel::shadingStopped()
{
    logInfoP("Shading stopped");  
    _anyShadingModeActive = false;
    KoSHC_CHShadingActive.value(false, DPT_Switch);
    // <Enumeration Text="Keine Änderung" Value="0" Id="%ENID%" />
    // <Enumeration Text="Position vor Beschattungsstart" Value="1" Id="%ENID%" />
    // <Enumeration Text="Fährt Auf" Value="2" Id="%ENID%" />
    // <Enumeration Text="Lamelle Waagrecht" Value="3" Id="%ENID%" />
    switch (ParamSHC_ChannelAfterShading)
    {
    case 1:
        _positionController.restoreLastManualPosition();
        break;
    case 2: // Fährt auf
        _positionController.setManualPosition(0); // Handled as manual operation because the value should be stored
        _positionController.setManualSlat(0); // Handled as manual operation because the value should be stored
        break;
    case 3:
        _positionController.setManualSlat(50); // Handled as manual operation because the value should be stored
        break;
    }
}
