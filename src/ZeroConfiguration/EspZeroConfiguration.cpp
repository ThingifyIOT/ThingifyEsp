#include "EspZeroConfiguration.h"
#include "ThingifyUtils.h"
#include "Api/ZeroConfigurationResponsePacket.h"
#include "Api/ZeroConfigurationDeviceInfoRequestPacket.h"
#include "Api/ZeroConfigurationDeviceInfoResponsePacket.h"

EspZeroConfiguration::EspZeroConfiguration(SettingsStorage& settingsStorage, ThingProperties &properties):
_settingsStorage(settingsStorage),
_properties(properties),
_logger(LoggerInstance)
{

}
void EspZeroConfiguration::Start()
{    
    _smartConfigServer.Start();
    _espSmartConfig.Start();
}
void EspZeroConfiguration::Loop()
{    
    auto request = _smartConfigServer.GetRequest();
    if(request == nullptr)
    {
        return;
    }

    auto packet = request->Packet;
    if(packet->PacketType() == ThingifyPacketType::ZeroConfigurationDeviceInfoRequestPacket)
    {   
        auto zeroConfigurationDeviceInfoRequestPacket = static_cast<ZeroConfigurationDeviceInfoRequestPacket*>(packet);
        _logger.info(L("[LOCAL_PACKET] -> ZeroConfigurationDeviceInfoRequestPacket"));

        auto response = new ZeroConfigurationDeviceInfoResponsePacket();
        response->DefaultName = _properties.DefaultName;
        _smartConfigServer.SendPacket(request->Client, response);
        delete response;
    }

    if(packet->PacketType() == ThingifyPacketType::ZeroConfigurationPacket)
    {          
        auto zeroConfigurationPacket = static_cast<ZeroConfigurationPacket*>(packet);
         _logger.info(L("[LOCAL_PACKET] -> ZeroConfigurationPacket"));

        ThingSettings settings;
        ZeroConfigurationPacketToSettings(zeroConfigurationPacket, settings);

        _logger.info(L("Got settings from smart config:"));
        ThingifyUtils::LogSettings(_logger, settings);
        _settingsStorage.Clear();
        _settingsStorage.Set(settings);
        auto response = new ZeroConfigurationResponsePacket();
        _smartConfigServer.SendPacket(request->Client, response);
        delete response;

        _isReady = true;
    }

    request->Client.flush();
    delete request;    
}
void EspZeroConfiguration::ZeroConfigurationPacketToSettings(ZeroConfigurationPacket *packet, ThingSettings &settings)
{
    settings.ThingName = packet->ThingName;
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
    return _isReady;
}