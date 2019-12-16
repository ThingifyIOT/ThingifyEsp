#ifndef _THING_SETTINGS_H
#define _THING_SETTINGS_H

#include <FixedString.h>

class WifiNetwork
{
    FixedString32 Name;
    FixedString64 Password;
};

class ThingSettings
{
public:
	FixedString32 Token;
    FixedString32 ApiAddress;
	std::vector<WifiNetwork*> WifiNetworks;
};

#endif