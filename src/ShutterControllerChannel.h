#pragma once
#include "OpenKNX.h"
#include "ModeBase.h"
#include "CallContext.h"

class ShutterControllerChannel : public OpenKNX::Channel
{
        volatile bool _triggered = false;
        std::string _name = std::string();
        std::vector<ModeBase*> _modes;
        ModeManual* _modeManual = nullptr;
        ModeIdle* _modeIdle = nullptr;
        ModeBase* _currentMode = nullptr;
        bool _shadingControlActive;
        bool _shadowActive = false;
        unsigned long _waitTimeForReactivateShadingAfterManualStarted = 0;
        bool _waitForShadingPeriodEnd = false;
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
};