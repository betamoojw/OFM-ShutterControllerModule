#include "ModeBase.h"

void ModeBase::setup(uint8_t _channelIndex)
{
    this->_channelIndex = _channelIndex;
    _logPrefix = openknx.logger.buildPrefix("SC", _channelIndex + 1);  
    _logPrefix += name();
    initGroupObjects();
}

const std::string& ModeBase::logPrefix() const
{
    return _logPrefix;
}

bool ModeBase::isModeShading() const
{
    return false;
}