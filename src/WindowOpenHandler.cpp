#include "ModeBase.h"
#include "CallContext.h"
#include "WindowOpenHandler.h"
#include "PositionController.h"

#ifdef SHC_CWindowOpenPosition2
// redefine SHC_ParamCalcIndex to add offset for Window Mode 2
#undef SHC_ParamCalcIndex
#define SHC_ParamCalcIndex(index) (index + SHC_ParamBlockOffset + _channelIndex * SHC_ParamBlockSize + (SHC_CWindowOpenPosition2 - SHC_CWindowOpenPosition1) * (_index - 1))

// redefine SHC_KoCalcNumber to add offset for Window Mode 2
#undef SHC_KoCalcNumber
#define SHC_KoCalcNumber(index) (index + SHC_KoBlockOffset + _channelIndex * SHC_KoBlockSize + (_index - 1) * (SHC_KoCWindowOpenModeActive2 - SHC_KoCWindowOpenModeActive1))

// redefine SHC_KoCalcIndex to add offset for Window Mode 2
#define SHC_KoCalcIndex(number) ((number >= SHC_KoCalcNumber(0) && number < SHC_KoCalcNumber(SHC_KoBlockSize)) ? (number - SHC_KoBlockOffset - (_index - 1) * (SHC_KoCWindowOpenModeActive2 - SHC_KoCWindowOpenModeActive1)) % SHC_KoBlockSize : -1)

#endif

WindowOpenHandler::WindowOpenHandler(uint8_t _channelIndex, uint8_t index, bool isTilt)
    : _channelIndex(_channelIndex), _index(index), _isTiltHandler(isTilt)
{
    if (_isTiltHandler)
        _name = "WindowTilt";
    else
        _name = "WindowOpen";
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
    if (_isTiltHandler)
        return 13;
    else
        return 14;
}

void WindowOpenHandler::initGroupObjects()
{
    KoSHC_CWindowOpenModeActive1.value(false, DPT_Switch);
    KoSHC_CWindowOpenLockActive1.value(false, DPT_Switch);
}

bool WindowOpenHandler::allowed(const CallContext &callContext)
{
    if (_isTiltHandler)
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
        if (!KoSHC_CWindowOpenOpened1.value(DPT_OpenClose))
        {
            _allowed = false;
            if (callContext.diagnosticLog)
                logInfoP("Window closed");
        }
        if (KoSHC_CWindowOpenLockActive1.value(DPT_Switch))
        {
            _allowed = false;
            if (callContext.diagnosticLog)
                logInfoP("Lock KO active");
        }
    }
    return _allowed;
}
void WindowOpenHandler::start(const CallContext &callContext, const WindowOpenHandler *previous, PositionController &positionController)
{
    KoSHC_CWindowOpenModeActive1.value(true, DPT_Switch);

    // Check lockout prevention
    bool manualOrIdleMode = callContext.modeCurrentActive == (ModeBase *)callContext.modeManual || callContext.modeCurrentActive == (ModeBase *)callContext.modeIdle;
    if (!_isTiltHandler && 
        manualOrIdleMode && 
        ParamSHC_CWindowOpenLockOut1 != 10 &&
        positionController.targetPosition() <= ParamSHC_CWindowOpenLockOut1 * 10)
    {
        auto position = ParamSHC_CWindowOpenLockOut1 * 10;
        if (callContext.diagnosticLog)
            logInfoP("Set position limit to %d%% for lockout prevention", (int)(position));
        positionController.setPositionLowerLimit(position, false);
    }
    else
    {

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
    KoSHC_CWindowOpenModeActive1.value(false, DPT_Switch);
}
void WindowOpenHandler::processInputKo(GroupObject &ko, PositionController &positionController)
{
    switch (SHC_KoCalcIndex(ko.asap()))
    {
    case SHC_KoCWindowOpenOpened1:
        if (ko.value(DPT_OpenClose))
            logInfoP("opened");
        else
            logInfoP("closed");
         
        _recalcAllowed = true;
        break;
    case SHC_KoCWindowOpenLock1:
        _recalcAllowed = true;
        break;
    }
}
