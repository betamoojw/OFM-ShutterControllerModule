#include "PositionController.h"
#include "knxprod.h"
#include "CallContext.h"

PositionController::PositionController(uint8_t channelIndex)
    : _channelIndex(channelIndex)
{
    _hasSlat = ParamSHC_ChannelType == 1;
    _logPrefix = openknx.logger.buildPrefix("SC", _channelIndex + 1);
    _logPrefix += "PosCtrl";
}
const std::string &PositionController::logPrefix()
{
    return _logPrefix;
}

bool PositionController::hasSlat() const
{
    return _hasSlat;
}

void PositionController::setAutomaticPosition(uint8_t position)
{
    _setPosition = position;
}
void PositionController::setAutomaticSlat(uint8_t position)
{
    if (_hasSlat)
        _setSlat = position;
}
void PositionController::setManualPosition(uint8_t position)
{
    _startWaitForManualPositionFeedback = 0;
    _startWaitForManualSlatPositionFeedback = 0;

    _lastManaulPosition = position;
    // <Enumeration Text="Manuelle Bedienung über Aktor" Value="0" Id="%ENID%" />
    // <Enumeration Text="Modul sendet Auf/Ab zum Aktor" Value="1" Id="%ENID%" />
    // <Enumeration Text="Modul sendet 0/100% zum Aktor " Value="2" Id="%ENID%" />
    if (ParamSHC_ChannelUpDownTypeForManualControl != 0)
        _setPosition = position;
}
void PositionController::setManualSlat(uint8_t position)
{
    _startWaitForManualPositionFeedback = 0;
    _startWaitForManualSlatPositionFeedback = 0;

    _lastManualSlat = position;
    // <Enumeration Text="Manuelle Bedienung über Aktor" Value="0" Id="%ENID%" />
    // <Enumeration Text="Modul sendet Auf/Ab zum Aktor" Value="1" Id="%ENID%" />
    // <Enumeration Text="Modul sendet 0/100% zum Aktor " Value="2" Id="%ENID%" />
    if (_hasSlat && ParamSHC_ChannelUpDownTypeForManualControl != 0)
        _setSlat = position;
}
void PositionController::setManualStep(bool step)
{
    if (_hasSlat && ParamSHC_ChannelUpDownTypeForManualControl != 0)
    {
        logInfoP("Set step: %d", (int)step);
        KoSHC_CHShutterStopStepOutput.value(step, DPT_Step);
        _startWaitForManualPositionFeedback = millis();
        if (_startWaitForManualPositionFeedback == 0)
            _startWaitForManualPositionFeedback = 1;
        if (_hasSlat)
        {
            _startWaitForManualSlatPositionFeedback = millis();
            if (_startWaitForManualSlatPositionFeedback == 0)
                _startWaitForManualSlatPositionFeedback = 1;
        }
    }
}
void PositionController::setManualUpDown(bool up)
{
    _startWaitForManualPositionFeedback = 0;
    _startWaitForManualSlatPositionFeedback = 0;
    if (up)
    {
        _lastManaulPosition = 0;
        _lastManualSlat = 0;
    }
    else
    {
        _lastManaulPosition = 100;
        _lastManualSlat = 100;
    }
    // <Enumeration Text="Manuelle Bedienung über Aktor" Value="0" Id="%ENID%" />
    // <Enumeration Text="Modul sendet Auf/Ab zum Aktor" Value="1" Id="%ENID%" />
    // <Enumeration Text="Modul sendet 0/100% zum Aktor " Value="2" Id="%ENID%" />
    switch (ParamSHC_ChannelUpDownTypeForManualControl)
    {
    case 1:
        logInfoP("Set up: %d", (int)up);
        KoSHC_CHShutterUpDownOutput.value(up, DPT_UpDown);
        break;
    case 2:
        _setPosition = up ? 0 : 100;
        break;
    }
}
void PositionController::restoreLastManualPosition()
{
    if (_lastManaulPosition != 255)
        _setPosition = _lastManaulPosition;
    if (_lastManualSlat != 255)
        _setSlat = _lastManualSlat;
}
void PositionController::processInputKo(GroupObject &ko)
{
    switch (ko.asap())
    {
    case SHC_KoCHShutterPercentInput:
        if (millis() - _startWaitForManualPositionFeedback < 3000)
        {
            _lastManaulPosition = (uint8_t)ko.value(DPT_Scaling);
        }
        break;
    case SHC_KoCHShutterSlatInput:
        if (millis() - _startWaitForManualSlatPositionFeedback < 3000)
        {
            _lastManualSlat = (uint8_t)ko.value(DPT_Scaling);
        }
        break;
    }
}
void PositionController::control(const CallContext &callContext)
{
    auto now = millis();
    if (millis() - _startWaitForManualSlatPositionFeedback > 3000)
    {
        _startWaitForManualSlatPositionFeedback = 0;
    }
    if (_setPosition != 255)
    {
        logInfoP("Set position: %d", (int)_setPosition);
        KoSHC_CHShutterPercentOutput.value(_setPosition, DPT_Scaling);
        _setPosition = 255;
    }
    if (_setSlat != 255)
    {
        logInfoP("Set slat position: %d", (int)_setSlat);
        KoSHC_CHShutterSlatOutput.value(_setSlat, DPT_Scaling);
        _setSlat = 255;
    }
    if (callContext.diagnosticLog)
    {
        logInfoP("Last manual position: %d", (int)_lastManaulPosition);
        if (_hasSlat)
            logInfoP("Last manual slat position: %d", (int)_lastManualSlat);
    }
}
