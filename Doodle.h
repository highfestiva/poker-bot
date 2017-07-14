
// Author: Jonas Byström, highfestiva@gmail.com. August 2003.
// Open source without neither limitations nor warranty.


#ifndef DOODLE_H
#define DOODLE_H


#include "afxwin.h"
#include "HfGameState.h"


// Wacko name for a wacko IO-box.
class CDoodle: public CEdit
{
public:
	HfPoker::GAME_STATE mState;

	CDoodle();
	~CDoodle();

	DECLARE_MESSAGE_MAP()
	afx_msg void OnChar(UINT nChar, UINT nRepCnt, UINT nFlags);

protected:
	bool mActive;
	bool mLocked;
};


#endif // DOODLE_H
