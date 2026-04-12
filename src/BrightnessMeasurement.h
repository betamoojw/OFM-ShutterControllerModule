#pragma once
#include "MeasurementWatchdog.h"
#include <vector>

enum class BrightnessAggregation : uint8_t
{
    Mean = 0,
    Max = 1
};

struct BrightnessSensor
{
    const MeasurementWatchdog* watchdog = nullptr;
    uint16_t azimuth = 0;
    bool hasAzimuth = false;
    bool enabled = false;
};

class BrightnessMeasurement : public MeasurementSource
{
public:
    void init(const char* name, MeasurementWatchdogFallbackBehavior fallbackBehavior, float fallbackLux);
    void setSensors(const std::vector<BrightnessSensor>& sensors);
    void setAggregation(BrightnessAggregation aggregation);
    void update(unsigned long currentMillis, bool diagnosticLog, float sunAzimuth, bool sunAzimuthValid);
    void setUseAzimuth(bool useAzimuth);
    void setAggregateUseMaxOverride(bool useMax);
    void setAggregatePreferUnassigned(bool preferUnassigned);

    const std::string& logPrefix() const override;
    KNXValue getValue() const override;
    bool ignoreValue() const override;
    bool useFallback() const override;
    bool waitForValue() const override;
    bool isChanged() const override;

    bool resetChanged();
    void logState(bool includeValue) const;
    void logSensorMapping(uint8_t channelIndex, bool useAzimuth) const;

private:
    struct ValueState
    {
        float valueLux = 0.0f;
        bool ignoreValue = true;
        bool useFallback = false;
        bool waitForValue = true;
        bool changed = false;
    };

    std::string _name;
    std::vector<BrightnessSensor> _sensors;
    MeasurementWatchdogFallbackBehavior _fallbackBehavior = MeasurementWatchdogFallbackBehavior::IgnoreValue;
    float _fallbackLux = 0.0f;
    BrightnessAggregation _aggregation = BrightnessAggregation::Mean;
    bool _useAzimuth = true;
    bool _aggregateUseMaxOverride = false;
    bool _aggregatePreferUnassigned = false;
    size_t _aggregateUnassignedCount = 0;
    ValueState _aggregateStateMean;
    ValueState _aggregateStateMax;
    ValueState _aggregateStateMeanUnassigned;
    ValueState _aggregateStateMaxUnassigned;
    ValueState _azimuthState;
    bool _anySensorChanged = false;

    struct AggregateStates
    {
        ValueState mean;
        ValueState max;
        size_t count = 0;
    };

    const ValueState& getAggregateState() const;
    AggregateStates buildAggregateStates(bool onlyUnassigned) const;
    ValueState buildAzimuthState(float sunAzimuth, bool sunAzimuthValid, const ValueState& aggregateState) const;
    static float normalizeAzimuth(float azimuth);
    static bool isSensorValid(const BrightnessSensor& sensor);
};
