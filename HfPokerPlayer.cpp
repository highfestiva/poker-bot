#include "SysInclude.h"
#include "HfPokerPlayer.h"


namespace HfPoker
{


Player::Player()
{
	mDebugText[0] = '\0';
}


void Player::Play(GAME_ACTION pAction)
{
	mDebugText[0] = '\0';
	switch (pAction)
	{
		case WAIT:
		{
			//::sprintf(mDebugText, "Wait\r\n");
		}
		break;
		case FOLD_OR_CHECK:
		{
			::sprintf(mDebugText, "Fold/Check\r\n");
		}
		break;
		case CALL:
		{
			::sprintf(mDebugText, "Call\r\n");
		}
		break;
		case RAISE:
		{
			::sprintf(mDebugText, "Raise\r\n");
		}
		break;
		default:
		{
			::sprintf(mDebugText, "Player::Play(): WTF?\r\n");
			assert(false);
		}
		break;
	}
}


char* const Player::GetDebugText() const
{
	return ((char* const)mDebugText);
}


};
