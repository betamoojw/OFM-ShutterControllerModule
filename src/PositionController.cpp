#include "PositionController.h"
#include "knxprod.h"
#include "CallContext.h"
#include "ShutterSimulation.h"

#define MOVING_TIMEOUT 120000               // 2 minutes
#define MOVING_TIMEOUT_AFTER_FEEDBACK 30000 // 30 seconds

PositionController::PositionController(uint8_t channelIndex)
    : _channelIndex(channelIndex)
{
    _hasSlat = ParamSHC_CType == 1;
    _logPrefix = openknx.logger.buildPrefix("SC", _channelIndex + 1);
    _logPrefix += "PosCtrl";
}
const std::string &PositionController::logPrefix()
{
    return _logPrefix;
}

bool PositionController::hasSlat() const
{
    return _hasSlat;
}

void PositionController::setAutomaticPosition(uint8_t position)
{
    _setPosition = position;
}
void PositionController::setAutomaticSlat(uint8_t position)
{
    if (_hasSlat)
        _setSlat = position;
}
void PositionController::setManualPosition(uint8_t position, bool noControllingModeCheck)
{
    _startWaitForManualPositionFeedback = 0;
    _startWaitForManualSlatPositionFeedback = 0;

    _lastManualPosition = position;
    // <Enumeration Text="Manuelle Bedienung über Aktor" Value="0" Id="%ENID%" />
    // <Enumeration Text="Modul sendet Auf/Ab zum Aktor" Value="1" Id="%ENID%" />
    // <Enumeration Text="Modul sendet 0/100% zum Aktor " Value="2" Id="%ENID%" />
    if (noControllingModeCheck || ParamSHC_CManualUpDownType != 0)
        _setPosition = position;
}
void PositionController::setManualSlat(uint8_t position, bool noControllingModeCheck)
{
    _startWaitForManualPositionFeedback = 0;
    _startWaitForManualSlatPositionFeedback = 0;

    _lastManualSlat = position;
    // <Enumeration Text="Manuelle Bedienung über Aktor" Value="0" Id="%ENID%" />
    // <Enumeration Text="Modul sendet Auf/Ab zum Aktor" Value="1" Id="%ENID%" />
    // <Enumeration Text="Modul sendet 0/100% zum Aktor " Value="2" Id="%ENID%" />
    if (_hasSlat && (noControllingModeCheck || ParamSHC_CManualUpDownType != 0))
        _setSlat = position;
}
void PositionController::setManualStep(bool step)
{
    if (_hasSlat && ParamSHC_CManualUpDownType != 0)
    {
        logInfoP("Set step: %d", (int)step);
        KoSHC_CShutterStopStepOutput.value(step, DPT_Step);
        if (_shutterSimulation != nullptr)
            _shutterSimulation->processInputKo(KoSHC_CShutterStopStepOutput);
        _startWaitForManualPositionFeedback = millis();
        if (_startWaitForManualPositionFeedback == 0)
            _startWaitForManualPositionFeedback = 1;
        if (_hasSlat)
        {
            _startWaitForManualSlatPositionFeedback = millis();
            if (_startWaitForManualSlatPositionFeedback == 0)
                _startWaitForManualSlatPositionFeedback = 1;
        }
    }
}
void PositionController::setManualUpDown(bool up)
{
    _startWaitForManualPositionFeedback = 0;
    _startWaitForManualSlatPositionFeedback = 0;
    if (up)
    {
        _lastManualPosition = 0;
        _lastManualSlat = 0;
    }
    else
    {
        _lastManualPosition = 100;
        _lastManualSlat = 100;
    }
    // <Enumeration Text="Manuelle Bedienung über Aktor" Value="0" Id="%ENID%" />
    // <Enumeration Text="Modul sendet Auf/Ab zum Aktor" Value="1" Id="%ENID%" />
    // <Enumeration Text="Modul sendet 0/100% zum Aktor " Value="2" Id="%ENID%" />
    switch (ParamSHC_CManualUpDownType)
    {
    case 1:
        logInfoP("Set up: %d", (int)up);
        KoSHC_CShutterUpDownOutput.value(up, DPT_UpDown);
        if (_shutterSimulation != nullptr)
            _shutterSimulation->processInputKo(KoSHC_CShutterUpDownOutput);

        break;
    case 2:
        _setPosition = up ? 0 : 100;
        break;
    }
}
void PositionController::restoreLastManualPosition()
{
    if (_lastManualPosition != 255)
        _setPosition = _lastManualPosition;
    if (_lastManualSlat != 255)
        _setSlat = _lastManualSlat;
}
void PositionController::processInputKo(GroupObject &ko)
{
    if (_shutterSimulation != nullptr)
        _shutterSimulation->processInputKo(ko);
    auto koIndex = SHC_KoCalcIndex(ko.asap());
    // Handle moving
    switch (koIndex)
    {
    case SHC_KoCShutterPercentInput:
    {
        uint8_t currentPosition = KoSHC_CShutterPercentInput.value(DPT_Scaling);
        if (currentPosition == 0 || currentPosition == 100)
            setState(PositionControllerState::Idle);
        else
            setMovingTimeout(MOVING_TIMEOUT_AFTER_FEEDBACK);
        break;
    }
    case SHC_KoCShutterPercentOutput:
    {
        uint8_t currentPosition = KoSHC_CShutterPercentInput.value(DPT_Scaling);
        uint8_t newPosition = ko.value(DPT_Scaling);
        if (newPosition < currentPosition)
            setState(PositionControllerState::MovingDown);
        else if (newPosition > currentPosition)
            setState(PositionControllerState::MovingUp);
        else
            setState(PositionControllerState::Idle);
        break;
    }
    case SHC_KoCShutterSlatOutput:
    {
        uint8_t currentPosition = KoSHC_CShutterSlatInput.value(DPT_Scaling);
        uint8_t newPosition = ko.value(DPT_Scaling);
        if (newPosition < currentPosition)
            setState(PositionControllerState::MovingDown);
        else if (newPosition > currentPosition)
            setState(PositionControllerState::MovingUp);
        else
            setState(PositionControllerState::Idle);
        break;
    }
    case SHC_KoCShutterStopStepOutput:
    case SHC_KoCManualStepStop:
    case SHC_KoCShutterUpDownOutput:
    case SHC_KoCManualUpDown:
    {
        if (state() == PositionControllerState::MovingDown || state() == PositionControllerState::MovingUp)
            setState(PositionControllerState::Idle);
        else if ((bool)ko.value(DPT_Step) == false)
        {
            // Up
            uint8_t currentPosition = KoSHC_CShutterSlatInput.value(DPT_Scaling);
            if (currentPosition > 0)
            {
                setState(PositionControllerState::MovingUp);
                if (koIndex == SHC_KoCShutterStopStepOutput || koIndex == SHC_KoCManualStepStop)
                    _waitForMovingTimeout = MOVING_TIMEOUT_AFTER_FEEDBACK;
            }
        }
        else
        {
            // Down
            uint8_t currentPosition = KoSHC_CShutterSlatInput.value(DPT_Scaling);
            if (currentPosition < 100)
            {
                setState(PositionControllerState::MovingDown);
                if (koIndex == SHC_KoCShutterStopStepOutput || koIndex == SHC_KoCManualStepStop)
                    _waitForMovingTimeout = MOVING_TIMEOUT_AFTER_FEEDBACK;
            }
        }
        break;
    }
    }
    // Handling manual position feedback
    switch (koIndex)
    {
    case SHC_KoCShutterPercentInput:
        if (millis() - _startWaitForManualPositionFeedback < 3000)
        {
            _lastManualPosition = (uint8_t)ko.value(DPT_Scaling);
        }
        break;
    case SHC_KoCShutterSlatInput:
        if (millis() - _startWaitForManualSlatPositionFeedback < 3000)
        {
            _lastManualSlat = (uint8_t)ko.value(DPT_Scaling);
        }
        break;
    }
}

