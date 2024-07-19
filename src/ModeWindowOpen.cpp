#include "ModeWindowOpen.h"
#include "PositionController.h"

const char *ModeWindowOpen::name() const
{
    return "WindowOpen";
}

uint8_t ModeWindowOpen::sceneNumber() const
{
    return 13;
}

void ModeWindowOpen::initGroupObjects()
{
    KoSHC_CHWindowOpenModeActive.value(false, DPT_Switch);
    KoSHC_CHWindowOpenLockActive.value(false, DPT_Switch);
}
bool ModeWindowOpen::modeWindowOpenAllowed() const
{
    return true;
}
bool ModeWindowOpen::allowed(const CallContext &callContext)
{
    if (!callContext.modeCurrentActive->modeWindowOpenAllowed())
    {
        if (callContext.diagnosticLog)
            logInfoP("Window open mode not allowed by current active mode");
        return false;
    }
    if (_recalcAllowed || callContext.diagnosticLog)
    {
        _allowed = true;
        if (ParamSHC_ChannelModeWindowOpenPositionControl == 0 && (ParamSHC_ChannelType != 1 || ParamSHC_ChannelModeWindowOpenSlatPositionControl == 0))
        {
            _allowed = false;
            if (callContext.diagnosticLog)
                logInfoP("whether shutter nor slat position configured");
        }
        if (!KoSHC_CHWindow.value(DPT_OpenClose))
        {
            _allowed = false;
            if (callContext.diagnosticLog)
                logInfoP("Window closed");
        }
        if (KoSHC_CHWindowOpenLockActive.value(DPT_Switch))
        {
            _allowed = false;
            if (callContext.diagnosticLog)
                logInfoP("Lock KO active");
        }
    }
    return _allowed;
}
void ModeWindowOpen::start(const CallContext &callContext, const ModeBase *previous, PositionController& positionController)
{
    _positionToRestore = -1;
    _slatToRestore = -1;
    KoSHC_CHWindowOpenModeActive.value(true, DPT_Switch);
    auto positionControl = ParamSHC_ChannelModeWindowOpenPositionControl;
    if (positionControl > 0)
    {
        int8_t currentPositionValue = -1;
        if (KoSHC_CHShutterPercentOutput.commFlag() == ComFlag::WriteRequest)
            currentPositionValue = (uint8_t)KoSHC_CHShutterPercentOutput.value(DPT_Scaling);
        else if (KoSHC_CHShutterPercentInput.initialized())
            currentPositionValue = KoSHC_CHShutterPercentInput.value(DPT_Scaling);

        auto position = ParamSHC_ChannelModeWindowOpenPosition;

        // 	<Enumeration Text="Nein" Value="0" Id="%ENID%" />
        //  <Enumeration Text="Nur wenn weiter geschlossen" Value="1" Id="%ENID%" />
        //  <Enumeration Text="Immer" Value="2" Id="%ENID%" />
        switch (positionControl)
        {
        case 1:
            if (currentPositionValue != -1 && (uint8_t)currentPositionValue > position)
            {
                _positionToRestore = currentPositionValue;
                if (callContext.diagnosticLog)
                    logInfoP("Set shutter to %d%%", (int)(position));
                positionController.setAutomaticPosition(position);
            }
            break;
        case 2:
            _positionToRestore = currentPositionValue;
            if (callContext.diagnosticLog)
                logInfoP("Set shutter to %d%%", (int)(position));
            positionController.setAutomaticPosition(position);
            break;
        }
    }
    if (ParamSHC_ChannelType == 1)
    {
        auto slatPositionControl = ParamSHC_ChannelModeWindowOpenSlatPositionControl;
        if (slatPositionControl > 0)
        {
            int8_t currentSlatValue = -1;
            if (KoSHC_CHShutterSlatOutput.commFlag() == ComFlag::WriteRequest)
                currentSlatValue = (uint8_t)KoSHC_CHShutterSlatOutput.value(DPT_Scaling);
            else if (KoSHC_CHShutterSlatInput.initialized())
                currentSlatValue = KoSHC_CHShutterSlatInput.value(DPT_Scaling);
        
            auto slatPosition = ParamSHC_ChannelModeWindowOpenSlatPosition;

            // 	<Enumeration Text="Nein" Value="0" Id="%ENID%" />
            //  <Enumeration Text="Nur wenn weiter geschlossen" Value="1" Id="%ENID%" />
            //  <Enumeration Text="Immer" Value="2" Id="%ENID%" />
            switch (slatPositionControl)
            {
            case 1:
                if (currentSlatValue != -1 && (uint8_t)currentSlatValue > slatPosition)
                {
                    _slatToRestore = currentSlatValue;
                    if (callContext.diagnosticLog)
                        logInfoP("Set slat to %d%%", (int)(slatPosition));
                    positionController.setAutomaticSlat(slatPosition);
                }
                break;
            case 2:
                _slatToRestore = currentSlatValue;
                if (callContext.diagnosticLog)
                    logInfoP("Set slat to %d%%", (int)(slatPosition));
                positionController.setAutomaticSlat(slatPosition);
                break;
            }
        }
    }
}
void ModeWindowOpen::control(const CallContext &callContext, PositionController& positionController)
{
}
void ModeWindowOpen::stop(const CallContext &callContext, const ModeBase *next, PositionController& positionController)
{
    if (_positionToRestore != -1)
    {
        positionController.setAutomaticPosition(_positionToRestore);
        _positionToRestore = -1;
    }
    if (_slatToRestore != -1)
    {
        positionController.setAutomaticSlat(_slatToRestore);
        _slatToRestore = -1;
    }
    KoSHC_CHWindowOpenModeActive.value(false, DPT_Switch);
}
void ModeWindowOpen::processInputKo(GroupObject &ko)
{
    switch (ko.asap())
    {
    case SHC_KoCHWindow:
    case SHC_KoCHWindowOpenLock:
        _recalcAllowed = true;
        break;
    }
}