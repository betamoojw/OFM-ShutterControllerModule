#include "ShutterControllerModule.h"
#include "ShutterContollerChannel.h"


ShutterControllerModule::ShutterControllerModule()
 : ShutterControllerChannelOwnerModule(SHC_ChannelCount)
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
    openknx.logger.logWithPrefix("Shutter Controller Module");
}

void ShutterControllerModule::showHelp()
{

}


bool ShutterControllerModule::processCommand(const std::string cmd, bool diagnoseKo)
{
    return false;
}


OpenKNX::Channel* ShutterControllerModule::createChannel(uint8_t _channelIndex /* this parameter is used in macros, do not rename */)
{
    return new ShutterControllerChannelOwnerModule();
}

ShutterControllerModule openknxShutterControllerModule;
