
// Author: Jonas Byström, highfestiva@gmail.com. August 2003.
// Open source without neither limitations nor warranty.


#ifndef HFSTD_H
#define HFSTD_H


typedef	__int64			int64;
typedef	unsigned __int64	uint64;


void InitRandom();
double GetRandomDouble();

int CountSetBits(int pBits);
int GetSetBit(int pBits, int pBitIndex);

inline unsigned Abs(int n)
{
	return (n >= 0? n : -n);
};
unsigned Faculty(unsigned n);
unsigned n_Choose_k(unsigned n, unsigned k);

double Lerp(double d1, double d2, double l);

void StringNCopy(char* s1, const char* s2, unsigned n);

void DumpScreenshot(int pWidth, int pHeight, int pBPP, const unsigned char* pBitmap);


#endif // HFSTD_H
