#include "ShutterControllerChannel.h"
#include "ModeManual.h"
#include "ModeWindowOpen.h"
#include "ModeNight.h"
#include "ModeShading.h"
#include "ModeIdle.h"

ShutterControllerChannel::ShutterControllerChannel(uint8_t channelIndex)
    : _modes()
{
    _name = "SC";
}

const std::string ShutterControllerChannel::name()
{
    return _name;
}

void ShutterControllerChannel::setup()
{
    KoSHC_CHShadingControl.value(false, DPT_Switch);
    KoSHC_CHShadingControlActive.value(false, DPT_Switch);
    KoSHC_CHShadingActive.value(false, DPT_Switch);
  
    // add modes, highest priority first
    _modeManual = new ModeManual();
    if (ParamSHC_ChannelModeWindowOpen)
        _modes.push_back(new ModeWindowOpen());

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
    case SHC_KoShadingControlDailyActivation:
        KoSHC_ShadingControlDailyActivationStatus.value(ko.value(DPT_Switch), DPT_Switch);
        break;
    case SHC_KoCHShadingControl:
        _shadingControlActive = ko.value(DPT_Switch);
        _waitTimeForReactivateShadingAfterManualStarted = 0;
        _waitForShadingPeriodEnd = false;
        KoSHC_CHShadingControlActive.value(_shadingControlActive, DPT_Switch);
        break;
    }
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
    return false;
}

void ShutterControllerChannel::execute(CallContext &callContext)
{
    ModeBase *nextMode = nullptr;
    callContext.newStarted = false;
    callContext.modeIdle = _modeIdle;
    callContext.modeManual = _modeManual;
    callContext.modeCurrentActive = _currentMode;
    if (_currentMode == nullptr)
    {
        _currentMode = _modeIdle;
        _currentMode->start(callContext, callContext.modeCurrentActive);
        KoSHC_CHActiveMode.value(_currentMode->sceneNumber() - 1, DPT_SceneNumber);
    }
    // Handle daily reactivation
    if (callContext.minuteChanged && 
        callContext.minute == 0 && 
        callContext.hour == 0 && 
        KoSHC_ShadingControlDailyActivationStatus.value(DPT_Switch))
    {
        KoSHC_CHShadingControl.value(true, DPT_Switch);
        processInputKo(KoSHC_CHShadingControl);
    }
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
            if (mode->isShading())
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
    for (auto mode : _modes)
    {
        if (callContext.diagnosticLog)
            logInfoP("Mode %s", mode->name());

        logIndentUp();
        if (mode->allowed(callContext))
        {
            if (callContext.diagnosticLog)
                logInfoP("-> allowed");
            if (nextMode == nullptr &&                         // check if this is the first allowed mode
                (_shadingControlActive || !mode->isShading())) // check shading allowed to activate
            {
                nextMode = mode;
                // Do not break because allowed should be called for all modes
                // because it is a replacment for the loop function
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
            if (_currentMode->isShading())
            {
                _shadingControlActive = false;
                KoSHC_CHShadingControlActive.value(false, DPT_Switch);
                if (ParamSHC_ChannelWaitTimeAfterManualUsageForShading != 0)
                    _waitTimeForReactivateShadingAfterManualStarted = callContext.currentMillis;
                else
                    _waitForShadingPeriodEnd = true;
            }
        }
        if (_currentMode != nullptr)
        {
            logDebugP("Changing mode from %s to %s", _currentMode->name(), nextMode->name());
            _currentMode->stop(callContext, nextMode);
            if (_currentMode->isShading() && !_shadowActive)
                KoSHC_CHShadingActive.value(false, DPT_Switch);
        }
        else
        {
            logDebugP("Start mode %s", nextMode->name());
        }
        auto previousMode = _currentMode;
        _currentMode = nextMode;
        _currentMode->start(callContext, previousMode);
        KoSHC_CHActiveMode.value(_currentMode->sceneNumber() - 1, DPT_SceneNumber);
        callContext.newStarted = true;
        if (_currentMode->isShading() != _shadowActive)
        {
            _shadowActive = _currentMode->isShading();
            if (_shadowActive)
                KoSHC_CHShadingActive.value(true, DPT_Switch);
        }
    }
    _currentMode->control(callContext);
    callContext.modeIdle = nullptr;
    callContext.modeManual = nullptr;
    callContext.modeCurrentActive = nullptr;
}