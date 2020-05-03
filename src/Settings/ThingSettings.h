#ifndef _THING_SETTINGS_H
#define _THING_SETTINGS_H

#include <FixedString.h>

class WifiNetwork
{    
public:
    WifiNetwork()
    {
        
    }
    WifiNetwork(const char* name, const char* password) :Name(name), Password(password)
    {

    }
    FixedString32 Name;
    FixedString64 Password;
};

class ThingSettings
{
public:
	FixedString32 Token;
    FixedString32 ServerName;
    int ServerPort;
    FixedString32 ThingName;
	std::vector<WifiNetwork*> WifiNetworks;
};

#endif