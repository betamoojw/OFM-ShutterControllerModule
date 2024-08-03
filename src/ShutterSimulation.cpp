#include "ShutterSimulation.h"
#include "PositionController.h"

ShutterSimulation::ShutterSimulation(uint8_t channelIndex, PositionController &positionController)
    : _channelIndex(channelIndex), _positionController(positionController)
{
    _logPrefix = "Simulation";
    _logPrefix += channelIndex + 1;
    KoSHC_CShutterPercentInput.value(_currentPosition, DPT_Scaling);
    if (positionController.hasSlat())
        KoSHC_CShutterSlatInput.value(_currentSlatPosition, DPT_Scaling);
}

const std::string &ShutterSimulation::logPrefix()
{
    return _logPrefix;
}

void ShutterSimulation::processInputKo(GroupObject &ko)
{
    auto targetPosition = _targetPosition;
    switch (SHC_KoCalcIndex(ko.asap()))
    {
    case SHC_KoCShutterPercentOutput:
        targetPosition = ko.value(DPT_Scaling);
        break;
    case SHC_KoCShutterSlatOutput:
        _targetSlatPosition = ko.value(DPT_Scaling);
        break;
    case SHC_KoCManualPercent:
        targetPosition = ko.value(DPT_Scaling);
        if (targetPosition < _targetPosition)
            _targetSlatPosition = 0;
        else if (targetPosition > _targetPosition)
            _targetSlatPosition = 100;
        break;
    case SHC_KoCManualSlatPercent:
        _targetSlatPosition = ko.value(DPT_Scaling);
        break;
    case SHC_KoCManualUpDown:
        if (ko.value(DPT_Switch))
        {
            targetPosition = 100;
            _targetSlatPosition = 100;
        }
        else
        {
            targetPosition = 0;
            _targetSlatPosition = 0;
        }
        break;
    case SHC_KoCManualStepStop:
        if (_targetPosition != _currentPosition || _targetSlatPosition != _currentSlatPosition)
        {
            // stop
            targetPosition = _currentPosition;
            _targetSlatPosition = _currentSlatPosition;
            logDebugP("Stop");
        }
        else
        {
            if (ko.value(DPT_Step))
            {
                // erhöhen
                logDebugP("Erhöhen");
                if (_targetSlatPosition != 0)
                {
                    _targetSlatPosition = max(_targetSlatPosition - 20, 0);
                }
                else if (_targetPosition != 0)
                {
                    _targetPosition = max(_targetPosition - 2, 0);
                }
            }
            else
            {
                // verringern
                logDebugP("Verringern");
                if (_targetSlatPosition != 100)
                {
                    logDebugP("Slat: %d", (int)_targetSlatPosition);
                    _targetSlatPosition = min(_targetSlatPosition + 20, 100);
                      logDebugP("Slat: %d", (int)_targetSlatPosition);
                }
                else if (_targetPosition != 100)
                {
                    logDebugP("Position: %d", (int)_targetPosition);
                    _targetPosition = min(_targetPosition + 2, 100);
                    logDebugP("Position: %d", (int)_targetPosition);
                }
            }
        }
        break;
    default:
        return;
    }
    if (targetPosition != _targetPosition)
    {
        _targetPosition = targetPosition;
        if (_targetPosition != _currentPosition || _targetSlatPosition != _currentSlatPosition)
            setMoving(true);
        else
            setMoving(false);
    }
    logDebugP("Simulated target position: %d", (int)_targetPosition);
    logDebugP("Simulated target slat position: %d", (int)_targetSlatPosition);
}

