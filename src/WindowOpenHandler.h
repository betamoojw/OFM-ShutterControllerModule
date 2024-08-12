#pragma once
#include "OpenKNX.h"

class CallContext;

class WindowOpenHandler
{
    uint8_t _channelIndex = 0;
    std::string _name;
    uint8_t _index;
    bool _isTilt;
    bool _recalcAllowed = true;
    bool _allowed = false;
    int16_t koChannelOffset();
    bool _deactivatedWhileOpen = false;
    GroupObject& getKo(uint8_t ko);
public:
    WindowOpenHandler(uint8_t _channelIndex, uint8_t index, bool isTilt);
protected:
    const std::string& logPrefix() const;
public:
    const char *name() const;
    uint8_t sceneNumber() const;
    void initGroupObjects();
    bool allowed(const CallContext& callContext);
    void start(const CallContext& callContext, const WindowOpenHandler* previous, PositionController& positionController);
    void stop(const CallContext& callContext, const WindowOpenHandler* next, PositionController& positionController);
    void processInputKo(GroupObject &ko, PositionController& positionController);
};