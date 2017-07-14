
// Author: Jonas Byström, highfestiva@gmail.com. August 2003.
// Open source without neither limitations nor warranty.


#include "SysInclude.h"
#include "HfPokerBitmap.h"


namespace HfPoker
{


Bitmap::Bitmap(int pWidth, int pHeight, int pBPP, const unsigned char* pcBits):
	mWidth(pWidth),
	mHeight(pHeight),
	mBPP(pBPP),
	mBits(pcBits)
{
}

Bitmap::~Bitmap()
{
	delete[] (mBits);
}


int Bitmap::GetWidth() const
{
	return (mWidth);
}

int Bitmap::GetHeight() const
{
	return (mHeight);
}

int Bitmap::GetBPP() const
{
	return (mBPP);
}

const unsigned char* Bitmap::GetBits() const
{
	return (mBits);
}

void Bitmap::SetBits(const unsigned char* pcBits)
{
	mBits = pcBits;
}


};
