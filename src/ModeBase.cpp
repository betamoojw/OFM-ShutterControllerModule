#include "ModeBase.h"

void ModeBase::setup(uint8_t _channelIndex)
{
    this->_channelIndex = _channelIndex;
    _logPrefix = name();
    _logPrefix +=  std::to_string(_channelIndex);
}

const std::string ModeBase::logPrefix()
{
    return _logPrefix;
}