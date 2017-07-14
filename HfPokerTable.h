
// Author: Jonas Byström, highfestiva@gmail.com. August 2003.
// Open source without neither limitations nor warranty.


#ifndef HFPOKERTABLE_H
#define HFPOKERTABLE_H


#include <list>


namespace HfPoker
{


class Hand;


class Table
{
public:
					Table();
	virtual				~Table();

	virtual void			Reset();
	virtual void			AddHand(Hand* pHand);
	virtual void			DeleteAllHands();
	int				CountHandsOfSize(int pCardCount) const;
	void				CountHandSizes(int& pLeastCardCount, int& pMostCardCount) const;
	const std::list<Hand*>&		GetHands() const;
	const Hand*			GetMyHand() const;

	bool				operator==(const Table& pTable) const;
	bool				operator!=(const Table& pTable) const;

	const char*			GetDebugText() const;

protected:
	std::list<Hand*>		mHands;
	Hand*				mMyHand;

	char				mDebugText[10000];
};


};


#endif // HFPOKERTABLE_H
