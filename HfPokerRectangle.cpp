
// Author: Jonas Byström, highfestiva@gmail.com. August 2003.
// Open source without neither limitations nor warranty.


#include "SysInclude.h"
#include "HfPokerRectangle.h"


namespace HfPoker
{


Rectangle::Rectangle(const Rectangle& pCopy):
	mX1(pCopy.mX1),
	mY1(pCopy.mY1),
	mX2(pCopy.mX2),
	mY2(pCopy.mY2)
{
}

Rectangle::Rectangle(int pX1, int pY1, int pX2, int pY2):
	mX1(pX1),
	mY1(pY1),
	mX2(pX2),
	mY2(pY2)
{
}


void Rectangle::AdjustSize(unsigned pWidth, unsigned pHeight)
{
	if (mX1 < 0)
	{
		mX1 = 0;
	}
	if (mY1 < 0)
	{
		mY1 = 0;
	}
	if (mX2 >= (int)pWidth)
	{
		mX2 = pWidth-1;
	}
	if (mY2 >= (int)pHeight)
	{
		mY2 = pHeight-1;
	}
}


};
