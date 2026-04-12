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
// <Enumeration Text="Leseanforderung schicken, dann ignorieren" Value="1" Id="%ENID%" />
// <Enumeration Text="Fixen Wert vorgeben" Value="2" Id="%ENID%" />
// <Enumeration Text="Leseanforderung schicken, dann fixen Wert vorgeben" Value="3" Id="%ENID%" />

enum MeasurementWatchdogFallbackBehavior : uint8_t
{
    IgnoreValue = 0,
    RequestValueAndIgnore = 1,
    ProvideFallbackValue = 2,
    RequestValueAndProvideFallbackValue = 3
};

class MeasurementSource
{
public:
    virtual ~MeasurementSource() = default;
    virtual const std::string& logPrefix() const = 0;
    virtual KNXValue getValue() const = 0;
    virtual bool ignoreValue() const = 0;
    virtual bool useFallback() const = 0;
    virtual bool waitForValue() const = 0;
    virtual bool isChanged() const = 0;
};

class MeasurementWatchdog : public MeasurementSource
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
    const std::string& logPrefix() const override;
    void init(const char* name, GroupObject* groupObject, uint8_t timeoutParameterValue, const KNXValue& fallbackValue, const Dpt& dpt, MeasurementWatchdogFallbackBehavior fallbackBehaviour);
    void setup();
    void update(unsigned long currentMillis, bool diagnosticLog);
    KNXValue getValue() const override;
    bool ignoreValue() const override;
    bool useFallback() const override;
    bool waitForValue() const override;
    void processIputKo(GroupObject& go);
    bool isChanged() const override;
    bool resetChanged();
    void logState(bool incudeValue);
};