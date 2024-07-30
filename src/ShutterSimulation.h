#pragma once
#include "OpenKNX.h"
#include "CallContext.h"
class PositionController;

class ShutterSimulation 
{
    private:
        PositionController& _positionController;
        uint8_t _channelIndex;
        uint8_t _currentPosition = 0;
        uint8_t _targetPosition = 0;
        uint8_t _currentSlatPosition = 0;
        uint8_t _targetSlatPosition = 0;
        unsigned long _lastPositionChange = 0;
        std::string _logPrefix;
    public:
        ShutterSimulation(uint8_t channelIndex, PositionController& positionController);
        void processInputKo(GroupObject& ko);
        void update(const CallContext& callContext);
        const std::string& logPrefix();
};