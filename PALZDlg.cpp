
// Author: Jonas Byström, highfestiva@gmail.com. August 2003.
// Open source without neither limitations nor warranty.


#include "SysInclude.h"
#include "PALZ.h"
#include "PALZDlg.h"
#include "HfPokerBitmap.h"
#include "HfPokerCard.h"
#include "HfPokerGambler.h"
#include "HfPokerGameState.h"
#include "HfPokerHand.h"
#include "HfPokerImageAnalyzer.h"
#include "HfPokerSCSAnalyzer.h"
#include "HfPokerSCSHand.h"
#include "HfPokerTable.h"
#include "HfStd.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#endif


class CAboutDlg : public CDialog
{
public:
	CAboutDlg();

	enum { IDD = IDD_ABOUTBOX };

	protected:
	virtual void DoDataExchange(CDataExchange* pDX);

protected:
	DECLARE_MESSAGE_MAP()
};

CAboutDlg::CAboutDlg() : CDialog(CAboutDlg::IDD)
{
}

void CAboutDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
}

BEGIN_MESSAGE_MAP(CAboutDlg, CDialog)
END_MESSAGE_MAP()

CPALZDlg::CPALZDlg(CWnd* pParent):
	CDialog(CPALZDlg::IDD, pParent),
	mMode(HfPoker::PAUSE),
	mDesiredMode(HfPoker::PAUSE),
	mFrequency(2),
	mWaitTime(0),
	mWidth(-1),
	mHeight(-1),
	mBPP(-1),
	mDebugBitmap(0),
	mImageAnalyzer(new HfPoker::ImageAnalyzer),
	mGameAnalyzer(new HfPoker::SCSAnalyzer),
	mGambler(new HfPoker::Gambler),
	mLastAction(HfPoker::WAIT),
	mIAmBackButton(0),
	mAnteButton(0),
	mStableButtonCount(0),
	mBringIn(false)
{
	mGameButtonCount[0] = 0;
	mChildCheckFoldButton[0] = 0;
	mChildCallButton[0] = 0;
	mChildRaiseButton[0] = 0;

	mModeChangeTime = GetTime();
	mClickTime = (uint64)-1;

	mState.mUpdated = false;

	mLastWinnerString[0] = '\0';

	m_hIcon = AfxGetApp()->LoadIcon(IDR_MAINFRAME);
}

CPALZDlg::~CPALZDlg()
{
	mFile << "\n";

	delete (mGambler);
	mGambler = 0;
	delete (mGameAnalyzer);
	mGameAnalyzer = 0;
	delete (mImageAnalyzer);
	mImageAnalyzer = 0;
	delete[] (mDebugBitmap);
	mDebugBitmap = 0;
}

void CPALZDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	DDX_Control(pDX, IDC_EDIT, mOutput);
	DDX_Control(pDX, IDC_IMAGE, mImage);
}



void CPALZDlg::AddOutput(const CString& pOutputText)
{
	if (pOutputText.IsEmpty())
	{
		return;
	}
	if (pOutputText.GetLength() >= 4)
	{
		assert(pOutputText[3] >= 0);
	}

	// Add to file.
	CString lOutput = pOutputText;
	lOutput.Remove('\r');
	mFile << lOutput;
	// Flush now and then.
	static int lFlushCount = 0;
	if (++lFlushCount > 30)
	{
		mFile << std::flush;
	}

	UpdateData(TRUE);
	mOutput.GetWindowText(lOutput);
	lOutput += pOutputText;
	int lCrCount = 0;
	for (int i = 0; i < lOutput.GetLength(); ++i)
	{
		if (lOutput[i] == '\n')
		{
			++lCrCount;
		}
	}
	bool lDroppedLines = false;
	while (lCrCount > 25)
	{
		lDroppedLines = true;
		lOutput = lOutput.Mid(lOutput.Find('\n')+1);
		--lCrCount;
	}
	if (lDroppedLines)
	{
		mOutput.SetWindowText(lOutput);
	}
	else
	{
		int lLength = mOutput.GetWindowTextLength();
		mOutput.SetSel(lLength, lLength);
		mOutput.ReplaceSel(pOutputText);
	}
	UpdateData(FALSE);
}


BEGIN_MESSAGE_MAP(CPALZDlg, CDialog)
	ON_WM_SYSCOMMAND()
	ON_WM_PAINT()
	ON_WM_QUERYDRAGICON()
	ON_WM_TIMER()
	ON_WM_CLOSE()
	/*
	ON_BN_CLICKED(IDC_PAUSE, OnPause)
	ON_BN_CLICKED(IDC_ANALYZE, OnAnalyze)
	ON_BN_CLICKED(IDC_PLAY, OnPlay)
	ON_BN_CLICKED(IDC_QUIET_PLAY, OnQuietPlay)
	*/
END_MESSAGE_MAP()


