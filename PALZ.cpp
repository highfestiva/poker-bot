
// Author: Jonas Byström, highfestiva@gmail.com. August 2003.
// Open source without neither limitations nor warranty.


#include "SysInclude.h"
#include "PALZ.h"
#include "PALZDlg.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#endif


BEGIN_MESSAGE_MAP(CPALZApp, CWinApp)
	ON_COMMAND(ID_HELP, CWinApp::OnHelp)
END_MESSAGE_MAP()


CPALZApp::CPALZApp()
{
}


CPALZApp theApp;


BOOL CPALZApp::InitInstance()
{
	InitCommonControls();

	CWinApp::InitInstance();

	CPALZDlg dlg;
	m_pMainWnd = &dlg;
	dlg.DoModal();

	return FALSE;
}
