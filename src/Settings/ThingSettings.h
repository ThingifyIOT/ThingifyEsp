#ifndef _THING_SETTINGS_H
#define _THING_SETTINGS_H

#include <FixedString.h>

class WifiNetwork
{    
public:
    WifiNetwork(const char* name, const char* password) :Name(name), Password(password)
    {

    }
    WifiNetwork(FixedString32 name, FixedString64 password) :Name(name), Password(password)
    {

    }
    FixedString32 Name;
    FixedString64 Password;
};

class ThingSettings
{
public:
	FixedString32 Token;
    FixedString32 ApiServer;
    int ApiPort;
    FixedString32 ThingName;
	std::vector<WifiNetwork*> WifiNetworks;

    ~ThingSettings()
	{
		for(int i=0; i < WifiNetworks.size(); i++)
		{
			delete WifiNetworks[i];
		}
	}
};

#endif