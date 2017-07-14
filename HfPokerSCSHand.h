
// Author: Jonas Byström, highfestiva@gmail.com. August 2003.
// Open source without neither limitations nor warranty.


#ifndef HFPOKERSCSHAND_H
#define HFPOKERSCSHAND_H


#include "HfPokerHand.h"


namespace HfPoker
{


class SCSHand: public Hand
{
public:
			SCSHand(const Hand& pcCopy, int pTotalCardCount);

	virtual Hand*	GetCopy() const;

	int		GetTotalCardCount() const;

protected:
	int		mTotalCardCount;	// Including hidden cards.

	void operator=(const SCSHand& pHand) const;
};


};


#endif // HFPOKERSCSHAND_H
