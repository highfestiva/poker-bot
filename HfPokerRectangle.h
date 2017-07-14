
// Author: Jonas Byström, highfestiva@gmail.com. August 2003.
// Open source without neither limitations nor warranty.


#ifndef HFPOKERRECTANGLE_H
#define HFPOKERRECTANGLE_H


namespace HfPoker
{


class Rectangle
{
public:
	int	mX1;
	int	mY1;
	int	mX2;
	int	mY2;

	Rectangle(const Rectangle& pCopy);
	Rectangle(int pX1, int pY1, int pX2, int pY2);

	void AdjustSize(unsigned pWidth, unsigned pHeight);
};


};


#endif // HFPOKERRECTANGLE_H
