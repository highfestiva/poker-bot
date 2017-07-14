
// Author: Jonas Byström, highfestiva@gmail.com. August 2003.
// Open source without neither limitations nor warranty.


#ifndef HFPOKERHAND_H
#define HFPOKERHAND_H


#include "HfPokerCard.h"


namespace HfPoker
{


class Hand
{
public:
			Hand(int pPlayerPosX, int pPlayerPosY);
			Hand(const Hand& pcCopy);

	virtual Hand*	GetCopy() const;

	void		SetChance(double pChance);
	double	GetChance() const;
	int		GetPlayerPosX() const;
	int		GetPlayerPosY() const;

	void		AddCard(const Card& pCard);
	void		DropCard(const Card& pCard);
	void		DropCardValue(int pCardValue);
	void		DropMask(int pDropMask);
	bool		IsCardInHand(const Card& pCard) const;
	bool		IsCardValueInHand(int pCardValue) const;
	const Card&	GetCard(int pIndex) const;
	void		SetCard(int pIndex, const Card& pCard);
	int		CountDifferentValues() const;
	inline int	CountCard(const Card& pCard) const;
	inline int	GetCardCount() const;

	bool		operator==(const Hand& pHand) const;
	bool		operator!=(const Hand& pHand) const;

protected:
	double	mChance;
	int		mPlayerPosX;
	int		mPlayerPosY;
	Card		mCards[8];
	int		mCardCount;

	void operator=(const Hand& pHand) const;
};


inline int Hand::CountCard(const Card& pCard) const
{
	int lCount = 0;
	for (int i = 0; i < mCardCount; ++i)
	{
		if (mCards[i] == pCard)
		{
			++lCount;
		}
	}
	return (lCount);
}

inline int Hand::GetCardCount() const
{
	return (mCardCount);
}

};


#endif // HFPOKERHAND_H
