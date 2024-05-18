#include "ShutterControllerChannel.h"

ShutterControllerChannel::ShutterControllerChannel(uint8_t channelIndex)
{
    _channelIndex = channelIndex;
     char buffer[10] = {0};
    _name = itoa(channelIndex, buffer, 10);
}

const std::string ShutterControllerChannel::name()
{
    return _name;
}

void ShutterControllerChannel::processInputKo(GroupObject &ko)
{
     // channel ko
    auto index = SHC_KoCalcIndex(ko.asap());
    switch (index)
    {
      
    }
}

bool ShutterControllerChannel::processCommand(const std::string cmd, bool diagnoseKo)
{
    return false;
}