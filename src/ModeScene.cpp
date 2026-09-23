#include "ModeScene.h"
#include "PositionController.h"
#include "ShutterControllerChannel.h"

ModeScene::ModeScene(ShutterControllerChannel &channel) : _channel(channel)
{
}

const char *ModeScene::name() const
{
    return "Scene";
}

uint8_t ModeScene::sceneNumber() const
{
    return 20 + _activeSlot;
}

void ModeScene::initGroupObjects()
{
}
bool ModeScene::windowOpenAllowed() const
{
    return true;
}
bool ModeScene::windowTiltAllowed() const
{
    return true;
}

ModeScene::SceneRow ModeScene::readSceneRow(uint8_t slot) const
{
    switch (slot)
    {
    case 1:
        return {ParamSHC_CScene1Active, ParamSHC_CScene1Number, ParamSHC_CScene1Storable, ParamSHC_CScene1HeightEnabled, (uint8_t)ParamSHC_CScene1Height, ParamSHC_CScene1SlatEnabled, (uint8_t)ParamSHC_CScene1Slat, (uint8_t)ParamSHC_CScene1Lock27, ParamSHC_CScene1Delay, ParamSHC_CScene1Hold};
    case 2:
        return {ParamSHC_CScene2Active, ParamSHC_CScene2Number, ParamSHC_CScene2Storable, ParamSHC_CScene2HeightEnabled, (uint8_t)ParamSHC_CScene2Height, ParamSHC_CScene2SlatEnabled, (uint8_t)ParamSHC_CScene2Slat, (uint8_t)ParamSHC_CScene2Lock27, ParamSHC_CScene2Delay, ParamSHC_CScene2Hold};
    case 3:
        return {ParamSHC_CScene3Active, ParamSHC_CScene3Number, ParamSHC_CScene3Storable, ParamSHC_CScene3HeightEnabled, (uint8_t)ParamSHC_CScene3Height, ParamSHC_CScene3SlatEnabled, (uint8_t)ParamSHC_CScene3Slat, (uint8_t)ParamSHC_CScene3Lock27, ParamSHC_CScene3Delay, ParamSHC_CScene3Hold};
    case 4:
        return {ParamSHC_CScene4Active, ParamSHC_CScene4Number, ParamSHC_CScene4Storable, ParamSHC_CScene4HeightEnabled, (uint8_t)ParamSHC_CScene4Height, ParamSHC_CScene4SlatEnabled, (uint8_t)ParamSHC_CScene4Slat, (uint8_t)ParamSHC_CScene4Lock27, ParamSHC_CScene4Delay, ParamSHC_CScene4Hold};
    case 5:
        return {ParamSHC_CScene5Active, ParamSHC_CScene5Number, ParamSHC_CScene5Storable, ParamSHC_CScene5HeightEnabled, (uint8_t)ParamSHC_CScene5Height, ParamSHC_CScene5SlatEnabled, (uint8_t)ParamSHC_CScene5Slat, (uint8_t)ParamSHC_CScene5Lock27, ParamSHC_CScene5Delay, ParamSHC_CScene5Hold};
    case 6:
        return {ParamSHC_CScene6Active, ParamSHC_CScene6Number, ParamSHC_CScene6Storable, ParamSHC_CScene6HeightEnabled, (uint8_t)ParamSHC_CScene6Height, ParamSHC_CScene6SlatEnabled, (uint8_t)ParamSHC_CScene6Slat, (uint8_t)ParamSHC_CScene6Lock27, ParamSHC_CScene6Delay, ParamSHC_CScene6Hold};
    case 7:
        return {ParamSHC_CScene7Active, ParamSHC_CScene7Number, ParamSHC_CScene7Storable, ParamSHC_CScene7HeightEnabled, (uint8_t)ParamSHC_CScene7Height, ParamSHC_CScene7SlatEnabled, (uint8_t)ParamSHC_CScene7Slat, (uint8_t)ParamSHC_CScene7Lock27, ParamSHC_CScene7Delay, ParamSHC_CScene7Hold};
    case 8:
        return {ParamSHC_CScene8Active, ParamSHC_CScene8Number, ParamSHC_CScene8Storable, ParamSHC_CScene8HeightEnabled, (uint8_t)ParamSHC_CScene8Height, ParamSHC_CScene8SlatEnabled, (uint8_t)ParamSHC_CScene8Slat, (uint8_t)ParamSHC_CScene8Lock27, ParamSHC_CScene8Delay, ParamSHC_CScene8Hold};
    case 9:
        return {ParamSHC_CScene9Active, ParamSHC_CScene9Number, ParamSHC_CScene9Storable, ParamSHC_CScene9HeightEnabled, (uint8_t)ParamSHC_CScene9Height, ParamSHC_CScene9SlatEnabled, (uint8_t)ParamSHC_CScene9Slat, (uint8_t)ParamSHC_CScene9Lock27, ParamSHC_CScene9Delay, ParamSHC_CScene9Hold};
    case 10:
        return {ParamSHC_CScene10Active, ParamSHC_CScene10Number, ParamSHC_CScene10Storable, ParamSHC_CScene10HeightEnabled, (uint8_t)ParamSHC_CScene10Height, ParamSHC_CScene10SlatEnabled, (uint8_t)ParamSHC_CScene10Slat, (uint8_t)ParamSHC_CScene10Lock27, ParamSHC_CScene10Delay, ParamSHC_CScene10Hold};
    case 11:
        return {ParamSHC_CScene11Active, ParamSHC_CScene11Number, ParamSHC_CScene11Storable, ParamSHC_CScene11HeightEnabled, (uint8_t)ParamSHC_CScene11Height, ParamSHC_CScene11SlatEnabled, (uint8_t)ParamSHC_CScene11Slat, (uint8_t)ParamSHC_CScene11Lock27, ParamSHC_CScene11Delay, ParamSHC_CScene11Hold};
    case 12:
        return {ParamSHC_CScene12Active, ParamSHC_CScene12Number, ParamSHC_CScene12Storable, ParamSHC_CScene12HeightEnabled, (uint8_t)ParamSHC_CScene12Height, ParamSHC_CScene12SlatEnabled, (uint8_t)ParamSHC_CScene12Slat, (uint8_t)ParamSHC_CScene12Lock27, ParamSHC_CScene12Delay, ParamSHC_CScene12Hold};
    case 13:
        return {ParamSHC_CScene13Active, ParamSHC_CScene13Number, ParamSHC_CScene13Storable, ParamSHC_CScene13HeightEnabled, (uint8_t)ParamSHC_CScene13Height, ParamSHC_CScene13SlatEnabled, (uint8_t)ParamSHC_CScene13Slat, (uint8_t)ParamSHC_CScene13Lock27, ParamSHC_CScene13Delay, ParamSHC_CScene13Hold};
    case 14:
        return {ParamSHC_CScene14Active, ParamSHC_CScene14Number, ParamSHC_CScene14Storable, ParamSHC_CScene14HeightEnabled, (uint8_t)ParamSHC_CScene14Height, ParamSHC_CScene14SlatEnabled, (uint8_t)ParamSHC_CScene14Slat, (uint8_t)ParamSHC_CScene14Lock27, ParamSHC_CScene14Delay, ParamSHC_CScene14Hold};
    case 15:
        return {ParamSHC_CScene15Active, ParamSHC_CScene15Number, ParamSHC_CScene15Storable, ParamSHC_CScene15HeightEnabled, (uint8_t)ParamSHC_CScene15Height, ParamSHC_CScene15SlatEnabled, (uint8_t)ParamSHC_CScene15Slat, (uint8_t)ParamSHC_CScene15Lock27, ParamSHC_CScene15Delay, ParamSHC_CScene15Hold};
    case 16:
        return {ParamSHC_CScene16Active, ParamSHC_CScene16Number, ParamSHC_CScene16Storable, ParamSHC_CScene16HeightEnabled, (uint8_t)ParamSHC_CScene16Height, ParamSHC_CScene16SlatEnabled, (uint8_t)ParamSHC_CScene16Slat, (uint8_t)ParamSHC_CScene16Lock27, ParamSHC_CScene16Delay, ParamSHC_CScene16Hold};
    default:
        return {false, 0, false, false, 0, false, 0, 0, 0, false};
    }
}

