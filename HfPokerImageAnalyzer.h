
// Author: Jonas Byström, highfestiva@gmail.com. August 2003.
// Open source without neither limitations nor warranty.


#ifndef HFPOKERIMAGEANALYZER_H
#define HFPOKERIMAGEANALYZER_H


#include <list>
#include "HfPoker.h"
#include "HfPokerCardSlice.h"
#include "HfPokerLine.h"
#include "HfPokerRectangle.h"


namespace HfPoker
{


class Bitmap;
class Table;


class ImageAnalyzer
{
public:
				ImageAnalyzer();
				~ImageAnalyzer();

	virtual bool		AnalyzeBitmap(Table& pTable);
	virtual void		AddGlyph(Bitmap* pGlyph);
	void			SetBitmaps(const Bitmap& pBitmap, const Bitmap& pDebugBitmap);
	const char*		GetDebugText() const;

protected:
	int			mWidth;
	int			mHeight;
	int			mBPP;	// Bytes per pixel.
	const unsigned char*	mBitmap;
	unsigned char*		mDebugBitmap;
	bool			mDumpScreenshot;
	int			mTopLine;
	int			mLeftLine;
	std::list<Line>		mLines;
	std::list<Rectangle>	mRectangles;
	int			mCardWidth;	// Cards' width in pixels.
	int			mCardHeight;	// Cards' height in pixels.
	int			mCardCount;	// The number of cards on the table.
	int			mCardOverlap;	// The number of pixels that cards of same hand overlap.
	char			mDebugText[10000];
	std::list<Bitmap*>	mCardGlyphs;	// "2", "3", ..., "10", "J", "Q", "K", "A".
	std::list<CardSlice>	mCardSlices;

	static const unsigned	mscCardValueBrightColorThreshold;	// Threshold color; defines bright part of card.
	static const unsigned	mscRedCardBrightColorThreshold;		// Threshold color; defines bright part of card.
	static const unsigned	mscBlackCardBrightColorThreshold;	// Threshold color; defines bright part of card.
	static const unsigned	mscLineDarkColorThreshold;		// Threshold color; defines an dark edge.
	static const unsigned	mscLineColorDiffThreshold;		// Threshold color difference; defines an edge.
	static const unsigned	mscLineCloseThreshold;			// Distans in pixels.
	static const unsigned	mscLineHorizPickUpMinLengthThreshold;	// Length in pixels.
	static const unsigned	mscLineMinHorizLengthThreshold;		// Length in pixels.
	static const unsigned	mscLineMinVertLengthThreshold;		// Length in pixels.
	static const unsigned	mscLineMaxLengthThreshold;		// Length in pixels.

	virtual void		DetectLines(const unsigned char* pcBitmap);
	virtual void		FixBrokenLines();
	virtual void		MakeRectangles();
	virtual void		SortRectangles();
	virtual void		CalculateSizes();
	virtual void		SortOutCardRectangles(const unsigned char* pBitmap);
	virtual void		DetectCards(const unsigned char* pcBitmap);
	virtual void		SortHands(Table& pTable);
	virtual CARD_TYPE	DetectHeartsOrDiamonds(const unsigned char* pcBitmap, int pMid, int& pBottom, int pEdge) const;
	virtual CARD_TYPE	DetectClubsOrSpades(const unsigned char* pcBitmap, int pMid, int& pBottom, int pEdge) const;
	virtual CARD_VALUE	DetectCardValue(const unsigned char* pBitmap, int pLeft, int pRight, int pTop, int pBottom);

	virtual void		DebugDraw();

	inline void		SetDebugBitmapPixel(int pX, int pY, unsigned char pR, unsigned char pG, unsigned char pB)
	{
		assert(pX >= 0 && pX < mWidth);
		assert(pY >= 0 && pY < mHeight);
		mDebugBitmap[(pY*mWidth+pX)*mBPP+0] = pB;
		mDebugBitmap[(pY*mWidth+pX)*mBPP+1] = pG;
		mDebugBitmap[(pY*mWidth+pX)*mBPP+2] = pR;
	};
};


};


#endif // HFPOKERIMAGEANALYZER_H
