#include "ModeNight.h"
#include "PositionController.h"
#include "BrightnessMeasurement.h"

namespace
{
    constexpr int16_t MinutesPerDay = 1440;
    constexpr int16_t Noon = 720; // the night cycle starts at 12:00
    constexpr uint16_t SwitchPointStep = SHC_CNightPoint2Mon - SHC_CNightPoint1Mon;

    // Position of a time on the night cycle (minutes since 12:00)
    int16_t cyclePosition(int16_t minuteOfDay)
    {
        int16_t minute = minuteOfDay % MinutesPerDay;
        if (minute < 0)
            minute += MinutesPerDay;
        return (minute + MinutesPerDay - Noon) % MinutesPerDay;
    }
}

#define NightPointByte(param, index) knx.paramByte(SHC_ParamCalcIndex((param) + (index) * SwitchPointStep))
#define NightPointWord(param, index) knx.paramWord(SHC_ParamCalcIndex((param) + (index) * SwitchPointStep))
#define NightPointBits(param, index) ((NightPointByte(param, index) & param##Mask) >> param##Shift)

const char *ModeNight::name() const
{
    return "Night";
}

uint8_t ModeNight::sceneNumber() const
{
    return 12;
}

void ModeNight::initGroupObjects()
{
    KoSHC_CNightActive.value(false, DPT_Switch);
    KoSHC_CNightLockActive.value(false, DPT_Switch);
    updateStageStatus();
    readSwitchPoints();
}

void ModeNight::updateStageStatus()
{
    // 0 = Tag, 1 = Vorstufe Abend, 2 = Nacht, 3 = Vorstufe Morgen
    const uint8_t status = _allowed ? _stage : 0;
    if (status == _stageStatus)
        return;
    _stageStatus = status;
    KoSHC_CNightStage.value(status, DPT_Value_1_Ucount);
}

bool ModeNight::windowOpenAllowed() const
{
    return ParamSHC_CNightWindowOpenAllowed;
}

bool ModeNight::windowTiltAllowed() const
{
    return ParamSHC_CNightWindowTiltAllowed;
}

const char* ModeNight::stageName(uint8_t stage)
{
    switch (stage)
    {
    case StageEvening:
        return "Evening";
    case StageNight:
        return "Night";
    case StageMorning:
        return "Morning";
    case StageDay:
        return "Day";
    default:
        return "None";
    }
}

void ModeNight::readSwitchPoints()
{
    if (!ParamSHC_CNightMigrated)
    {
        // The ETS has not converted the former night start/end settings yet, use them directly
        readLegacySwitchPoint(_switchPoints[0], StageNight, ParamSHC_CNightStartBehavior, knx.paramWord(SHC_ParamCalcIndex(SHC_CNightFromTime)), ParamSHC_CNightSunSet, ParamSHC_CNightSunSetElevationOffset);
        readLegacySwitchPoint(_switchPoints[1], StageDay, ParamSHC_CNightEndBehavior, knx.paramWord(SHC_ParamCalcIndex(SHC_CNightToTime)), ParamSHC_CNightSunRise, ParamSHC_CNightSunRiseElevationOffset);
        return;
    }
    const bool brightnessUsed = ParamSHC_CNightBrightnessSource != 0;
    for (uint8_t i = 0; i < NumberOfSwitchPoints; i++)
    {
        auto& switchPoint = _switchPoints[i];
        // ETS: Mon = bit 7 ... Sat = bit 2, Sun = bit 1; internal: bit = day of week (0 = Sunday)
        uint8_t dayBits = NightPointByte(SHC_CNightPoint1Mon, i);
        switchPoint.days = (dayBits & 0x02) ? 1 : 0;
        for (uint8_t day = 1; day <= 6; day++)
        {
            if (dayBits & (0x80 >> (day - 1)))
                switchPoint.days |= 1 << day;
        }
        switchPoint.stage = NightPointBits(SHC_CNightPoint1Stage, i);
        // evening and morning variants of trigger and brightness link share the same memory
        switchPoint.trigger = NightPointBits(SHC_CNightPoint1TriggerEvening, i);
        switchPoint.brightnessLink = brightnessUsed ? NightPointBits(SHC_CNightPoint1LinkEvening, i) : 0;
        switchPoint.condition = NightPointBits(SHC_CNightPoint1Condition, i);
        switchPoint.time = NightPointWord(SHC_CNightPoint1Time, i);
        switchPoint.timeOffset = NightPointWord(SHC_CNightPoint1TimeOffset, i);
        switchPoint.elevation = NightPointByte(SHC_CNightPoint1Elevation, i);
        switchPoint.lux = NightPointWord(SHC_CNightPoint1Lux, i);
        switchPoint.linkLux = NightPointWord(SHC_CNightPoint1AndLux, i);
        switchPoint.conditionTime = NightPointWord(SHC_CNightPoint1ConditionTime, i);
    }
}

