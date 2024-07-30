#include "ShutterSimulation.h"
#include "PositionController.h"

ShutterSimulation::ShutterSimulation(uint8_t channelIndex, PositionController& positionController)
    : _channelIndex(channelIndex), _positionController(positionController)
{
    _logPrefix = "Simulation";
    _logPrefix += channelIndex + 1;
}

const std::string& ShutterSimulation::logPrefix()
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
    }
    if (targetPosition != _targetPosition)
    {
        _targetPosition = targetPosition;
        if (_targetPosition != _currentPosition)
            _lastPositionChange = millis();
        else if (_targetSlatPosition != _currentSlatPosition)
            _lastPositionChange = 0;
    }
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
             if (millis() - _lastPositionChange > 800)
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
                        logInfoP("Postion: %d", (int)_currentPosition);
                        _positionController.processInputKo(KoSHC_CShutterPercentInput);
                    }
                    _lastPositionChange = callContext.currentMillis;
                }
                else
                {
                    KoSHC_CShutterPercentInput.value(_currentPosition, DPT_Scaling);
                    logInfoP("Postion: %d", (int)_currentPosition);
                    _positionController.processInputKo(KoSHC_CShutterPercentInput);
                    if (callContext.hasSlat)
                    {
                        KoSHC_CShutterSlatInput.value(_currentSlatPosition, DPT_Scaling);
                        logInfoP("Slat: %d", (int)_currentSlatPosition);
                        _positionController.processInputKo(KoSHC_CShutterSlatInput);
                    }
                    if (_targetSlatPosition != _currentSlatPosition)
                        _lastPositionChange = 0;
                }
            }
        }
        else if (_targetSlatPosition != _currentSlatPosition)
        {
            if (millis() - _lastPositionChange > 10)
            {
                if (_targetSlatPosition > _currentSlatPosition)
                    _currentSlatPosition++;
                else
                    _currentSlatPosition--;

                if (_targetSlatPosition != _currentSlatPosition)
                    _lastPositionChange = callContext.currentMillis;
                else
                {
                    _lastPositionChange = 0;
                    if (callContext.hasSlat)
                    {
                        KoSHC_CShutterSlatInput.value(_currentSlatPosition, DPT_Scaling);
                        logInfoP("Slat: %d", (int)_currentSlatPosition);
                        _positionController.processInputKo(KoSHC_CShutterSlatInput);
                    }
                }
            }
        }
    }
}
