#include <Arduino.h>

enum ButtonEvent
{
    None,
    Pressed,
    Released
};

class DebounceButton
{
    private:
        int _pin;
        uint64_t _lastChangeStateTime = 0;
        bool _isPressed = false;       
        bool _previousInputState = false;
        int _debounceInterval = 50;
        int _buttonMode;
    public:
        DebounceButton(int pin, int buttonMode = INPUT_PULLUP, int debounceTime = 50);
        void Init();
        ButtonEvent Loop();
        ButtonEvent Loop(bool inputState);
        bool IsPressed();
};