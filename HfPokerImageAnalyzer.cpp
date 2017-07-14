
// Author: Jonas Byström, highfestiva@gmail.com. August 2003.
// Open source without neither limitations nor warranty.


#include "SysInclude.h"
#include <math.h>
#include <Windows.h>
#include "HfPerformanceTimer.h"
#include "HfPokerBitmap.h"
#include "HfPokerTable.h"
#include "HfPokerHand.h"
#include "HfPokerImageAnalyzer.h"
#include "HfStd.h"


namespace HfPoker
{


ImageAnalyzer::ImageAnalyzer():
	mWidth(-1),
	mHeight(-1),
	mBPP(-1),
	mDebugBitmap(0),
	mDumpScreenshot(false),
	mCardWidth(-1),
	mCardHeight(-1),
	mCardCount(0),
	mCardOverlap(-1)
{
	mDebugText[0] = '\0';
}

ImageAnalyzer::~ImageAnalyzer()
{
	while (!mCardGlyphs.empty())
	{
		Bitmap* lGlyph = mCardGlyphs.front();
		mCardGlyphs.pop_front();
		delete (lGlyph);
	}

	mDebugBitmap = 0;
}


bool ImageAnalyzer::AnalyzeBitmap(Table& pTable)
{
	pTable.Reset();

	mDebugText[0] = '\0';
	PerformanceTimer lTimer;

	if (mWidth <= 0 || mHeight <= 0 || mBPP <= 0)
	{
		return (false);
	}

	// Find horizontal and vertical lines; for card edges.
	DetectLines(mBitmap);
	FixBrokenLines();

	// Sort out those who doesn't seem to be card edges.
	MakeRectangles();
	SortRectangles();
	CalculateSizes();
	SortOutCardRectangles(mBitmap);

	// Check what cards they are, and place them in hands.
	DetectCards(mBitmap);
	SortHands(pTable);

	// Draw debug and print information.
	DebugDraw();

	if (mDumpScreenshot)
	{
		mDumpScreenshot = false;
		DumpScreenshot(mWidth, mHeight, mBPP, mBitmap);
	}

	char a[100];
	::sprintf(a, "ImgAnlz=%g\r\n", lTimer.GetTime());
	//strcat(mDebugText, a);

	if (pTable.GetHands().empty())
	{
		/*char a[50];
		::strcpy(a, "No hands found!\r\n");
		::strcat(mDebugText, a);*/
		return (false);
	}
	return (true);
}

void ImageAnalyzer::AddGlyph(Bitmap* pGlyph)
{
	mCardGlyphs.push_back(pGlyph);
}

void ImageAnalyzer::SetBitmaps(const Bitmap& pcBitmap, const Bitmap& pcDebugBitmap)
{
	mWidth = pcDebugBitmap.GetWidth();
	mHeight = pcDebugBitmap.GetHeight();
	mBPP = pcDebugBitmap.GetBPP();
	mBitmap = pcBitmap.GetBits();
	mDebugBitmap = (unsigned char*)pcDebugBitmap.GetBits();
}

const char* ImageAnalyzer::GetDebugText() const
{
	assert(::strlen(mDebugText) < sizeof(mDebugText)/2);
	return (mDebugText);
}



void ImageAnalyzer::DetectLines(const unsigned char* pcBitmap)
{
	bool lFoundTopLine = false;
	bool lFoundLeftLine = false;
	mTopLine = 0;	// Assume top of window.
	mLeftLine = 0;	// Assume left side of window.
	mLines.clear();

	int x;
	int y;

	if (mBPP == 4)
	{
		// Use optimized version.

		// Pick the horizontal lines.
		for (y = 0; y < mHeight-1; ++y)
		{
			unsigned lFoundLinePixelCount = 0;
			bool lFoundLine = false;
			for (x = 0; x < mWidth-1; ++x)
			{
				unsigned b = pcBitmap[((y*mWidth+x)<<2)+0];
				unsigned g = pcBitmap[((y*mWidth+x)<<2)+1];
				unsigned r = pcBitmap[((y*mWidth+x)<<2)+2];
				if (r+g+b < mscLineDarkColorThreshold)
				{
					int db = pcBitmap[(((y+1)*mWidth+x)<<2)+0] - (int)b;
					int dg = pcBitmap[(((y+1)*mWidth+x)<<2)+1] - (int)g;
					int dr = pcBitmap[(((y+1)*mWidth+x)<<2)+2] - (int)r;
					if (Abs(dr)+Abs(dg)+Abs(db) < (int)mscLineColorDiffThreshold)
					{
						if (lFoundLinePixelCount)
						{
							lFoundLine = true;
						}
					}
					else
					{
						++lFoundLinePixelCount;
					}
				}
				else if (lFoundLinePixelCount)
				{
					lFoundLine = true;
				}

				if (lFoundLine || x == mWidth-2)
				{
					if (lFoundLinePixelCount && !lFoundTopLine)
					{
						lFoundTopLine = true;
						mTopLine = y;
					}
					if (lFoundLinePixelCount >= mscLineHorizPickUpMinLengthThreshold &&
						lFoundLinePixelCount < mscLineMaxLengthThreshold)
					{
						mLines.push_back(Line(x-lFoundLinePixelCount, y+1, x-1, y+1));
					}
					lFoundLinePixelCount = 0;
					lFoundLine = false;
				}
			}
		}

		// Pick the vertical lines.
		for (x = 0; x < mWidth-1; ++x)
		{
			unsigned lFoundLinePixelCount = 0;
			bool lFoundLine = false;
			for (y = 0; y < mHeight-1; ++y)
			{
				unsigned b = pcBitmap[((y*mWidth+x)<<2)+0];
				unsigned g = pcBitmap[((y*mWidth+x)<<2)+1];
				unsigned r = pcBitmap[((y*mWidth+x)<<2)+2];
				if (r+g+b < mscLineDarkColorThreshold)
				{
					int db = pcBitmap[((y*mWidth+x+1)<<2)+0] - (int)b;
					int dg = pcBitmap[((y*mWidth+x+1)<<2)+1] - (int)g;
					int dr = pcBitmap[((y*mWidth+x+1)<<2)+2] - (int)r;
					if (Abs(dr)+Abs(dg)+Abs(db) < (int)mscLineColorDiffThreshold)
					{
						if (lFoundLinePixelCount)
						{
							lFoundLine = true;
						}
					}
					else
					{
						++lFoundLinePixelCount;
					}
				}
				else if (lFoundLinePixelCount)
				{
					lFoundLine = true;
				}

				if (lFoundLine || y == mHeight-2)
				{
					if (lFoundLinePixelCount && !lFoundLeftLine)
					{
						lFoundLeftLine = true;
						mLeftLine = x;
					}
					if (lFoundLinePixelCount >= mscLineMinVertLengthThreshold &&
						lFoundLinePixelCount < mscLineMaxLengthThreshold)
					{
						mLines.push_back(Line(x+1, y-lFoundLinePixelCount, x+1, y-1));
					}
					lFoundLinePixelCount = 0;
					lFoundLine = false;
				}
			}
		}
	}
	else
	{
		// Pick the horizontal lines.
		for (y = 0; y < mHeight-1; ++y)
		{
			unsigned lFoundLinePixelCount = 0;
			bool lFoundLine = false;
			for (x = 0; x < mWidth-1; ++x)
			{
				unsigned b = pcBitmap[(y*mWidth+x)*mBPP+0];
				unsigned g = pcBitmap[(y*mWidth+x)*mBPP+1];
				unsigned r = pcBitmap[(y*mWidth+x)*mBPP+2];
				if (r+g+b < mscLineDarkColorThreshold)
				{
					int db = pcBitmap[((y+1)*mWidth+x)*mBPP+0] - (int)b;
					int dg = pcBitmap[((y+1)*mWidth+x)*mBPP+1] - (int)g;
					int dr = pcBitmap[((y+1)*mWidth+x)*mBPP+2] - (int)r;
					if (Abs(dr)+Abs(dg)+Abs(db) < (int)mscLineColorDiffThreshold)
					{
						if (lFoundLinePixelCount)
						{
							lFoundLine = true;
						}
					}
					else
					{
						++lFoundLinePixelCount;
					}
				}
				else if (lFoundLinePixelCount)
				{
					lFoundLine = true;
				}

				if (lFoundLine || x == mWidth-2)
				{
					if (lFoundLinePixelCount && !lFoundTopLine)
					{
						lFoundTopLine = true;
						mTopLine = y;
					}
					if (lFoundLinePixelCount >= mscLineHorizPickUpMinLengthThreshold &&
						lFoundLinePixelCount < mscLineMaxLengthThreshold)
					{
						mLines.push_back(Line(x-lFoundLinePixelCount, y+1, x-1, y+1));
					}
					lFoundLinePixelCount = 0;
					lFoundLine = false;
				}
			}
		}

		// Pick the vertical lines.
		for (x = 0; x < mWidth-1; ++x)
		{
			unsigned lFoundLinePixelCount = 0;
			bool lFoundLine = false;
			for (y = 0; y < mHeight-1; ++y)
			{
				unsigned b = pcBitmap[(y*mWidth+x)*mBPP+0];
				unsigned g = pcBitmap[(y*mWidth+x)*mBPP+1];
				unsigned r = pcBitmap[(y*mWidth+x)*mBPP+2];
				if (r+g+b < mscLineDarkColorThreshold)
				{
					int db = pcBitmap[(y*mWidth+x+1)*mBPP+0] - (int)b;
					int dg = pcBitmap[(y*mWidth+x+1)*mBPP+1] - (int)g;
					int dr = pcBitmap[(y*mWidth+x+1)*mBPP+2] - (int)r;
					if (Abs(dr)+Abs(dg)+Abs(db) < (int)mscLineColorDiffThreshold)
					{
						if (lFoundLinePixelCount)
						{
							lFoundLine = true;
						}
					}
					else
					{
						++lFoundLinePixelCount;
					}
				}
				else if (lFoundLinePixelCount)
				{
					lFoundLine = true;
				}

				if (lFoundLine || y == mHeight-2)
				{
					if (lFoundLinePixelCount && !lFoundLeftLine)
					{
						lFoundLeftLine = true;
						mLeftLine = x;
					}
					if (lFoundLinePixelCount >= mscLineMinVertLengthThreshold &&
						lFoundLinePixelCount < mscLineMaxLengthThreshold)
					{
						mLines.push_back(Line(x+1, y-lFoundLinePixelCount, x+1, y-1));
					}
					lFoundLinePixelCount = 0;
					lFoundLine = false;
				}
			}
		}
	}
}

void ImageAnalyzer::FixBrokenLines()
{
	for (std::list<Line>::iterator i = mLines.begin(); i != mLines.end(); )
	{
		Line& l(*i);
		std::list<Line>::iterator j = i;
		++j;
		bool bStep = true;
		if (j != mLines.end())
		{
			if (l.IsHorizontal())
			{
				const Line& k(*j);
				if (l.mY1 == k.mY1 && l.mX2 >= k.mX1-(int)mscLineCloseThreshold)
				{
					// Check that the line hole doesn't contain crossing lines.
					std::list<Line>::iterator m = j;
					bool lAttach = true;
					for (++m; m != mLines.end(); ++m)
					{
						const Line& n(*m);
						if (n.IsVertical() &&
							n.mX1 > l.mX2 && n.mX1 < k.mX1 &&	// In hole (X)?
							n.mY1 < k.mY1 && n.mY2 > k.mY1)	// Crossing hole (Y)?
						{
							lAttach = false;
							break;
						}
					}
					if (lAttach)
					{
						l.mX2 = k.mX2;
						mLines.erase(j);
						bStep = false;
					}
				}
			}
			else	// Comes here if vertical line.
			{
				const Line& k(*j);
				if (l.mX1 == k.mX1 && l.mY2 >= k.mY1-(int)mscLineCloseThreshold)
				{
					l.mY2 = k.mY2;
					mLines.erase(j);
					bStep = false;
				}
			}
		}
		// Now the line is attached again. Check that it doesn't
		// exceed our line length limitations.
		if (bStep)
		{
			if ((l.mX2 < l.mX1+(int)mscLineMinHorizLengthThreshold &&
				l.mY2 < l.mY1+(int)mscLineMinVertLengthThreshold) ||
				l.mX2 > l.mX1+(int)mscLineMaxLengthThreshold ||
				l.mY2 > l.mY1+(int)mscLineMaxLengthThreshold)
			{
				mLines.erase(i++);
			}
			else
			{
				++i;
			}
		}
	}
}

void ImageAnalyzer::MakeRectangles()
{
	mRectangles.clear();

	for (std::list<Line>::iterator i = mLines.begin(); i != mLines.end(); ++i)
	{
		Line l(*i);
		if (l.IsHorizontal())
		{
			// Find vertical lines that begin near this one.
			// Also OK with a close proximity to a "T-intersection".
			for (std::list<Line>::iterator j = mLines.begin(); j != mLines.end(); ++j)
			{
				if (i != j)
				{
					Line l2(*j);
					if (l2.IsVertical())
					{
						if ((unsigned)Abs(l2.mX1-l.mX1) <= mscLineCloseThreshold*5/2 &&
							(unsigned)Abs(l2.mY1-l.mY1) <= mscLineCloseThreshold*5/2)
						{
							// Found top-left corner.
							Rectangle r(l2.mX1, l.mY1, l.mX2+l.mX1-l2.mX1, l2.mY2+l2.mY1-l.mY1);
							r.AdjustSize(mWidth, mHeight);
							assert(r.mX1 >= 0);
							assert(r.mY1 >= 0);
							assert(r.mX2 < mWidth);
							assert(r.mY2 < mHeight);
							assert(r.mX2 > r.mX1 || r.mY2 > r.mY1);
							if (r.mX2 > r.mX1 && r.mY2 > r.mY1)
							{
								mRectangles.push_back(r);
							}
						}
						else if ((unsigned)Abs(l2.mY1-l.mY1) <= mscLineCloseThreshold*3 &&
								(l.mX1 <= l2.mX1-(int)mscLineCloseThreshold &&
								l.mX2 >= l2.mX1+(int)mscLineCloseThreshold))
						{
							// Found top-left corner from an intersecting line.
							Rectangle r(l2.mX1, l.mY1, l.mX2, l2.mY2);
							r.AdjustSize(mWidth, mHeight);
							assert(r.mX1 >= 0);
							assert(r.mY1 >= 0);
							assert(r.mX2 < mWidth);
							assert(r.mY2 < mHeight);
							assert(r.mX2 > r.mX1 || r.mY2 > r.mY1);
							if (r.mX2 > r.mX1 && r.mY2 > r.mY1)
							{
								mRectangles.push_back(r);
							}
						}
					}
				}
			}
			// Add rectangle if it touches window's left side.
			if (l.mX1 <= mLeftLine+(int)mscLineCloseThreshold)
			{
				// Found top-left corner.
				Rectangle r(0, l.mY1, l.mX2+l.mX1, l.mY2+mCardHeight);
				r.AdjustSize(mWidth, mHeight);
				assert(r.mX1 >= 0);
				assert(r.mY1 >= 0);
				assert(r.mX2 < mWidth);
				assert(r.mY2 < mHeight);
				assert(r.mX2 > r.mX1 || r.mY2 > r.mY1);
				if (r.mX2 > r.mX1 && r.mY2 > r.mY1)
				{
					mRectangles.push_back(r);
				}
			}
			// Add rectangle if it touches window's right side.
			/*if (l.mX2 >= mWidth-(int)mscLineCloseThreshold)
			{
				// Found top-left corner.
				Rectangle r(l.mX1, l.mY1, mWidth-1, l.mY2+mCardHeight);
				r.AdjustSize(mWidth, mHeight);
				assert(r.mX1 >= 0);
				assert(r.mY1 >= 0);
				assert(r.mX2 < mWidth);
				assert(r.mY2 < mHeight);
				assert(r.mX2 > r.mX1 || r.mY2 > r.mY1);
				if (r.mX2 > r.mX1 && r.mY2 > r.mY1)
				{
					mRectangles.push_back(r);
				}
			}*/
		}
		else	// Means vertical line.
		{
			// Add rectangle if it touches window's top side.
			if (l.mY1 <= mTopLine+(int)mscLineCloseThreshold)
			{
				// Found top-left corner.
				Rectangle r(l.mX1, 0, l.mX1+mCardWidth, l.mY2+l.mY1);
				r.AdjustSize(mWidth, mHeight);
				assert(r.mX1 >= 0);
				assert(r.mY1 >= 0);
				assert(r.mX2 < mWidth);
				assert(r.mY2 < mHeight);
				assert(r.mX2 > r.mX1 || r.mY2 > r.mY1);
				if (r.mX2 > r.mX1 && r.mY2 > r.mY1)
				{
					mRectangles.push_back(r);
				}
			}
		}
	}

	// Remove double rectangles.
	for (std::list<Rectangle>::iterator j = mRectangles.begin(); j != mRectangles.end(); ++j)
	{
		Rectangle* r = &(*j);
		std::list<Rectangle>::iterator k = j;
		++k;
		while (k != mRectangles.end())
		{
			const Rectangle& r2(*k);
			if ((unsigned)Abs(r->mX1-r2.mX1) < mscLineCloseThreshold &&	// Pretty close overlapping rectangles.
				(unsigned)Abs(r->mY1-r2.mY1) < mscLineCloseThreshold &&
				(unsigned)Abs(r->mY2-r2.mY2) < mscLineCloseThreshold*3)	// Only positive if these get blended.
			{
				r->mX1 = max(r->mX1, r2.mX1);
				r->mX2 = max(r->mX2, r2.mX2);
				r->mY1 = max(r->mY1, r2.mY1);
				r->mY2 = min(r->mY2, r2.mY2);
				mRectangles.erase(k++);
			}
			else if (r->mX1 == r2.mX1 &&	// Rectangles overlapping only in y.
				(unsigned)Abs(r->mY1-r2.mY1) < mscLineCloseThreshold*3 &&
				(unsigned)Abs(r->mY2-r2.mY2) < mscLineCloseThreshold*3)	// Only positive if these get blended.
			{
				r->mX2 = max(r->mX2, r2.mX2);
				r->mY1 = max(r->mY1, r2.mY1);
				r->mY2 = min(r->mY2, r2.mY2);
				mRectangles.erase(k++);
			}
			else
			{
				++k;
			}
		}
	}

	// Remove too small rectangles.
	for (std::list<Rectangle>::iterator j = mRectangles.begin(); j != mRectangles.end(); )
	{
		const Rectangle& r(*j);
		if (r.mX2-r.mX1 <= (int)mscLineCloseThreshold*2)
		{
			mRectangles.erase(j++);
		}
		else if (r.mY2-r.mY1 <= (int)mscLineCloseThreshold*6)
		{
			mRectangles.erase(j++);
		}
		else
		{
			++j;
		}
	}

	// Drop rectangles outside of window.
	for (std::list<Rectangle>::iterator j = mRectangles.begin(); j != mRectangles.end();)
	{
		const Rectangle& r(*j);
		if (r.mX1 < 0 ||
		    r.mX1+mCardOverlap >= mWidth ||
		    r.mY1 < 0 ||
		    r.mY1+mCardHeight >= mHeight)
		{
			mRectangles.erase(j++);
		}
		else
		{
			++j;
		}
	}
}

void ImageAnalyzer::SortRectangles()
{
	// Sort all rectangles right-to-left.
	for (std::list<Rectangle>::iterator k = mRectangles.begin(); k != mRectangles.end(); ++k)
	{
		std::list<Rectangle>::iterator l = k;
		for (++l; l != mRectangles.end();)
		{
			bool lMove = false;
			std::list<Rectangle>::iterator m = k;
			bool b = true;
			while (b)
			{
				if (m->mX1 > l->mX1)
				{
					lMove = true;
				}
				else
				{
					++m;
					break;
				}
				if (m == mRectangles.begin())
				{
					break;
				}
				--m;
			}
			if (lMove)
			{
				Rectangle r(*l);
				mRectangles.erase(l++);
				mRectangles.insert(m, r);
			}
			else
			{
				++l;
			}
		}
	}

#ifdef _DEBUG
	for (std::list<Rectangle>::iterator m = mRectangles.begin(); m != mRectangles.end(); ++m)
	{
		std::list<Rectangle>::iterator n = m;
		++n;
		if (n != mRectangles.end())
		{
			const Rectangle& r1(*m);
			const Rectangle& r2(*n);
			assert (r1.mX1 <= r2.mX1);
			/*if (r1.mX1 == r2.mX1)
			{
				assert(r1.mY1 <= r2.mY1);
			}*/
		}
	}
#endif // _DEBUG

	/*
	Already done?
	// Check if rectangle has "inner rectangles"; if so: drop 'em!!!
	for (std::list<Rectangle>::iterator j = mRectangles.begin(); j != mRectangles.end(); )
	{
		std::list<Rectangle>::iterator k = j;
		++k;
		if (k != mRectangles.end())
		{
			const Rectangle& r1(*j);
			const Rectangle& r2(*k);
			if (r2.mX1 <= r1.mX1+3*mCardOverlap/4 &&
				r2.mY1 >= r1.mY1 &&
				r2.mY1 < r1.mY2)
			{
				mRectangles.erase(k);
			}
			else
			{
				++j;
			}
		}
		else
		{
			++j;
		}
	}
	*/

	// Sort the rectangles in distance order. They
	// are already sorted right-to-left.
	for (std::list<Rectangle>::iterator i = mRectangles.begin(); i != mRectangles.end();)
	{
		std::list<Rectangle>::iterator j = i;
		std::list<Rectangle>::iterator lClosest;
		int lClosestDistance = mCardWidth;
		bool lFoundCloseRect = false;
		for (++j; j != mRectangles.end(); ++j)
		{
			//if (i->mX1 < j->mX1)
			{
				int lDistance = Abs(i->mX1 - j->mX1) + Abs(i->mY1 - j->mY1);
				if (lDistance <= lClosestDistance)
				{
					lFoundCloseRect = true;
					lClosestDistance = lDistance;
					lClosest = j;
				}
			}
		}
		++i;
		if (lFoundCloseRect && i != lClosest)
		{
			Rectangle r(*lClosest);
			mRectangles.erase(lClosest);
			mRectangles.insert(i, r);
			--i;
		}
	}
}

void ImageAnalyzer::CalculateSizes()
{
	mCardWidth = -1;
	mCardHeight = -1;
	mCardOverlap = -1;
	mCardCount = 0;
	// Calculate card width and height.
	for (std::list<Rectangle>::iterator i = mRectangles.begin(); i != mRectangles.end(); )
	{
		const Rectangle& r(*i);
		double lCardAspect = (r.mX2-r.mX1)*1.35 / (r.mY2-r.mY1);
		if (lCardAspect > 0.8 && lCardAspect < 1.25)
		{
			++mCardCount;
			mCardWidth += r.mX2-r.mX1;
			mCardHeight += r.mY2-r.mY1;
			++i;
		}
		else if (lCardAspect > 0.2 && lCardAspect < 3)
		{
			++i;
		}
		else
		{
			mRectangles.erase(i++);
			//++i;
		}
	}
	if (mCardCount)
	{
		mCardWidth /= mCardCount;
		mCardHeight /= mCardCount;

		// Calculate card overlap.
		int lOverlapCount = 0;
		for (std::list<Rectangle>::iterator j = mRectangles.begin(); j != mRectangles.end(); ++j)
		{
			const Rectangle& r(*j);
			std::list<Rectangle>::iterator k = j;
			++k;
			if (k != mRectangles.end())
			{
				const Rectangle& r2(*k);
				if ((unsigned)Abs(r2.mY1-r.mY1) < mscLineCloseThreshold*5)
				{
					if (r2.mX1-r.mX1 > (int)mscLineCloseThreshold*2 && r2.mX1-r.mX1 < mCardWidth/2)
					{
						mCardOverlap += r2.mX1-r.mX1;
						++lOverlapCount;
					}
				}
			}
		}
		if (lOverlapCount)
		{
			mCardOverlap /= lOverlapCount;
		}
	}
}

void ImageAnalyzer::SortOutCardRectangles(const unsigned char* pcBitmap)
{
	mCardSlices.clear();

	// Check size and that it has white contents.
	for (std::list<Rectangle>::iterator i = mRectangles.begin(); i != mRectangles.end();)
	{
		bool lDropCard = true;
		Rectangle* r = &(*i);
		if (r->mY2-r->mY1 <= (int)mscLineMinVertLengthThreshold*2)
		{
			lDropCard = true;
		}
		else if ((unsigned)Abs(r->mY2-r->mY1-mCardHeight) <= mscLineCloseThreshold*3 ||
			(unsigned)Abs(r->mX2-r->mX1-mCardWidth) <= mscLineCloseThreshold*2)
		{
			if (r->mX1+mCardOverlap < mWidth && r->mY1+mCardHeight < mHeight)
			{
				// If the top of the card is on a black line then
				// move the rectangle down one pixel.
				if (pcBitmap[(r->mY1*mWidth+r->mX1+mCardOverlap/2)*mBPP+0] < 100)
				{
					++r->mY1;
				}

				// Check brightness.
				unsigned lAverageRed = 0;
				unsigned lAverageGreen = 0;
				unsigned lAverageBlue = 0;
				for (int y = r->mY1; y < r->mY1+mCardHeight; ++y)
				{
					for (int x = r->mX1; x < r->mX1+mCardOverlap; ++x)
					{
						lAverageBlue += pcBitmap[(y*mWidth+x)*mBPP+0];
						lAverageGreen += pcBitmap[(y*mWidth+x)*mBPP+1];
						lAverageRed += pcBitmap[(y*mWidth+x)*mBPP+2];
					}
				}
				if (lAverageRed/(mCardOverlap*mCardHeight) > 150 &&
					lAverageGreen/(mCardOverlap*mCardHeight) > 145 &&
					lAverageBlue/(mCardOverlap*mCardHeight) > 145)
				{
					// Yup, mostly white.
					lDropCard = false;
					if (lAverageRed*2 > lAverageGreen+lAverageBlue+10*mCardOverlap*mCardHeight)
					{
						CardSlice lSlice(true, *r);
						mCardSlices.push_back(lSlice);
					}
					else
					{
						CardSlice lSlice(false, *r);
						mCardSlices.push_back(lSlice);
					}
				}
				else
				{
					// Nope, not a white card.
					lDropCard = true;
				}
			}
			else
			{
				// Card not completely within window.
				lDropCard = true;
			}
		}
		else
		{
			// Not same size as the other cards.
			lDropCard = true;
		}

		if (lDropCard)
		{
			mRectangles.erase(i++);
		}
		else
		{
			++i;
		}
	}

	/*
	for (std::list<Rectangle>::iterator jonas = mRectangles.begin(); jonas != mRectangles.end(); ++jonas)
	{
		char a[100];
		::sprintf(a, "(%i,%i), ", jonas->mX1, jonas->mY1);
		::strcat(mDebugText, a);
	}
	*/
}

void ImageAnalyzer::DetectCards(const unsigned char* pcBitmap)
{
	for (std::list<CardSlice>::iterator i = mCardSlices.begin(); i != mCardSlices.end();)
	{
		const Rectangle& r(i->mRectangle);
		// Start at bottom, work the way up to the type symbol, then
		// move further up to the value glyph(s).
		int lStep = r.mY1+mCardHeight*3/4;
		// Check for color.
		if (i->mIsRed)
		{
			// Found hearts or diamonds.
			i->mCardId.mType = DetectHeartsOrDiamonds(pcBitmap, r.mX1+mCardOverlap/2, lStep, mCardOverlap*9/20);
			if (i->mCardId.mType == HEARTS)
			{
				for (int y = r.mY1+mCardHeight/2; y < r.mY1+mCardHeight; ++y)
				{
					for (int x = r.mX1; x < r.mX1+mCardOverlap; ++x)
					{
						mDebugBitmap[(y*mWidth+x)*mBPP+0] = 0;
						mDebugBitmap[(y*mWidth+x)*mBPP+1] = 0;
						mDebugBitmap[(y*mWidth+x)*mBPP+2] = 255;
					}
				}
			}
			else if (i->mCardId.mType == DIAMONDS)
			{
				for (int y = r.mY1+mCardHeight/2; y < r.mY1+mCardHeight; ++y)
				{
					for (int x = r.mX1; x < r.mX1+mCardOverlap; ++x)
					{
						mDebugBitmap[(y*mWidth+x)*mBPP+0] = 150;
						mDebugBitmap[(y*mWidth+x)*mBPP+1] = 150;
						mDebugBitmap[(y*mWidth+x)*mBPP+2] = 255;
					}
				}
			}
		}
		else
		{
			// Found clubs or spades.
			i->mCardId.mType = DetectClubsOrSpades(pcBitmap, r.mX1+mCardOverlap/2, lStep, mCardOverlap*9/20);
			if (i->mCardId.mType == SPADES)
			{
				for (int y = r.mY1+mCardHeight/2; y < r.mY1+mCardHeight; ++y)
				{
					for (int x = r.mX1; x < r.mX1+mCardOverlap; ++x)
					{
						mDebugBitmap[(y*mWidth+x)*mBPP+0] = 0;
						mDebugBitmap[(y*mWidth+x)*mBPP+1] = 0;
						mDebugBitmap[(y*mWidth+x)*mBPP+2] = 0;
					}
				}
			}
			else if (i->mCardId.mType == CLUBS)
			{
				for (int y = r.mY1+mCardHeight/2; y < r.mY1+mCardHeight; ++y)
				{
					for (int x = r.mX1; x < r.mX1+mCardOverlap; ++x)
					{
						mDebugBitmap[(y*mWidth+x)*mBPP+0] = 50;
						mDebugBitmap[(y*mWidth+x)*mBPP+1] = 50;
						mDebugBitmap[(y*mWidth+x)*mBPP+2] = 50;
					}
				}
			}
		}

		// Check for value (if color detected).
		if (i->mCardId.mType != CARD_TYPE_INVALID && lStep-r.mY1 > (int)mscLineCloseThreshold*3)
		{
			// Check if we can make out a local overlap size
			// to improve the accuracy (glyph-compare wants it).
			int lOverlap = mCardOverlap;
			std::list<CardSlice>::iterator j = i;
			++j;
			if (j != mCardSlices.end())
			{
				const Rectangle& lNextCard(j->mRectangle);
				if (lNextCard.mX1-r.mX1 < mCardOverlap*4/3 && (int)Abs(lNextCard.mY1-r.mY1) < mCardWidth/4)
				{
					if (lNextCard.mX1-r.mX1 > mCardOverlap*3/4)
					{
						lOverlap = lNextCard.mX1-r.mX1;
					}
				}
			}

			// Glyph-compare.
			i->mCardId.mValue = DetectCardValue(pcBitmap, r.mX1, r.mX1+lOverlap-1, r.mY1+2, lStep);
			assert(i->mCardId.mValue >= CARD_2 && i->mCardId.mValue <= CARD_A);
			++i;
		}
		else
		{
			char a[100];
			sprintf(a, "ERROR: unknown card type on coordinate (%i, %i).\r\n", (i->mRectangle.mX1+i->mRectangle.mX2)/2, (i->mRectangle.mY1+i->mRectangle.mY2)/2);
			strcat(mDebugText, a);
			mDumpScreenshot = true;
			std::list<CardSlice>::iterator j = i;
			++i;
			mCardSlices.erase(j);
		}
	}
}

void ImageAnalyzer::SortHands(Table& pTable)
{
	pTable.DeleteAllHands();

	Hand* lHand = 0;
	Rectangle* lPreviousRect = 0;
	Card lPreviousCard(CARD_TYPE_INVALID, CARD_VALUE_INVALID);
	for (std::list<CardSlice>::iterator i = mCardSlices.begin(); i != mCardSlices.end(); ++i)
	{
		const Rectangle& r(i->mRectangle);
		if (lHand)
		{
			if (Abs(r.mX1-lPreviousRect->mX1) < 12 && Abs(r.mY1-lPreviousRect->mY1) < 12)	// Very close to each other.
			{
				Card lCard = i->mCardId;
				char a[100];
				sprintf(a, "ERROR: two cards close to each other found: "
					"%s and %s on coordinates (%i, %i) and (%i, %i).\r\n",
					lPreviousCard.GetValueName(), lCard.GetValueName(), r.mX1, r.mY1, lPreviousRect->mX1, lPreviousRect->mY1);
				strcat(mDebugText, a);
				mDumpScreenshot = true;
				/*DumpScreenshot(mWidth, mHeight, mBPP, mBitmap);
				if (lCard != lPreviousCard)
				{
					assert(false);	// Make sure we interpreted them the same. TODO: pick the best choice!
				}
				assert(lHand->IsCardInHand(lCard));*/
			}
			else if ((int)(Abs(r.mX1-lPreviousRect->mX1) + Abs(r.mY1-lPreviousRect->mY1)) < mCardWidth)
			{
				// Add a new card.
				Card lCard = i->mCardId;
				if (lHand->IsCardInHand(lCard))
				{
					char a[100];
					sprintf(a, "ERROR: cards already in hand: "
						"%s on coordinate (%i, %i).\r\n",
						lCard.GetValueName(), r.mX1, r.mY1);
					strcat(mDebugText, a);
					DumpScreenshot(mWidth, mHeight, mBPP, mBitmap);
				}
				lHand->AddCard(i->mCardId);
			}
			else
			{
				// Add a new card to a new hand.
				pTable.AddHand(lHand);
				lHand = new Hand(i->mRectangle.mX1, i->mRectangle.mY1);
				lHand->AddCard(i->mCardId);
			}
		}
		else
		{
			// First card to the first hand.
			lHand = new Hand(i->mRectangle.mX1, i->mRectangle.mY1);
			lHand->AddCard(i->mCardId);
		}
		lPreviousRect = &(i->mRectangle);
		lPreviousCard = i->mCardId;
	}
	// Add last hand.
	if (lHand)
	{
		pTable.AddHand(lHand);
	}
}

CARD_TYPE ImageAnalyzer::DetectHeartsOrDiamonds(const unsigned char* pcBitmap, int pMid, int& pBottom, int pEdge) const
{
	// Assume that the card type mark is placed in the middle of the
	// overlap. Step up to the mark, go through it and check if it is
	// wider than just a few pixels on the top. If it is we found
	// ourselves a heart, otherwise a diamond.
	CARD_TYPE lCard = CARD_TYPE_INVALID;
	bool lFoundColor = false;
	int& y = pBottom;
	int lStop = pBottom-mCardHeight;
	if (lStop < 0)
	{
		lStop = 0;
	}
	for (; y > lStop; --y)
	{
		// Check for non-white (=red) colors. Low on blue for instance.
		if (pcBitmap[(y*mWidth+pMid)*mBPP+0] < mscRedCardBrightColorThreshold)
		{
			lFoundColor = true;
			mDebugBitmap[(y*mWidth+pMid)*mBPP+0] = 0;
			mDebugBitmap[(y*mWidth+pMid)*mBPP+1] = 255;
			mDebugBitmap[(y*mWidth+pMid)*mBPP+2] = 0;
		}
		else if (lFoundColor)
		{
			break;
		}
	}
	// Check if it's wide on the top.
	for (int x = pMid-pEdge; x < pMid-3; ++x)
	{
		mDebugBitmap[(y*mWidth+x)*mBPP+0] = 255;
		mDebugBitmap[(y*mWidth+x)*mBPP+1] = 80;
		mDebugBitmap[(y*mWidth+x)*mBPP+2] = 0;
		if (pcBitmap[(y*mWidth+x)*mBPP+0] < mscRedCardBrightColorThreshold)
		{
			lCard = HEARTS;
			// Step up the hearts value bottom a few pixels.
			y -= 3;
			break;
		}
	}
	if (lCard != HEARTS)
	{
		lCard = DIAMONDS;
	}
	return (lCard);
}

CARD_TYPE ImageAnalyzer::DetectClubsOrSpades(const unsigned char* pcBitmap, int pMid, int& pBottom, int pEdge) const
{
	// Assume that the card type mark is placed in the middle of the
	// overlap. Step up to and into the mark. Check if it has a
	// "buldge" on the side. If it is we found ourselves a club,
	// otherwise a spade.
	CARD_TYPE lCard = CARD_TYPE_INVALID;
	int& y = pBottom;
	int lStop = pBottom-mCardHeight;
	if (lStop < 0)
	{
		lStop = 0;
	}
	unsigned lStepCount = 0;
	for (; y > lStop; --y)
	{
		// Check for non-white (=black) colors. Low on blue for instance.
		if (pcBitmap[(y*mWidth+pMid)*mBPP+0] < mscBlackCardBrightColorThreshold)
		{
			++lStepCount;
		}
		if (lStepCount > mscLineCloseThreshold*2)
		{
			break;
		}
	}
	int lMinStepX = 0;
	for (; y > lStop; --y)
	{
		// Check for a "buldge", which then would indicate clubs.
		bool lFoundColor = false;
		int lStepCount = 0;
		int x;
		for (x = pMid-pEdge; x < pMid-1; ++x)
		{
			++lStepCount;
			mDebugBitmap[(y*mWidth+x)*mBPP+0] = 0;
			mDebugBitmap[(y*mWidth+x)*mBPP+1] = 255;
			mDebugBitmap[(y*mWidth+x)*mBPP+2] = 0;
			if (pcBitmap[(y*mWidth+x)*mBPP+0] < mscBlackCardBrightColorThreshold)
			{
				lFoundColor = true;
				break;
			}
		}
		if (!lFoundColor)
		{
			lCard = SPADES;
			break;
		}
		if (lStepCount >= lMinStepX)
		{
			lMinStepX = lStepCount;
		}
		else
		{
			lCard = CLUBS;
			break;
		}
	}
	// Finish the Y stepping so the value's bottom won't strech
	// out until here (probably at the middle of type symbol).
	for (; y > lStop; --y)
	{
		// Check for non-white (=black) colors. Low on blue for instance.
		if (pcBitmap[(y*mWidth+pMid)*mBPP+0] > mscBlackCardBrightColorThreshold)
		{
			break;
		}
	}

	/* Don't! // Just try to save us if something goes wrong.
	if (lCard != CLUBS && lCard != SPADES)
	{
		lCard = SPADES;
	}*/

	return (lCard);
}

CARD_VALUE ImageAnalyzer::DetectCardValue(const unsigned char* pcBitmap, int pLeft, int pRight, int pTop, int pBottom)
{
	if (pTop >= pBottom || pLeft >= pRight)
	{
		char a[100];
		sprintf(a, "ERROR: card value OLD area small: (%i, %i).\r\n", pRight-pLeft, pBottom-pTop);
		strcat(mDebugText, a);
		DumpScreenshot(mWidth, mHeight, mBPP, mBitmap);
	}
	assert(pTop < pBottom);
	assert(pLeft < pRight);

	int lLeft = pRight;
	int lRight = pLeft;
	int lTop = pBottom;
	int lBottom = pTop;
	bool lFoundData = false;
	for (int y = pBottom-1; y >= pTop; --y)
	{
		int lFoundPixels = 0;
		for (int x = pLeft; x < pRight; ++x)
		{
			//mDebugBitmap[(y*mWidth+x)*mBPP+0] = 0xFF-mDebugBitmap[(y*mWidth+x)*mBPP+0];
			//mDebugBitmap[(y*mWidth+x)*mBPP+1] = 0xFF-mDebugBitmap[(y*mWidth+x)*mBPP+1];
			//mDebugBitmap[(y*mWidth+x)*mBPP+2] = 0xFF-mDebugBitmap[(y*mWidth+x)*mBPP+2];
			if (pcBitmap[(y*mWidth+x)*mBPP+0] < mscCardValueBrightColorThreshold)
			{
				++lFoundPixels;
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
		if (lFoundPixels <= 1)
		{
			if (lFoundData)
			{
				break;
			}
		}
		else
		{
			lFoundData = true;

			lTop = y;
			if (y > lBottom)
			{
				lBottom = y;
			}
		}
	}
	if (lTop >= lBottom || lLeft >= lRight)
	{
		char a[100];
		sprintf(a, "ERROR: card value NEW area small: (%i, %i).\r\n", pRight-pLeft, pBottom-pTop);
		strcat(mDebugText, a);
		DumpScreenshot(mWidth, mHeight, mBPP, mBitmap);
	}
	assert(lTop < lBottom);
	assert(lLeft < lRight);
	/*
	// Enlarge some to compensate for strict regulations!
	--lLeft;
	++lRight;
	--lTop;
	++lBottom;
	*/

	/*for (int i = lTop; i <= lBottom; ++i)
	{
		for (int j = lLeft; j < lRight; ++j)
		{
			mDebugBitmap[(i*mWidth+j)*mBPP+0] = 0xFF-mDebugBitmap[(i*mWidth+j)*mBPP+0];
			mDebugBitmap[(i*mWidth+j)*mBPP+1] = 0xFF-mDebugBitmap[(i*mWidth+j)*mBPP+1];
			mDebugBitmap[(i*mWidth+j)*mBPP+2] = 0xFF-mDebugBitmap[(i*mWidth+j)*mBPP+2];
		}
	}*/

	// Walk through the glyphs and check which one is the
	// most likely.
#define TEN_VISIBLE	0.9
	unsigned				lClosestApprox = 0xFFFFFFFF;
	CARD_VALUE				lBestCard = CARD_VALUE_INVALID;
	std::list<Bitmap*>::const_iterator	lBestIter;
	CARD_VALUE				lCardValue = CARD_2;
	for (std::list<Bitmap*>::const_iterator i = mCardGlyphs.begin(); i != mCardGlyphs.end(); ++i, ++(int&)lCardValue)
	{
		unsigned lApprox = 0;
		double lScaleX = (double)((*i)->GetWidth()-1)/(lRight-lLeft);
		// TODO: remove special case for PartyPoker: the number 10
		// is always partly hidden on the right side of the overlap.
		if (lCardValue == CARD_10)
		{
			lScaleX = (double)((*i)->GetWidth()*TEN_VISIBLE-1)/(lRight-lLeft);
		}
		double lScaleY = (double)((*i)->GetHeight()-1)/(lBottom-lTop);
		const unsigned char* lcGlyphBits = (*i)->GetBits();
		for (int v = lTop; v <= lBottom; ++v)
		{
			int y = (int)((v-lTop)*lScaleY);
			for (int u = lLeft; u <= lRight; ++u)
			{
				int x = (int)((u-lLeft)*lScaleX);
				unsigned lImage1;
				lImage1 = pcBitmap[((v+0)*mWidth+u+0)*mBPP+0]>>1;
				unsigned lImage2;
				lImage2 = pcBitmap[((v-1)*mWidth+u+0)*mBPP+0];
				lImage2 += pcBitmap[((v+1)*mWidth+u+0)*mBPP+0];
				lImage2 += pcBitmap[((v+0)*mWidth+u+1)*mBPP+0];
				lImage2 += pcBitmap[((v+0)*mWidth+u-1)*mBPP+0];
				lImage2 >>= 3;
				int lImage = lImage1+lImage2;
				//int lImage = pcBitmap[((v+0)*mWidth+u+0)*mBPP+0];
				int lGlyph = lcGlyphBits[y*(*i)->GetWidth()+x];
				int lDiff = lImage-lGlyph;
				lApprox += Abs(lDiff*lDiff*lDiff);
			}
		}
		/*if (lCardValue == 9-2)
		{
		}
		else*/
		//TRACE2("%i: %u\r\n", lCardValue+2, lApprox);
		assert(lApprox > 0);
		if (lApprox < lClosestApprox)
		{
			lClosestApprox = lApprox;
			lBestCard = lCardValue;
			lBestIter = i;
		}
		else if (lApprox == lClosestApprox)
		{
			char a[100];
			sprintf(a, "ERROR: glyph interpretation very similar: "
				"value %i and value %i on coordinate (%i, %i).\r\n",
				lCardValue+2, lBestCard+2, (lLeft+lRight)/2, (lTop+lBottom)/2);
			strcat(mDebugText, a);
			mDumpScreenshot = true;
		}
	}
	ASSERT(lBestCard >= 0);
	std::list<Bitmap*>::const_iterator k = lBestIter;
	double lScaleX = (double)((*k)->GetWidth()-1)/(lRight-lLeft);
	// TODO: remove special case for PartyPoker: the number 10
	// is always partly hidden on the right side of the overlap.
	if (lBestCard == CARD_10)
	{
		lScaleX = (double)((*k)->GetWidth()*TEN_VISIBLE-1)/(lRight-lLeft);
	}
	double lScaleY = (double)((*k)->GetHeight()-1)/(lBottom-lTop);
	const unsigned char* lcGlyphBits = (*k)->GetBits();
	for (int t = lTop; t <= lBottom; ++t)
	{
		int y = (int)((t-lTop)*lScaleY);
		for (int s = lLeft; s <= lRight; ++s)
		{
			int x = (int)((s-lLeft)*lScaleX);
			unsigned char lColor = lcGlyphBits[y*(*k)->GetWidth()+x];
			//mDebugBitmap[(t*mWidth+s)*mBPP+0] = 255-Abs(lColor-mDebugBitmap[(t*mWidth+s)*mBPP+0]);
			//mDebugBitmap[(t*mWidth+s)*mBPP+1] = 255-Abs(lColor-mDebugBitmap[(t*mWidth+s)*mBPP+1]);
			//mDebugBitmap[(t*mWidth+s)*mBPP+2] = 255-Abs(lColor-mDebugBitmap[(t*mWidth+s)*mBPP+2]);
			mDebugBitmap[(t*mWidth+s)*mBPP+0] ^= 255-lColor;
			mDebugBitmap[(t*mWidth+s)*mBPP+1] ^= 255-lColor;
			mDebugBitmap[(t*mWidth+s)*mBPP+2] ^= 255-lColor;
		}
	}
	/*for (int t = lTop; t < lTop+(*k)->GetHeight(); ++t)
	{
		int y = t-lTop;
		for (int s = lLeft; s < lLeft+(*k)->GetWidth(); ++s)
		{
			int x = s-lLeft;
			unsigned char lColor = (*k)->GetBits()[y*(*k)->GetWidth()+x];
			mDebugBitmap[(t*mWidth+s)*mBPP+0] = lColor;
			mDebugBitmap[(t*mWidth+s)*mBPP+1] = lColor;
			mDebugBitmap[(t*mWidth+s)*mBPP+2] = lColor;
		}
	}*/

	return ((CARD_VALUE)lBestCard);
}


void ImageAnalyzer::DebugDraw()
{
	for (std::list<Line>::iterator i = mLines.begin(); i != mLines.end(); ++i)
	{
		Line l(*i);
		if (l.IsHorizontal())
		{
			for (int j = l.mX1; j < l.mX2; ++j)
			{
				SetDebugBitmapPixel(j, l.mY1, 0, 0, 255);
			}
		}
		else
		{
			unsigned char r = 0;
			unsigned char g = 128;
			unsigned char b = 200;
			if (l.mY1 < (int)mscLineCloseThreshold)
			{
				r = 255;
				g = 0;
			}
			for (int j = l.mY1; j < l.mY2; ++j)
			{
				SetDebugBitmapPixel(l.mX1, j, r, g, b);
			}
		}
	}

	for (std::list<Rectangle>::iterator j = mRectangles.begin(); j != mRectangles.end(); ++j)
	{
		const Rectangle& r(*j);
		int k;
		// Top + bottom.
		//for (k = r.mX1; k < r.mX1+mCardOverlap; ++k)
		for (k = r.mX1; k < r.mX2; ++k)
		{
			SetDebugBitmapPixel(k, r.mY1, 0, 255, 0);
			//SetDebugBitmapPixel(k, r.mY1+mCardHeight, 255, 0, 0);
			//SetDebugBitmapPixel(k, r.mY2, 255, 0, 0);
		}
		// Left + right.
		//for (k = r.mY1; k < r.mY1+mCardHeight; ++k)
		for (k = r.mY1; k < r.mY2; ++k)
		{
			SetDebugBitmapPixel(r.mX1, k, 0, 255, 0);
			//SetDebugBitmapPixel(r.mX1+mCardOverlap, k, 255, 255, 0);
			//SetDebugBitmapPixel(r.mX2, k, 255, 255, 0);
		}
	}
}


const unsigned	ImageAnalyzer::mscCardValueBrightColorThreshold = 160;
const unsigned	ImageAnalyzer::mscRedCardBrightColorThreshold = 190;
const unsigned	ImageAnalyzer::mscBlackCardBrightColorThreshold = 190;
const unsigned	ImageAnalyzer::mscLineDarkColorThreshold = 15;
const unsigned	ImageAnalyzer::mscLineColorDiffThreshold = 400;
const unsigned	ImageAnalyzer::mscLineCloseThreshold = 4;
const unsigned	ImageAnalyzer::mscLineHorizPickUpMinLengthThreshold = 3;
const unsigned	ImageAnalyzer::mscLineMinHorizLengthThreshold = 10;
const unsigned	ImageAnalyzer::mscLineMinVertLengthThreshold = 20;
const unsigned	ImageAnalyzer::mscLineMaxLengthThreshold = 200;


};
