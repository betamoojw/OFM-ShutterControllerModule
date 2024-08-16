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
    uint8_t _restorePosition = 0;
    uint8_t _restoreSlat = 0;
    uint8_t _setPosition = 255;
    unsigned long _lastSetPosition = 0;
    uint8_t _calculatedTargetPosition = 255;
    uint8_t _positionLimit = 255;
    uint8_t _blockedPosition = 255;
    uint8_t _slatLimit = 255;
    uint8_t _blockedSlat = 255;
    uint8_t _setSlat = 255;
    unsigned long _lastSetSlat = 0;
    uint8_t _channelIndex;
    bool _hasSlat = false;
    unsigned long _startWaitForManualPositionFeedback = 0;
    unsigned long _startWaitForManualSlatPositionFeedback = 0;
    unsigned long _waitForMovingFinshed = 0;
    unsigned long _waitForMovingTimeout = 0; 
    std::string _logPrefix = "";
    const std::string& logPrefix();
    void setState(PositionControllerState state, const char* reason);
    void setMovingTimeout(unsigned long timeout);
public:
    PositionController(uint8_t channelIndex);
    bool hasSlat() const;
    void setAutomaticPosition(uint8_t automaticPosition);
    void setAutomaticSlat(uint8_t automaticSlat);
    void setAutomaticPositionAndStoreForRestore(uint8_t automaticPosition);
    void setAutomaticSlatAndStoreForRestore(uint8_t automaticSlat);
    void setPositionLowerLimit(uint8_t positionLimit, bool);
    void setSlatLowerLimit(uint8_t slatLimit, bool moveToLimit);
    void resetPositionLowerLimit();
    void resetSlatLowerLimit();
    void resetBlockedPositionAndLimits();
    void setManualPosition(uint8_t manualPosition);
    void setManualSlat(uint8_t manualSlat);
    void setManualStep(bool step);
    void setManualUpDown(bool up);
    void restoreLastManualPosition(); 
    bool processInputKo(GroupObject &ko, CallContext* callContext);
    void control(const CallContext& callContext);
    bool startSimulation(bool fastSimulation);
    bool stopSimulation();
    uint8_t simulationMode() const;
    PositionControllerState state() const;
    uint8_t position() const;
    int8_t targetPosition() const;
    uint8_t slat() const;
};
