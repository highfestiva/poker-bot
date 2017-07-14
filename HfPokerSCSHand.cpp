
// Author: Jonas Byström, highfestiva@gmail.com. August 2003.
// Open source without neither limitations nor warranty.


#include "SysInclude.h"
#include "HfPokerSCSHand.h"


namespace HfPoker
{


SCSHand::SCSHand(const Hand& pcCopy, int pTotalCardCount):
	Hand(pcCopy),
	mTotalCardCount(pTotalCardCount)
{
}


Hand* SCSHand::GetCopy() const
{
	SCSHand* lCopy = new SCSHand(*this);
	return (lCopy);
}


int SCSHand::GetTotalCardCount() const
{
	return (mTotalCardCount);
}


void SCSHand::operator=(const SCSHand& pHand) const
{
	assert(false);
}


};