uint8_t ModeScene::findSlotForSceneNumber(uint8_t knxSceneNumber) const
{
    for (uint8_t slot = 1; slot <= 16; slot++)
    {
        auto row = readSceneRow(slot);
        if (row.active && row.number == knxSceneNumber)
            return slot;
    }
    return 0;
}

// Lock value: shading = lock/9, night = (lock/3)%3, hand = lock%3
// 0 = unveraendert, 1 = sperren, 2 = freigeben
void ModeScene::applyLock(uint8_t lockCode, bool armRestore)
{
    uint8_t hand = lockCode % 3;
    uint8_t night = (lockCode / 3) % 3;
    uint8_t shading = lockCode / 9;

    if (shading != 0)
    {
        bool locked = (shading == 1);
        _channel.activateShadingControl(!locked);
        _shadingSetByScene = locked;
        _shadingTouched = armRestore;
    }
    if (night != 0)
    {
        bool locked = (night == 1);
        KoSHC_CNightLockActive.value(locked, DPT_Switch);
        _nightSetByScene = locked;
        _nightTouched = armRestore;
    }
    if (hand != 0)
    {
        bool locked = (hand == 1);
        KoSHC_CManualLockActive.value(locked, DPT_Switch);
        _manualSetByScene = locked;
        _manualTouched = armRestore;
    }
}

void ModeScene::restoreLocks()
{
    if (_shadingTouched)
    {
        bool current = !_channel.shadingControlActive();
        if (current == _shadingSetByScene)
            _channel.activateShadingControl(!_shadingBefore);
        _shadingTouched = false;
    }
    if (_nightTouched)
    {
        bool current = KoSHC_CNightLockActive.value(DPT_Switch);
        if (current == _nightSetByScene)
            KoSHC_CNightLockActive.value(_nightBefore, DPT_Switch);
        _nightTouched = false;
    }
    if (_manualTouched)
    {
        bool current = KoSHC_CManualLockActive.value(DPT_Switch);
        if (current == _manualSetByScene)
            KoSHC_CManualLockActive.value(_manualBefore, DPT_Switch);
        _manualTouched = false;
    }
}

