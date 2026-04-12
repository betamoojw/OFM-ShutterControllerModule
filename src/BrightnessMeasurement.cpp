#include "BrightnessMeasurement.h"
#include <algorithm>
#include <cmath>

void BrightnessMeasurement::init(const char* name, MeasurementWatchdogFallbackBehavior fallbackBehavior, float fallbackLux)
{
    _name = name;
    _fallbackBehavior = fallbackBehavior;
    _fallbackLux = fallbackLux;
}

void BrightnessMeasurement::setSensors(const std::vector<BrightnessSensor>& sensors)
{
    _sensors = sensors;
}

void BrightnessMeasurement::setAggregation(BrightnessAggregation aggregation)
{
    _aggregation = aggregation;
}

void BrightnessMeasurement::setUseAzimuth(bool useAzimuth)
{
    if (_useAzimuth == useAzimuth)
        return;
    _useAzimuth = useAzimuth;
    _aggregateStateMean.changed = true;
    _aggregateStateMax.changed = true;
    _azimuthState.changed = true;
}

void BrightnessMeasurement::setAggregateUseMaxOverride(bool useMax)
{
    if (_aggregateUseMaxOverride == useMax)
        return;
    _aggregateUseMaxOverride = useMax;
    _aggregateStateMean.changed = true;
    _aggregateStateMax.changed = true;
    _aggregateStateMeanUnassigned.changed = true;
    _aggregateStateMaxUnassigned.changed = true;
}

void BrightnessMeasurement::setAggregatePreferUnassigned(bool preferUnassigned)
{
    if (_aggregatePreferUnassigned == preferUnassigned)
        return;
    _aggregatePreferUnassigned = preferUnassigned;
    _aggregateStateMean.changed = true;
    _aggregateStateMax.changed = true;
    _aggregateStateMeanUnassigned.changed = true;
    _aggregateStateMaxUnassigned.changed = true;
}

const std::string& BrightnessMeasurement::logPrefix() const
{
    return _name;
}

KNXValue BrightnessMeasurement::getValue() const
{
    const auto& state = _useAzimuth ? _azimuthState : getAggregateState();
    return KNXValue(state.valueLux);
}

bool BrightnessMeasurement::ignoreValue() const
{
    const auto& state = _useAzimuth ? _azimuthState : getAggregateState();
    return state.ignoreValue;
}

bool BrightnessMeasurement::useFallback() const
{
    const auto& state = _useAzimuth ? _azimuthState : getAggregateState();
    return state.useFallback;
}

bool BrightnessMeasurement::waitForValue() const
{
    const auto& state = _useAzimuth ? _azimuthState : getAggregateState();
    return state.waitForValue;
}

bool BrightnessMeasurement::isChanged() const
{
    const auto& state = _useAzimuth ? _azimuthState : getAggregateState();
    return state.changed;
}

bool BrightnessMeasurement::resetChanged()
{
    bool changed = _aggregateStateMean.changed || _aggregateStateMax.changed ||
        _aggregateStateMeanUnassigned.changed || _aggregateStateMaxUnassigned.changed ||
        _azimuthState.changed;
    _aggregateStateMean.changed = false;
    _aggregateStateMax.changed = false;
    _aggregateStateMeanUnassigned.changed = false;
    _aggregateStateMaxUnassigned.changed = false;
    _azimuthState.changed = false;
    return changed;
}

void BrightnessMeasurement::logState(bool includeValue) const
{
    const auto& state = _useAzimuth ? _azimuthState : getAggregateState();
    logInfoP("State: %s", _useAzimuth ? "azimuth" : "aggregate");
    if (state.ignoreValue)
        logInfoP("Value ignored");
    else if (state.useFallback)
        logInfoP("Using fallback");
    if (includeValue && !state.ignoreValue)
        logInfoP("Value: %lf", (double)state.valueLux);
}

void BrightnessMeasurement::logSensorMapping(uint8_t channelIndex, bool useAzimuth) const
{
    size_t enabledCount = 0;
    size_t azimuthCount = 0;
    for (const auto& sensor : _sensors)
    {
        if (!sensor.enabled)
            continue;
        enabledCount++;
        if (sensor.hasAzimuth)
            azimuthCount++;
    }

    const char* aggregationName = _aggregation == BrightnessAggregation::Max ? "max" : "mean";
    logInfoP("Brightness mapping CH%u: %s, aggregation=%s, sensors=%u, azimuths=%u",
             (unsigned int)(channelIndex + 1),
             useAzimuth ? "azimuth on" : "azimuth off",
             aggregationName,
             (unsigned int)enabledCount,
             (unsigned int)azimuthCount);

    for (size_t i = 0; i < _sensors.size(); i++)
    {
        const auto& sensor = _sensors[i];
        if (!sensor.enabled)
        {
            logInfoP("Brightness sensor %u: disabled", (unsigned int)(i + 1));
            continue;
        }
        if (sensor.hasAzimuth)
            logInfoP("Brightness sensor %u: enabled, azimuth=%u", (unsigned int)(i + 1), (unsigned int)sensor.azimuth);
        else
            logInfoP("Brightness sensor %u: enabled, azimuth=none", (unsigned int)(i + 1));
    }
}

