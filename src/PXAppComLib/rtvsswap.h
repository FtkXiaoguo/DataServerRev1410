/*
 * rtvsswap.h
 **---------------------------------------------------------------------
 *		Copyright (c) 2001, TeraRecon, Inc., All rights reserved.
 *
 *	PURPOSE:
 *   
 *	  Endian swap functions
 *
 *	AUTHOR(S):  T.C. Zhao, June 2001
 *----------------------------------------------------------------------- 
 */

#ifndef RTVSSWAP_H_
#define RTVSSWAP_H_

#include <assert.h>

/* swap a 2-byte entity */
template <class T> inline T iRTVSSwap16(T in)
{
	assert(sizeof (in) == 2);
	return in ? ((in & 0xff) << 8) | ((in >> 8) & 0xff) : 0;
}

/*/ Swap a 4-byte quantity. Should work for both ints and floats */
template <class T> inline T iRTVSSwap32(T iNum)
{
	assert(sizeof(iNum) == 4);

	unsigned int in = *(unsigned int *)&iNum;

	in =  ((in >> 24 ) & 0x000000ff) |
		  ((in >> 8  ) & 0x0000ff00) |
		  ((in << 8  ) & 0x00ff0000) |
		  ((in << 24));
	return *(T *)&in;
}


inline unsigned short iRTVSSwap(unsigned short& s)		{ return iRTVSSwap16(s);}
inline short		  iRTVSSwap(short& s)				{ return iRTVSSwap16(*(unsigned short*)&s);}
inline unsigned int	  iRTVSSwap(unsigned int& i)		{ return iRTVSSwap32(i);}
inline int	          iRTVSSwap(int& i)					{ return iRTVSSwap32(i);}
inline float		  iRTVSSwap(float &f)				{ return iRTVSSwap32(f);}
inline char			  iRTVSSwap(char& c)				{ return c;}
inline unsigned char  iRTVSSwap(unsigned char& c)		{ return c;}

#endif /* RTVSSWAP_H */
