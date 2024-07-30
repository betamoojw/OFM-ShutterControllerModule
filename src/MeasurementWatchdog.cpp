#include "MeasurementWatchdog.h"

MeasurementWatchdog::MeasurementWatchdog()
{

}

const std::string &MeasurementWatchdog::logPrefix() const
{
    return _name;
}

void MeasurementWatchdog::init(const char* name, GroupObject *groupObject, uint8_t timeoutParameterValue, const KNXValue &fallbackValue, const Dpt &dpt, MeasurementWatchdogFallbackBehavior fallbackBehaviour)
{
    _name = name;
    _groupObject = groupObject;
    _fallbackValue = fallbackValue;
    _dpt = dpt;
    _fallbackBehaviour = fallbackBehaviour;
    if (_groupObject == nullptr)
    {
        _state = MeasurementWatchdogState::MeasurementWatchdogStateIgnoreValue;
        return;
    }
    // <Enumeration Text="Aus" Value="0" Id="%ENID%" />
    // <Enumeration Text="10 Minuten" Value="1" Id="%ENID%" />
    // <Enumeration Text="30 Minuten" Value="2" Id="%ENID%" />
    // <Enumeration Text="1 Stunde" Value="3" Id="%ENID%" />
    // <Enumeration Text="2 Stunden" Value="4" Id="%ENID%" />
    // <Enumeration Text="3 Stunden" Value="5" Id="%ENID%" />
    // <Enumeration Text="4 Stunden" Value="6" Id="%ENID%" />
    // <Enumeration Text="8 Stunden" Value="7" Id="%ENID%" />
    // <Enumeration Text="12 Stunden" Value="8" Id="%ENID%" />
    switch (timeoutParameterValue)
    {
    case 0:
        break;
    case 1:
        _timeoutMillis = 10 * 60 * 1000;
        break;
    case 2:
        _timeoutMillis = 30 * 60 * 1000;
        break;
    case 3:
        _timeoutMillis = 60 * 60 * 1000;
        break;
    case 4:
        _timeoutMillis = 2 * 60 * 60 * 1000;
        break;
    case 5:
        _timeoutMillis = 3 * 60 * 60 * 1000;
        break;
    case 6:
        _timeoutMillis = 4 * 60 * 60 * 1000;
        break;
    case 7:
        _timeoutMillis = 8 * 60 * 60 * 1000;
        break;
    case 8:
        _timeoutMillis = 12 * 60 * 60 * 1000;
        break;
    }
    _state = MeasurementWatchdogState::MeasurementWatchdogStateInitialize;
}

void MeasurementWatchdog::update(unsigned long currentMillis, bool diagnosticLog)
{
    if (_groupObject == nullptr)
        return;
    switch (_state)
    {
    case MeasurementWatchdogState::MeasurementWatchdogStateInitialize:
        _waitTimeStartMillis = currentMillis;
        _state = MeasurementWatchdogState::MeasurementWatchdogStateWaitForResponseValue;
        _groupObject->requestObjectRead();
        break;
    case MeasurementWatchdogState::MeasurementWatchdogStateWaitForResponseValue:
        if (_timeoutMillis > 0 && currentMillis - _waitTimeStartMillis > 10000)
        {
            switch (_fallbackBehaviour)
            {
            case MeasurementWatchdogFallbackBehavior::RequestValueAndProvideFallbackValue:
            case MeasurementWatchdogFallbackBehavior::ProvideFallbackValue:
                _groupObject->value(_fallbackValue, _dpt);
                _state = MeasurementWatchdogState::MeasurementWatchdogStateProvideFallbackValue;
                break;
            case MeasurementWatchdogFallbackBehavior::IgnoreValue:
            case MeasurementWatchdogFallbackBehavior::RequestValueAndIgnore:
                _state = MeasurementWatchdogState::MeasurementWatchdogStateIgnoreValue;
                break;
            }
        }
        break;
    case MeasurementWatchdogState::MeasurementWatchdogStateWaitForTimeout:
        if (_timeoutMillis > 0 && currentMillis - _waitTimeStartMillis > _timeoutMillis)
        {
            switch (_fallbackBehaviour)
            {
            case MeasurementWatchdogFallbackBehavior::RequestValueAndIgnore:
            case MeasurementWatchdogFallbackBehavior::RequestValueAndProvideFallbackValue:
                _groupObject->requestObjectRead();
                _state = MeasurementWatchdogState::MeasurementWatchdogStateWaitForResponseValue;
                break;
            case MeasurementWatchdogFallbackBehavior::ProvideFallbackValue:
                _groupObject->value(_fallbackValue, _dpt);
                _state = MeasurementWatchdogState::MeasurementWatchdogStateProvideFallbackValue;
                break;
            case MeasurementWatchdogFallbackBehavior::IgnoreValue:
                _state = MeasurementWatchdogState::MeasurementWatchdogStateIgnoreValue;
                break;
            }
        }
        break;
    case MeasurementWatchdogState::MeasurementWatchdogStateProvideFallbackValue:
    case MeasurementWatchdogState::MeasurementWatchdogStateIgnoreValue:
        break;
    }
    if (diagnosticLog)
    {
        switch (_state)
        {
            case MeasurementWatchdogState::MeasurementWatchdogStateWaitForResponseValue:
                logInfoP("State: wait for response value");
                logInfoP("Timeout in: %lu", 10000 - (currentMillis - _waitTimeStartMillis));
                break;
            case MeasurementWatchdogState::MeasurementWatchdogStateWaitForTimeout:
                logInfoP("State: wait for timeout");
                logInfoP("Timeout in: %lu", _timeoutMillis - (currentMillis - _waitTimeStartMillis));
                break;
            case MeasurementWatchdogState::MeasurementWatchdogStateProvideFallbackValue:
                logInfoP("State: fallback value");
                break;
            case MeasurementWatchdogState::MeasurementWatchdogStateIgnoreValue:
                logInfoP("State: ignore value)");
                break;
            case MeasurementWatchdogState::MeasurementWatchdogStateInitialize:
                logInfoP("State: initialize");
                break;
            case MeasurementWatchdogState::MeasurementWatchdogStateNotInitialized:
                logInfoP("State: not initialized");
                break; 
        }
        if (ignoreValue())
            logInfoP("Ignore Value");
        else
            logInfoP("Value: %s", (double) getValue());
    }
}

KNXValue MeasurementWatchdog::getValue() const
{
    return _groupObject->value(_dpt);
}

bool MeasurementWatchdog::ignoreValue() const
{
    return _state == MeasurementWatchdogState::MeasurementWatchdogStateIgnoreValue;
}

bool MeasurementWatchdog::waitForValue() const
{
    return _groupObject == nullptr || !_groupObject->initialized();
}

void MeasurementWatchdog::processIputKo(GroupObject &go)
{
    if (_groupObject != nullptr && go.asap() == _groupObject->asap())
    {
        _state = MeasurementWatchdogState::MeasurementWatchdogStateWaitForTimeout;
        if (_timeoutMillis > 0)
            _waitTimeStartMillis = millis();
    }
}