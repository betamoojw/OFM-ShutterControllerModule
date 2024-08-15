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

void PositionController::setAutomaticPosition(uint8_t automaticPosition)
{
    if (_positionLimit != 100)
        _blockedPosition = automaticPosition;
    if (_positionLimit < automaticPosition)
    {
        logDebugP("Position limit reached");
        automaticPosition = _positionLimit;
    }
    _setPosition = automaticPosition;
}

void PositionController::setAutomaticSlat(uint8_t automaticSlat)
{
    if (!_hasSlat)
        return;
    if (_slatLimit != 255)
        _blockedSlat = automaticSlat;
    if (_slatLimit < automaticSlat)
    {
        logDebugP("Slat limit reached");
        automaticSlat = _slatLimit;
    }
    _setSlat = automaticSlat;
}

void PositionController::setAutomaticPositionAndStoreForRestore(uint8_t automaticPosition)
{
    _restorePosition = automaticPosition;
    setAutomaticPosition(automaticPosition);
}

void PositionController::setAutomaticSlatAndStoreForRestore(uint8_t automaticSlat)
{
    _restoreSlat = automaticSlat;
    setAutomaticSlat(automaticSlat);
}

void PositionController::setManualPosition(uint8_t manualPosition)
{
    _startWaitForManualPositionFeedback = 0;
    _startWaitForManualSlatPositionFeedback = 0;

    _restorePosition = manualPosition;
    // <Enumeration Text="Manuelle Bedienung über Aktor" Value="0" Id="%ENID%" />
    // <Enumeration Text="Modul sendet Auf/Ab zum Aktor" Value="1" Id="%ENID%" />
    // <Enumeration Text="Modul sendet 0/100% zum Aktor " Value="2" Id="%ENID%" />
    if (ParamSHC_CManualUpDownType != 0)
    {
        setAutomaticPosition(manualPosition);
    }
    auto currentPosition = position();
    if (manualPosition != currentPosition)
    {
        setState(_setPosition < position() ? PositionControllerState::MovingUp : PositionControllerState::MovingDown, "Manual position");
        _calculatedTargetPosition = _setPosition;
        setMovingTimeout(MOVING_TIMEOUT);
    }
}