void ShutterSimulation::update(const CallContext &callContext)
{
    if (callContext.diagnosticLog)
    {
        logInfoP("Position current: %d, target: %d", (int)_currentPosition, (int)_targetPosition);
        if (callContext.hasSlat)
            logInfoP("Slat current: %d, target: %d", (int)_currentSlatPosition, (int)_targetSlatPosition);
    }
    if (_lastPositionChange != 0)
    {
        if (_targetPosition != _currentPosition)
        {
            if (callContext.currentMillis - _lastPositionChange > (_fastSimulation ? 80 : 800))
            {
                if (_targetPosition > _currentPosition)
                {
                    _currentPosition++;
                    _currentSlatPosition = 100;
                }
                else
                {
                    _currentPosition--;
                    _currentSlatPosition = 0;
                }
                if (_targetPosition != _currentPosition)
                {
                    if (abs((uint8_t)KoSHC_CShutterPercentInput.value(DPT_Scaling)) - _currentPosition >= 10)
                    {
                        KoSHC_CShutterPercentInput.value(_currentPosition, DPT_Scaling);
                        logDebugP("Postion: %d", (int)_currentPosition);
                    }
                    setMoving(true);
                }
                else
                {
                    KoSHC_CShutterPercentInput.value(_currentPosition, DPT_Scaling);
                    logDebugP("Postion: %d", (int)_currentPosition);
                    if (callContext.hasSlat)
                    {
                        KoSHC_CShutterSlatInput.value(_currentSlatPosition, DPT_Scaling);
                        logDebugP("Slat: %d", (int)_currentSlatPosition);
                    }
                    if (_targetSlatPosition != _currentSlatPosition)
                        setMoving(false);
                }
            }
        }
        else if (_targetSlatPosition != _currentSlatPosition)
        {
            if (callContext.currentMillis - _lastPositionChange > 10)
            {
                if (_targetSlatPosition > _currentSlatPosition)
                    _currentSlatPosition++;
                else
                    _currentSlatPosition--;

                if (_targetSlatPosition != _currentSlatPosition)
                    setMoving(true);
                else
                {
                    setMoving(false);
                    if (callContext.hasSlat)
                    {
                        KoSHC_CShutterSlatInput.value(_currentSlatPosition, DPT_Scaling);
                        logDebugP("Slat: %d", (int)_currentSlatPosition);
                    }
                }
            }
        }
        if (_lastGroupObjectUpdate != 0 && callContext.currentMillis - _lastGroupObjectUpdate > (_fastSimulation ? 1000 : 10000))
        {
            if (KoSHC_CShutterPercentInput.commFlag() != ComFlag::WriteRequest)
                if (KoSHC_CShutterPercentInput.valueCompare(_currentPosition, DPT_Scaling))
                    logDebugP("Position: %d", (int)_currentPosition);
            if (callContext.hasSlat)
            {
                if (KoSHC_CShutterSlatInput.commFlag() != ComFlag::WriteRequest)
                    if (KoSHC_CShutterSlatInput.valueCompare(_currentSlatPosition, DPT_Scaling))
                        logDebugP("Slat: %d", (int)_currentSlatPosition);
            }
            _lastGroupObjectUpdate = callContext.currentMillis;
        }
    }
}

void ShutterSimulation::setFastSimulation(bool fastSimulation)
{
    _fastSimulation = fastSimulation;
}

bool ShutterSimulation::getFastSimulation()
{
    return _fastSimulation;
}

void ShutterSimulation::setMoving(bool moving)
{
    if (moving)
    {
        if (_lastPositionChange == 0)
            logDebugP("Moving");
        _lastPositionChange = max(millis(), 1ul);
        if (_lastGroupObjectUpdate == 0)
            _lastGroupObjectUpdate = _lastPositionChange;
    }
    else
    {
        if (_lastPositionChange != 0)
            logDebugP("Stopped");
        _lastPositionChange = 0;
        _lastGroupObjectUpdate = 0;
        if (KoSHC_CShutterPercentInput.valueCompare(_currentPosition, DPT_Scaling))
            logDebugP("Position: %d", (int)_currentPosition);
        if (KoSHC_CShutterSlatInput.valueCompare(_currentSlatPosition, DPT_Scaling))
            logDebugP("Slat: %d", (int)_currentSlatPosition);
    }
}
