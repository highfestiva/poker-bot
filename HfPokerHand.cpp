
// Author: Jonas Byström, highfestiva@gmail.com. August 2003.
// Open source without neither limitations nor warranty.


#include "SysInclude.h"
#include "HfPokerHand.h"
#include "HfStd.h"


namespace HfPoker
{


Hand::Hand(int pPlayerPosX, int pPlayerPosY):
	mChance(0),
	mPlayerPosX(pPlayerPosX),
	mPlayerPosY(pPlayerPosY),
	mCardCount(0)
{
}

Hand::Hand(const Hand& pcCopy):
	mChance(pcCopy.mChance),
	mPlayerPosX(pcCopy.mPlayerPosX),
	mPlayerPosY(pcCopy.mPlayerPosY),
	mCardCount(pcCopy.mCardCount)
{
	for (int i = 0; i < mCardCount; ++i)
	{
		mCards[i] = pcCopy.mCards[i];
	}
}


Hand* Hand::GetCopy() const
{
	Hand* lCopy = new Hand(*this);
	return (lCopy);
}


void Hand::SetChance(double pChance)
{
	//assert(pChance >= 0);
	mChance = pChance;
}

double Hand::GetChance() const
{
	return (mChance);
}

int Hand::GetPlayerPosX() const
{
	return (mPlayerPosX);
}

int Hand::GetPlayerPosY() const
{
	return (mPlayerPosY);
}


void Hand::AddCard(const Card& pCard)
{
	assert(mCardCount >= 0 && mCardCount < sizeof(mCards)/sizeof(Card));
	assert(!IsCardInHand(pCard) || pCard.mType == CARD_TYPE_INVALID);
	mCards[mCardCount] = pCard;
	++mCardCount;
}

void Hand::DropCard(const Card& pCard)
{
	ASSERT(mCardCount > 0);
	for (int i = 0; i < mCardCount; ++i)
	{
		if (mCards[i] == pCard)
		{
			for (; i < mCardCount; ++i)
			{
				mCards[i] = mCards[i+1];
			}
			--mCardCount;
			return;
		}
	}
	assert(false);
}

void Hand::DropCardValue(int pCardValue)
{
	assert(pCardValue >= 0 && pCardValue < 13);
	for (int i = 0; i < mCardCount; ++i)
	{
		if (mCards[i].mValue == pCardValue)
		{
			DropCard(mCards[i]);
			return;
		}
	}
	assert(false);
}

void Hand::DropMask(int pDropMask)
{
#ifdef _DEBUG
	int lDropCount = ::CountSetBits(pDropMask);
	assert(lDropCount <= GetCardCount());
	assert(::GetSetBit(pDropMask, lDropCount-1) <= GetCardCount()-1);
#endif // _DEBUG
	while (pDropMask)
	{
		int lBitIndex = ::GetSetBit(pDropMask, 0);
		ASSERT(lBitIndex >= 0 && lBitIndex < 32);
		for (int j = lBitIndex; j < mCardCount-1; ++j)
		{
			mCards[j] = mCards[j+1];
		}
		--mCardCount;
		assert(mCardCount >= 0);
		pDropMask >>= lBitIndex+1;
	}
}

bool Hand::IsCardInHand(const Card& pCard) const
{
	return (CountCard(pCard) > 0);
}

bool Hand::IsCardValueInHand(int pCardValue) const
{
	assert(pCardValue >= 0 && pCardValue < 13);
	for (int i = 0; i < mCardCount; ++i)
	{
		if (mCards[i].mValue == pCardValue)
		{
			return (true);
		}
	}
	return (false);
}

const Card& Hand::GetCard(int pIndex) const
{
	assert(pIndex < mCardCount);
	return (mCards[pIndex]);
}

void Hand::SetCard(int pIndex, const Card& pCard)
{
	assert(pIndex < mCardCount);
	mCards[pIndex] = pCard;
}

int Hand::CountDifferentValues() const
{
	int lValueCount[13];
	int i;
	for (i = 0; i < mCardCount; ++i)
	{
		++lValueCount[mCards[i].mValue];
	}
	int lDifferentValues = 0;
	for (i = 0; i < 13; ++i)
	{
		if (lValueCount[i])
		{
			++lDifferentValues;
		}
	}
	return (lDifferentValues);
}


bool Hand::operator==(const Hand& pHand) const
{
	if (mCardCount != pHand.mCardCount)
	{
		return (false);
	}

	for (int i = 0; i < mCardCount; ++i)
	{
		if (mCards[i].mType != pHand.mCards[i].mType ||
			mCards[i].mValue != pHand.mCards[i].mValue)
		{
			return (false);
		}
	}
	return (true);
}

bool Hand::operator!=(const Hand& pHand) const
{
	return (!(*this == pHand));
}


void Hand::operator=(const Hand& pHand) const
{
	assert(false);
}


};