void ModeNight::readLegacySwitchPoint(SwitchPoint& switchPoint, uint8_t stage, uint8_t behavior, uint16_t time, uint8_t sun, uint8_t elevation)
{
    // <Enumeration Text="Kein automatischer Start" Value="0" Id="%ENID%" />
    // <Enumeration Text="Uhrzeit" Value="1" Id="%ENID%" />
    // <Enumeration Text="Sonne" Value="2" Id="%ENID%" />
    // <Enumeration Text="Uhrzeit, Sonne (früheres Ereignis)" Value="3" Id="%ENID%" />
    // <Enumeration Text="Uhrzeit, Sonne (späteres Ereignis)" Value="4" Id="%ENID%" />
    switchPoint.days = 0x7F;
    switchPoint.stage = behavior == 0 ? StageNone : stage;
    switchPoint.time = time;
    switchPoint.conditionTime = time;
    switchPoint.elevation = elevation;
    if (behavior == 1)
    {
        switchPoint.trigger = 0;
        return;
    }
    // vor/bei/nach Sonnenuntergang -> über Horizont/bei/unter Horizont; at sunrise the other way round
    const bool evening = stage == StageNight;
    switch (sun)
    {
    case 0:
        switchPoint.trigger = evening ? 4 : 5;
        break;
    case 2:
        switchPoint.trigger = evening ? 5 : 4;
        break;
    default:
        switchPoint.trigger = 1;
        break;
    }
    switchPoint.condition = behavior == 3 ? 2 : (behavior == 4 ? 1 : 0);
}

bool ModeNight::isTimeReached(const CallContext &callContext, int16_t minuteOfDay, bool evening)
{
    if (!evening)
    {
        // morning stages are only evaluated until 11:59
        if (minuteOfDay < 0)
            minuteOfDay = 0;
        if (minuteOfDay >= Noon)
            minuteOfDay = Noon - 1;
    }
    return cyclePosition(callContext.minuteOfDay) >= cyclePosition(minuteOfDay);
}

bool ModeNight::readBrightness(const CallContext &callContext, float& lux)
{
    // <Enumeration Text="Nein" Value="0" Id="%ENID%" />
    // <Enumeration Text="Dämmerungssensor" Value="1" Id="%ENID%" />
    // <Enumeration Text="Helligkeitssensoren: Mittelwert" Value="2" Id="%ENID%" />
    // <Enumeration Text="Helligkeitssensoren: Maximum" Value="3" Id="%ENID%" />
    // <Enumeration Text="Helligkeitssensor in Fensterrichtung" Value="4" Id="%ENID%" />
    const uint8_t source = ParamSHC_CNightBrightnessSource;
    switch (source)
    {
    case 1:
        if (!ParamSHC_HasDuskInput || !KoSHC_DuskInput.initialized())
            return false;
        lux = (float)KoSHC_DuskInput.value(DPT_Value_Lux);
        return true;
    case 2:
    case 3:
    case 4:
    {
        if (callContext.measurementBrightness == nullptr)
            return false;
        auto brightnessMeasurement = static_cast<const BrightnessMeasurement *>(callContext.measurementBrightness);
        return brightnessMeasurement->getNightLux(
            source == 3 ? BrightnessAggregation::Max : BrightnessAggregation::Mean,
            source == 4 && callContext.windowHasAzimuth,
            callContext.windowAzimuth,
            lux);
    }
    default:
        return false;
    }
}

bool ModeNight::isBrightnessReached(const CallContext &callContext, uint16_t lux, bool evening, unsigned long& since, bool& valid)
{
    float value = 0;
    valid = readBrightness(callContext, value);
    const bool fulfilled = valid && (evening ? value < lux : value > lux);
    if (!fulfilled)
    {
        since = 0;
        return false;
    }
    if (since == 0)
        since = callContext.currentMillis;
    return callContext.currentMillis - since >= ParamSHC_CNightBrightnessDuration * 60000UL;
}

