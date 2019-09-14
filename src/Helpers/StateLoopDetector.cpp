
#include "StateLoopDetector.h"

int LoopStateDetector::State1()
{
    return _state1;
}
int LoopStateDetector::State2()
{
    return _state2;
}
void LoopStateDetector::FeedState(int state)
{
    if (state == _previousState)
    {
        return;
    }
    _previousState = state;
    if (_state2 == -1)
    {
        _state2 = state;
        return;
    }

    if (state == _state2)
    {
        _loopCount++;
    }
    else if(state != _state1)
    {
        _state1 = state;
        _state2 = -1;
        _loopCount = 0;
    }
}

int LoopStateDetector::LoopCount()
{
    return _loopCount;
}

void LoopStateDetector::Reset()
{
    _state1 = -1;
    _state2 = -1;
    _previousState = -1;
    _loopCount = 0;
}