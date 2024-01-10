#include <Arduino.h>
#include "DebounceButton.h"

DebounceButton::DebounceButton(int pin, int buttonMode, int debounceTime)
{
    _pin = pin;
    _debounceTime = debounceTime;
    _buttonMode = buttonMode;
}

void DebounceButton::Init()
{
    if(_pin != -1)
    {
        pinMode(_pin, _buttonMode);
    }
}

ButtonEvent DebounceButton::Loop(bool inputState)
{
    ButtonEvent event = ButtonEvent::None;

    if (inputState != _lastFlickerableState) 
    {
        _lastDebounceTime = millis();
    }
    _lastFlickerableState = inputState;

    if ((millis() - _lastDebounceTime) > _debounceTime) 
    {
        if(_lastSteadyState != inputState)
        {
            if(inputState)
            {
                event = ButtonEvent::Pressed;
            }
            else 
            {
                event = ButtonEvent::Released;
            }
        }
        _lastSteadyState = inputState;
    }
    return event;
}

ButtonEvent DebounceButton::Loop()
{
    bool currentState = digitalRead(_pin);    
    return Loop(currentState);
}

bool DebounceButton::IsPressed()
{
    return _lastSteadyState;
}