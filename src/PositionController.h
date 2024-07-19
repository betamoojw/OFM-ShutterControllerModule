#pragma once
#include "OpenKNX.h"

class CallContext;

class PositionController
{
    uint8_t _lastManaulPosition = 0;
    uint8_t _lastManualSlat = 0;
    uint8_t _setPosition = 255;
    uint8_t _setSlat = 255;
    uint8_t _channelIndex;
    bool _hasSlat = false;
    unsigned long _startWaitForManualPositionFeedback = 0;
    unsigned long _startWaitForManualSlatPositionFeedback = 0;
    std::string _logPrefix = "";
    const std::string& logPrefix();
public:
    PositionController(uint8_t channelIndex);
    void setAutomaticPosition(uint8_t position);
    void setAutomaticSlat(uint8_t position);
    void setManualPosition(uint8_t position);
    void setManualSlat(uint8_t position);
    void setManualStep(bool step);
    void setManualUpDown(bool up);
    void restoreLastManualPosition(); 
    void processInputKo(GroupObject &ko);
    void control(const CallContext& callContext);
};