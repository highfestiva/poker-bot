
// Author: Jonas Byström, highfestiva@gmail.com. August 2003.
// Open source without neither limitations nor warranty.


#ifndef HFPOKERCARD_H
#define HFPOKERCARD_H


#include "HfPoker.h"


namespace HfPoker
{


class Card
{
public:
	CARD_TYPE	mType;
	CARD_VALUE	mValue;

	Card();
	Card(CARD_TYPE pType, CARD_VALUE pValue);

	static const char* GetTypeName(CARD_TYPE pType);
	const char* GetTypeName() const;
	static const char* GetValueName(CARD_VALUE pValue);
	const char* GetValueName() const;
	void GetName(char* pName) const;

	inline void operator=(const Card& pCard);
	inline bool operator==(const Card& pCard) const;
	inline bool operator!=(const Card& pCard) const;
};


inline void Card::operator=(const Card& pCard)
{
	mType = pCard.mType;
	mValue = pCard.mValue;
}

inline bool Card::operator==(const Card& pCard) const
{
	return (mType == pCard.mType && mValue == pCard.mValue);
}

inline bool Card::operator!=(const Card& pCard) const
{
	return (mType != pCard.mType || mValue != pCard.mValue);
}


};


#endif // HFPOKERCARD_H