BOOL CPALZDlg::OnInitDialog()
{
	InitRandom();

	// Change our priority class.
	DWORD dw = ::GetCurrentProcessId();
	HANDLE h = ::OpenProcess(PROCESS_ALL_ACCESS, FALSE, dw);
	::SetPriorityClass(h, IDLE_PRIORITY_CLASS);
	::CloseHandle(h);

	CString lVersionNumber;
	lVersionNumber.LoadString(IDS_VERSIONINFO);

	mFile.open("Game.log", std::ios_base::app | std::ios_base::out);
	mFile << "\n---\n\nApplication start. Version number = " << lVersionNumber;
#ifdef _DEBUG
	mFile << " (debug)" << "\n";
#else // !_DEBUG
	mFile << " (release)" << "\n";
#endif // _DEBUG / !_DEBUG

	CDialog::OnInitDialog();

	assert((IDM_ABOUTBOX & 0xFFF0) == IDM_ABOUTBOX);
	assert(IDM_ABOUTBOX < 0xF000);

	CMenu* pSysMenu = GetSystemMenu(FALSE);
	if (pSysMenu != NULL)
	{
		CString strAboutMenu;
		strAboutMenu.LoadString(IDS_ABOUTBOX);
		if (!strAboutMenu.IsEmpty())
		{
			pSysMenu->AppendMenu(MF_SEPARATOR);
			pSysMenu->AppendMenu(MF_STRING, IDM_ABOUTBOX, strAboutMenu);
		}
	}

	SetIcon(m_hIcon, TRUE);
	SetIcon(m_hIcon, FALSE);

	::MoveWindow(m_hWnd, 300, 545, 950, 450, FALSE);
	OnPause();

	mBuggedWindow = 0;

	return TRUE;
}

void CPALZDlg::OnSysCommand(UINT nID, LPARAM lParam)
{
	if ((nID & 0xFFF0) == IDM_ABOUTBOX)
	{
		CAboutDlg dlgAbout;
		dlgAbout.DoModal();
	}
	else
	{
		CDialog::OnSysCommand(nID, lParam);
	}
}

void CPALZDlg::OnPaint()
{
	if (IsIconic())
	{
		CPaintDC dc(this);

		SendMessage(WM_ICONERASEBKGND, reinterpret_cast<WPARAM>(dc.GetSafeHdc()), 0);

		int cxIcon = GetSystemMetrics(SM_CXICON);
		int cyIcon = GetSystemMetrics(SM_CYICON);
		CRect rect;
		GetClientRect(&rect);
		int x = (rect.Width() - cxIcon + 1) / 2;
		int y = (rect.Height() - cyIcon + 1) / 2;

		dc.DrawIcon(x, y, m_hIcon);
	}
	else
	{
		DrawDebugBitmap();

		CDialog::OnPaint();
	}
}

HCURSOR CPALZDlg::OnQueryDragIcon()
{
	return static_cast<HCURSOR>(m_hIcon);
}

void CPALZDlg::OnTimer(UINT_PTR)
{
	CaptureActiveWindow();
}

int CPALZDlg::OnCmdMsg(UINT nID, int nCode, void* pExtra, AFX_CMDHANDLERINFO* pHandlerInfo)
{
	if(nID == IDCANCEL || nID == IDOK)
	{
		return (0);
	}
	return (CDialog::OnCmdMsg(nID, nCode, pExtra, pHandlerInfo));
}

void CPALZDlg::OnClose()
{
	EndDialog(0);
}


uint64 CPALZDlg::GetTime() const
{
	FILETIME lCurrent;
	::GetSystemTimeAsFileTime(&lCurrent);
	return ((*(uint64*)&lCurrent)/(uint64)10000);
}

int64 CPALZDlg::GetTimeDiff(uint64 pTime) const
{
	return ((int64)GetTime() - (int64)pTime);
}



void CPALZDlg::OnPause()
{
	StopCapture();

	AddOutput("^\r\n");
	mMode = HfPoker::PAUSE;
	mState.mUpdated = false;
	mOutput.mState = mMode;
	mDesiredMode = HfPoker::PAUSE;
	mBuggedWindow = m_hWnd;
	mGameAnalyzer->Reset();

	StartCapture();
}

void CPALZDlg::OnAnalyze()
{
	if(mMode != HfPoker::ANALYZE)
	{
		StopCapture();
		AddOutput("Analyzing...\r\n");
		mMode = HfPoker::ANALYZE;
		mOutput.mState = mMode;
		StartCapture();
	}
}

void CPALZDlg::OnPlay()
{
	if(mMode != HfPoker::PLAY)
	{
		StopCapture();
		AddOutput("Playing...\r\n");
		mMode = HfPoker::PLAY;
		mOutput.mState = mMode;
		mDesiredMode = HfPoker::PAUSE;
		StoreCursor();
		::Sleep(1000);
		StoreCursor();
		StartCapture();
	}
}

void CPALZDlg::OnQuietPlay()
{
	if(mMode != HfPoker::PLAY_HIDDEN)
	{
		StopCapture();
		AddOutput("Quiet play...\r\n");
		mMode = HfPoker::PLAY_HIDDEN;
		mOutput.mState = mMode;
		mDesiredMode = HfPoker::PAUSE;
		::ShowWindow(m_hWnd, SW_HIDE);
		StoreCursor();
		::Sleep(1000);
		StoreCursor();
		StartCapture();
	}
}


void CPALZDlg::StartCapture()
{
	SetTimer(7, (int)(1000/mFrequency), 0);
}

void CPALZDlg::StopCapture()
{
	KillTimer(7);
}

