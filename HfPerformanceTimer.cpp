
// Author: Jonas Byström, highfestiva@gmail.com. August 2003.
// Open source without neither limitations nor warranty.


#include "SysInclude.h"
#include "HfPerformanceTimer.h"


namespace HfPoker
{


PerformanceTimer::PerformanceTimer()
{
	Restart();
}

void PerformanceTimer::Restart()
{
	LARGE_INTEGER lTime;
	::QueryPerformanceCounter(&lTime);
	mStartTime = lTime.QuadPart;
}

double PerformanceTimer::GetTime() const
{
	LARGE_INTEGER lTime;
	::QueryPerformanceCounter(&lTime);
	return ((lTime.QuadPart-mStartTime)/(double)mFrequency);
}

double PerformanceTimer::GetTimeAndRestart()
{
	LARGE_INTEGER lTime;
	::QueryPerformanceCounter(&lTime);
	double lTimer = (lTime.QuadPart-mStartTime)/(double)mFrequency;
	mStartTime = lTime.QuadPart;
	return (lTimer);
}


uint64 PerformanceTimer::QueryFrequency()
{
	LARGE_INTEGER lFrequency;
	::QueryPerformanceFrequency(&lFrequency);
	return (lFrequency.QuadPart);
}


uint64 PerformanceTimer::mFrequency = PerformanceTimer::QueryFrequency();


}
