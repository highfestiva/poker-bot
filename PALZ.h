
// Author: Jonas Byström, highfestiva@gmail.com. August 2003.
// Open source without neither limitations nor warranty.


#pragma once

#ifndef __AFXWIN_H__
#error "Crapface!"
#endif

#include "resource.h"


class CPALZApp : public CWinApp
{
public:
	CPALZApp();

	public:
	virtual BOOL InitInstance();

	DECLARE_MESSAGE_MAP()
};

extern CPALZApp theApp;