void CPALZDlg::CaptureActiveWindow()
{
	// Make sure we won't go again before time.
	StopCapture();

	// Check for user mode change.
	if (mMode != mOutput.mState)
	{
		switch (mOutput.mState)
		{
			case HfPoker::PAUSE:		OnPause();	break;
			case HfPoker::ANALYZE:		OnAnalyze();	break;
			case HfPoker::PLAY:		OnPlay();	break;
			case HfPoker::PLAY_HIDDEN:	OnQuietPlay();	break;
		}
	}

	// Check if we are to go back to analyze mode again.
	if (mMode >= HfPoker::PLAY)
	{
		if (IsCursorMoved())
		{
			mDesiredMode = mMode;
			::ShowWindow(m_hWnd, SW_SHOW);
			OnAnalyze();
			return;
		}
	}
	else if (mDesiredMode >= HfPoker::PLAY)
	{
		// Reset the cursor timing if the cursor was moved.
		IsCursorMoved();
		StoreCursor();

		// W8 7 seconds before going back to the old mode.
		if (GetTimeDiff(mModeChangeTime) > (int64)7*1000)
		{
			switch (mDesiredMode)
			{
				case HfPoker::PLAY:
				{
					OnPlay();
				}
				break;
				case HfPoker::PLAY_HIDDEN:
				{
					OnQuietPlay();
				}
				break;
			}
			mDesiredMode = HfPoker::PAUSE;
		}
	}
	else if (mMode == HfPoker::PAUSE)
	{
		// Pause yields nothing.
		StartCapture();
		return;
	}

	// Get ahold of a font to compare the card values against.
	static bool lGotFont = false;
	if (!lGotFont)
	{
		lGotFont = true;
		InitFont();
	}

	// Catch the active window.
	HWND lForegroundWindow = ::GetForegroundWindow();
	if (mBuggedWindow == m_hWnd || mBuggedWindow == 0)
	{
		mBuggedWindow = lForegroundWindow;
	}
	else
	{
		static int lTryCloseCount = 0;
		if (lTryCloseCount > 10)
		{
			TRACE0("Could not remove top window - exiting!\r\n");
			for (int i = 0; i < 10; ++i)
			{
				::Beep(500+i*200, 500);
				::Sleep(20);
			}
			::PostQuitMessage(1);
			StartCapture();
			return;
		}
		else if (mBuggedWindow != lForegroundWindow &&
			mMode >= HfPoker::PLAY_HIDDEN)
		{
			if (!::IsWindow(mBuggedWindow))
			{
				TRACE0("Shit! Window closed!\r\n");
			}
			else
			{
				::PostMessage(lForegroundWindow, WM_CLOSE, 0, 0);
			}
			::Sleep(200);
			// Retry!
			++lTryCloseCount;
			StartCapture();
			return;
		}
		else
		{
			lTryCloseCount = 0;
		}
	}

	// Check if window title resembles "poker".
	char lWindowName[200];
	lWindowName[0] = '\0';
	::GetWindowText(mBuggedWindow, lWindowName, sizeof(lWindowName));
	_strupr(lWindowName);
	if (!strstr(lWindowName, "SEVEN CARD STUD") &&
		!strstr(lWindowName, "SNAG-") &&
		!strstr(lWindowName, "7 CARD STUD"))
	{
		mBuggedWindow = m_hWnd;

		if (mWaitTime < 500/mFrequency)
		{
			AddOutput("Waiting...\r\n");
		}
		mWaitTime += 1000/mFrequency;
		StartCapture();
		return;
	}
	if (mWaitTime > 0)
	{
		AddOutput("Capping!\r\n");
		mWaitTime = 0;
	}

	// Get ahold of the window contents as a bitmap image.
	RECT lRect;
	::GetClientRect(mBuggedWindow, &lRect);
	unsigned char* lBitmap = 0;
	int w = lRect.right-lRect.left;
	int h = lRect.bottom-lRect.top;
	if (w <= 0 || h <= 0)
	{
		AddOutput(".");
		StartCapture();
		return;
	}
	int bpp = 0;
	BITMAPINFO lBmInfo;
	ZeroMemory(&lBmInfo, sizeof(lBmInfo));
	HDC lBuggedWindowHandle = ::GetDC(mBuggedWindow);
	HBITMAP lImageHandle = ::CreateCompatibleBitmap(lBuggedWindowHandle, w, h);
	if (lImageHandle)
	{
		HDC lCopyHandle = ::CreateCompatibleDC(lBuggedWindowHandle);
		::SelectObject(lCopyHandle, lImageHandle);
		::BitBlt(lCopyHandle, 0, 0, w, h, lBuggedWindowHandle, lRect.left, lRect.top, SRCCOPY);
		lBmInfo.bmiHeader.biSize = sizeof(lBmInfo.bmiHeader);
		::GetDIBits(lCopyHandle, lImageHandle, 0, 0, 0, &lBmInfo, DIB_RGB_COLORS);
		lBmInfo.bmiHeader.biHeight = -h;
		lBmInfo.bmiHeader.biCompression = 0;
		bpp = lBmInfo.bmiHeader.biBitCount/8;
		lBitmap = new(unsigned char[w*h*bpp]);
		h = ::GetDIBits(lCopyHandle, lImageHandle, 0, h, lBitmap, &lBmInfo, DIB_RGB_COLORS);
		::SelectObject(lCopyHandle, ::GetStockObject(WHITE_PEN));
		::DeleteDC(lCopyHandle);
		::DeleteObject(lImageHandle);
	}
	::ReleaseDC(mBuggedWindow, lBuggedWindowHandle);

	// Handle the bitmap image.
	if (lBitmap)
	{
		// Setup the debug bitmap.
		HfPoker::Bitmap lBmpObj(w, h, bpp, lBitmap);	// Automatically drops raw bitmap when done.
		SetupDebugBitmap(lBitmap, w, h, bpp);
		HfPoker::Bitmap lDbgBmpObj(w, h, bpp, mDebugBitmap);	// Automatically drops raw bitmap when done.
		mImageAnalyzer->SetBitmaps(lBmpObj, lDbgBmpObj);
		mGameAnalyzer->SetBitmaps(lBmpObj, lDbgBmpObj);

		// Analyze the image.
		HfPoker::Table* lTable = new HfPoker::Table;
		if (mImageAnalyzer->AnalyzeBitmap(*lTable))
		{
			// Analyze the game.
			HfPoker::GameState lState;
			if (mGameAnalyzer->AnalyzeTable(lTable, lState))
			{
				lState.mLastCardCount = mState.mCardCount;
				memcpy(&mState, &lState, sizeof(mState));
				DescidePlay();
				if (mState.mNewDeal)
				{
					AddOutput("NEW DEAL!\r\n");
					mFile << "\n" << mGambler->GetLogText() << ", " << mState.mPlayerCount << " plyrs: " << std::flush;
					mState.mCheckCount = 0;
					mState.mCallCount = 0;
					mState.mRaiseCount = 0;
				}
				else if (mState.mCardCount != mState.mLastCardCount)
				{
					mFile << "  " << mState.mCardCount << " st.";
				}

				if (mState.mSelfPlaying)
				{
					mFile << " " << mGambler->GetMyChance() << "/" << mState.mMyChance;
				}
			}
		}
		else
		{
			mGameAnalyzer->Reset();
			delete (lTable);
		}
		lTable = 0;

		// Draw and print debug info.
		DrawDebugBitmap();
		AddOutput(mImageAnalyzer->GetDebugText());
		AddOutput(mGameAnalyzer->GetDebugText());

		// We don't want to delete the raw debug bitmap.
		lDbgBmpObj.SetBits(0);

		// Find all playing buttons.
		if (GetStableButtons())
		{
			if (Play())
			{

				DescidePlay();
			}
		}
	}

	// Start the capture timer again for next time.
	StartCapture();
}

