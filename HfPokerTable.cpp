
// Author: Jonas Byström, highfestiva@gmail.com. August 2003.
// Open source without neither limitations nor warranty.


#include "SysInclude.h"
#include "HfPokerTable.h"
#include "HfPokerHand.h"


namespace HfPoker
{


Table::Table():
	mMyHand(0)
{
}

Table::~Table()
{
	DeleteAllHands();
}


void Table::Reset()
{
	DeleteAllHands();
	mDebugText[0] = '\0';
}

void Table::AddHand(Hand* pHand)
{
	for (int i = 0; i < pHand->GetCardCount(); ++i)
	{
		char a[100];
		pHand->GetCard(i).GetName(a);
		strcat(mDebugText, a);
		strcat(mDebugText, ", ");
	}
	strcat(mDebugText, "\r\n");

	mHands.push_back(pHand);
	if (mHands.size() > 1)
	{
		if (mMyHand)
		{
			if (pHand->GetCardCount() > mMyHand->GetCardCount())
			{
				mMyHand = pHand;
			}
			else if (pHand->GetCardCount() == mMyHand->GetCardCount())
			{
				mMyHand = 0;
			}
		}
		else
		{
			if (mHands.front()->GetCardCount() < pHand->GetCardCount())
			{
				mMyHand = pHand;
			}
			else if (mHands.front()->GetCardCount() > pHand->GetCardCount())
			{
				mMyHand = mHands.front();
			}
		}
	}
	else
	{
		mMyHand = 0;
	}
}

void Table::DeleteAllHands()
{
	mMyHand = 0;
	while (!mHands.empty())
	{
		Hand* lHand = mHands.front();
		mHands.pop_front();
		delete (lHand);
	}
}

int Table::CountHandsOfSize(int pCardCount) const
{
	int lCount = 0;
	std::list<Hand*>::const_iterator i = mHands.begin();
	for (; i != mHands.end(); ++i)
	{
		if ((*i)->GetCardCount() == pCardCount)
		{
			++lCount;
		}
	}
	return (lCount);
}

void Table::CountHandSizes(int& pLeastCardCount, int& pMostCardCount) const
{
	pLeastCardCount = 100;
	pMostCardCount = -1;
	std::list<Hand*>::const_iterator i = mHands.begin();
	for (; i != mHands.end(); ++i)
	{
		if ((*i)->GetCardCount() < pLeastCardCount)
		{
			pLeastCardCount = (*i)->GetCardCount();
		}
		if ((*i)->GetCardCount() > pMostCardCount)
		{
			pMostCardCount = (*i)->GetCardCount();
		}
	}
}

const std::list<Hand*>& Table::GetHands() const
{
	return (mHands);
}


bool Table::operator==(const Table& pTable) const
{
	if (mHands.size() != pTable.mHands.size())
	{
		return (false);
	}

	std::list<Hand*>::const_iterator i = mHands.begin();
	std::list<Hand*>::const_iterator j = pTable.mHands.begin();
	for (; i != mHands.end(); ++i, ++j)
	{
		if (*(*i) != *(*j))
		{
			return (false);
		}
	}
	return (true);
}

bool Table::operator!=(const Table& pTable) const
{
	return (!(*this == pTable));
}


const char* Table::GetDebugText() const
{
	assert(::strlen(mDebugText) < sizeof(mDebugText)/2);
	return (mDebugText);
}


};
