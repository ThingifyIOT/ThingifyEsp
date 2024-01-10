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
        uint64_t _lastDebounceTime = 0;
        bool _lastSteadyState = false;       
        bool _lastFlickerableState = false;
        int _debounceTime = 50;
        int _buttonMode;
    public:
        DebounceButton(int pin, int buttonMode = INPUT_PULLUP, int debounceTime = 50);
        void Init();
        ButtonEvent Loop();
        ButtonEvent Loop(bool inputState);
        bool IsPressed();
};