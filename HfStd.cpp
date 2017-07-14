
// Author: Jonas Byström, highfestiva@gmail.com. August 2003.
// Open source without neither limitations nor warranty.


#include "SysInclude.h"
#include <fstream>
#include "HfStd.h"


void InitRandom()
{
	srand((unsigned)time(0));
}

double GetRandomDouble()
{
	return (rand()/(double)RAND_MAX);
}


int CountSetBits(int pBits)
{
	int lBitCount = 0;
	for (int i = 0; i < sizeof(int)*8; ++i)
	{
		int lBitMask = 1<<i;
		if (lBitMask&pBits)
		{
			++lBitCount;
			pBits &= ~lBitMask;
			if (!pBits)
			{
				break;
			}
		}
	}
	return (lBitCount);
}

int GetSetBit(int pBits, int pBitIndex)
{
	for (int i = pBitIndex; i < sizeof(int)*8; ++i)
	{
		if ((1<<i)&pBits)
		{
			return (i);
		}
	}
	assert(false);
	return (-1);
}


unsigned Faculty(unsigned n)
{
	unsigned lFac = 1;
	for (unsigned u = 2; u <= n; ++u)
	{
		lFac *= u;
	}
	return (lFac);
}

unsigned n_Choose_k(unsigned n, unsigned k)
{
	assert(n >= k);
	unsigned lResult;
	unsigned lFac1 = ::Faculty(n);
	unsigned lFac2 = ::Faculty(k) * ::Faculty(n-k);
	if (lFac2 <= lFac1)
	{
		lResult = lFac1/lFac2;
	}
	else
	{
		lResult = 0;
	}
	return (lResult);
}


double Lerp(double d1, double d2, double l)
{
	return ((1-l)*d1 + l*d2);
}


void StringNCopy(char* s1, const char* s2, unsigned n)
{
	::strncpy(s1, s2, n);
	s1[n] = '\0';
}


void DumpScreenshot(int pWidth, int pHeight, int pBPP, const unsigned char* pBitmap)
{
	const unsigned lStoreByteWidth = (pWidth*3+3)&(~3);
	const unsigned lStoreByteCount = lStoreByteWidth*pHeight;
	BITMAPFILEHEADER lFileHeader;
	::memset(&lFileHeader, 0, sizeof(lFileHeader));
	lFileHeader.bfType = ('M'<<8)|'B';
	lFileHeader.bfSize = sizeof(lFileHeader)+sizeof(BITMAPINFOHEADER)+lStoreByteCount;
	lFileHeader.bfOffBits = sizeof(lFileHeader)+sizeof(BITMAPINFOHEADER);
	BITMAPINFOHEADER lHeader;
	lHeader.biSize = sizeof(lHeader);
	lHeader.biWidth = pWidth;
	lHeader.biHeight = -pHeight;
	lHeader.biPlanes = 1;
	lHeader.biBitCount = 3*8;
	lHeader.biCompression = BI_RGB;
	lHeader.biSizeImage = 0;
	lHeader.biXPelsPerMeter = 0;
	lHeader.biYPelsPerMeter = 0;
	lHeader.biClrUsed = 0;
	lHeader.biClrImportant = 0;
	static unsigned lLastFileNumber = 0;
	for (lLastFileNumber; lLastFileNumber < 500; ++lLastFileNumber)
	{
		char lFilename[100];
		::sprintf(lFilename, "7 card stud %3.3u.bmp", lLastFileNumber);
		std::fstream lFile;
		lFile.open(lFilename, std::ios_base::in);
		if (!lFile.is_open())
		{
			lFile.close();
			lFile.clear();
			lFile.open(lFilename, std::ios_base::app|std::ios_base::binary|std::ios_base::out);
			if (lFile.is_open())
			{
				lFile.write((const char*)&lFileHeader, sizeof(lFileHeader));
				lFile.write((const char*)&lHeader, sizeof(lHeader));
				unsigned char* lNewBitmap = 0;
				const char* lBitmap = (const char*)pBitmap;
				if (pBPP == 4)	// Use 3-byte instead of 4-byte pixels.
				{
					lNewBitmap = new unsigned char[lStoreByteCount];
					lBitmap = (const char*)lNewBitmap;
					for (int y = 0; y < pHeight; ++y)
					{
						int yp1 = y*lStoreByteWidth;
						int yp2 = y*pWidth*4;
						for (int x = 0; x < pWidth; ++x)
						{
							lNewBitmap[yp1+x*3+0] = pBitmap[yp2+x*4+0];
							lNewBitmap[yp1+x*3+1] = pBitmap[yp2+x*4+1];
							lNewBitmap[yp1+x*3+2] = pBitmap[yp2+x*4+2];
						}
					}
				}
				else
				{
					// TODO: handle writing 3-byte bitmaps as well. Remember to pad bytes
					// on a scanline to a LONG (4 byte) boundary.
					assert(false);
				}
				lFile.write(lBitmap, lStoreByteCount);
				lFile.close();
				if (lNewBitmap)
				{
					delete (lNewBitmap);
				}
				break;	// Simple algorithm.
			}
		}
	}
}