int CALLBACK CPALZDlg::EnumFontCallback(CONST LOGFONT* lplf, CONST TEXTMETRIC*, DWORD, LPARAM lpData)
{
	CPALZDlg* lDialog = (CPALZDlg*)lpData;
	lDialog->AddOutput(lplf->lfFaceName);
	return (1);
}

void CPALZDlg::InitFont()
{
	const int lcBitmapSize = 256;
	int w = lcBitmapSize;
	int h = lcBitmapSize;
	int bpp = 0;
	BITMAPINFO lBmInfo;
	ZeroMemory(&lBmInfo, sizeof(lBmInfo));
	HDC lSubWindowDc = ::GetDC(mImage.m_hWnd);//::GetDC(m_hWnd);
	HBITMAP lImageHandle = ::CreateCompatibleBitmap(lSubWindowDc, w, h);
	if (lImageHandle)
	{
		HDC lFontDcHandle = ::CreateCompatibleDC(lSubWindowDc);
		//::EnumFonts(lSubWindowDc, "Century ", EnumFontCallback, (LPARAM)this);
		HFONT lFont = ::CreateFont
		(
			77,				// nHeight
			0,				// nWidth
			0,				// nEscapement
			0,				// nOrientation
			1000,				// nWeight
			FALSE,				// bItalic
			FALSE,				// bUnderline
			0,				// cStrikeOut
			ANSI_CHARSET,			// nCharSet
			OUT_RASTER_PRECIS,		// nOutPrecision
			CLIP_DEFAULT_PRECIS,		// nClipPrecision
			PROOF_QUALITY,			// nQuality
			DEFAULT_PITCH | FF_DONTCARE,	// nPitchAndFamily
			"Century "			// lpszFacename
		);
		const char* lcCardValues[] = {"2","3","4","5","6","7","8","9","10","J","Q","K","A"};
		int i;
		// Show font.
		for (i = 0; i < sizeof(lcCardValues)/sizeof(char*); ++i)
		{
			HFONT lOldFont = (HFONT)::SelectObject(lSubWindowDc, lFont);
			::TextOut(lSubWindowDc, 40*i, 0, lcCardValues[i], (int)::strlen(lcCardValues[i]));
			::SelectObject(lSubWindowDc, lOldFont);
		}
		// Extract font.
		HFONT lOldFont = (HFONT)::SelectObject(lFontDcHandle, lFont);
		::SelectObject(lFontDcHandle, lImageHandle);
		for (i = 0; i < sizeof(lcCardValues)/sizeof(char*); ++i)
		{
			RECT lRect;
			lRect.left = 0;
			lRect.right = w-1;
			lRect.top = 0;
			lRect.bottom = h-1;
			HBRUSH lBrush = (HBRUSH)::GetStockObject(WHITE_BRUSH);
			::FillRect(lFontDcHandle, &lRect, lBrush);
			::TextOut(lFontDcHandle, 0, 0, lcCardValues[i], (int)strlen(lcCardValues[i]));
			lBmInfo.bmiHeader.biSize = sizeof(lBmInfo.bmiHeader);
			::GetDIBits(lFontDcHandle, lImageHandle, 0, 0, 0, &lBmInfo, DIB_RGB_COLORS);
			lBmInfo.bmiHeader.biHeight = -h;
			lBmInfo.bmiHeader.biCompression = 0;
			bpp = lBmInfo.bmiHeader.biBitCount/8;
			unsigned char* lBitmap = new(unsigned char[w*h*bpp]);
			h = ::GetDIBits(lFontDcHandle, lImageHandle, 0, h, lBitmap, &lBmInfo, DIB_RGB_COLORS);

			int lLeft = w;
			int lRight = -1;
			int lTop = h;
			int lBottom = -1;
			for (int y = h-10; y >= 0; --y)
			{
				bool lRowHoldsData = false;
				for (int x = w-10; x >= 0; --x)
				{
					if (lBitmap[(y*w+x)*bpp+0] < 100)
					{
						lRowHoldsData = true;
						if (x > lRight)
						{
							lRight = x;
						}
						if (x < lLeft)
						{
							lLeft = x;
						}
					}
				}
				if (lRowHoldsData)
				{
					if (y < lTop)
					{
						lTop = y;
					}
					if (y > lBottom)
					{
						lBottom = y;
					}
				}
			}

			HfPoker::Bitmap* lGlyphBitmap = new HfPoker::Bitmap(lRight-lLeft+1, lBottom-lTop+1, 1, 0);
			unsigned char* lGlyphRawBitmap = new (unsigned char[lGlyphBitmap->GetWidth()*lGlyphBitmap->GetHeight()]);
			lGlyphBitmap->SetBits(lGlyphRawBitmap);
			for (int y = lTop; y <= lBottom; ++y)
			{
				for (int x = lLeft; x <= lRight; ++x)
				{
					// Blur the glyph at copy. Helps glyph compare later on.
					unsigned u1 = lBitmap[(y*w+(x-1))*bpp+0];
					unsigned u2 = lBitmap[(y*w+(x+1))*bpp+0];
					unsigned u3 = lBitmap[((y-1)*w+x)*bpp+0];
					unsigned u4 = lBitmap[((y+1)*w+x)*bpp+0];
					unsigned u5 = lBitmap[(y*w+x)*bpp+0];
					lGlyphRawBitmap[(y-lTop)*lGlyphBitmap->GetWidth()+x-lLeft] = (unsigned char)((u1+u2+u3+u4+u5)/5);
				}
			}
			mImageAnalyzer->AddGlyph(lGlyphBitmap);
			delete[] (lBitmap);
		}
		::SelectObject(lFontDcHandle, ::GetStockObject(WHITE_PEN));
		::SelectObject(lFontDcHandle, lOldFont);
		::DeleteObject(lFont);
		::DeleteObject(lImageHandle);
		::DeleteDC(lFontDcHandle);
	}
	::ReleaseDC(m_hWnd, lSubWindowDc);
}

