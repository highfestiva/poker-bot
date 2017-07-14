
// Author: Jonas Byström, highfestiva@gmail.com. August 2003.
// Open source without neither limitations nor warranty.


#ifndef HFPOKERGAMESTATE_H
#define HFPOKERGAMESTATE_H


#include "HfPoker.h"


namespace HfPoker
{


class GameState
{
public:
			GameState():
				mUpdated(false),
				mNewDeal(false),
				mCheckCount(0),
				mCallCount(0),
				mRaiseCount(0),
				mLastCardCount(0)
			{
			};

	bool		mUpdated;
	bool		mNewDeal;
	bool		mDuringDeal;
	int		mCheckCount;
	int		mCallCount;
	int		mRaiseCount;
	int		mCardCount;
	int		mLastCardCount;
	int		mPlayerCount;
	bool		mSelfPlaying;
	double	mMyChance;
	int		mMyPrice;
	CARD_VALUE	mMyHighCard;
	double	mMyVisibleChance;
	int		mMyVisiblePrice;
	CARD_VALUE	mMyVisibleHighCard;
	double	mOthersChance;
	double	mOthersBestChance;
	int		mOthersBestKnownPrice;
	CARD_VALUE	mOthersBestKnownHighCard;
};


};


#endif // HFPOKERGAMESTATE_H
