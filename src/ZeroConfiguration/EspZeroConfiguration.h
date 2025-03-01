#ifndef _ESP_ZERO_CONFIGURATION_H
#define _ESP_ZERO_CONFIGURATION_H

#include "EspSmartConfig.h"
#include "SmartConfigServer.h"
#include "ThingProperties.h"
#include "Logging/Logger.h"
#include "Settings/ThingSettings.h"
#include "Settings/SettingsStorage.h"


class EspZeroConfiguration
{
    private:
        const int WifiConfigurationTimeout = 40000;
        const int ServerConfigurationTimeout = 30000;
        EspSmartConfig _espSmartConfig;
        SmartConfigServer _smartConfigServer;
        SettingsStorage& _settingsStorage;
        ThingProperties& _properties;
        bool _isReady = false;
        uint64_t _stateChangeTime = 0;
        Logger& _logger;        
        void ZeroConfigurationPacketToSettings(ZeroConfigurationPacket *packet, ThingSettings &settings);

    public:
        EspZeroConfiguration(SettingsStorage& settingsStorage, ThingProperties& properties);
        void Start();
        void Loop();
        bool IsReady();

};

#endif