BOOL CPALZDlg::EnumChildWindow(HWND pHWnd)
{
	if (!::IsWindowVisible(pHWnd) ||
		!::IsWindowEnabled(pHWnd))
	{
		return (TRUE);
	}

	char lButtonName[25];
	::GetWindowText(pHWnd, lButtonName, sizeof(lButtonName));
	::_strupr(lButtonName);
	if (::strlen(lButtonName) > 20)
	{
		return (TRUE);
	}
	else if (lButtonName[0] == '\0')
	{
		char lCtrlClassName[100];
		if (::GetClassName(pHWnd, lCtrlClassName, sizeof(lCtrlClassName)) > 0)
		{
			if (strcmp(lCtrlClassName, "RICHEDIT") == 0)
			{
				LRESULT lFirstLine = ::SendMessage(pHWnd, EM_GETFIRSTVISIBLELINE, 0, 0);
				LRESULT lLineCount = ::SendMessage(pHWnd, EM_GETLINECOUNT, 0, 0);
				int lWinnerTextLineCount = 0;
				mWinnerString[0] = '\0';
				for (LRESULT i = lFirstLine; i < lLineCount; ++i)
				{
					char lLineText[200];
					*(WORD*)lLineText = sizeof(lLineText);
					::SendMessage(pHWnd, EM_GETLINE, i, (LPARAM)lLineText);
					char* lWinnerString = strstr(lLineText, " wins ");
					if (lWinnerString)
					{
						if (lWinnerTextLineCount)
						{
							lWinnerTextLineCount = 0;
							::strcat(mWinnerString, ", ");
						}
						++lWinnerTextLineCount;
						while (*--lWinnerString != ':');
						lWinnerString += 2;
						int len = (int)::strlen(lWinnerString);
						if (::strchr(lWinnerString, '\r'))
						{
							len -= 2;
						}
						StringNCopy(mWinnerString+strlen(mWinnerString), lWinnerString, len);
					}
					else if (lWinnerTextLineCount > 0)
					{
						if (::strchr(lLineText, ':'))
						{
							lWinnerTextLineCount = 0;
						}
						else
						{
							++lWinnerTextLineCount;
							int len = (int)::strlen(lLineText);
							if (::strchr(lLineText, '\r'))
							{
								len -= 2;
							}
							StringNCopy(mWinnerString+strlen(mWinnerString), lLineText, len);
							/*if (::strchr(lLineText, '.'))
							{
								lWinnerTextLineCount = 0;
							}*/
						}
					}
				}
				if(mWinnerString[0] != '\0' &&
					::strcmp(mWinnerString, mLastWinnerString) != 0)
				{
					mFile << " " << mWinnerString << "\n";
					AddOutput(mWinnerString);
					::strcpy(mLastWinnerString, mWinnerString);
				}
			}
		}
		return (TRUE);
	}

	if (StringCompare(lButtonName, "BRING IN"))
	{
		mBringIn = true;
		++mCallButtonCount;
		++mGameButtonCount[0];
		mChildCheckFoldButton[0] = pHWnd;
		mChildCallButton[0] = pHWnd;
	}
	else if (StringCompare(lButtonName, "CALL ANY") ||
		StringCompare(lButtonName, "CHECK/CALL"))
	{
		++mCallButtonCount;
		++mGameButtonCount[0];
	}
	else if (StringCompare(lButtonName, "COMPLETE/RAISE") ||
		StringCompare(lButtonName, "BET/RAISE") ||
		StringCompare(lButtonName, "RAISE ANY"))
	{
		++mRaiseButtonCount;
		++mGameButtonCount[0];
	}
	else if (StringCompare(lButtonName, "COMPLETE TO"))
	{
		++mRaiseButtonCount;
		++mGameButtonCount[0];
		mChildRaiseButton[0] = pHWnd;
	}
	else if (StringCompare(lButtonName, "CHECK/FOLD"))
	{
		++mCheckButtonCount;
		++mGameButtonCount[0];
	}
	else if (StringCompare(lButtonName, "CHECK"))
	{
		++mCheckButtonCount;
		++mGameButtonCount[0];
		if (!mChildCheckFoldButton[0])
		{
			mChildCheckFoldButton[0] = pHWnd;
		}
		if (!mChildCallButton[0])
		{
			mChildCheckFoldButton[0] = pHWnd;
			mChildCallButton[0] = pHWnd;
		}
	}
	else if (StringCompare(lButtonName, "CALL"))
	{
		++mCallButtonCount;
		++mGameButtonCount[0];
		if (!mChildCallButton[0])
		{
			mChildCallButton[0] = pHWnd;
		}
	}
	else if (StringCompare(lButtonName, "BET") ||
		StringCompare(lButtonName, "RAISE"))
	{
		++mRaiseButtonCount;
		++mGameButtonCount[0];
		if (!mChildRaiseButton[0])
		{
			mChildRaiseButton[0] = pHWnd;
		}
	}
	else if (StringCompare(lButtonName, "FOLD TO"))
	{
		// Nothing! Must be here to assure the below case.
	}
	else if (StringCompare(lButtonName, "FOLD"))
	{
		++mFoldButtonCount;
		++mGameButtonCount[0];
		if (!mChildCheckFoldButton[0])
		{
			mChildCheckFoldButton[0] = pHWnd;
		}
	}
	else if (StringCompare(lButtonName, "I AM BACK"))
	{
		mIAmBackButton = pHWnd;
	}
	else if (StringCompare(lButtonName, "AUTO POST ANTE"))
	{
		mAnteButton = pHWnd;
	}

	return (TRUE);
}

