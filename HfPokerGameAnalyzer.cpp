
// Author: Jonas Byström, highfestiva@gmail.com. August 2003.
// Open source without neither limitations nor warranty.


#include "SysInclude.h"
#include "HfPokerBitmap.h"
#include "HfPokerGameAnalyzer.h"
#include "HfPokerGameState.h"
#include "HfPokerTable.h"
#include "HfStd.h"


namespace HfPoker
{


GameAnalyzer::GameAnalyzer():
	mWidth(-1),
	mHeight(-1),
	mBPP(-1),
	mDebugBitmap(0),
	mDealSameCount(0),
	mWantNextDeal(true),
	mTable(0),
	mPreviousTable(0)
{
	mDebugText[0] = '\0';
}

GameAnalyzer::~GameAnalyzer()
{
	Reset();

	mDebugBitmap = 0;
}


void GameAnalyzer::Reset()
{
	ResetOld();

	char a[50];
	if (mTable)
	{
		::strcpy(a, "Reset!\r\n");
		::strcat(mDebugText, a);
	}
	else
	{
		::strcpy(mDebugText, "HardReset!\r\n");
	}

	if (mTable)
	{
		delete (mTable);
		mTable = 0;
	}

	// Paint the debug bitmap red upon Reset. Note that
	// ResetOld already made the bitmap yellowish.
	if (mDebugBitmap)
	{
		for (int y = 0; y < mHeight; ++y)
		{
			for (int x = 0; x < mWidth; ++x)
			{
				mDebugBitmap[(y*mWidth+x)*mBPP+1] /= 3;
			}
		}
	}
}

void GameAnalyzer::ResetOld()
{
	while (!mOldTables.empty())
	{
		Table* lTable = mOldTables.front();
		mOldTables.pop_front();
		delete (lTable);
	}
	if (mPreviousTable)
	{
		delete (mPreviousTable);
		mPreviousTable = 0;
	}
	mDealSameCount = 0;
	mWantNextDeal = true;
	char a[50];
	::sprintf(a, "ResetOld!\r\n");
	::strcat(mDebugText, a);

	// Paint the debug bitmap yellow upon ResetOld.
	if (mDebugBitmap)
	{
		for (int y = 0; y < mHeight; ++y)
		{
			for (int x = 0; x < mWidth; ++x)
			{
				mDebugBitmap[(y*mWidth+x)*mBPP+0] /= 3;
			}
		}
	}
}


bool GameAnalyzer::AnalyzeTable(Table* pTable, GameState& pState)
{
	char a[100];
	mDebugText[0] = '\0';

	pState.mUpdated = false;
	pState.mDuringDeal = false;
	pState.mNewDeal = false;

	if (!mTable)
	{
		// First time.
		mDealSameCount = 0;
		mTable = pTable;
		mWantNextDeal = false;	// Wait until table settles.
		return (false);
	}

	if (mWantNextDeal)
	{
		if (*mTable != *pTable)
		{
			// Found new deal! Now wait until it
			// settles down.
			mDealSameCount = 0;
			pState.mDuringDeal = true;
			delete (mTable);
			mTable = pTable;
			mWantNextDeal = false;	// Wait until table settles.
			return (false);
		}
		else
		{
			// Same deal as last time we checked.
			delete (pTable);
			return (false);
		}
	}
	else
	{
		// Check if table stabilized.
		if (*mTable != *pTable)
		{
			// Deal hasn't settled yet.
			mDealSameCount = 0;
			pState.mDuringDeal = true;
			delete (mTable);
			mTable = pTable;
			return (false);
		}
		else
		{
			++mDealSameCount;
			if (mDealSameCount < 2)
			{
				// We wait some more before thinking of a settled deal.
				pState.mDuringDeal = true;
				delete (mTable);
				mTable = pTable;
				return (false);
			}
			else
			{
				// Deal seems to have settled.
				mWantNextDeal = true;	// After this we want a new deal.
			}
		}
	}

	// Check for empty table.
	if (mTable->GetHands().empty())
	{
		delete (pTable);
		::sprintf(a, "ERROR: Deal not found!\r\n");
		::strcat(mDebugText, a);
		DumpScreenshot(mWidth, mHeight, mBPP, mBitmap);
		return (false);
	}

	// Flush old tables.
	while (mOldTables.size() > 20)
	{
		delete (mOldTables.front());
		mOldTables.pop_front();
	}

	bool lChance = CalculateChance(pState);

	// Use as previous table, also verify if new deal.
	if  (mPreviousTable)
	{
		mOldTables.push_back(mPreviousTable);
	}
	else
	{
		pState.mNewDeal = true;
	}
	mPreviousTable = mTable;
	mTable = pTable;

	return (lChance);
}

Table* GameAnalyzer::GetLastAnalyzedTable() const
{
	return (mPreviousTable);
}


void GameAnalyzer::SetBitmaps(const Bitmap& pBitmap, const Bitmap& pDebugBitmap)
{
	mWidth = pDebugBitmap.GetWidth();
	mHeight = pDebugBitmap.GetHeight();
	mBPP = pDebugBitmap.GetBPP();
	mBitmap = pBitmap.GetBits();
	mDebugBitmap = (unsigned char*)pDebugBitmap.GetBits();
}

const char* GameAnalyzer::GetDebugText() const
{
	assert(::strlen(mDebugText) < sizeof(mDebugText)/2);
	return (mDebugText);
}


};