bool ModeNight::isTriggerReached(const CallContext &callContext, SwitchPoint& switchPoint, bool evening)
{
    // <Enumeration Text="Uhrzeit" Value="0" Id="%ENID%" />
    // <Enumeration Text="bei Sonnenuntergang/-aufgang" Value="1" Id="%ENID%" />
    // <Enumeration Text="Sonnenuntergang/-aufgang minus Zeitversatz" Value="2" Id="%ENID%" />
    // <Enumeration Text="Sonnenuntergang/-aufgang plus Zeitversatz" Value="3" Id="%ENID%" />
    // <Enumeration Text="Sonnenuntergang/-aufgang: über Horizont" Value="4" Id="%ENID%" />
    // <Enumeration Text="Sonnenuntergang/-aufgang: unter Horizont" Value="5" Id="%ENID%" />
    // <Enumeration Text="dunkler/heller als" Value="6" Id="%ENID%" />
    // <Enumeration Text="Ende/Beginn bürgerliche Dämmerung (6° unter Horizont)" Value="7" Id="%ENID%" />
    // <Enumeration Text="Ende/Beginn nautische Dämmerung (12° unter Horizont)" Value="8" Id="%ENID%" />
    if (switchPoint.trigger == 0)
        return isTimeReached(callContext, switchPoint.time, evening);
    if (switchPoint.trigger == 6)
    {
        bool valid = false;
        return ParamSHC_CNightBrightnessSource != 0 &&
               isBrightnessReached(callContext, switchPoint.lux, evening, switchPoint.luxSince, valid);
    }
    if (!callContext.timeAndSunValid)
        return false;
    if (switchPoint.trigger == 2 || switchPoint.trigger == 3)
    {
        auto sunTime = evening ? openknx.sun.sunSetLocalTime() : openknx.sun.sunRiseLocalTime();
        int16_t minute = sunTime.hour * 60 + sunTime.minute;
        minute += switchPoint.trigger == 2 ? -(int16_t)switchPoint.timeOffset : (int16_t)switchPoint.timeOffset;
        return isTimeReached(callContext, minute, evening);
    }
    double elevation;
    switch (switchPoint.trigger)
    {
    case 4:
        elevation = switchPoint.elevation;
        break;
    case 5:
        elevation = -(double)switchPoint.elevation;
        break;
    case 7:
        elevation = -6;
        break;
    case 8:
        elevation = -12;
        break;
    default:
        elevation = 0;
        break;
    }
    return evening ? callContext.elevation <= elevation : callContext.elevation >= elevation;
}

bool ModeNight::isSwitchPointReached(const CallContext &callContext, SwitchPoint& switchPoint)
{
    const bool evening = switchPoint.stage <= StageNight;
    // morning stages are only evaluated between 00:00 and 11:59
    if (!evening && cyclePosition(callContext.minuteOfDay) < Noon)
        return false;

    // evening stages use the weekday on which the night cycle started (also after midnight),
    // the holiday input applies to the current day
    uint8_t dayOfWeek = callContext.dayOfWeek;
    if (evening && callContext.minuteOfDay < Noon)
        dayOfWeek = (dayOfWeek + 6) % 7;
    else if (callContext.todayLikeSunday)
        dayOfWeek = 0;
    if (!(switchPoint.days & (1 << dayOfWeek)))
        return false;

    bool reached = isTriggerReached(callContext, switchPoint, evening);
    if (switchPoint.brightnessLink != 0)
    {
        bool valid = false;
        const bool brightness = isBrightnessReached(callContext, switchPoint.linkLux, evening, switchPoint.linkLuxSince, valid);
        if (switchPoint.brightnessLink == 1)
            reached = reached && (brightness || !valid); // missing brightness must not block the switch point
        else
            reached = reached || brightness;
    }
    switch (switchPoint.condition)
    {
    case 1: // frühestens um
        reached = reached && isTimeReached(callContext, switchPoint.conditionTime, evening);
        break;
    case 2: // spätestens um
        reached = reached || isTimeReached(callContext, switchPoint.conditionTime, evening);
        break;
    }
    return reached;
}

bool ModeNight::isStageReached(const CallContext &callContext, uint8_t stage)
{
    bool reached = false;
    for (auto& switchPoint : _switchPoints)
    {
        if (switchPoint.stage != stage)
            continue;
        // evaluate all switch points, the brightness durations must be tracked continuously
        if (isSwitchPointReached(callContext, switchPoint))
        {
            if (callContext.diagnosticLog)
                logInfoP("Switch point for stage %s reached", stageName(stage));
            reached = true;
        }
    }
    return reached;
}

