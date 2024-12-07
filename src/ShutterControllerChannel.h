#pragma once
#include "OpenKNX.h"
#include "ModeBase.h"
#include "WindowOpenHandler.h"
#include "CallContext.h"
#include "PositionController.h"
#include "MeasurementWatchdog.h"
#include <vector>

class ModeWindowOpen;
class ModeShading;

class ShutterControllerChannel : public OpenKNX::Channel
{
    private:
        MeasurementWatchdog _measurementRoomTemperature = MeasurementWatchdog();
        MeasurementWatchdog _measurementHeading = MeasurementWatchdog();
        std::vector<ModeBase*> _modes;
        std::vector<WindowOpenHandler*> _windowOpenHandlers;
   
        volatile bool _triggered = false;
        bool _channelLockActive = false;
        std::string _name = std::string();
        WindowOpenHandler* _currentWindowOpenHandler = nullptr;
        ModeManual* _modeManual = nullptr;
        ModeIdle* _modeIdle = nullptr;
        ModeBase* _currentMode = nullptr;
        PositionController _positionController;
        bool _anyAutoModeActive = false;
        unsigned long _waitTimeForReactivateShadingAfterManualStarted = 0;
        bool _waitForShadingPeriodEnd = false;
        bool _shadingPeriodActive = false;
       

        bool __shadingControlActive = false;
        void shadingControlActive(bool active);
       
        bool __anyShadingModeActive = false;
        void anyShadingModeActive(bool active);
        bool anyShadingModeActive() const;
        unsigned long getManualShadingWaitTimeInMs() const;
    public:
        ShutterControllerChannel(uint8_t channelIndex);
        bool shadingControlActive() const;
        void activateShadingControl(bool active);
       
        const std::string name() override;
        void setup() override;
        bool needCall();  
        const char* getPhoneNumber();
        uint8_t getCancelCallTime();
        void processInputKo(GroupObject &ko);
        bool processCommand(const std::string cmd, bool diagnoseKo, bool& diagnosticLogLoopRequest);
        void execute(CallContext& callContext);
        void activateShading();
};