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
    pinMode(_pin, _buttonMode);
}

ButtonEvent DebounceButton::Loop()
{
    bool currentState = digitalRead(_pin);
    ButtonEvent event = ButtonEvent::None;

    if (currentState != _lastFlickerableState) 
    {
        _lastDebounceTime = millis();
    }
    _lastFlickerableState = currentState;

    if ((millis() - _lastDebounceTime) > _debounceTime) 
    {
        if(_lastSteadyState != currentState)
        {
            if(currentState)
            {
                event = ButtonEvent::Pressed;
            }
            else 
            {
                event = ButtonEvent::Released;
            }
        }
        _lastSteadyState = currentState;
    }
    return event;
}

bool DebounceButton::IsPressed()
{
    return _lastSteadyState;
}