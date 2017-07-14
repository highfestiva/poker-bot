
// Author: Jonas Byström, highfestiva@gmail.com. August 2003.
// Open source without neither limitations nor warranty.


#include "SysInclude.h"
#include "Doodle.h"
#include <string.h>


CDoodle::CDoodle():
	mState(HfPoker::PAUSE),
	mActive(false),
	mLocked(false)
{
}

CDoodle::~CDoodle()
{
}


BEGIN_MESSAGE_MAP(CDoodle, CEdit)
	ON_WM_CHAR()
END_MESSAGE_MAP()

void CDoodle::OnChar(UINT nChar, UINT nRepCnt, UINT nFlags)
{
	static char lLastChars[10] = {0,0,0,0,0,0,0,0,0,0};
	for (int i = 9; i >= 1; --i)
	{
		lLastChars[i] = lLastChars[i-1];
	}
	lLastChars[0] = (char)nChar;

	// Lock if the wrong chars has been punched too many times.
	if (mLocked)
	{
		mState = HfPoker::PAUSE;
		return;
	}

	if (::strncmp(lLastChars, "?=/&%¤#", 7) == 0)	// Activation password.
	{
		if (mActive)
		{
			mLocked = true;
		}
		mActive = !mActive;
		lLastChars[0] = '\0';
	}
	else if (strlen(lLastChars) >= 8)
	{
		mLocked = true;
		return;
	}
	if (!mActive)
	{
		mState = HfPoker::PAUSE;
		return;
	}

	if (::strncmp(lLastChars, "zp", 2) == 0)		// pz
	{
		mState = HfPoker::PAUSE;
		lLastChars[0] = '\0';
	}
	else if (::strncmp(lLastChars, "zla", 3) == 0)		// alz
	{
		mState = HfPoker::ANALYZE;
		lLastChars[0] = '\0';
	}
	else if (::strncmp(lLastChars, "4yalp", 5) == 0)	// play4
	{
		mState = HfPoker::PLAY;
		lLastChars[0] = '\0';
	}
	else if (::strncmp(lLastChars, "ylp8q", 5) == 0)	// q8ply
	{
		mState = HfPoker::PLAY_HIDDEN;
		lLastChars[0] = '\0';
	}
	else if (strlen(lLastChars) >= 10)
	{
		mLocked = true;
	}

	CEdit::OnChar(nChar, nRepCnt, nFlags);
}
