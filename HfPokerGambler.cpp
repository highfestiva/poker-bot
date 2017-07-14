
// Author: Jonas Byström, highfestiva@gmail.com. August 2003.
// Open source without neither limitations nor warranty.


#include "SysInclude.h"
#include "HfPokerGambler.h"
#include "HfPokerGameState.h"
#include "HfStd.h"


namespace HfPoker
{


Gambler::Gambler():
	mMyChance(0),
	mChoice(WAIT),
	mDealPlayerCount(-1),
	mTotalRaiseCount(0),
	mcChanceModel(mscNormalChance)
{
	memset(mRaiseCount, 0, sizeof(mRaiseCount));
	mDebugText[0] = '\0';
	mLogText[0] = '\0';
}


GAME_ACTION Gambler::DescidePlay(const GameState& pState)
{
	mDebugText[0] = '\0';
	mLogText[0] = '\0';
	mMyChance = 0;

	// Don't do anything if not updated.
	if (!pState.mUpdated)
	{
		return (WAIT);
	}

	// Reset if at new deal.
	if (pState.mNewDeal)
	{
		memset(mRaiseCount, 0, sizeof(mRaiseCount));
		mTotalRaiseCount = 0;
	}

	mChoice = WAIT;
	if (pState.mSelfPlaying)
	{
		if (pState.mNewDeal)			// Or just more players.
		{
			mDealPlayerCount = pState.mPlayerCount;
			memset(mRaiseCount, 0, sizeof(mRaiseCount));
			mTotalRaiseCount = 0;

			::strcpy(mLogText, "[N]");

			// Select the mode (every new deal).
			/*double lModelSelect = GetRandomDouble();
			if (lModelSelect < 0.66)
			{
				::strcpy(mLogText, "[Normal model]");
				mcChanceModel = mscNormalChance;
			}
			else if (lModelSelect < 0.83)
			{
				::strcpy(mLogText, "[High model]  ");
				mcChanceModel = mscHighChance;
			}
			else
			{
				::strcpy(mLogText, "[Low model]   ");
				mcChanceModel = mscLowChance;
			}*/
		}

		// Take a descision from the game state.
		DescideMyPlay(pState);

		// Adjust the descisions to normal brain capacity.
		/*if (mDealPlayerCount <= 4)	// Four or less players (me & 3 others).
		{
			// When less players, we need more bluffin'.
			if (mChoice == CALL &&						// Wants us to call.
				pState.mMyPrice <= pState.mOthersBestKnownPrice)	// Best view.
				pState.mMyPrice <= 6)					// Three of a kind or better.
			{
				if (mTotalRaiseCount < 5)	// Don't raise too much.
				{
					++mTotalRaiseCount;
					mChoice = RAISE;	// Raise on three of a kind or better!
				}
			}
		}*/
		if (mChoice == FOLD_OR_CHECK &&					// Wants us to fold or check.
			pState.mMyPrice <= 8 &&					// Pair or higher.
			pState.mMyPrice <= pState.mOthersBestKnownPrice)	// Best view?
		{
			if (pState.mMyPrice < pState.mOthersBestKnownPrice ||
				pState.mMyHighCard > pState.mOthersBestKnownHighCard)
			{
				::strcat(mDebugText, "Re-guess: ");
				GameState lState(pState);
				if (pState.mMyPrice < pState.mOthersBestKnownPrice &&
					pState.mMyPrice <= 7)	// At least two pairs.
				{
					//lState.mMyChance *= Lerp(1, 1.2, pState.mMyHighCard/(double)CARD_A);
					lState.mMyChance *= Lerp(1.03, 3, (7-pState.mMyPrice)/10.0);
				}
				else
				{
					//lState.mMyChance *= Lerp(1, 1.15, pState.mMyHighCard/(double)CARD_A);
				}
				lState.mMyChance *= Lerp(1.4, 1, pState.mCardCount/7.0);
				DescideMyPlay(lState);
			}
		}
	}
	else
	{
		::strcpy(mDebugText, "Self not in deal!\r\n");
		memset(mRaiseCount, 0, sizeof(mRaiseCount));
		mTotalRaiseCount = 0;
	}
	return (mChoice);
}


const char* Gambler::GetDebugText() const
{
	assert(::strlen(mDebugText) < sizeof(mDebugText)/2);
	return (mDebugText);
}

const char* Gambler::GetLogText() const
{
	assert(::strlen(mLogText) < sizeof(mLogText)/2);
	return ((const char*)mLogText);
}

double Gambler::GetMyChance() const
{
	return (mMyChance);
}


void Gambler::DescideMyPlay(const GameState& pState)
{
	int lChanceBase = (pState.mCardCount-3)*2;

	double lMidChance = pState.mMyChance*(pState.mPlayerCount-1)/pState.mOthersChance;
	double lOneOnOneChance = pState.mMyChance/pState.mOthersBestChance;
	double lWorstChance = lOneOnOneChance;
	if (lMidChance < lWorstChance)
	{
		lWorstChance = lMidChance;
	}

	// Let the chance distribution depend on the street.
	mMyChance = 0;
	switch (pState.mCardCount)
	{
		case 3:
		case 4:
		{
			//mMyChance = Lerp(lMidChance, lWorstChance, (pState.mCardCount-3)/2.0);
			mMyChance = lWorstChance;
			if (mMyChance > mcChanceModel[lChanceBase+0])
			{
				mChoice = RAISE;
			}
			else if (mMyChance > mcChanceModel[lChanceBase+1])
			{
				mChoice = CALL;
			}
			else
			{
				mChoice = FOLD_OR_CHECK;
			}
		}
		break;
		case 5:
		case 6:
		{
			mMyChance = lWorstChance;
			if (mMyChance > mcChanceModel[lChanceBase+0])
			{
				mChoice = RAISE;
			}
			else if (mMyChance > mcChanceModel[lChanceBase+1])
			{
				mChoice = CALL;
			}
			else
			{
				mChoice = FOLD_OR_CHECK;
			}
		}
		break;
		case 7:
		{
			if (pState.mMyPrice > pState.mOthersBestKnownPrice)
			{
				lWorstChance /= 100;
			}
			else if (pState.mMyPrice == pState.mOthersBestKnownPrice)
			{
				if (pState.mMyHighCard < pState.mOthersBestKnownHighCard)
				{
					lWorstChance /= 10;
				}
				else if (pState.mMyHighCard == pState.mOthersBestKnownHighCard)
				{
					lWorstChance *= 0.95;
				}
			}
			mMyChance = lWorstChance;
			if (mMyChance > mcChanceModel[lChanceBase+0])
			{
				mChoice = RAISE;
			}
			else if (mMyChance > mcChanceModel[lChanceBase+1])
			{
				mChoice = CALL;
			}
			else if (mMyChance > mcChanceModel[lChanceBase+2])
			{
				if (pState.mMyPrice <= pState.mOthersBestKnownPrice)
				{
					mChoice = CALL;
				}
				else
				{
					mChoice = FOLD_OR_CHECK;
				}
			}
			else
			{
				mChoice = FOLD_OR_CHECK;
			}
		}
		break;
	}

	// Limit the number of times I raise.
	if (mChoice == RAISE)
	{
		++mRaiseCount[pState.mCardCount-3];
		++mTotalRaiseCount;
		if (mMyChance < 2.0)
		{
			/*
			if (pState.mMyPrice >= 8 &&			// I have pair or worse.
				pState.mOthersBestKnownPrice <= 8 &&	// Other have pair or better.
				pState.mMyHighCard < pState.mOthersBestKnownHighCard &&	// Lower high card than other.
			{
			}
			*/
			/*
			if (pState.mMyPrice >= 8 &&			// Pair or worse.
				pState.mCardCount == 7)
			{
				--mRaiseCount[pState.mCardCount-3];
				--mTotalRaiseCount;
				mChoice = CALL;
			}
			else if (pState.mMyPrice == 7 &&		// Two pairs.
				(mRaiseCount[pState.mCardCount-3] > 1 ||	// More than one raise this round?
				pState.mRaiseCount > 2))		// More than this total number of raises.
			{
				--mRaiseCount[pState.mCardCount-3];
				--mTotalRaiseCount;
				mChoice = CALL;
			}
			/*else if (pState.mMyPrice == 6 &&		// Three of a kind.
				(mRaiseCount[pState.mCardCount-3] > 1 ||	// More than one raise this round?
				pState.mRaiseCount > 3))		// More than this total number of raises.
			{
				--mRaiseCount[pState.mCardCount-3];
				--mTotalRaiseCount;
				mChoice = CALL;
			}
			else if (pState.mMyPrice == 5 &&		// Straight.
				(mRaiseCount[pState.mCardCount-3] > 1 ||	// More than one raise this round?
				pState.mRaiseCount > 4))		// More than this total number of raises.
			{
				--mRaiseCount[pState.mCardCount-3];
				--mTotalRaiseCount;
				mChoice = CALL;
			}
			else if (pState.mMyPrice == 4 &&	// Flush.
				pState.mRaiseCount > 5)		// More than this total number of raises.
			{
				--mRaiseCount[pState.mCardCount-3];
				--mTotalRaiseCount;
				mChoice = CALL;
			}*/
		}
	}
	else if (mChoice == CALL)
	{
		// Perhaps we shouldn't call too much, if we've got
		// bad cards. This is probably a money-saver.
		// An on the other hand: if we've had almost only
		// checks before, then we try a small raise...
		//double lHighMid = Lerp(1, mcChanceModel[lChanceBase+0], 0.8);
		//double lLowMid = Lerp(1, mcChanceModel[lChanceBase+1], 0.8);
		switch (pState.mCardCount)
		{
			case 3:
			{
				if (pState.mMyPrice <= 6)	// 3oak.
				{
					mChoice = RAISE;
				}
				/*else if (mMyChance < lLowMid &&
					pState.mCallCount >= 1 &&
					pState.mMyPrice >= 9)	// Nothing.
				{
					mChoice = FOLD_OR_CHECK;
				}*/
			}
			break;
			case 4:
			{
				if (pState.mMyPrice <= 7)	// 2prs.
				{
					mChoice = RAISE;
				}
				/*else if (mMyChance < lLowMid &&
					pState.mCallCount >= 3 &&
					pState.mMyPrice >= 9)
				{
					mChoice = FOLD_OR_CHECK;
				}*/
			}
			break;
			case 5:
			{
				/*if (mMyChance < lLowMid &&
					//pState.mCallCount >= 5 &&
					pState.mMyPrice >= 9)	// Nothing.
				{
					mChoice = FOLD_OR_CHECK;
				}
				else*/ if (mMyChance > 1 &&
					//pState.mCallCount+pState.mRaiseCount <= 1 &&
					pState.mMyPrice <= 6)	// 3oak.
				{
					mChoice = RAISE;
				}
			}
			break;
			case 6:
			{
				/*if (mMyChance < lLowMid &&
					pState.mCallCount >= 6)
				{
					mChoice = FOLD_OR_CHECK;
				}
				else*/  if (mMyChance > 1 &&
					//pState.mCallCount+pState.mRaiseCount <= 1 &&
					pState.mMyPrice <= 6)	// 3oak.
				{
					mChoice = RAISE;
				}
			}
			break;
			case 7:
			{
				/*if (mMyChance < 0.8 &&
					pState.mCallCount >= 7)
				{
					//mChoice = FOLD_OR_CHECK;
				}
				else if (mMyChance > 1.9 &&
					//pState.mCallCount+pState.mRaiseCount <= 1 &&
					pState.mMyPrice <= 6)	// 3oak.
				{
					mChoice = RAISE;
				}*/
			}
			break;
		}
	}
	// Special handling for two-player river.
	else if (mChoice == FOLD_OR_CHECK &&	// Fold?
		pState.mCardCount == 7 &&	// On river?
		pState.mPlayerCount == 2 &&	// With only one other player?
		pState.mMyPrice <= 8 &&		// And I have a pair?
		mMyChance >= 0.7)		// And I still have a chance?
	{
		mChoice = CALL;
	}

	char a[100];
	::sprintf(a, "Odds: %g %%\r\n", mMyChance*100);
	::strcat(mDebugText, a);
}


const double Gambler::mscNormalChance[11] =	{1.30, 0.80, 1.35, 0.74, 1.30, 0.75, 1.30, 0.87, 1.00, 0.99, 0.84};
const double Gambler::mscHighChance[11] =	{1.70, 0.82, 1.70, 0.83, 1.25, 0.75, 1.30, 0.87, 1.00, 0.99, 0.84};
//const double Gambler::mscHighChance[11] =	{1.40, 0.76, 1.40, 0.84, 1.32, 0.89, 1.31, 0.90, 1.20, 1.00, 0.86};
const double Gambler::mscLowChance[11] =	{1.30, 0.73, 1.33, 0.73, 1.31, 0.75, 1.29, 0.86, 1.01, 0.99, 0.81};


};
