#include <Arduino.h>
#include "DebounceButton.h"

DebounceButton::DebounceButton(int pin, int buttonMode, int debounceInterval)
{
    _pin = pin;
    _debounceInterval = debounceInterval;
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

    if (inputState != _previousInputState) 
    {
        _lastChangeStateTime = millis();
        _previousInputState = inputState;
    }

    if(_lastChangeStateTime == 0)
    {
        return ButtonEvent::None;
    }   
    
    if(millis() - _lastChangeStateTime < _debounceInterval)
    {
        return ButtonEvent::None;
    }

    if(_isPressed != inputState)
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
    _isPressed = inputState;
    _lastChangeStateTime = 0;
    return event;
}

ButtonEvent DebounceButton::Loop()
{
    bool currentState = digitalRead(_pin);    
    return Loop(currentState);
}

bool DebounceButton::IsPressed()
{
    return _isPressed;
}