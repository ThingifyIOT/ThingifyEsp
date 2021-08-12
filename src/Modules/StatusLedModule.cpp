#include "StatusLedModule.h"


StatusLedModule::StatusLedModule(Thingify& thing, int ledPin, bool isLedInverted): 
_thing(thing),
_ledPin(ledPin),
_isLedInverted(isLedInverted)
{

}
const char* StatusLedModule::GetName()
{
	return "Status Led";
}

bool StatusLedModule::Init()
{	
    pinMode(_ledPin, OUTPUT);
	return true;
}

bool StatusLedModule::Tick()
{	
    static int n = -1;
    static bool ledState = true;

    if(_previousState != _thing.GetCurrentState())
    {
        n = -1;
        _nextLedChangeTime = 0;
    }
    _previousState = _thing.GetCurrentState();

    std::vector<int> empty;
    std::vector<int>& pattern = empty;
    for(int i =0; i < ledPatterns.size(); i++)
    {
        if(ledPatterns[i][0] == (int)_thing.GetCurrentState())
        {
            pattern = ledPatterns[i];
        }
    }
    if(pattern.empty())
    {
        return true;
    }

    if(millis() < _nextLedChangeTime)
    {
        return true;
    }

    n++;
    if(n >= pattern.size() || n == 0)
    {
        n = 1;
        ledState = true;
    }
    else
    {
        ledState = !ledState;
    }
    _nextLedChangeTime = millis() + pattern[n];

    digitalWrite(_ledPin, _isLedInverted ? !ledState: ledState);
      
	return true;
}