BOOL CPALZDlg::EnumBuggedChildWindow(HWND pHWnd, LPARAM pLParam)
{
	CPALZDlg* lDlg = (CPALZDlg*)pLParam;
	return (lDlg->EnumChildWindow(pHWnd));
}

bool CPALZDlg::GetStableButtons()
{
	mBringIn = false;
	mIAmBackButton = 0;
	mAnteButton = 0;
	mFoldButtonCount = 0;
	mCheckButtonCount = 0;
	mCallButtonCount = 0;
	mRaiseButtonCount = 0;
	mGameButtonCount[1] = mGameButtonCount[0];
	mChildCheckFoldButton[1] = mChildCheckFoldButton[0];
	mChildCallButton[1] = mChildCallButton[0];
	mChildRaiseButton[1] = mChildRaiseButton[0];
	mGameButtonCount[0] = 0;
	mChildCheckFoldButton[0] = 0;
	mChildCallButton[0] = 0;
	mChildRaiseButton[0] = 0;
	::EnumChildWindows(mBuggedWindow, EnumBuggedChildWindow, (LPARAM)this);
	if (mGameButtonCount[0] == mGameButtonCount[1] &&
		mChildCheckFoldButton[0] == mChildCheckFoldButton[1] &&
		mChildCallButton[0] == mChildCallButton[1] &&
		mChildRaiseButton[0] == mChildRaiseButton[1])
	{
		++mStableButtonCount;
	}
	else
	{
		mStableButtonCount = 0;
	}

	if (mStableButtonCount >= 2)
	{
		if (mIAmBackButton)
		{
			// Disable "I am back" click!
			//return (true);
			return (false);
		}
		else if (mGameButtonCount[0] >= 2 &&
			mGameButtonCount[0] <= 4)
		{
			if (mFoldButtonCount >= 0 && mFoldButtonCount <= 1 &&		// 0=bring in, 1=normal.
				(mCheckButtonCount^mCallButtonCount) == 1 &&		// One of them, not more.
				mRaiseButtonCount >= 0 && mRaiseButtonCount <= 2)	// Maximum two raise buttons.
			{
				// Make sure it's 1 fold, 1 check/call and 1 or 2 raise.
				//return (mGameButtonCount[0]-2 == mRaiseButtonCount);
				return (true);
			}
		}
	}
	return (false);
}

void CPALZDlg::DescidePlay()
{
	if (!mState.mUpdated ||
		!mState.mSelfPlaying ||
		mState.mDuringDeal)
	{
		mLastAction = HfPoker::WAIT;
		return;
	}

	HfPoker::GAME_ACTION lAction = mGambler->DescidePlay(mState);
	AddOutput(mGambler->GetDebugText());
	if (lAction != HfPoker::WAIT)
	{
		if (lAction != HfPoker::CALL)
		{
			// Fold or raise. Rarely uses "very long thinking",
			// otherwise just "long thinking".
			if (mState.mCardCount >= 6 || GetRandomDouble() > 0.9)
			{
				mClickTime = (uint64)(mState.mCardCount*mState.mPlayerCount*GetRandomDouble()*70 + GetRandomDouble()*1500 + 500);
			}
			else
			{
				mClickTime = (uint64)(mState.mCardCount*mState.mPlayerCount*GetRandomDouble()*60 + GetRandomDouble()*1000 + 200);
			}

			// Fold uses up less time.
			if (lAction == HfPoker::FOLD_OR_CHECK)
			{
				mClickTime = (uint64)(mClickTime*0.7);
			}
		}
		else
		{
			// Call uses less thinking.
			mClickTime = (uint64)(mState.mCardCount*mState.mPlayerCount*GetRandomDouble()*50 + GetRandomDouble()*500 + 10);
		}
		mLastAction = lAction;
	}
}

