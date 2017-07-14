
// Author: Jonas Byström, highfestiva@gmail.com. August 2003.
// Open source without neither limitations nor warranty.


#ifndef HFPOKERBITMAP_H
#define HFPOKERBITMAP_H


namespace HfPoker
{


class Bitmap
{
public:
				Bitmap(int pWidth, int pHeight, int pBPP, const unsigned char* pcBits);
				~Bitmap();

	int			GetWidth() const;
	int			GetHeight() const;
	int			GetBPP() const;
	const unsigned char*	GetBits() const;
	void			SetBits(const unsigned char* pcBits);

protected:
	int			mWidth;
	int			mHeight;
	int			mBPP;
	const unsigned char*	mBits;
};


};


#endif // HFPOKERBITMAP_H
