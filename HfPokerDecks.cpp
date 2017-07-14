
// Author: Jonas Byström, highfestiva@gmail.com. August 2003.
// Open source without neither limitations nor warranty.


#include "SysInclude.h"
#include "HfPokerDecks.h"
#include "HfStd.h"


namespace HfPoker
{


Decks::Decks(int pDeckCount):
	mOriginalDeckCount(pDeckCount)
{
	Reset();
}

Decks::Decks(const Decks& pDecks):
	mOriginalDeckCount(pDecks.mOriginalDeckCount),
	mCardCount(pDecks.mCardCount)
{
	for (int i = 0; i < 52; ++i)
	{
		mCards[i] = pDecks.mCards[i];
	}
}


void Decks::Reset()
{
	mUntouched = true;
	mCardCount = 52*mOriginalDeckCount;
	for (int i = 0; i < 52; ++i)
	{
		mCards[i] = mOriginalDeckCount;
	}
}

bool Decks::IsUntouched() const
{
	return (mUntouched);
}


void Decks::AddCard(const Card& pcCard)
{
	++mCardCount;
	assert(mCardCount <= 52*mOriginalDeckCount);
	int lCardTotalValue = (pcCard.mType-1)*13 + pcCard.mValue;
	assert(lCardTotalValue >= 0 && lCardTotalValue < 52);
	++mCards[lCardTotalValue];
	ASSERT(mCards[lCardTotalValue] <= mOriginalDeckCount);
}

void Decks::AddCards(const Hand& pcHand)
{
	mCardCount += pcHand.GetCardCount();
	if (mCardCount > 52*mOriginalDeckCount)
	{
		throw (false);
	}
	for (int i = 0; i < pcHand.GetCardCount(); ++i)
	{
		int lCardTotalValue = (pcHand.GetCard(i).mType-1)*13 + pcHand.GetCard(i).mValue;
		assert(lCardTotalValue >= 0 && lCardTotalValue < 52);
		++mCards[lCardTotalValue];
		if (mCards[lCardTotalValue] > mOriginalDeckCount)
		{
			throw (false);
		}
	}
}

void Decks::DropCard(const Card& pcCard)
{
	mUntouched = false;
	if (mCardCount <= 0)
	{
		throw (false);
	}
	--mCardCount;
	assert(mCardCount >= 0);
	int lCardTotalValue = (pcCard.mType-1)*13 + pcCard.mValue;
	assert(lCardTotalValue >= 0 && lCardTotalValue < 52);
	if (mCards[lCardTotalValue] <= 0)
	{
		throw (false);
	}
	--mCards[lCardTotalValue];
	ASSERT(mCards[lCardTotalValue] >= 0);
}

void Decks::DropCards(const Hand& pcHand)
{
	mCardCount -= pcHand.GetCardCount();
	if (mCardCount < 0)
	{
		throw (false);
	}
	for (int i = 0; i < pcHand.GetCardCount(); ++i)
	{
		mUntouched = false;
		int lCardTotalValue = (pcHand.GetCard(i).mType-1)*13 + pcHand.GetCard(i).mValue;
		assert(lCardTotalValue >= 0 && lCardTotalValue < 52);
		--mCards[lCardTotalValue];
		if (mCards[lCardTotalValue] < 0)
		{
			throw (false);
		}
	}
}

void Decks::DropCardsConditional(const Hand& pcHand)
{
	for (int i = 0; i < pcHand.GetCardCount(); ++i)
	{
		int lCardTotalValue = (pcHand.GetCard(i).mType-1)*13 + pcHand.GetCard(i).mValue;
		assert(lCardTotalValue >= 0 && lCardTotalValue < 52);
		if (mCards[lCardTotalValue] > 0)
		{
			mUntouched = false;
			--mCards[lCardTotalValue];
			--mCardCount;
		}
		assert(mCardCount >= 0);
		assert(mCards[lCardTotalValue] >= 0);
	}
}

double Decks::DrawChance(const Card& pcCard)
{
	mUntouched = false;
	int lCardTotalValue = (pcCard.mType-1)*13 + pcCard.mValue;
	double lChance = 0;
	assert(lCardTotalValue >= 0 && lCardTotalValue < 52);
	if (mCards[lCardTotalValue] >= 0)
	{
		lChance = mCards[lCardTotalValue]/(double)mCardCount;
		--mCards[lCardTotalValue];
		--mCardCount;
	}
	assert(mCardCount >= 0);
	return (lChance);
}

double Decks::DrawChanceNoDraw(const Card& pcCard) const
{
	int lCardTotalValue = (pcCard.mType-1)*13 + pcCard.mValue;
	double lChance = 0;
	assert(lCardTotalValue >= 0 && lCardTotalValue < 52);
	if (mCards[lCardTotalValue] >= 0)
	{
		lChance = mCards[lCardTotalValue]/(double)mCardCount;
	}
	return (lChance);
}


};
