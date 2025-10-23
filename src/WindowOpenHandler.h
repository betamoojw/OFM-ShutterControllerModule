#pragma once
#include "OpenKNX.h"

class CallContext;
class ModeNight;

enum WindowOpenState
{
    WindowOpenStateClosed = 0,
    WindowOpenStateTilted = 1,
    WindowOpenStateOpen = 2
};

class WindowOpenHandler
{
    WindowOpenState _lastWindowOpenState = WindowOpenStateClosed;
    uint8_t _channelIndex = 0;
    ModeNight* _nightMode = nullptr;
    std::string _name;
    uint8_t _index;
    bool _isTiltHandler;
    bool _recalcAllowed = true;
    bool _allowed = false;
public:
    WindowOpenHandler(uint8_t _channelIndex, uint8_t index, bool isTiltHandler, ModeNight* nightMode);
protected:
    const std::string& logPrefix() const;
public:
    uint8_t getParamterOpenPositionControl();
    uint8_t getParamterOpenSlatPositionControl();
    uint8_t getParamterOpenPosition();
    uint8_t getParamterOpenSlatPosition();
    const char *name() const;
    uint8_t sceneNumber() const;
    void setup();
    void initGroupObjects();
    bool allowed(const CallContext& callContext, WindowOpenState windowOpenState);
    void start(const CallContext& callContext, const WindowOpenHandler* previous, PositionController& positionController);
    void stop(const CallContext& callContext, const WindowOpenHandler* next, PositionController& positionController);
    void processInputKo(GroupObject &ko, PositionController& positionController);
};