
// Author: Jonas Byström, highfestiva@gmail.com. August 2003.
// Open source without neither limitations nor warranty.


#pragma once
#include "afxwin.h"
#include <iostream>
#include <fstream>
#include "HfGameState.h"
#include "HfPoker.h"
#include "HfPokerGameState.h"
#include "HfStd.h"
#include "doodle.h"


namespace HfPoker
{
class Gambler;
class GameAnalyzer;
class ImageAnalyzer;
};



class CPALZDlg : public CDialog
{
public:
				CPALZDlg(CWnd* pParent = NULL);
				~CPALZDlg();

	enum { IDD = IDD_PALZ_DIALOG };

	protected:
	virtual void		DoDataExchange(CDataExchange* pDX);

	void			AddOutput(const CString& pOutputText);

protected:
	HICON m_hIcon;

	virtual BOOL		OnInitDialog();
	afx_msg void		OnSysCommand(UINT nID, LPARAM lParam);
	afx_msg void		OnPaint();
	afx_msg HCURSOR		OnQueryDragIcon();
	afx_msg void		OnTimer(UINT_PTR nIDEvent);
	afx_msg int		OnCmdMsg(UINT nID, int nCode, void* pExtra, AFX_CMDHANDLERINFO* pHandlerInfo);
	afx_msg void		OnClose();
	DECLARE_MESSAGE_MAP()

protected:
	std::fstream		mFile;
	HfPoker::GAME_STATE	mMode;
	HfPoker::GAME_STATE	mDesiredMode;
	uint64			mModeChangeTime;
	double			mFrequency;
	double			mWaitTime;
	CDoodle			mOutput;
	CStatic			mImage;
	POINT			mOldPoint;
	void			OnPause();
	void			OnAnalyze();
	void			OnPlay();
	void			OnQuietPlay();

	uint64			GetTime() const;
	int64			GetTimeDiff(uint64 pTime) const;

	void			StartCapture();
	void			StopCapture();
	void			CaptureActiveWindow();
	static int CALLBACK	EnumFontCallback(CONST LOGFONT *lplf, CONST TEXTMETRIC *lptm, DWORD dwType, LPARAM lpData);
	void			InitFont();
	BOOL __stdcall		EnumChildWindow(HWND pHWnd);
	static BOOL __stdcall	EnumBuggedChildWindow(HWND pHWnd, LPARAM pLParam);
	bool			GetStableButtons();
	void			DescidePlay();
	bool			Play();
	bool			DoClick(HWND pChildButton);
	void			SetupDebugBitmap(const unsigned char* pBitmap, int pWidth, int pHeight, int pBPP);
	void			DrawDebugBitmap() const;
	bool			GetMidButtonClientPos(HWND pChildButton, int& pMidX, int& pMidY);
	void			SetClientCursorPos(int pWindowX, int pWindowY);
	void			StoreCursor();
	bool			IsCursorMoved();
	static bool		IsStringSeparator(char pChar);
	static bool		StringCompare(const char* pc1, const char* pc2);

	HWND			mBuggedWindow;
	int			mWidth;
	int			mHeight;
	int			mBPP;	// Bytes per pixel.
	unsigned char*		mDebugBitmap;
	HfPoker::GameState	mState;
	HfPoker::ImageAnalyzer*	mImageAnalyzer;
	HfPoker::GameAnalyzer*	mGameAnalyzer;
	HfPoker::Gambler*	mGambler;
	HfPoker::GAME_ACTION	mLastAction;
	bool			mBringIn;
	HWND			mIAmBackButton;
	HWND			mAnteButton;
	int			mStableButtonCount;
	int			mFoldButtonCount;
	int			mCheckButtonCount;
	int			mCallButtonCount;
	int			mRaiseButtonCount;
	int			mGameButtonCount[2];
	HWND			mChildCheckFoldButton[2];
	HWND			mChildCallButton[2];
	HWND			mChildRaiseButton[2];
	char			mWinnerString[1000];
	char			mLastWinnerString[1000];
	uint64			mClickTime;
};
