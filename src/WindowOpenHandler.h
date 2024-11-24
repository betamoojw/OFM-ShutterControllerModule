#pragma once
#include "OpenKNX.h"

class CallContext;

class WindowOpenHandler
{
    uint8_t _channelIndex = 0;
    std::string _name;
    uint8_t _index;
    bool _isTiltHandler;
    bool _recalcAllowed = true;
    bool _allowed = false;
    int16_t koChannelOffset();
    bool _deactivatedWhileOpen = false;
    const ModeBase* lastCurrentMode = nullptr;
    GroupObject& getKo(uint8_t ko);
public:
    WindowOpenHandler(uint8_t _channelIndex, uint8_t index, bool isTiltHandler);
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