bool CPALZDlg::Play()
{
	// Add current time first stable button time.
	if (mClickTime <= 20000)
	{
		mClickTime += GetTime();
	}

	const char* lcPriceNames[10] =
	{
		"royal",
		"str fl",
		"4oak",
		"full h",
		"flu",
		"str",
		"3oak",
		"2prs",
		"pa",
		"n"
	};
	switch (mLastAction)
	{
		case HfPoker::RAISE:
		{
			// Delay tingie; is used to seem like thinking.
			if (GetTimeDiff(mClickTime) >= (int64)0)
			{
				if (DoClick(mChildRaiseButton[0]))
				{
					mFile << " raise (" << lcPriceNames[mState.mMyPrice] << ", " << (int)mState.mMyHighCard+2 << " vs. " << lcPriceNames[mState.mOthersBestKnownPrice] << ", " << (int)mState.mOthersBestKnownHighCard << "), ";
					++mState.mRaiseCount;
					return (true);
				}
			}
			else
			{
				// Don't fall through on raise; it might cause a
				// call instead of raise!
				break;
			}
		}
		// Fall through.
		case HfPoker::CALL:
		{
			// Delay tingie; is used to seem like thinking.
			if (GetTimeDiff(mClickTime) >= (int64)0 || mBringIn)
			{
				if (DoClick(mChildCallButton[0]))
				{
					if (mCallButtonCount)
					{
						mFile << " call (";
						++mState.mCallCount;
					}
					else if (mCheckButtonCount)
					{
						mFile << " check (";
						++mState.mCheckCount;
					}
					mFile << lcPriceNames[mState.mMyPrice] << ", " << (int)mState.mMyHighCard+2 << " vs. " << lcPriceNames[mState.mOthersBestKnownPrice] << ", " << (int)mState.mOthersBestKnownHighCard << "), ";
					return (true);
				}
			}
		}
		break;
		case HfPoker::FOLD_OR_CHECK:
		{
			// Delay tingie; is used to seem like thinking.
			if (GetTimeDiff(mClickTime) >= (int64)0 || mBringIn)
			{
				if (DoClick(mChildCheckFoldButton[0]))
				{
					mFile << " fold (" << lcPriceNames[mState.mMyPrice] << ", " << (int)mState.mMyHighCard+2 << " vs. " << lcPriceNames[mState.mOthersBestKnownPrice] << ", " << (int)mState.mOthersBestKnownHighCard << "), ";
					return (true);
				}
			}
		}
		break;
		case HfPoker::WAIT:
		{
			// Check if we're out of the game.
			if (mMode >= HfPoker::PLAY)
			{
				if (mIAmBackButton)
				{
					AddOutput("Joining back in...\r\n");
					if (DoClick(mIAmBackButton))
					{
						::Sleep(200);
						AddOutput("Auto post ante...\r\n");
						DoClick(mAnteButton);
					}
					mIAmBackButton = 0;
				}
			}
		}
		break;
		default:
		{
			AddOutput("CPALZDlg::Play(): WTF?\r\n");
			assert(false);
		}
		break;
	}
	return (false);
}

void CPALZDlg::SetupDebugBitmap(const unsigned char* pBitmap, int pWidth, int pHeight, int pBPP)
{
	if (pWidth != mWidth || pHeight != mHeight || mBPP != pBPP)
	{
		if (mDebugBitmap)
		{
			delete[] (mDebugBitmap);
			mDebugBitmap = 0;
		}
		mWidth = pWidth;
		mHeight = pHeight;
		mBPP = pBPP;
		if (mWidth != 0 && mHeight != 0 && mBPP != 0)
		{
			mDebugBitmap = new (unsigned char[mWidth*mHeight*mBPP]);
		}
	}

	// Copy image to debug image, and make it black and white.
	for (int y = 0; y < mHeight; ++y)
	{
		for (int x = 0; x < mWidth; ++x)
		{
			unsigned r = pBitmap[(y*mWidth+x)*mBPP+0];
			unsigned g = pBitmap[(y*mWidth+x)*mBPP+1];
			unsigned b = pBitmap[(y*mWidth+x)*mBPP+2];
			mDebugBitmap[(y*mWidth+x)*mBPP+0] = (unsigned char)((r+g+b)/3);
			mDebugBitmap[(y*mWidth+x)*mBPP+1] = (unsigned char)((r+g+b)/3);
			mDebugBitmap[(y*mWidth+x)*mBPP+2] = (unsigned char)((r+g+b)/3);
		}
	}
}

