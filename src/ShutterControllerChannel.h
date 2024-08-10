#pragma once
#include "OpenKNX.h"
#include "ModeBase.h"
#include "CallContext.h"
#include "PositionController.h"
#include "MeasurementWatchdog.h"

class ModeWindowOpen;
class ModeShading;

class ShutterControllerChannel : public OpenKNX::Channel
{
    private:
        MeasurementWatchdog _measurementRoomTemperature = MeasurementWatchdog();
        MeasurementWatchdog _measurementHeading = MeasurementWatchdog();
        volatile bool _triggered = false;
        bool _channelLockActive = false;
        std::string _name = std::string();
        std::vector<ModeBase*> _modes;
        ModeManual* _modeManual = nullptr;
        ModeIdle* _modeIdle = nullptr;
        ModeBase* _currentMode = nullptr;
        PositionController _positionController;
        bool _anyAutoModeActive = false;
        ModeShading* _handleWindowOpenAsShading = nullptr;
        unsigned long _waitTimeForReactivateShadingAfterManualStarted = 0;
        bool _waitForShadingPeriodEnd = false;
       

        bool __shadingControlActive = false;
        void shadingControlActive(bool active);
        bool shadingControlActive();
       
        bool __anyShadingModeActive = false;
        void anyShadingModeActive(bool active);
        bool anyShadingModeActive();
    public:
        ShutterControllerChannel(uint8_t channelIndex);
        const std::string name() override;
        void setup() override;
        bool needCall();  
        const char* getPhoneNumber();
        uint8_t getCancelCallTime();
        void processInputKo(GroupObject &ko) override;
        bool processCommand(const std::string cmd, bool diagnoseKo, bool& diagnosticLogLoopRequest);
        void execute(CallContext& callContext);
        void activateShading();
};