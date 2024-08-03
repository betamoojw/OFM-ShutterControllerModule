#include "ModeWindowOpen.h"
#include "PositionController.h"

#ifdef SHC_CWindowOpenPosition2
// redefine SHC_ParamCalcIndex to add offset for Window Mode 2
#undef SHC_ParamCalcIndex
#define SHC_ParamCalcIndex(index) (index + SHC_ParamBlockOffset + _channelIndex * SHC_ParamBlockSize + (SHC_CWindowOpenPosition2 - SHC_CWindowOpenPosition1) * (_index - 1))
#endif

ModeWindowOpen::ModeWindowOpen(uint8_t index)
    : _index(index)
{
    _name = "WindowOpen";
    _name += std::to_string(index);
}

int16_t ModeWindowOpen::koChannelOffset()
{
    return (_index - 1) * (SHC_KoCWindowOpenModeActive2 - SHC_KoCWindowOpenModeActive1);
}

GroupObject &ModeWindowOpen::getKo(uint8_t ko)
{
    return knx.getGroupObject(SHC_KoCalcNumber(ko) + koChannelOffset());
}

const char *ModeWindowOpen::name() const
{
    return _name.c_str();
}

uint8_t ModeWindowOpen::sceneNumber() const
{
    return 13 + _index;
}

void ModeWindowOpen::initGroupObjects()
{
    getKo(SHC_KoCWindowOpenModeActive1).value(false, DPT_Switch);
    getKo(SHC_KoCWindowOpenLockActive1).value(false, DPT_Switch);
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
void ModeWindowOpen::start(const CallContext &callContext, const ModeBase *previous, PositionController &positionController)
{
    _positionToRestore = -1;
    _slatToRestore = -1;
    getKo(SHC_KoCWindowOpenModeActive1).value(true, DPT_Switch);
    auto positionControl = ParamSHC_CWindowOpenPositionControl1;
    if (positionControl > 0)
    {
        int8_t currentPositionValue = -1;
        if (KoSHC_CShutterPercentOutput.commFlag() == ComFlag::WriteRequest)
            currentPositionValue = (uint8_t)KoSHC_CShutterPercentOutput.value(DPT_Scaling);
        else if (KoSHC_CShutterPercentInput.initialized())
            currentPositionValue = KoSHC_CShutterPercentInput.value(DPT_Scaling);

        auto position = ParamSHC_CWindowOpenPosition1;

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
    if (positionController.hasSlat())
    {
        auto slatPositionControl = ParamSHC_CWindowOpenSlatPositionControl1;
        if (slatPositionControl > 0)
        {
            int8_t currentSlatValue = -1;
            if (KoSHC_CShutterSlatOutput.commFlag() == ComFlag::WriteRequest)
                currentSlatValue = (uint8_t)KoSHC_CShutterSlatOutput.value(DPT_Scaling);
            else if (KoSHC_CShutterSlatInput.initialized())
                currentSlatValue = KoSHC_CShutterSlatInput.value(DPT_Scaling);

            auto slatPosition = ParamSHC_CWindowOpenSlatPosition1;

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
void ModeWindowOpen::control(const CallContext &callContext, PositionController &positionController)
{
}
void ModeWindowOpen::stop(const CallContext &callContext, const ModeBase *next, PositionController &positionController)
{
    if ((ModeBase *)callContext.modeManual == next)
    {
        if (getKo(SHC_KoCWindowOpenOpened1).value(DPT_OpenClose))
        {
            _deactivatedWhileOpen = true;
            _recalcAllowed = false;
        }
    }
    else
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
    }
    getKo(SHC_KoCWindowOpenModeActive1).value(false, DPT_Switch);
}
void ModeWindowOpen::processInputKo(GroupObject &ko)
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

bool ModeWindowOpen::isModeWindowOpen() const
{
    return true;
}