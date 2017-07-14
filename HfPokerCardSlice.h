
// Author: Jonas Byström, highfestiva@gmail.com. August 2003.
// Open source without neither limitations nor warranty.


#ifndef HFPOKERCARDSLICE_H
#define HFPOKERCARDSLICE_H


#include "HfPokerCard.h"
#include "HfPokerRectangle.h"


namespace HfPoker
{


class CardSlice
{
public:
	bool		mIsRed;
	Rectangle	mRectangle;
	Card		mCardId;

	CardSlice(bool pIsRed, const Rectangle pRect);
};


};


#endif // HFPOKERCARDSLICE_H
