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
        {(int)ThingState::NotConfigured, 50, 300, 50, 300, 50, 300, 50, 4000}
    };
public:
	uint64_t _nextLedChangeTime = 0;
	StatusLedModule(Thingify& thing, int pin);	
	const char* GetName() override;
	bool Init() override;
	bool Tick() override;
};

