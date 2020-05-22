#ifndef _SMART_CONFIG_SERVER_H
#define _SMART_CONFIG_SERVER_H

#include <WiFiServer.h>
#include "Logging/Logger.h"
#include "Api/ZeroConfigurationPacket.h"
class SmartConfigServer
{
    private:
        WiFiServer _server;
        Logger& _logger;
    public:
        SmartConfigServer();
        void Start();
        void Stop();
        ZeroConfigurationPacket* Loop();
};

#endif