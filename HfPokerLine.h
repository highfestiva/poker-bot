
// Author: Jonas Byström, highfestiva@gmail.com. August 2003.
// Open source without neither limitations nor warranty.


#ifndef HFPOKERLINE_H
#define HFPOKERLINE_H


namespace HfPoker
{


class Line
{
public:
	int	mX1;
	int	mY1;
	int	mX2;
	int	mY2;

	inline Line(const Line& pCopy):
		mX1(pCopy.mX1),
		mY1(pCopy.mY1),
		mX2(pCopy.mX2),
		mY2(pCopy.mY2)
	{
	};
	inline Line(int pX1, int pY1, int pX2, int pY2):
		mX1(pX1),
		mY1(pY1),
		mX2(pX2),
		mY2(pY2)
	{
	};

	inline void operator=(const Line& pCopy)
	{
		mX1 = pCopy.mX1;
		mY1 = pCopy.mY1;
		mX2 = pCopy.mX2;
		mY2 = pCopy.mY2;
	};

	bool IsHorizontal() const;
	bool IsVertical() const;
};


};


#endif // HFPOKERLINE_H