void CPALZDlg::DrawDebugBitmap() const
{
	if (mDebugBitmap)
	{
		// Show the debug image.
		BITMAPINFO lBmInfo;
		ZeroMemory(&lBmInfo, sizeof(lBmInfo));
		lBmInfo.bmiHeader.biSize = sizeof(lBmInfo.bmiHeader);
		lBmInfo.bmiHeader.biWidth = mWidth;
		lBmInfo.bmiHeader.biHeight = -mHeight;
		lBmInfo.bmiHeader.biPlanes = 1;
		lBmInfo.bmiHeader.biSizeImage = mWidth*mHeight*mBPP;
		lBmInfo.bmiHeader.biCompression = 0;
		lBmInfo.bmiHeader.biBitCount = (WORD)mBPP*8;
		HDC lWindowHandle = ::GetDC(mImage.m_hWnd);
		HBITMAP lImageHandle = ::CreateCompatibleBitmap(lWindowHandle, mWidth, mHeight);
		if (lImageHandle)
		{
			HDC lCopyHandle = ::CreateCompatibleDC(lWindowHandle);
			::SelectObject(lCopyHandle, lImageHandle);
			::SetDIBits(lCopyHandle, lImageHandle, 0, mHeight, mDebugBitmap, &lBmInfo, DIB_RGB_COLORS);
			::BitBlt(lWindowHandle, 0, 0, mWidth, mHeight, lCopyHandle, 0, 0, SRCCOPY);
			::SelectObject(lCopyHandle, ::GetStockObject(WHITE_PEN));
			::DeleteDC(lCopyHandle);
			::DeleteObject(lImageHandle);
		}
		if (mGameAnalyzer->GetLastAnalyzedTable())
		{
			HfPoker::Table* lTable = mGameAnalyzer->GetLastAnalyzedTable();
			const std::list<HfPoker::Hand*>& lHands = lTable->GetHands();
			for (std::list<HfPoker::Hand*>::const_iterator i = lHands.begin(); i != lHands.end(); ++i)
			{
				char a[100];
				::sprintf(a, "v: %g", (*i)->GetChance());
				::TextOut(lWindowHandle, (*i)->GetPlayerPosX()+10, (*i)->GetPlayerPosY()+60, a, (int)strlen(a));
			}
		}
		::ReleaseDC(mImage.m_hWnd, lWindowHandle);
	}
}

bool CPALZDlg::GetMidButtonClientPos(HWND pChildButton, int& pMidX, int& pMidY)
{
	bool lGotPosition = false;
	if (pChildButton)
	{
		RECT lRect;
		if (::IsWindowVisible(pChildButton) &&
			::GetWindowRect(pChildButton, &lRect))
		{
			WINDOWINFO wi;
			wi.cbSize = sizeof(wi);
			if (::GetWindowInfo(mBuggedWindow, &wi))
			{
				pMidX = (lRect.left+lRect.right)/2 - wi.rcClient.left;
				pMidY = (lRect.top+lRect.bottom)/2 - wi.rcClient.top;
				pMidX += (int)(GetRandomDouble()*6.0-3.0);
				pMidX += (int)(GetRandomDouble()*20.0-10.0);
				lGotPosition = true;
			}
		}
	}
	return (lGotPosition);
}

void CPALZDlg::SetClientCursorPos(int pWindowX, int pWindowY)
{
	/*
	WINDOWPLACEMENT lWP;
	if (::GetWindowPlacement(mBuggedWindow, &lWP))
	{
		pWindowX += lWP.rcNormalPosition.left;
		pWindowY += lWP.rcNormalPosition.top;
	}
	*/

	WINDOWINFO wi;
	wi.cbSize = sizeof(wi);
	if (::GetWindowInfo(mBuggedWindow, &wi))
	{
		pWindowX += wi.rcClient.left;
		pWindowY += wi.rcClient.top;
	}

	::SetCursorPos(pWindowX, pWindowY);
}

void CPALZDlg::StoreCursor()
{
	::GetCursorPos(&mOldPoint);
}

bool CPALZDlg::IsCursorMoved()
{
	POINT lPoint;
	if (::GetCursorPos(&lPoint))
	{
		if (abs(lPoint.x-mOldPoint.x) > 600 ||
			abs(lPoint.y-mOldPoint.y) > 500)
		{
			mModeChangeTime = GetTime();
			return (true);
		}
	}
	return (false);
}

bool CPALZDlg::DoClick(HWND pChildButton)
{
	bool lDidClick = false;
	int pWindowX = 10;
	int pWindowY = 30;
	if (GetMidButtonClientPos(pChildButton, pWindowX, pWindowY))
	{
		if (mMode >= HfPoker::PLAY)
		{
			SetClientCursorPos(pWindowX, pWindowY);
			StoreCursor();
			//if (::SendMessage(pChildButton, (UINT)BM_GETSTATE, 0, 0) == BST_UNCHECKED)
			{
				::PostMessage(pChildButton, WM_LBUTTONDOWN, 0, MAKELPARAM(1, 1));
				::PostMessage(pChildButton, WM_LBUTTONUP, 0, MAKELPARAM(1, 1));
			}
		}
		else
		{
			//mLastAction = HfPoker::WAIT;
		}
		::Sleep(100);
		lDidClick = true;
	}
	else
	{
		/*if (mMode <= HfPoker::ANALYZE)	// mMode: 0==pause, 1==analyze, 2==play, 3==quiet_play.
		{
			//mLastAction = HfPoker::WAIT;
			lDidClick = true;
		}*/
	}
	return (lDidClick);
}

bool CPALZDlg::IsStringSeparator(char pChar)
{
	switch(pChar)
	{
		case ' ':
		case 255:
		case '\t':
		case '\v':
		case '\r':
		case '\n':
		case '\0':
		{
			return (true);
		}
	}
	return (false);
}

bool CPALZDlg::StringCompare(const char* pc1, const char* pc2)
{
	/*char* lStr = ::strstr(pc1, pc2);
	if (lStr)
	{
		if (lStr == pc1 || (lStr != pc1 && IsStringSeparator(*(lStr-1))))
		{
			if (IsStringSeparator(lStr[::strlen(pc2)]))
			{
				return (true);
			}
		}
	}
	return (false);*/
	return (::strncmp(pc1, pc2, ::strlen(pc2)) == 0);
}