void ModeNight::fireStage(uint8_t stage, bool silent)
{
    logInfoP("Stage %s reached%s", stageName(stage), silent ? " (restored without movement)" : "");
    _fired[stage] = true;
    switch (stage)
    {
    case StageEvening:
    case StageNight:
        if (stage == StageNight)
            _fired[StageEvening] = true;
        _allowed = true;
        _stage = stage;
        _pendingStage = silent ? StageNone : stage;
        break;
    case StageMorning:
        if (_allowed)
        {
            _stage = stage;
            _pendingStage = silent ? StageNone : stage;
        }
        break;
    case StageDay:
        _fired[StageMorning] = true;
        _allowed = false;
        _stage = StageNone;
        _pendingStage = StageNone;
        break;
    }
    updateStageStatus();
}

void ModeNight::evaluate(const CallContext &callContext, bool reconstruct)
{
    bool reached[StageDay + 1] = {};
    for (uint8_t stage = StageEvening; stage <= StageDay; stage++)
        reached[stage] = isStageReached(callContext, stage);

    // After a restart in the morning half the state is only restored, a shutter opened by hand must not close again
    const bool morningHalf = cyclePosition(callContext.minuteOfDay) >= Noon;
    const bool silent = reconstruct && morningHalf;

    if (!_fired[StageMorning] && !_fired[StageDay])
    {
        if (reached[StageNight] && !_fired[StageNight])
            fireStage(StageNight, silent);
        else if (reached[StageEvening] && !_fired[StageEvening])
            fireStage(StageEvening, silent);
    }
    if (morningHalf)
    {
        if (reached[StageDay] && !_fired[StageDay])
            fireStage(StageDay, silent);
        else if (reached[StageMorning] && !_fired[StageMorning])
            fireStage(StageMorning, silent);
    }
}

bool ModeNight::allowed(const CallContext &callContext)
{
    if (callContext.timeAndSunValid && callContext.minuteChanged)
    {
        const bool reconstruct = !_cycleInitialized;
        if (_cycleInitialized && _lastMinuteOfDay < Noon && callContext.minuteOfDay >= Noon)
        {
            logInfoP("New night cycle");
            for (auto& fired : _fired)
                fired = false;
        }
        _lastMinuteOfDay = callContext.minuteOfDay;
        _cycleInitialized = true;
        evaluate(callContext, reconstruct);
    }
    if (callContext.diagnosticLog)
    {
        logInfoP("Night active: %s, stage: %s, pending: %s", _allowed ? "true" : "false", stageName(_stage), stageName(_pendingStage));
        logInfoP("Fired: evening %d, night %d, morning %d, day %d", (int)_fired[StageEvening], (int)_fired[StageNight], (int)_fired[StageMorning], (int)_fired[StageDay]);
    }
    if (KoSHC_CNightLockActive.value(DPT_Switch))
    {
        if (callContext.diagnosticLog)
            logInfoP("Lock KO active");
        return false;
    }
    _yieldedToShading = _allowed && ParamSHC_CNightShadingPrecedence &&
                        (_stage == StageEvening || _stage == StageMorning) &&
                        callContext.shadingAllowedLastCycle;
    if (_yieldedToShading)
    {
        if (callContext.diagnosticLog)
            logInfoP("Shading has precedence in stage %s", stageName(_stage));
        return false;
    }
    return _allowed;
}

uint8_t ModeNight::stageAction(uint8_t stage)
{
    // <Enumeration Text="Nein" Value="0" Id="%ENID%" />
    // <Enumeration Text="Nur schließen / Nur öffnen" Value="1" Id="%ENID%" />
    // <Enumeration Text="Öffnen und Schließen" Value="2" Id="%ENID%" />
    if (!ParamSHC_CNightMigrated)
    {
        if (stage == StageNight)
            return ParamSHC_CNightStartPositionEnabled ? 1 : 0;
        if (stage == StageDay)
            return ParamSHC_CNightStopPositionEnabled ? 1 : 0;
    }
    switch (stage)
    {
    case StageEvening:
        return ParamSHC_CNightStage1Action;
    case StageNight:
        return ParamSHC_CNightStage2Action;
    case StageMorning:
        return ParamSHC_CNightStage3Action;
    case StageDay:
        return ParamSHC_CNightStage4Action;
    default:
        return 0;
    }
}

