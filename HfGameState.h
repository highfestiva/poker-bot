
// Author: Jonas Byström, highfestiva@gmail.com. August 2003.
// Open source without neither limitations nor warranty.


#ifndef GAMESTATE_H
#define GAMESTATE_H


namespace HfPoker
{

enum GAME_STATE
{
	PAUSE,
	ANALYZE,
	PLAY,
	PLAY_HIDDEN	// Move mouse some to show window again when hidden.
};

}


#endif // GAMESTATE_H
