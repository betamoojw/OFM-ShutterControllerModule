#pragma once
#include "OpenKNX.h"

class CallContext;
class ShutterSimulation;

enum class PositionControllerState
{
    Idle,
    MovingDown,
    MovingUp
};

class PositionController
{
    ShutterSimulation* _shutterSimulation = nullptr;
    PositionControllerState _state = PositionControllerState::Idle;
    uint8_t _lastManualPosition = 0;
    uint8_t _lastManualSlat = 0;
    uint8_t _setPosition = 255;
    uint8_t _setSlat = 255;
    uint8_t _channelIndex;
    bool _hasSlat = false;
    unsigned long _startWaitForManualPositionFeedback = 0;
    unsigned long _startWaitForManualSlatPositionFeedback = 0;
    unsigned long _waitForMovingFinshed = 0;
    unsigned long _waitForMovingTimeout = 0; 
    std::string _logPrefix = "";
    const std::string& logPrefix();
    void setState(PositionControllerState state);
    void setMovingTimeout(unsigned long timeout);
public:
    PositionController(uint8_t channelIndex);
    bool hasSlat() const;
    void setAutomaticPosition(uint8_t position);
    void setAutomaticSlat(uint8_t position);
    void setManualPosition(uint8_t position);
    void setManualSlat(uint8_t position);
    void setManualStep(bool step);
    void setManualUpDown(bool up);
    void restoreLastManualPosition(); 
    void processInputKo(GroupObject &ko);
    void control(const CallContext& callContext);
    bool startSimulation(bool fastSimulation);
    bool stopSimulation();
    uint8_t simulationMode();
    PositionControllerState state();
    uint8_t position();
};
