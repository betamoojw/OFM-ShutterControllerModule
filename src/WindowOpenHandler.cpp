#include "ModeBase.h"
#include "CallContext.h"
#include "WindowOpenHandler.h"
#include "PositionController.h"

#ifdef SHC_CWindowOpenPosition2
// redefine SHC_ParamCalcIndex to add offset for Window Mode 2
#undef SHC_ParamCalcIndex
#define SHC_ParamCalcIndex(index) (index + SHC_ParamBlockOffset + _channelIndex * SHC_ParamBlockSize + (SHC_CWindowOpenPosition2 - SHC_CWindowOpenPosition1) * (_index - 1))
#endif

WindowOpenHandler::WindowOpenHandler(uint8_t _channelIndex, uint8_t index, bool isTilt)
    : _channelIndex(_channelIndex), _index(index), _isTilt(isTilt)
{
    if (_isTilt)
        _name = "WindowTilt";
    else
        _name = "WindowOpen";
}

int16_t WindowOpenHandler::koChannelOffset()
{
    return (_index - 1) * (SHC_KoCWindowOpenModeActive2 - SHC_KoCWindowOpenModeActive1);
}

GroupObject &WindowOpenHandler::getKo(uint8_t ko)
{
    return knx.getGroupObject(SHC_KoCalcNumber(ko) + koChannelOffset());
}

const char *WindowOpenHandler::name() const
{
    return _name.c_str();
}

const std::string &WindowOpenHandler::logPrefix() const
{
    return _name;
}

uint8_t WindowOpenHandler::sceneNumber() const
{
    if (_isTilt)
        return 13;
    else
        return 14;
}

void WindowOpenHandler::initGroupObjects()
{
    getKo(SHC_KoCWindowOpenModeActive1).value(false, DPT_Switch);
    getKo(SHC_KoCWindowOpenLockActive1).value(false, DPT_Switch);
}

bool WindowOpenHandler::allowed(const CallContext &callContext)
{
    if (_isTilt)
    {
        if (!callContext.modeCurrentActive->windowTiltAllowed())
        {
            if (callContext.diagnosticLog)
                logInfoP("Window tilt mode not allowed by current active mode");
            return false;
        }
    }
    else
    {
        if (!callContext.modeCurrentActive->windowOpenAllowed())
        {
            if (callContext.diagnosticLog)
                logInfoP("Window open mode not allowed by current active mode");
            return false;
        }
    }
    if (_recalcAllowed || callContext.diagnosticLog)
    {
        _allowed = true;
        if (ParamSHC_CWindowOpenPositionControl1 == 0 && (callContext.hasSlat != 1 || ParamSHC_CWindowOpenSlatPositionControl1 == 0))
        {
            _allowed = false;
            if (callContext.diagnosticLog)
                logInfoP("whether shutter nor slat position configured");
        }
        if (!getKo(SHC_KoCWindowOpenOpened1).value(DPT_OpenClose))
        {
            _allowed = false;
            if (callContext.diagnosticLog)
                logInfoP("Window closed");
        }
        if (getKo(SHC_KoCWindowOpenLockActive1).value(DPT_Switch))
        {
            _allowed = false;
            if (callContext.diagnosticLog)
                logInfoP("Lock KO active");
        }
        if (_deactivatedWhileOpen)
        {
            _allowed = false;
            if (callContext.diagnosticLog)
                logInfoP("Deactivated while window open because of manual mode");
        }
    }
    return _allowed;
}
void WindowOpenHandler::start(const CallContext &callContext, const WindowOpenHandler *previous, PositionController &positionController)
{
    getKo(SHC_KoCWindowOpenModeActive1).value(true, DPT_Switch);
    auto positionControl = ParamSHC_CWindowOpenPositionControl1;
    // 	<Enumeration Text="Nein" Value="0" Id="%ENID%" />
    //  <Enumeration Text="Nur hochfahren" Value="1" Id="%ENID%" />
    //  <Enumeration Text="Immer" Value="2" Id="%ENID%" />
    if (positionControl > 0)
    {
        auto position = ParamSHC_CWindowOpenPosition1;
        if (callContext.diagnosticLog)
            logInfoP("Set position limit to %d%%", (int)(position));
        positionController.setPositionLowerLimit(position, positionControl == 2);
    }
    if (positionController.hasSlat())
    {
        auto slatPositionControl = ParamSHC_CWindowOpenSlatPositionControl1;
        // 	<Enumeration Text="Nein" Value="0" Id="%ENID%" />
        //  <Enumeration Text="Nur hochfahren" Value="1" Id="%ENID%" />
        //  <Enumeration Text="Immer" Value="2" Id="%ENID%" />
        if (slatPositionControl > 0)
        {
            auto slatPosition = ParamSHC_CWindowOpenSlatPosition1;
            if (callContext.diagnosticLog)
                logInfoP("Set slat limit to %d%%", (int)(slatPosition));
            positionController.setSlatLowerLimit(slatPosition, slatPositionControl == 2);
        }
    }
}

void WindowOpenHandler::stop(const CallContext &callContext, const WindowOpenHandler *next, PositionController &positionController)
{
    positionController.resetPositionLowerLimit();
    if (positionController.hasSlat())
        positionController.resetSlatLowerLimit();
    getKo(SHC_KoCWindowOpenModeActive1).value(false, DPT_Switch);
}
void WindowOpenHandler::processInputKo(GroupObject &ko, PositionController &positionController)
{
    switch (ko.asap() - SHC_KoBlockOffset - koChannelOffset())
    {
    case SHC_KoCWindowOpenOpened1:
        if (ko.value(DPT_OpenClose))
            logInfoP("opened");
        else
        {
            logInfoP("closed");
            _deactivatedWhileOpen = false;
        }
        _recalcAllowed = true;
        break;
    case SHC_KoCWindowOpenLock1:
        _recalcAllowed = true;
        break;
    }
}