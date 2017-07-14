
// Author: Jonas Byström, highfestiva@gmail.com. August 2003.
// Open source without neither limitations nor warranty.


#include "SysInclude.h"
#include <math.h>
#include "HfPerformanceTimer.h"
#include "HfPokerDecks.h"
#include "HfPokerGameState.h"
#include "HfPokerSCSHand.h"
#include "HfPokerSCSAnalyzer.h"
#include "HfPokerTable.h"
#include "HfStd.h"


#if SEVEN_CARD_STUD_DECK_COUNT != 1
#error "Code not made with respect to several decks!"
#endif // SEVEN_CARD_STUD_DECK_COUNT != 1


namespace HfPoker
{


double Lerp(double t, double f1, double f2)
{
	return (t*f2 + (1-t)*f1);
}

bool FloatCompare(double pDeviation, double f1, double f2)
{
	return (f1 >= f2/pDeviation && f1 <= f2*pDeviation);
}

void TryThrow(bool b)
{
	if (!b)
	{
		throw(false);
	}
}


SCSAnalyzer::SCSAnalyzer():
	mSharedLastCard(false),
	mDeck(SEVEN_CARD_STUD_DECK_COUNT),
	mLastChanceForFourOfAKind(0),
	mLastChanceForFullHouse(0),
	mLastChanceForThreeOfAKind(0),
	mLastChanceForTwoPairs(0)
{
	// Test statistics algorithm integrity.
	double d;

	// Test 5 card hand.
	Hand lHand(0, 0);
	SCSHand lPokerHand(lHand, 0);
	lPokerHand.AddCard(Card(CARD_TYPE_INVALID, CARD_VALUE_INVALID));
	lPokerHand.AddCard(Card(CARD_TYPE_INVALID, CARD_VALUE_INVALID));
	Reset(); d = ChanceForRoyalFlush(lPokerHand);
	TryThrow(FloatCompare(1.00002, 1/d, 649740));			// 1 in 649740.
	Reset(); d = ChanceForStraightFlush(lPokerHand);
	TryThrow(FloatCompare(1.00002, 1/d, 72193.33));			// 1 in 72193.33.
	Reset(); d = ChanceForFourOfAKind(lPokerHand);
	TryThrow(FloatCompare(1.00002, 1/d, 4165));			// 1 in 4165.
	Reset(); d = ChanceForFullHouse(lPokerHand);
	TryThrow(FloatCompare(1.00002, 1/d, 694.167));			// 1 in 694.167.
	Reset(); d = ChanceForFlush(lPokerHand);
	TryThrow(FloatCompare(1.00002, 1/d, 508.80));			// 1 in 508.80.
	Reset(); d = ChanceForStraight(lPokerHand);
	TryThrow(FloatCompare(1.00002, 1/d, 254.8));			// 1 in 254.8.
	Reset(); d = ChanceForThreeOfAKind(lPokerHand);
	TryThrow(FloatCompare(1.00002, 1/d, 47.3295));			// 1 in 47.3295.
	Reset(); d = ChanceForTwoPairs(lPokerHand);
	TryThrow(FloatCompare(1.00002, 1/d, 21.035));			// 1 in 21.035.
	Reset(); d = ChanceForPair(lPokerHand);
	TryThrow(FloatCompare(1.00021, 1/d, 2.366));			// 1 in 2.366.
	Reset();

	// Test 7 card hand.
	SCSHand lStudHand(lHand, 0);
	Reset(); d = ChanceForRoyalFlush(lStudHand);
	TryThrow(FloatCompare(1.2, d, 0.00003232));
	Reset(); d = ChanceForStraightFlush(lStudHand);
	TryThrow(FloatCompare(1.2, d, 0.00027851));
	Reset(); d = ChanceForFourOfAKind(lStudHand);
	TryThrow(FloatCompare(1.2, d, 0.00168067));
	Reset(); d = ChanceForFullHouse(lStudHand);
	TryThrow(FloatCompare(1.2, d, 0.02596102));
	Reset(); d = ChanceForFlush(lStudHand);
	TryThrow(FloatCompare(1.2, d, 0.03025494));
	Reset(); d = ChanceForStraight(lStudHand);
	TryThrow(FloatCompare(1.2, d, 0.04619382));
	Reset(); d = ChanceForThreeOfAKind(lStudHand);
	TryThrow(FloatCompare(1.2, d, 0.04829870));
	Reset(); d = ChanceForTwoPairs(lStudHand);
	TryThrow(FloatCompare(1.2, d, 0.23495536+0.07));	// TODO: is  7 % off, fixme.
	Reset(); d = ChanceForPair(lStudHand);
	TryThrow(FloatCompare(1.2, d, 0.43822546-0.07));	// TODO: is  7 % off, fixme.
	Reset();

	// Verify odds when we have some cards in our hand.
	lPokerHand.AddCard(Card(SPADES, CARD_A));
	lPokerHand.AddCard(Card(SPADES, CARD_2));
	lPokerHand.AddCard(Card(SPADES, CARD_3));
	mDeck.DropCard(Card(SPADES, CARD_A));
	mDeck.DropCard(Card(SPADES, CARD_2));
	mDeck.DropCard(Card(SPADES, CARD_3));
	d = ChanceForRoyalFlush(lPokerHand);
	TryThrow(FloatCompare(1.001, d, 0));	// Not a chance!
	d = ChanceForStraightFlush(lPokerHand);
	TryThrow(FloatCompare(1.001, d, 2*(1/49.0*1/48.0)));	// 4 and 5 of spades.
	d = ChanceForFlush(lPokerHand);
	TryThrow(FloatCompare(1.15, d, 10/49.0*8/48));	// All spades, except [4 AND 5].
	d = ChanceForStraight(lPokerHand);
	TryThrow(FloatCompare(1.001, d, 15*2*(1/49.0*1/48.0)));	// A specific combination of non-BOTH-spade 4 and 5. There are 15 such combinations.
	Reset();

	// A more complex analysis with only three cards in our hand. Comparing
	// against figures found on a Swedish Internet site.
	lPokerHand.DropMask(0x1F);	// Drop all cards.
	lPokerHand.AddCard(Card(HEARTS, CARD_A));
	lPokerHand.AddCard(Card(DIAMONDS, CARD_A));
	lPokerHand.AddCard(Card(CLUBS, CARD_A));
	mDeck.DropCard(Card(HEARTS, CARD_A));
	mDeck.DropCard(Card(DIAMONDS, CARD_A));
	mDeck.DropCard(Card(CLUBS, CARD_A));
	d = ChanceForRoyalFlush(lPokerHand);
	TryThrow(FloatCompare(1.00002, 1/d, 70625));	// This is my guess. Only 49 cards left in deck.
	d = ChanceForStraightFlush(lPokerHand);
	TryThrow(FloatCompare(1.001, 1/d, 70625));	// Of corse same as the royal flush.
	d = ChanceForFourOfAKind(lPokerHand);
	TryThrow(FloatCompare(1.001, d, 0.0817));	// 8.17 % according to the site.
	d = ChanceForFullHouse(lPokerHand);
	TryThrow(FloatCompare(1.03, d, 0.3202));	// 32.02 % according to the site.
	d = ChanceForFlush(lPokerHand);
	TryThrow(FloatCompare(1.003, d, 0.0070));	// 0.70 % according to the site.
	d = ChanceForStraight(lPokerHand);
	TryThrow(FloatCompare(1.003, d, 0.0072));	// 0.72 % is what I think. The site things 1/3 of that = 0.24 %. Perhaps someone made an error; there are three aces.
	// Move on by replacing one ace with a nine.
	lPokerHand.DropMask(0x01);	// Drop ace.
	lPokerHand.AddCard(Card(HEARTS, CARD_9));
	mDeck.AddCard(Card(HEARTS, CARD_A));
	mDeck.DropCard(Card(HEARTS, CARD_9));
	d = ChanceForRoyalFlush(lPokerHand);
	TryThrow(FloatCompare(1.00002, 1/d, 105938));	// This is my guess. Only 49 cards left in deck.
	d = ChanceForStraightFlush(lPokerHand);
	TryThrow(FloatCompare(1.001, 1/d, 30268));	// Of corse same as the royal flush.
	d = ChanceForFourOfAKind(lPokerHand);
	TryThrow(FloatCompare(1.006, d, 0.0054));	// 0.54 % according to the site.
	d = ChanceForFullHouse(lPokerHand);
	TryThrow(FloatCompare(1.041, d, 0.0757));	// 7.57 % according to the site, probably my bug.
	d = ChanceForFlush(lPokerHand);
	TryThrow(FloatCompare(1.03, d, 0.0070));	// 0.70 % according to the site, me thinks 0.68 %.
	d = ChanceForStraight(lPokerHand);
	TryThrow(FloatCompare(1.15, d, 0.0084));	// 0.84 % according to the site, probably my bug.
	d = ChanceForThreeOfAKind(lPokerHand);
	TryThrow(FloatCompare(1.06, d, 0.0989));	// 9.89 % according to the site, probably my bug.
	d = ChanceForTwoPairs(lPokerHand);
	TryThrow(FloatCompare(1.42, d, 0.4205));	// 42.05 % according to the site; BIG BUG! TODO: fixme!
	Reset();
}

SCSAnalyzer::~SCSAnalyzer()
{
}


void SCSAnalyzer::Reset()
{
	mGotPrice = false;
	mSharedLastCard = false;
	mDeck.Reset();
	GameAnalyzer::Reset();
}

void SCSAnalyzer::ResetOld()
{
	mGotPrice = false;
	mSharedLastCard = false;
	//mDeck.Reset();
	GameAnalyzer::ResetOld();
}


bool SCSAnalyzer::CalculateChance(GameState& pState)
{
	/*PerformanceTimer lTimer;

	for (unsigned x = 0; x < sizeof(mExactTimer)/sizeof(mExactTimer[0]); ++x)
	{
		mExactTimer[x] = 0;
	}*/

	if (!mTable)
	{
		Reset();
		return (false);
	}

	char a[100];
	int lLeastCards;
	int lMostCards;
	mTable->CountHandSizes(lLeastCards, lMostCards);
	if (lLeastCards < 1)
	{
		Reset();
		::sprintf(a, "ERROR: Least cards < 1!\r\n");
		::strcat(mDebugText, a);
		DumpScreenshot(mWidth, mHeight, mBPP, mBitmap);
		return (false);
	}
	if (lMostCards > 7)
	{
		Reset();
		::sprintf(a, "ERROR: Most cards > 7!\r\n");
		::strcat(mDebugText, a);
		DumpScreenshot(mWidth, mHeight, mBPP, mBitmap);
		return (false);
	}
	if (lLeastCards == lMostCards && lLeastCards == 7)
	{
		//ResetOld();
		::sprintf(a, "+Showdown!!!\r\n");
		::strcat(mDebugText, a);
		return (false);
	}

	if (mPreviousTable)
	{
		if (mPreviousTable->GetHands().size() < mTable->GetHands().size()-1)
		{
			// Seems more ppl joined in - drop all old data.
			// Only one new hand is allowed in extra: the open
			// last card.
			ResetOld();
			mDeck.Reset();
			::sprintf(a, "+New deal?\r\n");
			::strcat(mDebugText, a);
		}
		else
		{
			// Assume no special deal.
			mSharedLastCard = false;

			int lOldLeastCards;
			int lOldMostCards;
			mPreviousTable->CountHandSizes(lOldLeastCards, lOldMostCards);
			if (lMostCards < lOldMostCards)
			{
				if (lLeastCards < lOldLeastCards)
				{
					// Ppl have less cards, seems to be a new deal.
					ResetOld();
					mDeck.Reset();
					::sprintf(a, "+New deal?\r\n");
					::strcat(mDebugText, a);
				}
				else if (lLeastCards == lMostCards)
				{
					::sprintf(a, "+Self fold?\r\n");
					::strcat(mDebugText, a);
				}
				else
				{
					Reset();
					::sprintf(a, "ERROR: Less cards?!?\r\n");
					::strcat(mDebugText, a);
					DumpScreenshot(mWidth, mHeight, mBPP, mBitmap);
				}
			}
			else if (lMostCards-1 > lOldMostCards)
			{
				// Ppl have more than one new card, seems showdown.
				//ResetOld();
				::sprintf(a, "+Showdown (or missed a deal)?\r\n");
				::strcat(mDebugText, a);
			}
			else if (mTable->GetHands().size() >= 3 &&	// More than two hands (me, player2 & shared)?
				mTable->CountHandsOfSize(1) == 1)	// Just one hand sharing one card?
			{
				// The dealer doesn't have enough cards to give everyone
				// a seventh card, so he deals it face-up.
				mSharedLastCard = true;
				::sprintf(a, "Shared card?\r\n");
				::strcat(mDebugText, a);
			}
			else if (lLeastCards < lOldLeastCards)
			{
				// Suddenly we have less cards than before.
				ResetOld();
				::sprintf(a, "ERROR: Dropped cards?\r\n");
				::strcat(mDebugText, a);
				DumpScreenshot(mWidth, mHeight, mBPP, mBitmap);
			}
		}
	}

	if (!mTable)
	{
		// Crap. We seem to have blown up the table!
		Reset();
		::sprintf(a, "ERROR: Table gone!\r\n");
		::strcat(mDebugText, a);
		DumpScreenshot(mWidth, mHeight, mBPP, mBitmap);
		return (false);
	}
	else if (mTable->GetHands().size() == 1)
	{
		// Too many or too few players?
		Reset();
		::sprintf(a, "ERROR: Showdown? Mid deal?\r\n");
		::strcat(mDebugText, a);
		DumpScreenshot(mWidth, mHeight, mBPP, mBitmap);
		return (false);
	}
	else if (mTable->GetHands().size() < 1 || mTable->GetHands().size() > 8)
	{
		// Too many or too few players?
		Reset();
		::sprintf(a, "ERROR: Player overflow/underflow?\r\n");
		::strcat(mDebugText, a);
		DumpScreenshot(mWidth, mHeight, mBPP, mBitmap);
		return (false);
	}

	const std::list<Hand*>& lHands = mTable->GetHands();

	// Verify that no cards have been reused this deal.
	// Do it by pushing the old deal and pulling the new.
	Decks lDeckCopy(mDeck);
	if (!mDeck.IsUntouched() && mPreviousTable)
	{
		const std::list<Hand*>& lOldHands = mPreviousTable->GetHands();
		for (std::list<Hand*>::const_iterator k = lOldHands.begin(); k != lOldHands.end(); ++k)
		{
			try
			{
				lDeckCopy.AddCards(**k);
			}
			catch (bool)
			{
				// New types of cards = new deal!
				Reset();
				::sprintf(a, "ERROR: Old deal doesn't fit in deck?\r\n");
				::strcat(mDebugText, a);
				DumpScreenshot(mWidth, mHeight, mBPP, mBitmap);
				return (false);
			}
		}
	}
	bool lReused = false;
	for (std::list<Hand*>::const_iterator l = lHands.begin(); l != lHands.end(); ++l)
	{
		for (int i = 0; i < (*l)->GetCardCount(); ++i)
		{
			try
			{
				lDeckCopy.DropCard((*l)->GetCard(i));
			}
			catch (bool)
			{
				// New types of cards = new deal!
				lReused = true;
				char lCardName[100];
				(*l)->GetCard(i).GetName(lCardName);
				::sprintf(a, " -%s\r\n", lCardName);
				::strcat(mDebugText, a);
			}
		}
	}
	if (lReused)
	{
		Reset();
		::sprintf(a, "ERROR: Deal contain reused cards?\r\n");
		::strcat(mDebugText, a);
		DumpScreenshot(mWidth, mHeight, mBPP, mBitmap);
		return (false);
	}

	// Check if my hand is still on the table, and
	// that the deal is done.
	Hand* lMyHand = 0;
	Hand* lSharedHand = 0;
	int lTotalCardsInHand = -1;
	bool lHandCounts[20];
	memset(lHandCounts, 0, sizeof(lHandCounts));
	int lHandSizeCount = 0;
	for (std::list<Hand*>::const_iterator i = lHands.begin(); i != lHands.end(); ++i)
	{
		int lCardCount = (*i)->GetCardCount();
		if (mSharedLastCard && lCardCount == 1)
		{
			lSharedHand = *i;
		}
		else
		{
			if (!lHandCounts[lCardCount])
			{
				lHandCounts[lCardCount] = true;
				++lHandSizeCount;
			}
			if (lCardCount > lTotalCardsInHand)
			{
				lMyHand = *i;
				lTotalCardsInHand = lMyHand->GetCardCount();
			}
			else if (lCardCount == lTotalCardsInHand)
			{
				lMyHand = 0;
			}
		}
		//try
		{
			mDeck.DropCardsConditional(**i);
		}
		/*catch (bool)
		{
			// Misinterpreting a card? Anyway unable to handle the
			// card setting. Try again next time.
			Reset();
			char a[100];
			::sprintf(a, "Unable to handle deal!\r\n");
			::strcat(mDebugText, a);
			return (false);
		}*/
	}
	if (lHands.size() == 1 && lMyHand)
	{
		lMyHand = 0;
	}
	// Check if the deal isn't done yet.
	if (lHandSizeCount < 1 || lHandSizeCount > 2)
	{
		// Nope, keep on trying..
		mWantNextDeal = true;
		return (false);
	}
	// Descide on how many visible cards there are.
	if (lHandSizeCount == 1)
	{
		if (mSharedLastCard)
		{
			if (lTotalCardsInHand != 4)
			{
				::sprintf(a, "ERROR: Deal contain reused cards?\r\n");
				::strcat(mDebugText, a);
				DumpScreenshot(mWidth, mHeight, mBPP, mBitmap);
			}
			assert(lTotalCardsInHand == 4);
			lTotalCardsInHand = 7;	// 2 hidden and 1 shared cards.
		}
		else if (lTotalCardsInHand == 4)
		{
			lTotalCardsInHand = 7;	// 3 hidden cards.
		}
		else if (lTotalCardsInHand < 4)
		{
			lTotalCardsInHand += 2;	// 2 hidden cards.
		}
	}
	else if (lHandSizeCount == 2)	// Am I in?
	{
		if (mSharedLastCard)
		{
			if (lTotalCardsInHand != 6)
			{
				::sprintf(a, "ERROR: shared card, but I have less than six other cards!\r\n");
				::strcat(mDebugText, a);
				DumpScreenshot(mWidth, mHeight, mBPP, mBitmap);
			}
			assert(lTotalCardsInHand == 6);
			lTotalCardsInHand = 7;	// 2 hidden and 1 shared cards.
		}
	}
	else
	{
		::sprintf(a, "ERROR: too many types of visible hand sizes: %i!\r\n", lHandSizeCount);
		::strcat(mDebugText, a);
		DumpScreenshot(mWidth, mHeight, mBPP, mBitmap);
		assert(false);
	}
	if (lTotalCardsInHand < 1 || lTotalCardsInHand > 7)
	{
		::sprintf(a, "ERROR: total cards in hand out of range: %i!\r\n", lTotalCardsInHand);
		::strcat(mDebugText, a);
		DumpScreenshot(mWidth, mHeight, mBPP, mBitmap);
	}
	assert(lTotalCardsInHand >= 1 && lTotalCardsInHand <= 7);

	// Calculate the chance for each hand, and
	// especially the "visible" chance for me.
	mBestHighCard = CARD_2;
	CARD_VALUE lMyVisibleHighCard = CARD_2;
	int lMyVisiblePrice = GetPriceCount()-1;
	double lOthersChance = 1;
	double lBestOtherChance = 1;
	int lBestOtherKnownPrice = GetPriceCount()-1;
	for (std::list<Hand*>::const_iterator j = lHands.begin(); j != lHands.end(); ++j)
	{
		Hand* lHand = *j;
		// If my hand: just store the chance visible to others.
		if (lHand == lMyHand)
		{
			// Put my "hidden" cards back into the deck.
			if (lTotalCardsInHand >= 2)
			{
				mDeck.AddCard(lMyHand->GetCard(0));
				mDeck.AddCard(lMyHand->GetCard(1));
				if (lMyHand->GetCardCount() == 7)
				{
					mDeck.AddCard(lMyHand->GetCard(6));
				}
			}

			// Copy the hand and drop the cards hidden to others.
			SCSHand lCopyHand(*lMyHand, lTotalCardsInHand);
			int lDropMask = 0x03;
			if (lMyHand->GetCardCount() == 7)
			{
				lDropMask |= 0x40;
			}
			lCopyHand.DropMask(lDropMask);
			if (lSharedHand)
			{
				lCopyHand.AddCard(lSharedHand->GetCard(0));
			}
			CARD_VALUE lOthersBestHighCard = mBestHighCard;
			mBestHighCard = CARD_2;
			lMyHand->SetChance(CalculateHandChance(lCopyHand, GetPriceCount()-1, &lMyVisiblePrice));
			lMyVisibleHighCard = mBestHighCard;
			mBestHighCard = lOthersBestHighCard;

			// Remove my "hidden" cards from the deck again.
			if (lTotalCardsInHand >= 2)
			{
				mDeck.DropCard(lMyHand->GetCard(0));
				mDeck.DropCard(lMyHand->GetCard(1));
				if (lMyHand->GetCardCount() == 7)
				{
					mDeck.DropCard(lMyHand->GetCard(6));
				}
			}
		}
		else if (lHand == lSharedHand)
		{
			lHand->SetChance(-1);
		}
		else
		{
			SCSHand lCopyHand(*lHand, lTotalCardsInHand);
			if (lSharedHand)
			{
				lCopyHand.AddCard(lSharedHand->GetCard(0));
			}
			lHand->SetChance(CalculateHandChance(lCopyHand, GetPriceCount()-1, &lBestOtherKnownPrice));
			double lOtherPersonsChance = lHand->GetChance();
			if (lOtherPersonsChance > lBestOtherChance)
			{
				lBestOtherChance = lOtherPersonsChance;
			}
			lOthersChance += lOtherPersonsChance;
		}

		// Since the card counting takes so much CPU, we
		// must let the others in as well.
		::Sleep(50);
	}

	pState.mUpdated = true;
	pState.mOthersChance = lOthersChance;
	pState.mOthersBestChance = lBestOtherChance;
	pState.mOthersBestKnownPrice = lBestOtherKnownPrice;
	pState.mOthersBestKnownHighCard = mBestHighCard;
	pState.mCardCount = lTotalCardsInHand;
	pState.mPlayerCount = (int)lHands.size();
	if (lSharedHand)
	{
		--pState.mPlayerCount;
	}

	if (lMyHand)
	{
		pState.mSelfPlaying = true;

		// Set chance so that the betting is ramped up towards
		// the end of the game if the hand is good.
		SCSHand lCopyHand(*lMyHand, lTotalCardsInHand);
		if (lSharedHand)
		{
			lCopyHand.AddCard(lSharedHand->GetCard(0));
		}
		int lMyPrice = GetPriceCount()-1;
		mBestHighCard = CARD_2;
		double lFullChance = CalculateHandChance(lCopyHand, lBestOtherKnownPrice, &lMyPrice);
		pState.mMyChance = lFullChance;
		pState.mMyPrice = lMyPrice;
		pState.mMyHighCard = mBestHighCard;
		pState.mMyVisibleChance = lMyHand->GetChance();
		pState.mMyVisiblePrice = lMyVisiblePrice;
		pState.mMyVisibleHighCard = lMyVisibleHighCard;
	}
	else
	{
		pState.mSelfPlaying = false;
	}

	/*for (unsigned y = 0; y < sizeof(mExactTimer)/sizeof(mExactTimer[0]); ++y)
	{
		::sprintf(a, "Exact[%u]=%g\r\n", y, mExactTimer[y]);
		::strcat(mDebugText, a);
	}
	::sprintf(a, "StatAnlz=%g\r\n", lTimer.GetTime());
	::strcat(mDebugText, a);*/

	return (true);
}

double SCSAnalyzer::CalculateHandChance(const SCSHand& pcHand, int pMinPrice, int* pBestPrice)
{
	mHighCard = CARD_2;

	// Calculation loop.
	double lTotalChance = 0;
	double lTotalChanceWeight = 0;
	for (int lPrice = 0; lPrice <= pMinPrice; ++lPrice)
	{
		/*char a[100];
		PerformanceTimer lTimer;*/

		double lChance = 0;
		mGotPrice = false;
		switch(lPrice)
		{
			case 0:	// Royal flush.
			{
				lChance = ChanceForRoyalFlush(pcHand);
			}
			break;
			case 1:	// Straight flush.
			{
				lChance = ChanceForStraightFlush(pcHand);
				//::sprintf(a, "SF: %g\r\n", lTimer.GetTime());
				//::strcat(mDebugText, a);
			}
			break;
			case 2:	// Four of a kind.
			{
				lChance = ChanceForFourOfAKind(pcHand);
				//::sprintf(a, "4: %g\r\n", lTimer.GetTime());
				//::strcat(mDebugText, a);
			}
			break;
			case 3:	// Full house.
			{
				lChance = ChanceForFullHouse(pcHand);
				//::sprintf(a, "FH: %g\r\n", lTimer.GetTime());
				//::strcat(mDebugText, a);
			}
			break;
			case 4:	// Flush.
			{
				lChance = ChanceForFlush(pcHand);
				//::sprintf(a, "F: %g\r\n", lTimer.GetTime());
				//::strcat(mDebugText, a);
			}
			break;
			case 5:	// Straight.
			{
				lChance = ChanceForStraight(pcHand);
				//::sprintf(a, "S: %g\r\n", lTimer.GetTime());
				//::strcat(mDebugText, a);
			}
			break;
			case 6:	// Three of a kind.
			{
				lChance = ChanceForThreeOfAKind(pcHand);
				//::sprintf(a, "3: %g\r\n", lTimer.GetTime());
				//::strcat(mDebugText, a);
			}
			break;
			case 7:	// Two pairs.
			{
				lChance = ChanceForTwoPairs(pcHand);
				//::sprintf(a, "22: %g\r\n", lTimer.GetTime());
				//::strcat(mDebugText, a);
			}
			break;
			case 8:	// Pair.
			{
				lChance = ChanceForPair(pcHand);
				//::sprintf(a, "2: %g\r\n", lTimer.GetTime());
				//::strcat(mDebugText, a);
			}
			break;
			case 9:
			{
				lChance = 1-lTotalChance;
			}
			break;
			default:
			{
				char a[50];
				::sprintf(a, "ERROR: Checking hand chance on price %i!\r\n", lPrice);
				::strcat(mDebugText, a);
				DumpScreenshot(mWidth, mHeight, mBPP, mBitmap);
				assert(false);
			}
			break;
		}
		//assert(lChance >= 0);
		if (lChance >= 1)
		{
			lChance = 1;
			//mGotPrice = true;
		}
		else if (lChance < 0)
		{
			lChance = 0;
		}
		double lValueFactor = 1;
		if (mGotPrice)
		{
			// Check if we should store our price.
			if (pBestPrice && lPrice <= *pBestPrice)
			{
				if (lPrice < *pBestPrice)
				{
					mBestHighCard = mHighCard;
				}
				else if (mHighCard > mBestHighCard)	// Means same price.
				{
					mBestHighCard = mHighCard;
				}
				*pBestPrice = lPrice;
			}

			// Weigh in the value.
			lValueFactor = Lerp(1.00, 1.04, mHighCard/(double)CARD_A);
		}
		lTotalChance += lChance;
		lTotalChanceWeight += GetPriceWeight(lPrice) * lChance * lValueFactor;
		/*if (mGotPrice)
		{
			break;
		}*/
	}

	return (lTotalChanceWeight);
}


double SCSAnalyzer::ChanceForRoyalFlush(const SCSHand& pcHand)
{
	// lChance for royal flush hearts + ... + royal flush clubs.
	Card lHearts[] = {Card(HEARTS, CARD_10), Card(HEARTS, CARD_J), Card(HEARTS, CARD_Q), Card(HEARTS, CARD_K), Card(HEARTS, CARD_A)};
	Card lDiamonds[] = {Card(DIAMONDS, CARD_10), Card(DIAMONDS, CARD_J), Card(DIAMONDS, CARD_Q), Card(DIAMONDS, CARD_K), Card(DIAMONDS, CARD_A)};
	Card lSpades[] = {Card(SPADES, CARD_10), Card(SPADES, CARD_J), Card(SPADES, CARD_Q), Card(SPADES, CARD_K), Card(SPADES, CARD_A)};
	Card lClubs[] = {Card(CLUBS, CARD_10), Card(CLUBS, CARD_J), Card(CLUBS, CARD_Q), Card(CLUBS, CARD_K), Card(CLUBS, CARD_A)};
	double lChance =
		ChanceForExactCards(pcHand, lHearts, 5) +
		ChanceForExactCards(pcHand, lDiamonds, 5) +
		ChanceForExactCards(pcHand, lSpades, 5) +
		ChanceForExactCards(pcHand, lClubs, 5);
	if (lChance >= 1)
	{
		mHighCard = CARD_A;
		mGotPrice = true;
	}
	mLastChanceForRoyalFlush = lChance;
	return (lChance);
}

double SCSAnalyzer::ChanceForStraightFlush(const SCSHand& pcHand)
{
	unsigned lCombineCount = 0;
	double lChance = 0;
	Card lCards[5];
	Card lUnwantedCard;
	for (int i = -1; i <= 11-4; ++i)	// Don't count ace-high!
	{
		int k;
		for (k = 0; k < 5; ++k)
		{
			if (i+k == -1)
			{
				lCards[k].mValue = CARD_A;
			}
			else
			{
				lCards[k].mValue = (CARD_VALUE)(i+k);
			}
		}
		lUnwantedCard.mValue = (CARD_VALUE)(i+k);
		Card* lUnwantedCardPointer = &lUnwantedCard;
		int lUnwantedCardCount = 1;
		if (lUnwantedCard.mValue > CARD_A)
		{
			lUnwantedCardPointer = 0;
			lUnwantedCardCount = 0;
		}
		for (int j = HEARTS; j <= SPADES; ++j)
		{
			for (int k = 0; k < 5; ++k)
			{
				lCards[k].mType = (CARD_TYPE)j;
			}
			lUnwantedCard.mType = (CARD_TYPE)j;
			++lCombineCount;
			double lTmpChance = ChanceForExactCards(pcHand, lCards, 5, lUnwantedCardPointer, lUnwantedCardCount);
			lChance += lTmpChance;
			if (lTmpChance >= 1)
			{
				if (mHighCard < (CARD_VALUE)(i+4))
				{
					mHighCard = (CARD_VALUE)(i+4);
				}
				mGotPrice = true;
			}
		}
	}
	assert(lCombineCount == 9*4);
	mLastChanceForStraightFlush = lChance;
	return (lChance);
}

double SCSAnalyzer::ChanceForFourOfAKind(const SCSHand& pcHand)
{
	unsigned lCombineCount = 0;
	double lChance = 0;
	for (CARD_VALUE i = CARD_2; i <= CARD_A; ++(int&)i)
	{
		++lCombineCount;
		Card lCards[4];
		lCards[0].mType = HEARTS;
		lCards[1].mType = DIAMONDS;
		lCards[2].mType = CLUBS;
		lCards[3].mType = SPADES;
		lCards[0].mValue = i;
		lCards[1].mValue = i;
		lCards[2].mValue = i;
		lCards[3].mValue = i;
		double lTmpChance = ChanceForExactCards(pcHand, lCards, 4);
		lChance += lTmpChance;
		if (lTmpChance >= 1)
		{
			if (mHighCard < i)
			{
				mHighCard = i;
			}
			mGotPrice = true;
		}
	}
	assert(lCombineCount == 13);
	mLastChanceForFourOfAKind = lChance;
	return (lChance);
}

double SCSAnalyzer::ChanceForFullHouse(const SCSHand& pcHand)
{
	unsigned lCombineCount = 0;
	double lChance = 0;
	Card lUnwantedCards[3];
	int lUnwantedCardCounter;
	for (CARD_VALUE i = CARD_2; i <= CARD_A; ++(int&)i)
	{
		Card lCards[5];
		lCards[0].mValue = i;
		lCards[1].mValue = i;
		lCards[2].mValue = i;
		lUnwantedCards[0].mValue = i;
		for (CARD_VALUE j = CARD_2; j <= CARD_A; ++(int&)j)
		{
			// We don't want four or five of a kind nor a double of same card(s).
			if (i != j)
			{
				lCards[3].mValue = j;
				lCards[4].mValue = j;
				lUnwantedCards[1].mValue = j;
				lUnwantedCards[2].mValue = j;
				for (int it = 0; it < 4; ++it)
				{
					// The triplets.
					lCards[0].mType = mscThreeOutOfFour[it][0];
					lCards[1].mType = mscThreeOutOfFour[it][1];
					lCards[2].mType = mscThreeOutOfFour[it][2];
					lUnwantedCards[0].mType = mscOneOutOfFourComplement[it];
					for (int jt = 0; jt < 6; ++jt)
					{
						++lCombineCount;
						// The pair.
						lCards[3].mType = mscTwoOutOfFour[jt][0];
						lCards[4].mType = mscTwoOutOfFour[jt][1];
						lUnwantedCardCounter = 1;
						if (j > i)
						{
							lUnwantedCardCounter = 3;
							lUnwantedCards[1].mType = mscTwoOutOfFourComplement[jt][0];
							lUnwantedCards[2].mType = mscTwoOutOfFourComplement[jt][1];
						}
						double lTmpChance = ChanceForExactCards(pcHand, lCards, 5, lUnwantedCards, lUnwantedCardCounter);
						lChance += lTmpChance;
						if (lTmpChance >= 1)
						{
							if (mHighCard < i)
							{
								mHighCard = i;
							}
							mGotPrice = true;
						}
					}
				}
			}
		}
	}
	assert(lCombineCount == 13*12*4*6);
	mLastChanceForFullHouse = lChance;
	return (lChance);
}

double SCSAnalyzer::ChanceForFlush(const SCSHand& pcHand)
{
	unsigned lCombineCount = 0;
	unsigned lUnwantedCount = 0;
	double lChance = 0;
	Card lCards[5];
	Card lUnwantedCards[9];
	for (int l = 0; l < 9; ++l)
	{
		lUnwantedCards[l].mValue = (CARD_VALUE)(l+CARD_6);
	}
	for (int i = HEARTS; i <= SPADES; ++i)
	{
		for (int k = 0; k < 5; ++k)
		{
			lCards[k].mType = (CARD_TYPE)i;
		}
		for (int l = 0; l < 9; ++l)
		{
			lUnwantedCards[l].mType = (CARD_TYPE)i;
		}
		for (int it = 0x1F; it < (1<<13); ++it)
		{
			unsigned lSetBitCount = ::CountSetBits(it);
			if (lSetBitCount == 1)	// Optimization: speeds up the loop.
			{
				it |= 0xE;	// Next loop we'll hit 5 bits (...xxx1111, where one bit x =1 and the others =0).
			}
			else if (lSetBitCount == 5)
			{
				int j1 = ::GetSetBit(it, 0);
				lCards[0].mValue = (CARD_VALUE)j1;
				int j2 = ::GetSetBit(it, j1+1);
				lCards[1].mValue = (CARD_VALUE)j2;
				int j3 = ::GetSetBit(it, j2+1);
				lCards[2].mValue = (CARD_VALUE)j3;
				int j4 = ::GetSetBit(it, j3+1);
				lCards[3].mValue = (CARD_VALUE)j4;
				int j5 = ::GetSetBit(it, j4+1);
				lCards[4].mValue = (CARD_VALUE)j5;
				if (((j1+4 == j2+3) && (j1+4 == j3+2) && (j1+4 == j4+1) && (j1+4 == j5)) ||
					((j1 == CARD_2) && (j2 == CARD_3) && (j3 == CARD_4) && (j4 == CARD_5) && (j5 == CARD_A)))
				{
					// Don't calculate straight flush!
				}
				else
				{
					++lCombineCount;
					// Drop the unwanted cards. All higher combinations will
					// be calculated later in the loop, remove them now.
					Card* lUnwantedCardPointer = 0;
					int lUnwantedCardCounter = 0;
					if (j5 != CARD_A)
					{
						++lUnwantedCount;
						lUnwantedCardCounter = CARD_A-j5;
						lUnwantedCardPointer = &lUnwantedCards[9-lUnwantedCardCounter];
						assert(lUnwantedCardPointer >= lUnwantedCards);
						assert(lUnwantedCardPointer <= &lUnwantedCards[8]);
					}
					double lTmpChance = ChanceForExactCards(pcHand, lCards, 5, lUnwantedCardPointer, lUnwantedCardCounter);
					lChance += lTmpChance;
					if (lTmpChance >= 1)
					{
						if (mHighCard < lCards[4].mValue)
						{
							mHighCard = lCards[4].mValue;
						}
						mGotPrice = true;
					}
				}
			}
		}
	}
	assert(lCombineCount == 1277*4);
	// TODO: assert(lUnwantedCount == ???*4);
	mLastChanceForFlush = lChance;
	return (lChance);
}

double SCSAnalyzer::ChanceForStraight(const SCSHand& pcHand)
{
	// Wohhaaa! High complexity - O(n^6)!
	unsigned lCombineCount = 0;
	double lChance = 0;
	Card lCards[5];
	Card lUnwantedCards[4];
	Card* lUnwantedCardPointer;
	int lUnwantedCardCounter;
	for (int i = -1; i <= 12-4; ++i)
	{
		int k;
		for (k = 0; k < 5; ++k)
		{
			if (i+k == -1)
			{
				lCards[k].mValue = CARD_A;
			}
			else
			{
				lCards[k].mValue = (CARD_VALUE)(i+k);
			}
		}
		lUnwantedCardPointer = 0;
		lUnwantedCardCounter = 0;
		if ((CARD_VALUE)(i+k) <= CARD_A)
		{
			lUnwantedCardPointer = lUnwantedCards;
			lUnwantedCardCounter = 4;
			lUnwantedCards[0].mValue = (CARD_VALUE)(i+k);
			lUnwantedCards[0].mType = HEARTS;
			lUnwantedCards[1].mValue = lUnwantedCards[0].mValue;
			lUnwantedCards[1].mType = DIAMONDS;
			lUnwantedCards[2].mValue = lUnwantedCards[0].mValue;
			lUnwantedCards[2].mType = CLUBS;
			lUnwantedCards[3].mValue = lUnwantedCards[0].mValue;
			lUnwantedCards[3].mType = SPADES;
		}
		for (int j = HEARTS; j <= SPADES; ++j)
		{
			lCards[0].mType = (CARD_TYPE)j;
			for (int k = HEARTS; k <= SPADES; ++k)
			{
				lCards[1].mType = (CARD_TYPE)k;
				for (int l = HEARTS; l <= SPADES; ++l)
				{
					lCards[2].mType = (CARD_TYPE)l;
					for (int m = HEARTS; m <= SPADES; ++m)
					{
						lCards[3].mType = (CARD_TYPE)m;
						for (int n = HEARTS; n <= SPADES; ++n)
						{
							if ((j == k) && (j == l) && (j == m) && (j == n))
							{
								// Don't calculate straight flush.
							}
							else
							{
								++lCombineCount;
								lCards[4].mType = (CARD_TYPE)n;
								double lTmpChance = ChanceForExactCards(pcHand, lCards, 5, lUnwantedCardPointer, lUnwantedCardCounter);
								lChance += lTmpChance;
								if (lTmpChance >= 1)
								{
									if (mHighCard < (CARD_VALUE)(i+4))
									{
										mHighCard = (CARD_VALUE)(i+4);
									}
									mGotPrice = true;
								}
							}
						}
					}
				}
			}
		}
	}
	assert(lCombineCount == 10*1020);
	/*lChance -= mLastChanceForRoyalFlush;
	lChance -= mLastChanceForStraightFlush;*/
	mLastChanceForStraight = lChance;
	return (lChance);
}

double SCSAnalyzer::ChanceForThreeOfAKind(const SCSHand& pcHand)
{
	unsigned lCombineCount = 0;
	double lChance = 0;
	for (CARD_VALUE i = CARD_A; i >= CARD_2 ; --(int&)i)
	{
		Card lCards[3];
		lCards[0].mValue = i;
		lCards[1].mValue = i;
		lCards[2].mValue = i;
		for (int it = 0; it < 4; ++it)
		{
			++lCombineCount;
			lCards[0].mType = mscThreeOutOfFour[it][0];
			lCards[1].mType = mscThreeOutOfFour[it][1];
			lCards[2].mType = mscThreeOutOfFour[it][2];
			double lTmpChance = ChanceForExactCards(pcHand, lCards, 3);
			lChance += lTmpChance;
			if (lTmpChance >= 1)
			{
				if (mHighCard < i)
				{
					mHighCard = i;
				}
				mGotPrice = true;
			}
		}
	}
	assert(lCombineCount == 13*4);
	//lChance -= mLastChanceForRoyalFlush;
	//lChance -= mLastChanceForStraightFlush;
	lChance -= mLastChanceForFourOfAKind*4;
	lChance -= mLastChanceForFullHouse;
	//lChance -= mLastChanceForFlush;
	//lChance -= mLastChanceForStraight;
	mLastChanceForThreeOfAKind = lChance;
	return (lChance);
}

double SCSAnalyzer::ChanceForTwoPairs(const SCSHand& pcHand)
{
	unsigned lCombineCount = 0;
	double lChance = 0;
	for (CARD_VALUE i = CARD_A; i >= CARD_3; --(int&)i)
	{
		Card lCards[4];
		lCards[0].mValue = i;
		lCards[1].mValue = i;
		for (CARD_VALUE j = (CARD_VALUE)(i-1); j >= CARD_2; --(int&)j)
		{
			// We don't want four of a kind nor double of same card(s).
			if (i != j)
			{
				lCards[2].mValue = j;
				lCards[3].mValue = j;
				for (int it = 0; it < 6; ++it)
				{
					// First pair.
					lCards[0].mType = mscTwoOutOfFour[it][0];
					lCards[1].mType = mscTwoOutOfFour[it][1];
					for (int jt = 0; jt < 6; ++jt)
					{
						++lCombineCount;
						// Second pair.
						lCards[2].mType = mscTwoOutOfFour[jt][0];
						lCards[3].mType = mscTwoOutOfFour[jt][1];
						double lTmpChance = ChanceForExactCards(pcHand, lCards, 4);
						lChance += lTmpChance;
						if (lTmpChance >= 1)
						{
							if (mHighCard < i)
							{
								mHighCard = i;
							}
							mGotPrice = true;
						}
					}
				}
			}
		}
	}
	assert(lCombineCount == 13*12/2*6*6);
	lChance -= mLastChanceForFourOfAKind*6;
	lChance -= mLastChanceForFullHouse*2;
	//lChance -= mLastChanceForThreeOfAKind;
	mLastChanceForTwoPairs = lChance;
	return (lChance);
}

double SCSAnalyzer::ChanceForPair(const SCSHand& pcHand)
{
	unsigned lCombineCount = 0;
	double lChance = 0;
	for (CARD_VALUE i = CARD_A; i >= CARD_2; --(int&)i)
	{
		Card lCards[2];
		lCards[0].mValue = i;
		lCards[1].mValue = i;
		for (int it = 0; it < 6; ++it)
		{
			++lCombineCount;
			lCards[0].mType = mscTwoOutOfFour[it][0];
			lCards[1].mType = mscTwoOutOfFour[it][1];
			double lTmpChance = ChanceForExactCards(pcHand, lCards, 2);
			lChance += lTmpChance;
			if (lTmpChance >= 1)
			{
				if (mHighCard < i)
				{
					mHighCard = i;
				}
				mGotPrice = true;
			}
		}
	}
	assert(lCombineCount == 13*6);
	lChance -= mLastChanceForFourOfAKind*6;
	lChance -= mLastChanceForFullHouse*4;
	lChance -= mLastChanceForThreeOfAKind*3;
	lChance -= mLastChanceForTwoPairs*2;
	return (lChance);
}


double SCSAnalyzer::ChanceForExactCards(const SCSHand& pcHand, const Card* pcWantCards, int pWantCardCount, const Card* pcUnwantedCards, int pUnwantedCardCount)
{
	// Performance measurement.
	//PerformanceTimer lTimer;

	// Check if some of the sought cards already are in the hand.
	int lMissingCardCount = 0;
	int i;
	for (i = 0; i < pUnwantedCardCount; ++i)
	{
		//assert(pcUnwantedCards[i].mType >= HEARTS && pcUnwantedCards[i].mType <= SPADES);
		//assert(pcUnwantedCards[i].mValue >= CARD_2 && pcUnwantedCards[i].mValue <= CARD_A);
		if (pcHand.CountCard(pcUnwantedCards[i]))
		{
			return (0);	// Optimization: I don't want this card for this chance calculation.
		}
	}

	//mExactTimer[0] += lTimer.GetTimeAndRestart();

	// Find what cards are possibly available.
	for (i = 0; i < pWantCardCount; ++i)
	{
		//assert(pcWantCards[i].mType >= HEARTS && pcWantCards[i].mType <= SPADES);
		//assert(pcWantCards[i].mValue >= CARD_2 && pcWantCards[i].mValue <= CARD_A);
		if (pcHand.CountCard(pcWantCards[i]) != 1)
		{
			if (mDeck.CountCard(pcWantCards[i]) != 1)
			{
				return (0);	// Optimization: neither I nor the deck has the card.
			}
			++lMissingCardCount;
		}
	}
	// Sanity check.
	int lTakeCardCount = 7-pcHand.GetCardCount();
	int lLeftoverCardCount = lTakeCardCount-lMissingCardCount;
	if (lLeftoverCardCount < 0)
	{
		return (0);	// We can't take enough cards to get the wanted hand.
	}
	assert(lMissingCardCount <= 5);	// Algorithm doesn't work with more missing cards (int mul overflow).

	//mExactTimer[1] += lTimer.GetTimeAndRestart();

	// 1. Count on taking the needed cards.
	int lCardCount = mDeck.GetCardCount();
	int lLastCard = lCardCount-lMissingCardCount+1;
	assert(lLastCard >= 1);
	int lOutcomeCount1 = 1;
	for (i = lCardCount; i >= lLastCard; --i)
	{
		lOutcomeCount1 *= i;
	}
	double lChance = 1/(double)lOutcomeCount1;
	lChance *= ::Faculty(lTakeCardCount);

	//mExactTimer[2] += lTimer.GetTimeAndRestart();

	// 2. Count on taking the leftovers and unwanted cards.
	if (lLeftoverCardCount)	// Optimization.
	{
		lChance /= ::Faculty(lLeftoverCardCount);
		if (pUnwantedCardCount > 0)	// Optimization.
		{
			unsigned lUnwantedCardsLeftCount = 0;
			for (i = 0; i < pUnwantedCardCount; ++i)
			{
				if (mDeck.CountCard(pcUnwantedCards[i]))
				{
					++lUnwantedCardsLeftCount;
				}
			}
			lChance *= 1-(pWantCardCount)*lUnwantedCardsLeftCount/(double)lLastCard;
		}
	}

	//mExactTimer[3] += lTimer.GetTime();

	assert(lChance >= -0.0005 && lChance <= 1);
	return (lChance);
}


int SCSAnalyzer::GetPriceCount() const
{
	return (10);
}

double SCSAnalyzer::GetPriceWeight(int pPriceIndex) const
{
	assert(pPriceIndex >= 0 || pPriceIndex < 10);
	const double lPriceList[] =
	{
		/*
		// Original list weights.
		30940,	// Royal flush.
		3437.8,	// Straight flush.
		595,	// Four of a kind.
		33.06,	// Full house.
		24.23,	// Flush.
		12.133,	// Straight.
		22.037,	// Three of a kind.
		3.673,	// Two pairs.
		2.3611,	// Pair.
		0.0001	// Nothing.
		*/
		/*
		// Pretty well balanced...
		13,	// Royal flush.
		12,	// Straight flush.
		11,	// Four of a kind.
		10,	// Full house.
		9,	// Flush.
		8,	// Straight.
		7,	// Three of a kind.
		5,	// Two pairs.
		4,	// Pair.
		0.001	// Nothing.
		*/
		// My test weights...
		14,	// Royal flush.
		13,	// Straight flush.
		12,	// Four of a kind.
		11,	// Full house.
		10,	// Flush.
		8,	// Straight.
		7,	// Three of a kind.
		5,	// Two pairs.
		3.7,	// Pair.
		0.01	// Nothing.
	};
	return (lPriceList[pPriceIndex]);
}


const CARD_TYPE SCSAnalyzer::mscTwoOutOfFour[6][2] =
{
	{HEARTS, DIAMONDS},
	{HEARTS, CLUBS},
	{HEARTS, SPADES},
	{DIAMONDS, CLUBS},
	{DIAMONDS, SPADES},
	{CLUBS, SPADES}
};

const CARD_TYPE SCSAnalyzer::mscTwoOutOfFourComplement[6][2] =
{
	{CLUBS, SPADES},
	{DIAMONDS, SPADES},
	{DIAMONDS, CLUBS},
	{HEARTS, SPADES},
	{HEARTS, CLUBS},
	{HEARTS, DIAMONDS}
};

const CARD_TYPE SCSAnalyzer::mscThreeOutOfFour[4][3] =
{
	{HEARTS, DIAMONDS, CLUBS},
	{HEARTS, DIAMONDS, SPADES},
	{HEARTS, CLUBS, SPADES},
	{DIAMONDS, CLUBS, SPADES},
};

const CARD_TYPE SCSAnalyzer::mscOneOutOfFourComplement[4] =
{
	SPADES,
	CLUBS,
	DIAMONDS,
	HEARTS,
};


};