void PositionController::setManualSlat(uint8_t manualSlat)
{
    _startWaitForManualPositionFeedback = 0;
    _startWaitForManualSlatPositionFeedback = 0;

    _restoreSlat = manualSlat;
    // <Enumeration Text="Manuelle Bedienung über Aktor" Value="0" Id="%ENID%" />
    // <Enumeration Text="Modul sendet Auf/Ab zum Aktor" Value="1" Id="%ENID%" />
    // <Enumeration Text="Modul sendet 0/100% zum Aktor " Value="2" Id="%ENID%" />
    if (_hasSlat && (ParamSHC_CManualUpDownType != 0))
        setAutomaticSlat(manualSlat);
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
        _restorePosition = 0;
        _restoreSlat = 0;
    }
    else
    {
        _restorePosition = 100;
        _restoreSlat = 100;
    }
    auto currentPosition = position();
    if (currentPosition == 0 && _restorePosition)
    {
        setState(_restorePosition < position() ? PositionControllerState::MovingUp : PositionControllerState::MovingDown, "Manual up down");
        _calculatedTargetPosition = _restorePosition;
        setMovingTimeout(MOVING_TIMEOUT);
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
    if (_restorePosition != 255)
        _setPosition = _restorePosition;
    if (_restoreSlat != 255)
        _setSlat = _restoreSlat;
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
            setState(PositionControllerState::Idle, "SHC_KoCShutterPercentInput");
        else

            setMovingTimeout(MOVING_TIMEOUT_AFTER_FEEDBACK);
        break;
    }
    case SHC_KoCShutterPercentOutput:
    {
        uint8_t currentPosition = KoSHC_CShutterPercentInput.value(DPT_Scaling);
        uint8_t newPosition = ko.value(DPT_Scaling);
        if (newPosition < currentPosition)
            setState(PositionControllerState::MovingUp, "SHC_KoCShutterPercentOutput");
        else if (newPosition > currentPosition)
            setState(PositionControllerState::MovingDown, "SHC_KoCShutterPercentOutput");
        else
            setState(PositionControllerState::Idle, "SHC_KoCShutterPercentOutput");
        break;
    }
    case SHC_KoCShutterSlatOutput:
    {
        uint8_t currentPosition = KoSHC_CShutterSlatInput.value(DPT_Scaling);
        uint8_t newPosition = ko.value(DPT_Scaling);
        if (newPosition < currentPosition)
            setState(PositionControllerState::MovingUp, "SHC_KoCShutterSlatOutput");
        else if (newPosition > currentPosition)
            setState(PositionControllerState::MovingDown, "SHC_KoCShutterSlatOutput");
        else
            setState(PositionControllerState::Idle, "SHC_KoCShutterSlatOutput");
        break;
    }
    case SHC_KoCShutterStopStepOutput:
    case SHC_KoCManualStepStop:
    case SHC_KoCShutterUpDownOutput:
    case SHC_KoCManualUpDown:
    {
        if (state() == PositionControllerState::MovingDown || state() == PositionControllerState::MovingUp)
            setState(PositionControllerState::Idle, "Any Manual KO");
        else if ((bool)ko.value(DPT_Step) == false)
        {
            // Up
            uint8_t currentPosition = KoSHC_CShutterSlatInput.value(DPT_Scaling);
            if (currentPosition > 0)
            {
                setState(PositionControllerState::MovingUp, "Any Manual KO");
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
                setState(PositionControllerState::MovingDown, "Any Manual KO");
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
            _restorePosition = (uint8_t)ko.value(DPT_Scaling);
        }
        break;
    case SHC_KoCShutterSlatInput:
        if (millis() - _startWaitForManualSlatPositionFeedback < 3000)
        {
            _restoreSlat = (uint8_t)ko.value(DPT_Scaling);
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
        logDebugP("Moving timeout %ds reached", (int)(_waitForMovingTimeout / 1000));
        _waitForMovingFinshed = 0;
        setState(PositionControllerState::Idle, "Control");
    }

    if (now - _startWaitForManualSlatPositionFeedback > 3000)
    {
        _startWaitForManualSlatPositionFeedback = 0;
    }
    if (_setPosition != 255)
    {
        logInfoP("Set position: %d", (int)_setPosition);
        auto currentPosition = position();
        if (_setPosition != currentPosition)
        {
            setState(_setPosition < position() ? PositionControllerState::MovingUp : PositionControllerState::MovingDown, "_setPosition");
            _calculatedTargetPosition = _setPosition;
            setMovingTimeout(MOVING_TIMEOUT);
        }

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
        logInfoP("State: %s", _state == PositionControllerState::Idle ? "Idle" : _state == PositionControllerState::MovingDown ? "Moving down"
                                                                                                                               : "Moving up");
        logInfoP("Position: %d", (int)position());
        if (_positionLimit != 100)
            logInfoP("Position limit: %d", (int)_positionLimit);
        if (_blockedPosition != 255)
            logInfoP("Blocked position: %d", (int)_blockedPosition);
        logInfoP("Target position: %d", (int)targetPosition());
        if (_hasSlat)
        {
            logInfoP("Slat position: %d", (int) (uint8_t)KoSHC_CShutterSlatInput.value(DPT_Scaling));
            if (_slatLimit != 255)
                logInfoP("Slat limit: %d", (int)_slatLimit);
            if (_blockedSlat != 255)    
                logInfoP("Blocked slat position: %d", (int)_blockedSlat);
            logInfoP("Target slat position: %d", (int)_setSlat);
        }
        logInfoP("Last manual position: %d", (int)_restorePosition);
        if (_hasSlat)
            logInfoP("Last manual slat position: %d", (int)_restoreSlat);
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

uint8_t PositionController::simulationMode() const
{
    return _shutterSimulation != nullptr ? (_shutterSimulation->getFastSimulation() ? 2 : 1) : 0;
}

PositionControllerState PositionController::state() const
{
    return _state;
}

uint8_t PositionController::position() const
{
    return KoSHC_CShutterPercentInput.value(DPT_Scaling);
}

int8_t PositionController::targetPosition() const
{
    if (_calculatedTargetPosition == 255)
        return position();
    return _calculatedTargetPosition;
}

uint8_t PositionController::slat() const
{
    return KoSHC_CShutterSlatInput.value(DPT_Scaling);
}

void PositionController::setState(PositionControllerState state, const char *reason)
{
    if (_state != state)
    {
        _state = state;
        switch (_state)
        {
        case PositionControllerState::Idle:
            _waitForMovingFinshed = 0;
            _calculatedTargetPosition = 255;
            logDebugP("State: Idle at %d (%s)", (int)position(), reason);
            break;
        case PositionControllerState::MovingDown:
            setMovingTimeout(MOVING_TIMEOUT);
            logDebugP("State: Moving down (%s)", reason);
            break;
        case PositionControllerState::MovingUp:
            setMovingTimeout(MOVING_TIMEOUT);
            logDebugP("State: Moving up (%s)", reason);
            break;
        }
    }
}

void PositionController::resetPositionLowerLimit()
{
    setPositionLowerLimit(255, false);
}

void PositionController::setPositionLowerLimit(uint8_t positionLimit, bool moveToLimit)
{
    if (positionLimit >= 100)
        positionLimit = 255;
    if (_positionLimit == 100)
    {
        logDebugP("Position limit disabled");
        if (_blockedPosition != 255 && _setPosition == 255)
        {
            logDebugP("Set previous blocked position: %d", (int)_blockedPosition);
            _setPosition = _blockedPosition;
            _blockedPosition = 255;
        }
    }
    else if (targetPosition() > positionLimit || moveToLimit)
    {
        _blockedPosition = targetPosition();
        _calculatedTargetPosition = _positionLimit;
        _setPosition = _positionLimit;
        logDebugP("Position limit forces new position %d, store %d", (int)positionLimit, (int)_setPosition);
    }
    _positionLimit = positionLimit;
 
}

void PositionController::resetSlatLowerLimit()
{
    setSlatLowerLimit(255, false);
}

void PositionController::setSlatLowerLimit(uint8_t slatLimit, bool moveToLimit)
{
    if (!_hasSlat)
        return;
    if (slatLimit >= 100)
        slatLimit = 255;
    if (slatLimit == _slatLimit)
        return;
    if (slatLimit == 255)
    {
        logDebugP("Slat limit disabled");
        if (_blockedSlat != 255 && _setSlat == 255)
        {
            logDebugP("Set previous blocked slat position: %d", (int)_blockedSlat);
            _setSlat = _blockedSlat;
            _blockedSlat = 255;
        }
    }
    else if (slat() > slatLimit || moveToLimit)
    {
        _blockedSlat = slat();
        _setSlat = slatLimit;
        logDebugP("Slat limit forces new slat %d, store %d", (int)slatLimit, (int)_setSlat);
    }
    _slatLimit = slatLimit;
  
}

void PositionController::resetBlockedPositionAndLimits()
{
    _blockedPosition = 255;
    _blockedSlat = 255;
    _positionLimit = 255;
    _slatLimit = 255;
}