#include "ModeNight.h"

const char *ModeNight::name()
{
    return "Night";
}
void ModeNight::initGroupObjects()
{
    KoSHC_CHModeNightActive.value(false, DPT_Switch);
}
bool ModeNight::allowed(const CallContext& callContext)
{
    if (callContext.minuteChanged)
    {
        // 	<Enumeration Text="Kein Autstart" Value="0" Id="%ENID%" />
		// 	<Enumeration Text="Uhrzeit" Value="1" Id="%ENID%" />
		// 	<Enumeration Text="Sonne" Value="2" Id="%ENID%" />
		// 	<Enumeration Text="Uhrzeit, Sonne (früheres Ergeignis)" Value="3" Id="%ENID%" />
		// 	<Enumeration Text="Uhrzeit, Sonne (späteres Ergeignis)" Value="4" Id="%ENID%" />
        switch (ParamSHC_ChannelNightModeStartBehavior)
        {
            case 1:
                if  (callContext.minuteOfDay == ParamSHC_ChannelModeNightFromTime)
                    _allowed = true;
                break;
            case 2:
                if (callContext.hour < 12 && callContext.elevation == ParamSHC_ChannelModeNightSunRise)
          



        }
    }
    return _allowed;
}

void ModeNight::start()
{
}
void ModeNight::stop()
{
}
void ModeNight::processInputKo(GroupObject &ko)
{
   switch (ko.asap())
   {
   case SHC_KoCHModeNight:
    _allowed = ko.value(DPT_Switch);
    break;
   
   default:
    break;
   }
}