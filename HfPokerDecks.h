
// Author: Jonas Byström, highfestiva@gmail.com. August 2003.
// Open source without neither limitations nor warranty.


#ifndef HFPOKERDECKS_H
#define HFPOKERDECKS_H


#include "HfPokerHand.h"


namespace HfPoker
{


class Card;


class Decks
{
public:
	Decks(int pDeckCount);
	Decks(const Decks& pDecks);

	void Reset();
	bool IsUntouched() const;

	void AddCard(const Card& pcCard);
	void AddCards(const Hand& pcHand);
	void DropCard(const Card& pcCard);
	void DropCards(const Hand& pcHand);
	void DropCardsConditional(const Hand& pcHand);
	double DrawChance(const Card& pCard);
	double DrawChanceNoDraw(const Card& pCard) const;
	inline int CountCard(const Card& pcCard) const;
	inline int GetCardCount() const;

protected:
	bool	mUntouched;
	int	mOriginalDeckCount;
	int	mCards[52];
	int	mCardCount;
};


inline int Decks::CountCard(const Card& pcCard) const
{
	int lCardTotalValue = (pcCard.mType-1)*13 + pcCard.mValue;
	return (mCards[lCardTotalValue]);
}

inline int Decks::GetCardCount() const
{
	return (mCardCount);
}


};


#endif // HFPOKERDECKS_H