uint8_t ModeNight::stagePosition(uint8_t stage)
{
    switch (stage)
    {
    case StageEvening:
        return ParamSHC_CNightStage1Position;
    case StageNight:
        return ParamSHC_CNightStartPosition;
    case StageMorning:
        return ParamSHC_CNightStage3Position;
    default:
        return ParamSHC_CNightStopPosition;
    }
}

uint8_t ModeNight::stageSlat(uint8_t stage)
{
    switch (stage)
    {
    case StageEvening:
        return ParamSHC_CNightStage1Slat;
    case StageNight:
        return ParamSHC_CNightStartSlatPosition;
    case StageMorning:
        return ParamSHC_CNightStage3Slat;
    default:
        return ParamSHC_CNightStopSlatPosition;
    }
}

void ModeNight::applyStage(uint8_t stage, PositionController& positionController)
{
    const uint8_t action = stageAction(stage);
    if (action == 0)
    {
        logDebugP("Stage %s: no action", stageName(stage));
        return;
    }
    const bool closing = stage <= StageNight;
    const uint8_t position = stagePosition(stage);
    // without actor feedback the current position is unknown, the stage always moves
    const bool onlyStageDirection = action == 1 && KoSHC_CShutterPercentInput.initialized();
    const uint8_t currentPosition = positionController.position();
    const bool movePosition = !onlyStageDirection || (closing ? position > currentPosition : position < currentPosition);
    if (movePosition)
    {
        logDebugP("Stage %s: set position %d", stageName(stage), (int)position);
        positionController.setAutomaticPositionAndStoreForRestore(position);
    }
    else
    {
        logInfoP("Stage %s: position %d not set, current position %d is already further %s", stageName(stage), (int)position, (int)currentPosition, closing ? "closed" : "open");
    }
    if (!positionController.hasSlat())
        return;
    const uint8_t slat = stageSlat(stage);
    const bool onlySlatDirection = action == 1 && KoSHC_CShutterSlatInput.initialized();
    const uint8_t currentSlat = positionController.slat();
    if (movePosition || !onlySlatDirection || (closing ? slat > currentSlat : slat < currentSlat))
    {
        logDebugP("Stage %s: set slat %d", stageName(stage), (int)slat);
        positionController.setAutomaticSlatAndStoreForRestore(slat);
    }
}

void ModeNight::start(const CallContext &callContext, const ModeBase *previous, PositionController& positionController)
{
    KoSHC_CNightActive.value(true, DPT_Switch);
    logDebugP("Start in stage %s, pending %s", stageName(_stage), stageName(_pendingStage));
}

void ModeNight::control(const CallContext &callContext, PositionController& positionController)
{
    if (_pendingStage == StageNone)
        return;
    applyStage(_pendingStage, positionController);
    _pendingStage = StageNone;
}

void ModeNight::stop(const CallContext &callContext, const ModeBase *next, PositionController& positionController)
{
    KoSHC_CNightActive.value(false, DPT_Switch);
    if (next == (const ModeBase *)callContext.modeManual)
        return;
    if (_yieldedToShading)
    {
        logInfoP("Shading takes over in stage %s", stageName(_stage));
        return;
    }
    // <Enumeration Text="Tag-Position anfahren" Value="0" Id="%ENID%" />
    // <Enumeration Text="keine Aktion" Value="1" Id="%ENID%" />
    if (_allowed && KoSHC_CNightLockActive.value(DPT_Switch) && ParamSHC_CNightLockBehavior == 1)
    {
        logInfoP("Locked, no action");
        return;
    }
    applyStage(StageDay, positionController);
}

void ModeNight::processInputKo(GroupObject &ko, PositionController& positionController)
{
    switch (SHC_KoCalcIndex(ko.asap()))
    {
    case SHC_KoCNight:
        if (ko.value(DPT_Switch))
        {
            // same as stage "Nacht": a following "Vorstufe Abend" must not open again
            _allowed = true;
            _stage = StageNight;
            _fired[StageEvening] = true;
            _fired[StageNight] = true;
            _pendingStage = StageNight;
        }
        else
        {
            _allowed = false;
            _stage = StageNone;
            _pendingStage = StageNone;
        }
        updateStageStatus();
        break;
    case SHC_KoCNightLock:
        KoSHC_CNightLockActive.value(ko.value(DPT_Switch), DPT_Switch);
        break;
    default:
        break;
    }
}

bool ModeNight::isNight() const
{
    return _allowed;
}
