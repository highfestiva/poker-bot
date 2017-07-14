
// Author: Jonas Byström, highfestiva@gmail.com. August 2003.
// Open source without neither limitations nor warranty.


#ifndef HFPERFORMANCETIMER_H
#define HFPERFORMANCETIMER_H


#include "HfStd.h"


namespace HfPoker
{


class PerformanceTimer
{
public:
	PerformanceTimer();
	void Restart();
	double GetTime() const;
	double GetTimeAndRestart();

private:
	static uint64 QueryFrequency();

	uint64 mStartTime;
	static uint64 mFrequency;
};


};


#endif // HFPERFORMANCETIMER_H