void ModeScene::applyScene(uint8_t slot, PositionController &positionController)
{
    auto row = readSceneRow(slot);
    if (row.heightEnabled)
    {
        uint8_t height = _learned[slot].valid ? _learned[slot].height : row.height;
        positionController.setAutomaticPosition(height);
    }
    if (positionController.hasSlat() && row.slatEnabled)
    {
        uint8_t slat = _learned[slot].valid ? _learned[slot].slat : row.slat;
        positionController.setAutomaticSlat(slat);
    }
    applyLock(row.lock, row.hold);
    _activeSlot = slot;
    _releaseAfterApply = !row.hold;
    logInfoP("Scene %u applied (KNX scene %u), %s", (unsigned int)slot, (unsigned int)row.number,
             row.hold ? "holding the automatics back" : "automatics released again");
}

bool ModeScene::allowed(const CallContext &callContext)
{
    if (_releaseAfterApply)
    {
        // The scene has been applied in the previous cycle and does not hold
        // the automatics back, so release the mode again.
        _releaseAfterApply = false;
        _activeSlot = 0;
    }
    if (_pendingSlot != 0 && (long)(callContext.currentMillis - _pendingUntil) >= 0)
    {
        _activeSlot = _pendingSlot;
        _pendingSlot = 0;
        _requestStart = true;
        _channel.notifySceneRequested();
    }
    return _activeSlot != 0;
}

void ModeScene::start(const CallContext &callContext, const ModeBase *previous, PositionController &positionController)
{
    _shadingBefore = !_channel.shadingControlActive();
    _nightBefore = KoSHC_CNightLockActive.value(DPT_Switch);
    _manualBefore = KoSHC_CManualLockActive.value(DPT_Switch);
    _shadingTouched = false;
    _nightTouched = false;
    _manualTouched = false;

    applyScene(_activeSlot, positionController);
    _requestStart = false;
}

void ModeScene::control(const CallContext &callContext, PositionController &positionController)
{
    if (_requestStart)
    {
        _requestStart = false;
        applyScene(_activeSlot, positionController);
    }
}

void ModeScene::stop(const CallContext &callContext, const ModeBase *next, PositionController &positionController)
{
    restoreLocks();
    _activeSlot = 0;
    _pendingSlot = 0;
    _requestStart = false;
    _releaseAfterApply = false;
}

void ModeScene::processInputKo(GroupObject &ko, PositionController &positionController)
{
    switch (SHC_KoCalcIndex(ko.asap()))
    {
    case SHC_KoCScene:
    {
        uint8_t raw = ko.valueRef()[0];
        bool learn = (raw & 0x80) != 0;
        uint8_t knxSceneNumber = (raw & 0x3F) + 1;
        uint8_t slot = findSlotForSceneNumber(knxSceneNumber);
        if (slot == 0)
            return;
        auto row = readSceneRow(slot);
        if (learn)
        {
            if (row.storable)
            {
                _learned[slot].valid = true;
                _learned[slot].height = positionController.position();
                _learned[slot].slat = positionController.hasSlat() ? positionController.slat() : 0;
                logInfoP("Learned scene %u (slot %u): height=%u slat=%u", (unsigned int)knxSceneNumber, (unsigned int)slot, (unsigned int)_learned[slot].height, (unsigned int)_learned[slot].slat);
                openknx.flash.save();
            }
            return;
        }
        // Recall: a new request always replaces one still waiting for its delay
        _pendingSlot = 0;
        _pendingUntil = 0;
        if (row.delaySeconds == 0)
        {
            _channel.notifySceneRequested();
            _activeSlot = slot;
            _requestStart = true;
        }
        else
        {
            _pendingSlot = slot;
            _pendingUntil = millis() + (unsigned long)row.delaySeconds * 1000UL;
        }
        break;
    }
    case SHC_KoCManualPercent:
    case SHC_KoCManualSlatPercent:
    case SHC_KoCManualStepStop:
    case SHC_KoCManualUpDown:
    case SHC_KoCManualUpDownWithoutSpecialFunction:
        // A manual command discards a scene still waiting for its delay.
        _pendingSlot = 0;
        _pendingUntil = 0;
        break;
    default:
        break;
    }
}

bool ModeScene::hasLearnedValue(uint8_t slot) const
{
    if (slot < 1 || slot > 16)
        return false;
    return _learned[slot].valid;
}

uint8_t ModeScene::learnedHeight(uint8_t slot) const
{
    return _learned[slot].height;
}

uint8_t ModeScene::learnedSlat(uint8_t slot) const
{
    return _learned[slot].slat;
}

void ModeScene::loadLearnedValue(uint8_t slot, uint8_t height, uint8_t slat)
{
    if (slot < 1 || slot > 16)
        return;
    _learned[slot].valid = true;
    _learned[slot].height = height;
    _learned[slot].slat = slat;
}
