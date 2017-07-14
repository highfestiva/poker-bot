
// Author: Jonas Byström, highfestiva@gmail.com. August 2003.
// Open source without neither limitations nor warranty.


#include "SysInclude.h"
#include "HfPokerLine.h"


namespace HfPoker
{


bool Line::IsHorizontal() const
{
	return (mY1 == mY2);
}

bool Line::IsVertical() const
{
	return (mX1 == mX2);
}


};