void PositionController::setMovingTimeout(unsigned long timeout)
{
    if (simulationMode() == 2)
        timeout /= 10;
    _waitForMovingTimeout = timeout;
    _waitForMovingFinshed = max(millis(), 1ul);
}

void PositionController::control(const CallContext &callContext)
{
    if (_shutterSimulation != nullptr)
        _shutterSimulation->update(callContext);
    auto now = callContext.currentMillis;
    if (_waitForMovingFinshed > 0 && now - _waitForMovingFinshed > _waitForMovingTimeout)
    {
        logDebugP("Moving timeout %ds reached",(int) (_waitForMovingTimeout / 1000));
        _waitForMovingFinshed = 0;
        setState(PositionControllerState::Idle);
    }

    if (now - _startWaitForManualSlatPositionFeedback > 3000)
    {
        _startWaitForManualSlatPositionFeedback = 0;
    }
    if (_setPosition != 255)
    {
        logInfoP("Set position: %d", (int)_setPosition);
        KoSHC_CShutterPercentOutput.value(_setPosition, DPT_Scaling);
        if (_shutterSimulation != nullptr)
            _shutterSimulation->processInputKo(KoSHC_CShutterPercentOutput);
        _setPosition = 255;
    }
    if (_setSlat != 255)
    {
        logInfoP("Set slat position: %d", (int)_setSlat);
        KoSHC_CShutterSlatOutput.value(_setSlat, DPT_Scaling);
        if (_shutterSimulation != nullptr)
            _shutterSimulation->processInputKo(KoSHC_CShutterSlatOutput);
        _setSlat = 255;
    }
    if (callContext.diagnosticLog)
    {
        logInfoP("Last manual position: %d", (int)_lastManualPosition);
        if (_hasSlat)
            logInfoP("Last manual slat position: %d", (int)_lastManualSlat);
    }
}
bool PositionController::startSimulation(bool fastSimulation)
{
    if (_shutterSimulation == nullptr)
    {
        _shutterSimulation = new ShutterSimulation(_channelIndex, *this);
        _shutterSimulation->setFastSimulation(fastSimulation);
        return true;
    }
    _shutterSimulation->setFastSimulation(fastSimulation);
    return false;
}
bool PositionController::stopSimulation()
{
    if (_shutterSimulation != nullptr)
    {
        delete _shutterSimulation;
        _shutterSimulation = nullptr;
        return true;
    }
    return false;
}

uint8_t PositionController::simulationMode()
{
    return _shutterSimulation != nullptr ? (_shutterSimulation->getFastSimulation() ? 2 : 1) : 0;
}
PositionControllerState PositionController::state()
{
    return _state;
}

uint8_t PositionController::position()
{
    return KoSHC_CShutterPercentInput.value(DPT_Scaling);
}

void PositionController::setState(PositionControllerState state)
{
    if (_state != state)
    {
        _state = state;
        switch (_state)
        {
        case PositionControllerState::Idle:
            _waitForMovingFinshed = 0;
            logDebugP("State: Idle at %d", (int)position());
            break;
        case PositionControllerState::MovingDown:
            setMovingTimeout(MOVING_TIMEOUT);
            logDebugP("State: Moving down");
            break;
        case PositionControllerState::MovingUp:
            setMovingTimeout(MOVING_TIMEOUT);
            logDebugP("State: Moving up");
            break;
        }
    }
}