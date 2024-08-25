#include "MeasurementWatchdog.h"

bool MeasurementWatchdog::_missingValue = true;

void MeasurementWatchdog::resetMissingValue()
{
    _missingValue = true;
}

bool MeasurementWatchdog::missingValue()
{
    return _missingValue;
}

MeasurementWatchdog::MeasurementWatchdog()
{
}

const std::string &MeasurementWatchdog::logPrefix() const
{
    return _name;
}

void MeasurementWatchdog::init(const char *name, GroupObject *groupObject, uint8_t timeoutParameterValue, const KNXValue &fallbackValue, const Dpt &dpt, MeasurementWatchdogFallbackBehavior fallbackBehaviour)
{
    _name = name;
    _groupObject = groupObject;
    _fallbackValue = fallbackValue;
    _dpt = dpt;
    _fallbackBehaviour = fallbackBehaviour;
    if (_groupObject == nullptr)
    {
        setState(MeasurementWatchdogState::MeasurementWatchdogStateIgnoreValue);
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
    logInfoP("Watchdog time %ds", _timeoutMillis / 1000);

    setState(MeasurementWatchdogState::MeasurementWatchdogStateInitialize);
}

void MeasurementWatchdog::update(unsigned long currentMillis, bool diagnosticLog)
{
    if (_groupObject == nullptr)
    {
        if (diagnosticLog)
            logInfoP("KO not available");
        return;
    }
    switch (_state)
    {
    case MeasurementWatchdogState::MeasurementWatchdogStateInitialize:
        _waitTimeStartMillis = currentMillis;
        if (_groupObject->initialized())
        {
            processIputKo(*_groupObject);
        }
        else
        {
            setState(MeasurementWatchdogState::MeasurementWatchdogStateWaitForResponseValue);
            _groupObject->requestObjectRead();
            _missingValue = true;
        }
        break;
    case MeasurementWatchdogState::MeasurementWatchdogStateWaitForResponseValue:
        if (_timeoutMillis > 0 && currentMillis - _waitTimeStartMillis > _waitForValueTimeout)
        {
            switch (_fallbackBehaviour)
            {
            case MeasurementWatchdogFallbackBehavior::RequestValueAndProvideFallbackValue:
            case MeasurementWatchdogFallbackBehavior::ProvideFallbackValue:
                _groupObject->valueNoSend(_fallbackValue, _dpt);
                setState(MeasurementWatchdogState::MeasurementWatchdogStateProvideFallbackValue);
                logErrorP("Use fallback value %lf", (double)_fallbackValue);
                break;
            case MeasurementWatchdogFallbackBehavior::IgnoreValue:
            case MeasurementWatchdogFallbackBehavior::RequestValueAndIgnore:
                setState(MeasurementWatchdogState::MeasurementWatchdogStateIgnoreValue);
                break;
            }
        }
        _missingValue = true;
        break;
    case MeasurementWatchdogState::MeasurementWatchdogStateWaitForTimeout:
        if (_timeoutMillis > 0 && currentMillis - _waitTimeStartMillis > _timeoutMillis)
        {
            switch (_fallbackBehaviour)
            {
            case MeasurementWatchdogFallbackBehavior::RequestValueAndIgnore:
            case MeasurementWatchdogFallbackBehavior::RequestValueAndProvideFallbackValue:
                _groupObject->requestObjectRead();
                setState(MeasurementWatchdogState::MeasurementWatchdogStateWaitForResponseValue);
                break;
            case MeasurementWatchdogFallbackBehavior::ProvideFallbackValue:
                _groupObject->valueNoSend(_fallbackValue, _dpt);
                setState(MeasurementWatchdogState::MeasurementWatchdogStateProvideFallbackValue);
                logErrorP("Use fallback value %lf", (double)_fallbackValue);
                break;
            case MeasurementWatchdogFallbackBehavior::IgnoreValue:
                setState(MeasurementWatchdogState::MeasurementWatchdogStateIgnoreValue);
                break;
            }
        }
        break;
    case MeasurementWatchdogState::MeasurementWatchdogStateProvideFallbackValue:
        _missingValue = true;
        break;
    case MeasurementWatchdogState::MeasurementWatchdogStateIgnoreValue:
        _missingValue = true;
        break;
    }
    if (diagnosticLog)
    {
        logState(true);
    }
}

void MeasurementWatchdog::logState(bool includeValue)
{
    switch (_state)
    {
    case MeasurementWatchdogState::MeasurementWatchdogStateWaitForResponseValue:
        logInfoP("State: wait for response value");
        if (includeValue)
            logInfoP("Timeout in: %lus", (unsigned long) (_waitForValueTimeout - (millis() - _waitTimeStartMillis))/1000);
        break;
    case MeasurementWatchdogState::MeasurementWatchdogStateWaitForTimeout:
        logInfoP("State: wait for timeout");
        if (includeValue)
            logInfoP("Timeout in: %lus", (unsigned long) (_timeoutMillis - (millis() - _waitTimeStartMillis))/1000);
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
    if (includeValue && !ignoreValue() && !waitForValue())
        logInfoP("Value: %lf", (double)getValue());
}

void MeasurementWatchdog::setState(MeasurementWatchdogState state)
{
    if (_state != state)
    {
        _state = state;
        logState(false);
        _changed = true;
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
        logDebugP("Reveived KO, reset watchdog");
        setState(MeasurementWatchdogState::MeasurementWatchdogStateWaitForTimeout);
        _changed = true;
        if (_timeoutMillis > 0)
            _waitTimeStartMillis = max(millis(), 1ul);
    }
}

bool MeasurementWatchdog::isChanged() const
{
    return _changed;
}

bool MeasurementWatchdog::resetChanged()
{
    bool changed = _changed;
    _changed = false;
    return changed;
}