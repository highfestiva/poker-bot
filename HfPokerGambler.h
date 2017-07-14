
// Author: Jonas Byström, highfestiva@gmail.com. August 2003.
// Open source without neither limitations nor warranty.


#ifndef HFPOKERGAMBLER_H
#define HFPOKERGAMBLER_H


#include "HfPoker.h"


namespace HfPoker
{


class GameState;


class Gambler
{
public:
				Gambler();

	GAME_ACTION		DescidePlay(const GameState& pState);
	const char*		GetDebugText() const;
	const char*		GetLogText() const;
	double		GetMyChance() const;

protected:
	void			DescideMyPlay(const GameState& pState);

	double		mMyChance;
	GAME_ACTION		mChoice;
	int			mDealPlayerCount;
	int			mRaiseCount[5];
	int			mTotalRaiseCount;
	char			mDebugText[10000];
	char			mLogText[10000];

	const double*		mcChanceModel;
	static const double	mscNormalChance[11];
	static const double	mscHighChance[11];
	static const double	mscLowChance[11];
};


};


#endif // HFPOKERGAMBLER_H
