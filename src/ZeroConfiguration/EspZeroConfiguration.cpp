
#include "EspZeroConfiguration.h"
#include "ThingifyUtils.h"

EspZeroConfiguration::EspZeroConfiguration(SettingsStorage& settingsStorage):
_settingsStorage(settingsStorage),
_logger(LoggerInstance)
{

}
void EspZeroConfiguration::Start()
{    
    ChangeState(ZeroConfigurationState::SmartConfigWifi);
    _smartConfigServer.Start();
    _espSmartConfig.Start();
}
ZeroConfigurationState EspZeroConfiguration::Loop()
{    
    if(_state == ZeroConfigurationState::SmartConfigWifi)
    {
        auto smartConfigState = _espSmartConfig.Loop();

        if(smartConfigState == SmartConfigState::Success)
        {
            ChangeState(ZeroConfigurationState::SmartConfigServer);
        }
        if(smartConfigState == SmartConfigState::Error)
        {
            ChangeState(ZeroConfigurationState::Error);
        }
    }
    if(_state == ZeroConfigurationState::SmartConfigServer)
    {
        auto zeroConfigurationPacket = _smartConfigServer.Loop();  

        if(zeroConfigurationPacket != nullptr)
        {
            ThingSettings settings;
            ZeroConfigurationPacketToSettings(zeroConfigurationPacket, settings);
            delete zeroConfigurationPacket; 

            _logger.info(L("Got settings from smart config:"));
            ThingifyUtils::LogSettings(_logger, settings);
            _settingsStorage.Clear();
            _settingsStorage.Set(settings);
            ChangeState(ZeroConfigurationState::Success);    
        }
        else if(millis() - _stateChangeTime > ServerConfigurationTimeout)
        {
            _logger.err(F("Timout waiting for config packet in server"));
           ChangeState(ZeroConfigurationState::Error);
        } 
    }
    return _state;
}
void EspZeroConfiguration::ChangeState(ZeroConfigurationState state)
{
    if(_state == state)
    {
        return;
    }
    if(state == ZeroConfigurationState::Error || state == ZeroConfigurationState::Success || state == ZeroConfigurationState::Stopped)
    {
        _smartConfigServer.Stop();
        _espSmartConfig.Stop();
    }
    _state = state;
    _stateChangeTime = millis();    
}
void EspZeroConfiguration::ZeroConfigurationPacketToSettings(ZeroConfigurationPacket *packet, ThingSettings &settings)
{
    settings.Token = packet->Token;
    settings.ApiServer = packet->ApiServer;
    settings.ApiPort = packet->ApiPort;

    for(int i=0; i < packet->WifiNetworks.size(); i++)
    {
        auto network = new WifiCredential(
            packet->WifiNetworks[i]->Name, 
            packet->WifiNetworks[i]->Password
        );
        settings.WifiNetworks.push_back(network);
    }
}
bool EspZeroConfiguration::IsReady()
{
    return _state == ZeroConfigurationState::Success || _state == ZeroConfigurationState::Error;
}