#ifndef HFPOKERPLAYER_H
#define HFPOKERPLAYER_H


#include "HfPoker.h"


namespace HfPoker
{


class Player
{
public:
			Player();

	void		Play(GAME_ACTION pAction);
	char* const	GetDebugText() const;

protected:
	char		mDebugText[1000];
};


};


#endif // HFPOKERPLAYER_H
