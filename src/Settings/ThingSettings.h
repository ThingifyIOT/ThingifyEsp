#ifndef _THING_SETTINGS_H
#define _THING_SETTINGS_H

#include <FixedString.h>
#include <vector>


#include "WifiCredential.h"

class ThingSettings
{
public:
	FixedString32 Token;
    FixedString32 ApiServer;
    int ApiPort;
    FixedString32 ThingName;
	std::vector<WifiCredential*> WifiNetworks;

    ~ThingSettings()
	{
		for(int i=0; i < WifiNetworks.size(); i++)
		{
			delete WifiNetworks[i];
		}
	}
};

#endif