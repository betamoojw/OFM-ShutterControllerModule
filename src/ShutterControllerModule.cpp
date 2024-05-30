#include "ShutterControllerModule.h"
#include "ShutterControllerChannel.h"
#include "Helios.h"
#include "Timer.h"


ShutterControllerModule::ShutterControllerModule()
 : ShutterControllerChannelOwnerModule()
{

}

const std::string ShutterControllerModule::name()
{
    return "SHC";
}

const std::string ShutterControllerModule::version()
{
#ifdef MODULE_ShutterControllerModule_Version
    return MODULE_ShutterControllerModule_Version;
#else
    // hides the module in the version output on the console, because the firmware version is sufficient.
    return "";
#endif
}

void ShutterControllerModule::showInformations()
{
    openknx.logger.logWithPrefix(logPrefix(), "Shutter Controller Module");
}

void ShutterControllerModule::showHelp()
{

}


bool ShutterControllerModule::processCommand(const std::string cmd, bool diagnoseKo)
{
    if (cmd == "CalcSun")
    {
        
        Helios helios;
        helios.calcSunPos(2024, 5, 28, 20, 00, 00, 15.40664048676037, 47.071934457426266);
        logInfoP("Sun azimut: %d elevation: %d", helios.dAzimuth, helios.dElevation);
        return true;
    }
    return false;
}

void ShutterControllerModule::setup()
{
    setupChannels(ParamSHC_VisibleChannels);
}

OpenKNX::Channel* ShutterControllerModule::createChannel(uint8_t _channelIndex /* this parameter is used in macros, do not rename */)
{
    if (ParamSHC_ChannelDeactivated)
        return nullptr;
    return new ShutterControllerChannel(_channelIndex);
}

ShutterControllerModule openknxShutterControllerModule;
