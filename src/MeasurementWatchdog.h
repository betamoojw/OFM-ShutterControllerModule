#pragma once
#include "OpenKNX.h"

enum MeasurementWatchdogState : uint8_t
{
    MeasurementWatchdogStateNotInitialized,
    MeasurementWatchdogStateInitialize,
    MeasurementWatchdogStateWaitForResponseValue,
    MeasurementWatchdogStateWaitForTimeout,
    MeasurementWatchdogStateProvideFallbackValue,
    MeasurementWatchdogStateIgnoreValue
};

// <Enumeration Text="Wert Ignorieren" Value="0" Id="%ENID%" />
// <Enumeration Text="Leseanforderung schicken, dann Ignorieren" Value="1" Id="%ENID%" />
// <Enumeration Text="Fixen Wert vorgeben" Value="2" Id="%ENID%" />
// <Enumeration Text="Leseanforderung schicken, dann fixen Wert vorgeben" Value="3" Id="%ENID%" />

enum MeasurementWatchdogFallbackBehavior : uint8_t
{
    IgnoreValue = 0,
    RequestValueAndIgnore = 1,
    ProvideFallbackValue = 2,
    RequestValueAndProvideFallbackValue = 3
};

class MeasurementWatchdog 
{
private:
    const static unsigned long _waitForValueTimeout = 10000;
    static bool _missingValue;
    std::string _name;
    bool _changed = true;
    MeasurementWatchdogState _state = MeasurementWatchdogState::MeasurementWatchdogStateNotInitialized;
    unsigned long _timeoutMillis = 0;
    unsigned long _waitTimeStartMillis = 0;
    GroupObject* _groupObject = nullptr;
    KNXValue _fallbackValue = KNXValue(false);
    Dpt _dpt;
    MeasurementWatchdogFallbackBehavior _fallbackBehaviour = MeasurementWatchdogFallbackBehavior::IgnoreValue;
    void setState(MeasurementWatchdogState state);
 public:
    static void resetMissingValue();
    static bool missingValue();
    MeasurementWatchdog();
    const std::string& logPrefix() const;
    void init(const char* name, GroupObject* groupObject, uint8_t timeoutParameterValue, const KNXValue& fallbackValue, const Dpt& dpt, MeasurementWatchdogFallbackBehavior fallbackBehaviour);
    void setup();
    void update(unsigned long currentMillis, bool diagnosticLog);
    KNXValue getValue() const;
    bool ignoreValue() const;
    bool waitForValue() const;
    void processIputKo(GroupObject& go);
    bool isChanged() const;
    bool resetChanged();
    void logState(bool incudeValue);
};