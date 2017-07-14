
// Author: Jonas Byström, highfestiva@gmail.com. August 2003.
// Open source without neither limitations nor warranty.


#include "SysInclude.h"
#include "HfPokerCard.h"


namespace HfPoker
{


Card::Card():
	mType(CARD_TYPE_INVALID),
	mValue(CARD_VALUE_INVALID)
{
}

Card::Card(CARD_TYPE pType, CARD_VALUE pValue):
	mType(pType),
	mValue(pValue)
{
}


const char* Card::GetTypeName(CARD_TYPE pType)
{
	assert(pType >= 1 && pType <= 4);
	static const char* lscTypeNames[] =
	{
		"hearts",
		"diamonds",
		"clubs",
		"spades"
	};
	return (lscTypeNames[pType-1]);
}

const char* Card::GetTypeName() const
{
	return (GetTypeName(mType));
}

const char* Card::GetValueName(CARD_VALUE pValue)
{
	assert(pValue >= 0 && pValue <= 12);
	static const char* lscValueNames[] =
	{
		"2",
		"3",
		"4",
		"5",
		"6",
		"7",
		"8",
		"9",
		"10",
		"J",
		"Q",
		"K",
		"A"
	};
	return (lscValueNames[pValue]);
}

const char* Card::GetValueName() const
{
	return (GetValueName(mValue));
}

void Card::GetName(char* pName) const
{
	::sprintf(pName, "%s of %s", GetValueName(mValue), GetTypeName(mType));
}


};
