
// Author: Jonas Byström, highfestiva@gmail.com. August 2003.
// Open source without neither limitations nor warranty.


#ifndef HFPOKER_H
#define HFPOKER_H


namespace HfPoker
{


typedef enum
{
	WAIT,
	FOLD_OR_CHECK,
	CALL,
	RAISE
} GAME_ACTION;

typedef enum
{
	CARD_TYPE_INVALID = -1,
	HEARTS = 1,
	DIAMONDS = 2,
	CLUBS = 3,
	SPADES = 4
} CARD_TYPE;

typedef enum
{
	CARD_VALUE_INVALID = -1,
	CARD_2 = 0,
	CARD_3,
	CARD_4,
	CARD_5,
	CARD_6,
	CARD_7,
	CARD_8,
	CARD_9,
	CARD_10,
	CARD_J,
	CARD_Q,
	CARD_K,
	CARD_A
} CARD_VALUE;

#define	SEVEN_CARD_STUD_DECK_COUNT	1


};


#endif // HFPOKER_H
