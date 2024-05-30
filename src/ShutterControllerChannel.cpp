#include "ShutterControllerChannel.h"
#include "ModeManual.h"
#include "ModeWindowOpen.h"
#include "ModeNight.h"
#include "ModeShading.h"

ShutterControllerChannel::ShutterControllerChannel(uint8_t channelIndex)
: _modes()
{
    _channelIndex = channelIndex;
     char buffer[10] = {0};
    _name = itoa(channelIndex, buffer, 10);
}

const std::string ShutterControllerChannel::name()
{
    return _name;
}

void ShutterControllerChannel::setup()
{
    _modes.push_back(new ModeManual());
    if (ParamSHC_ChannelModeWindowOpen)
        _modes.push_back(new ModeWindowOpen());
    if (ParamSHC_ChannelModeNight)
        _modes.push_back(new ModeNight());
    if (ParamSHC_ChannelModeShading1)
        _modes.push_back(new ModeShading(1));
    // ToDo: comment in code after parameters are defined
    // if (ParamSHC_ChannelModeShading2)
    //     _modes.push_back(new ModeShading(2));

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