import math
import unittest


def normalize_azimuth(azimuth):
    while azimuth < 0.0:
        azimuth += 360.0
    while azimuth >= 360.0:
        azimuth -= 360.0
    return azimuth


def aggregate_states(sensors, only_unassigned):
    values = []
    for sensor in sensors:
        if not sensor["valid"]:
            continue
        if only_unassigned and sensor["has_azimuth"]:
            continue
        values.append(sensor["value"])
    if not values:
        return {"count": 0, "mean": None, "max": None}
    return {
        "count": len(values),
        "mean": sum(values) / float(len(values)),
        "max": max(values),
    }


def build_azimuth_state(sensors, sun_azimuth, sun_valid, aggregate_value):
    values = []
    for sensor in sensors:
        if not sensor["valid"]:
            continue
        if not sensor["has_azimuth"]:
            continue
        values.append((float(sensor["azimuth"]), float(sensor["value"])))

    if not sun_valid:
        values = []

    if not values:
        return aggregate_value

    values.sort(key=lambda x: x[0])
    merged = []
    for az, val in values:
        if merged and abs(merged[-1][0] - az) < 0.001:
            merged[-1] = (az, (merged[-1][1] + val) / 2.0)
        else:
            merged.append((az, val))

    if len(merged) == 1:
        return merged[0][1]

    target = normalize_azimuth(sun_azimuth)
    result = merged[0][1]
    for i in range(len(merged)):
        a0, v0 = merged[i]
        a1, v1 = merged[(i + 1) % len(merged)]
        segment_start = a0
        segment_end = a1
        if i == len(merged) - 1:
            segment_end += 360.0

        target_wrapped = target
        if target_wrapped < segment_start:
            target_wrapped += 360.0

        if segment_start <= target_wrapped <= segment_end:
            ratio = (target_wrapped - segment_start) / (segment_end - segment_start) if segment_end > segment_start else 0.0
            result = v0 + (v1 - v0) * ratio
            break

    return result


def evaluate_brightness(sensors, aggregation, use_azimuth, prefer_unassigned, force_max, sun_azimuth, sun_valid):
    states_all = aggregate_states(sensors, False)
    states_unassigned = aggregate_states(sensors, True)
    use_unassigned = prefer_unassigned and states_unassigned["count"] > 0

    mean_val = states_unassigned["mean"] if use_unassigned else states_all["mean"]
    max_val = states_unassigned["max"] if use_unassigned else states_all["max"]

    aggregate_value = max_val if (force_max or aggregation == "max") else mean_val

    if aggregate_value is None:
        aggregate_value = 0.0

    if use_azimuth:
        return build_azimuth_state(sensors, sun_azimuth, sun_valid, aggregate_value)

    return aggregate_value


class BrightnessLogicTests(unittest.TestCase):
    def test_azimuth_interpolation(self):
        sensors = [
            {"value": 100.0, "has_azimuth": True, "azimuth": 90, "valid": True},
            {"value": 200.0, "has_azimuth": True, "azimuth": 180, "valid": True},
        ]
        value = evaluate_brightness(
            sensors,
            aggregation="mean",
            use_azimuth=True,
            prefer_unassigned=False,
            force_max=False,
            sun_azimuth=135.0,
            sun_valid=True,
        )
        self.assertTrue(math.isclose(value, 150.0, rel_tol=0.0, abs_tol=0.01))

    def test_roof_prefers_unassigned(self):
        sensors = [
            {"value": 100.0, "has_azimuth": True, "azimuth": 90, "valid": True},
            {"value": 200.0, "has_azimuth": True, "azimuth": 180, "valid": True},
            {"value": 50.0, "has_azimuth": False, "azimuth": 0, "valid": True},
        ]
        value = evaluate_brightness(
            sensors,
            aggregation="mean",
            use_azimuth=False,
            prefer_unassigned=True,
            force_max=True,
            sun_azimuth=0.0,
            sun_valid=False,
        )
        self.assertTrue(math.isclose(value, 50.0, rel_tol=0.0, abs_tol=0.01))

    def test_roof_fallback_all_max(self):
        sensors = [
            {"value": 100.0, "has_azimuth": True, "azimuth": 90, "valid": True},
            {"value": 200.0, "has_azimuth": True, "azimuth": 180, "valid": True},
        ]
        value = evaluate_brightness(
            sensors,
            aggregation="mean",
            use_azimuth=False,
            prefer_unassigned=True,
            force_max=True,
            sun_azimuth=0.0,
            sun_valid=False,
        )
        self.assertTrue(math.isclose(value, 200.0, rel_tol=0.0, abs_tol=0.01))


if __name__ == "__main__":
    unittest.main()
