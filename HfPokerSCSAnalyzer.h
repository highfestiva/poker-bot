
// Author: Jonas Byström, highfestiva@gmail.com. August 2003.
// Open source without neither limitations nor warranty.


#ifndef HFPOKERSCSANALYZER_H
#define HFPOKERSCSANALYZER_H


#include "HfPokerDecks.h"
#include "HfPokerGameAnalyzer.h"


namespace HfPoker
{


class SCSHand;


class SCSAnalyzer: public GameAnalyzer
{
public:
	SCSAnalyzer();
	virtual ~SCSAnalyzer();

	void Reset();
	void ResetOld();

	bool CalculateChance(GameState& pState);
	double CalculateHandChance(const SCSHand& pcHand, const int pMinPrice, int* pBestPrice);

	double ChanceForRoyalFlush(const SCSHand& pcHand);
	double ChanceForStraightFlush(const SCSHand& pcHand);
	double ChanceForFourOfAKind(const SCSHand& pcHand);
	double ChanceForFullHouse(const SCSHand& pcHand);
	double ChanceForFlush(const SCSHand& pcHand);
	double ChanceForStraight(const SCSHand& pcHand);
	double ChanceForThreeOfAKind(const SCSHand& pcHand);
	double ChanceForTwoPairs(const SCSHand& pcHand);
	double ChanceForPair(const SCSHand& pcHand);

protected:
	bool	mSharedLastCard;
	Decks	mDeck;
	bool	mGotPrice;
	CARD_VALUE	mHighCard;
	CARD_VALUE	mBestHighCard;
	double	mLastChanceForRoyalFlush;
	double	mLastChanceForStraightFlush;
	double	mLastChanceForFourOfAKind;
	double	mLastChanceForFullHouse;
	double	mLastChanceForFlush;
	double	mLastChanceForStraight;
	double	mLastChanceForThreeOfAKind;
	double	mLastChanceForTwoPairs;
	static const CARD_TYPE	mscTwoOutOfFour[6][2];
	static const CARD_TYPE	mscTwoOutOfFourComplement[6][2];
	static const CARD_TYPE	mscThreeOutOfFour[4][3];
	static const CARD_TYPE	mscOneOutOfFourComplement[4];

	//double mExactTimer[4];

	double ChanceForExactCards(const SCSHand& pcHand, const Card* pcCards, int pCardCount, const Card* pcUnwantedCards = 0, int pUnwantedCardCount = 0);

	int GetPriceCount() const;
	double GetPriceWeight(int pPriceIndex) const;
};


};


#endif // HFPOKERSCSANALYZER_H
