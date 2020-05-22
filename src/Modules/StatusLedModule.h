#pragma once

#include <Thingify.h>
#include <IModule.h>
#include <ThingifyUtils.h>

class StatusLedModule: public IModule
{
	Thingify& _thing;
    int _ledPin;
	ThingState _previousState = ThingState::Disabled;
	std::vector<std::vector<int>> ledPatterns = 
    {
		{(int)ThingState::Disabled, 50, 15000},
        {(int)ThingState::NotConfigured, 50, 300, 50, 300, 50, 300, 50, 2000},
		{(int)ThingState::Configuring, 50, 150},
		{(int)ThingState::SearchingForNetwork, 50, 300, 50, 2000},
		{(int)ThingState::ConnectingToNetwork, 500, 500},
		{(int)ThingState::ConnectingToMqtt, 500, 50, 500, 50, 500, 2000},
		{(int)ThingState::Online, 50, 4000},
		{(int)ThingState::DisconnectedFromMqtt, 50, 2000},
		{(int)ThingState::Authenticating, 500, 1000},
		{(int)ThingState::ServiceUnavailable, 2000, 2000},
		{(int)ThingState::Error, 2000, 300}
    };
public:
	uint64_t _nextLedChangeTime = 0;
	StatusLedModule(Thingify& thing, int pin);	
	const char* GetName() override;
	bool Init() override;
	bool Tick() override;
};

