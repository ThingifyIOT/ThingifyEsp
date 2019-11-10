#ifndef _SMART_CONFIG_SERVER_H
#define _SMART_CONFIG_SERVER_H

#include <WiFiServer.h>

class SmartConfigServer
{
    private:
        WiFiServer _server;
    public:
        SmartConfigServer();
        void Start();
        void Loop();
};

#endif