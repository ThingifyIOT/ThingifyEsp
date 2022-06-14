#ifndef _ESP_ZERO_CONFIGURATION_H
#define _ESP_ZERO_CONFIGURATION_H

#include "EspSmartConfig.h"
#include "SmartConfigServer.h"
#include "Logging/Logger.h"
#include "Settings/ThingSettings.h"
#include "Settings/SettingsStorage.h"

enum class ZeroConfigurationState
{
    Stopped,
    SmartConfigWifi,
    SmartConfigServer,
    Success,
    Error
};

class EspZeroConfiguration
{
    private:
        const int WifiConfigurationTimeout = 40000;
        const int ServerConfigurationTimeout = 30000;
        EspSmartConfig _espSmartConfig;
        SmartConfigServer _smartConfigServer;
        SettingsStorage& _settingsStorage;
        bool _isReady = false;
        uint64_t _stateChangeTime = 0;
        ZeroConfigurationState _state = ZeroConfigurationState::Stopped;
        Logger& _logger;        
        void ZeroConfigurationPacketToSettings(ZeroConfigurationPacket *packet, ThingSettings &settings);
        void ChangeState(ZeroConfigurationState state);
    public:
        EspZeroConfiguration(SettingsStorage& settingsStorage);
        void Start();
        ZeroConfigurationState Loop();
        bool IsReady();
};

#endif