void BrightnessMeasurement::update(unsigned long currentMillis, bool diagnosticLog, float sunAzimuth, bool sunAzimuthValid)
{
    (void)currentMillis;
    (void)diagnosticLog;

    _anySensorChanged = false;
    for (const auto& sensor : _sensors)
    {
        if (sensor.watchdog != nullptr && sensor.watchdog->isChanged())
            _anySensorChanged = true;
    }

    auto aggregateStatesAll = buildAggregateStates(false);
    auto aggregateStatesUnassigned = buildAggregateStates(true);
    _aggregateUnassignedCount = aggregateStatesUnassigned.count;

    const auto& aggregateStateForAzimuth =
        (_aggregation == BrightnessAggregation::Max) ? aggregateStatesAll.max : aggregateStatesAll.mean;
    auto azimuthState = buildAzimuthState(sunAzimuth, sunAzimuthValid, aggregateStateForAzimuth);

    _aggregateStateMean.changed = _anySensorChanged ||
        std::fabs(_aggregateStateMean.valueLux - aggregateStatesAll.mean.valueLux) > 0.001f ||
        _aggregateStateMean.ignoreValue != aggregateStatesAll.mean.ignoreValue ||
        _aggregateStateMean.useFallback != aggregateStatesAll.mean.useFallback ||
        _aggregateStateMean.waitForValue != aggregateStatesAll.mean.waitForValue;

    _aggregateStateMax.changed = _anySensorChanged ||
        std::fabs(_aggregateStateMax.valueLux - aggregateStatesAll.max.valueLux) > 0.001f ||
        _aggregateStateMax.ignoreValue != aggregateStatesAll.max.ignoreValue ||
        _aggregateStateMax.useFallback != aggregateStatesAll.max.useFallback ||
        _aggregateStateMax.waitForValue != aggregateStatesAll.max.waitForValue;

    _aggregateStateMeanUnassigned.changed = _anySensorChanged ||
        std::fabs(_aggregateStateMeanUnassigned.valueLux - aggregateStatesUnassigned.mean.valueLux) > 0.001f ||
        _aggregateStateMeanUnassigned.ignoreValue != aggregateStatesUnassigned.mean.ignoreValue ||
        _aggregateStateMeanUnassigned.useFallback != aggregateStatesUnassigned.mean.useFallback ||
        _aggregateStateMeanUnassigned.waitForValue != aggregateStatesUnassigned.mean.waitForValue;

    _aggregateStateMaxUnassigned.changed = _anySensorChanged ||
        std::fabs(_aggregateStateMaxUnassigned.valueLux - aggregateStatesUnassigned.max.valueLux) > 0.001f ||
        _aggregateStateMaxUnassigned.ignoreValue != aggregateStatesUnassigned.max.ignoreValue ||
        _aggregateStateMaxUnassigned.useFallback != aggregateStatesUnassigned.max.useFallback ||
        _aggregateStateMaxUnassigned.waitForValue != aggregateStatesUnassigned.max.waitForValue;

    _azimuthState.changed = _anySensorChanged ||
        std::fabs(_azimuthState.valueLux - azimuthState.valueLux) > 0.001f ||
        _azimuthState.ignoreValue != azimuthState.ignoreValue ||
        _azimuthState.useFallback != azimuthState.useFallback ||
        _azimuthState.waitForValue != azimuthState.waitForValue;

    _aggregateStateMean = aggregateStatesAll.mean;
    _aggregateStateMax = aggregateStatesAll.max;
    _aggregateStateMeanUnassigned = aggregateStatesUnassigned.mean;
    _aggregateStateMaxUnassigned = aggregateStatesUnassigned.max;
    _azimuthState = azimuthState;
}

const BrightnessMeasurement::ValueState& BrightnessMeasurement::getAggregateState() const
{
    const bool useUnassigned = _aggregatePreferUnassigned && _aggregateUnassignedCount > 0;
    const auto& meanState = useUnassigned ? _aggregateStateMeanUnassigned : _aggregateStateMean;
    const auto& maxState = useUnassigned ? _aggregateStateMaxUnassigned : _aggregateStateMax;
    if (_aggregateUseMaxOverride || _aggregation == BrightnessAggregation::Max)
        return maxState;
    return meanState;
}

