
// Author: Jonas Byström, highfestiva@gmail.com. August 2003.
// Open source without neither limitations nor warranty.


#ifndef HFPOKERGAMEANALYZER_H
#define HFPOKERGAMEANALYZER_H


#include <list>


namespace HfPoker
{


class Bitmap;
class GameState;
class Table;


class GameAnalyzer
{
public:
				GameAnalyzer();
	virtual			~GameAnalyzer();

	virtual void		Reset();
	virtual void		ResetOld();

	virtual bool		AnalyzeTable(Table* pTable, GameState& pState);
	virtual bool		CalculateChance(GameState& pState) = 0;
	Table*			GetLastAnalyzedTable() const;

	void			SetBitmaps(const Bitmap& pBitmap, const Bitmap& pDebugBitmap);
	const char*		GetDebugText() const;

protected:
	int			mWidth;
	int			mHeight;
	int			mBPP;	// Bytes per pixel.
	const unsigned char*	mBitmap;
	unsigned char*		mDebugBitmap;
	int			mDealSameCount;
	bool			mWantNextDeal;
	Table*			mTable;
	Table*			mPreviousTable;
	std::list<Table*>	mOldTables;

	char			mDebugText[10000];
};


};


#endif // HFPOKERGAMEANALYZER_H
