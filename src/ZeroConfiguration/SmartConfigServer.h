#ifndef _SMART_CONFIG_SERVER_H
#define _SMART_CONFIG_SERVER_H

#include <WiFiServer.h>
#include "Logging/Logger.h"
#include "Api/ZeroConfigurationPacket.h"

#ifdef ESP32
#include <WiFi.h>
#else
#include <ESP8266WiFi.h>
#endif 

struct ClientRequestContext
{
    WiFiClient Client;
    PacketBase* Packet;

    ~ClientRequestContext()
    {
        delete Packet;
    }
};


class SmartConfigServer
{
    private:
        WiFiServer _server;
        Logger& _logger;
    public:
        SmartConfigServer();
        void Start();
        void Stop();
        ClientRequestContext* GetRequest();
        void SendPacket(WiFiClient client, PacketBase *packet);
};

#endif