BrightnessMeasurement::AggregateStates BrightnessMeasurement::buildAggregateStates(bool onlyUnassigned) const
{
    AggregateStates states;
    float sum = 0.0f;
    float maxValue = 0.0f;
    size_t count = 0;

    for (const auto& sensor : _sensors)
    {
        if (!isSensorValid(sensor))
            continue;
        if (onlyUnassigned && sensor.hasAzimuth)
            continue;
        float value = (float)sensor.watchdog->getValue();
        sum += value;
        if (count == 0 || value > maxValue)
            maxValue = value;
        count++;
    }
    states.count = count;

    if (count > 0)
    {
        states.mean.ignoreValue = false;
        states.mean.useFallback = false;
        states.mean.waitForValue = false;
        states.mean.valueLux = sum / (float)count;

        states.max.ignoreValue = false;
        states.max.useFallback = false;
        states.max.waitForValue = false;
        states.max.valueLux = maxValue;
        return states;
    }

    states.mean.waitForValue = true;
    states.max.waitForValue = true;
    switch (_fallbackBehavior)
    {
    case MeasurementWatchdogFallbackBehavior::ProvideFallbackValue:
    case MeasurementWatchdogFallbackBehavior::RequestValueAndProvideFallbackValue:
        states.mean.ignoreValue = false;
        states.mean.useFallback = true;
        states.mean.valueLux = _fallbackLux;

        states.max.ignoreValue = false;
        states.max.useFallback = true;
        states.max.valueLux = _fallbackLux;
        break;
    case MeasurementWatchdogFallbackBehavior::IgnoreValue:
    case MeasurementWatchdogFallbackBehavior::RequestValueAndIgnore:
    default:
        states.mean.ignoreValue = true;
        states.mean.useFallback = false;

        states.max.ignoreValue = true;
        states.max.useFallback = false;
        break;
    }
    return states;
}

BrightnessMeasurement::ValueState BrightnessMeasurement::buildAzimuthState(float sunAzimuth, bool sunAzimuthValid, const ValueState& aggregateState) const
{
    ValueState state;

    struct AzimuthValue
    {
        float azimuth = 0.0f;
        float valueLux = 0.0f;
    };

    std::vector<AzimuthValue> values;
    for (const auto& sensor : _sensors)
    {
        if (!isSensorValid(sensor) || !sensor.hasAzimuth)
            continue;
        values.push_back({(float)sensor.azimuth, (float)sensor.watchdog->getValue()});
    }

    if (!sunAzimuthValid)
        values.clear();

    if (values.empty())
    {
        return aggregateState;
    }

    std::sort(values.begin(), values.end(), [](const AzimuthValue& left, const AzimuthValue& right)
    {
        return left.azimuth < right.azimuth;
    });

    std::vector<AzimuthValue> merged;
    for (const auto& entry : values)
    {
        if (!merged.empty() && std::fabs(merged.back().azimuth - entry.azimuth) < 0.001f)
        {
            merged.back().valueLux = (merged.back().valueLux + entry.valueLux) / 2.0f;
            continue;
        }
        merged.push_back(entry);
    }

    if (merged.size() == 1)
    {
        state.ignoreValue = false;
        state.useFallback = false;
        state.waitForValue = false;
        state.valueLux = merged.front().valueLux;
        return state;
    }

    float target = normalizeAzimuth(sunAzimuth);
    float result = merged.front().valueLux;

    for (size_t i = 0; i < merged.size(); i++)
    {
        float a0 = merged[i].azimuth;
        float a1 = merged[(i + 1) % merged.size()].azimuth;
        float v0 = merged[i].valueLux;
        float v1 = merged[(i + 1) % merged.size()].valueLux;
        float segmentStart = a0;
        float segmentEnd = a1;

        if (i == merged.size() - 1)
            segmentEnd += 360.0f;

        float targetWrapped = target;
        if (targetWrapped < segmentStart)
            targetWrapped += 360.0f;

        if (targetWrapped >= segmentStart && targetWrapped <= segmentEnd)
        {
            float ratio = (segmentEnd - segmentStart) > 0.0f ?
                (targetWrapped - segmentStart) / (segmentEnd - segmentStart) : 0.0f;
            result = v0 + (v1 - v0) * ratio;
            break;
        }
    }

    state.ignoreValue = false;
    state.useFallback = false;
    state.waitForValue = false;
    state.valueLux = result;
    return state;
}

float BrightnessMeasurement::normalizeAzimuth(float azimuth)
{
    while (azimuth < 0.0f)
        azimuth += 360.0f;
    while (azimuth >= 360.0f)
        azimuth -= 360.0f;
    return azimuth;
}

bool BrightnessMeasurement::isSensorValid(const BrightnessSensor& sensor)
{
    if (sensor.watchdog == nullptr)
        return false;
    if (!sensor.enabled)
        return false;
    if (sensor.watchdog->ignoreValue())
        return false;
    if (sensor.watchdog->waitForValue())
        return false;
    return true;
}
