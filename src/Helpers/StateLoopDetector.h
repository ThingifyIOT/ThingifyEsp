#ifndef _STATE_LOOP_DETECTOR_H
#define _STATE_LOOP_DETECTOR_H

class LoopStateDetector
{
	int _state1 = -1;
	int _state2 = -1;
	int _previousState = -1;
	int _loopCount = 0;
public:
    int State1();
    int State2();
	void FeedState(int state);
	int LoopCount();
	void Reset();
};

#endif