/*********************************************************************
 *********************************************************************
 **************											**************
 **************	This document contains information that	**************
 **************	   is proprietary to PreXion 	**************
 **************		     All rights reserved.			**************
 **************			Copyright PreXion 		**************
 **************				 2005-2006					**************
 **************											**************
 *********************************************************************
 *********************************************************************
 *********************************************************************
 ****															  ****
 **** File       : AqMask.h										  ****
 **** Author     : Prashant Chopra								  ****
 **** Comments   : Interface of the AqMask class.			      ****
 **** 															  ****
 ****															  ****
 ****															  ****
 ****															  ****
 *********************************************************************
 **** Modification log [Please insert recent one on the top]	  ****

 PC 12-12-2005 -- Rewrote methods to pack/unpack mask buffers based
 on GL's recent memory management changes.
	
 GL 12-09-2005 -- Make AqVolume as the super class.
 Remove duplicated member functions and variables that were 
 in AqVolume. Removed empty fuinctions such as And, Or, and 
 Not.														  

 PC 4/21/06 -- Method to pack/unpack from an AqVolume
 ********************************************************************/










//////////////////////////////////////////////////////////////////////

#if !defined(AFX_AQMASK_H__A343CCE5_C143_441C_8A03_028D50C57589__INCLUDED_)
#define AFX_AQMASK_H__A343CCE5_C143_441C_8A03_028D50C57589__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#include "AqVolume.h"

#define kAqBitsPerByte 8

//GL 12-9-2005  make AqVolume as the super class
// Remove duplicated member functions and variables that were in AqVolume
// Removed empty fuinctions such as And, Or, and Not.

class AqMask : public AqVolume
{
public:
	AqMask():m_bitsPerPixel(1), m_voxelSizeX(0), m_voxelSizeY(0) {};
	
	// modified by shiying hu,2006-01-26
	// iReverseMask is a temporary solution before server takes "SelectedObjectValue" as input.
	// Currently, server always assume that
	// in mask file, table, ribcage, bone should be non zero value.
	bool Pack(char *iUnpackedBuffer, 
				   int iUnPackedXSize,
				   int iUnPackedYSize,
				   int iUnPackedZSize,
				   int iMaskId=0, 
				   bool	iReverseMask = 0,
				   int iUnpackedMasksPerByte=8);

	// Prashant Chopra 4/21/06 -- Methods to pack/unpack from an AqVolume
	bool Pack(AqVolume &iUnpackedMaskVolume, 				   
				   int iMaskId=0, 
				   int lastSliceIndex=-1,
				   bool	iReverseMask = 0,
				   int iUnpackedMasksPerByte=8);
	
	bool Unpack(AqVolume &iUnpackedMaskVolume, 				   
				   int iMaskId=0, 
				   bool	iReverseMask = 0,
				   int iUnpackedMasksPerByte=8);
	// Prashant Chopra 4/21/06 -- Methods to pack/unpack from an AqVolume


	bool Unpack(char *oUnpackedBuffer, 
				   int iUnPackedXSize,
				   int iUnPackedYSize,
				   int iUnPackedZSize,	
				   int iMaskId=0, 
				   int iUnpackedMasksPerByte=8);

	// PrashantChopra 4/14/06 - Specific for WS
	bool UnpackDownsampled(char* opUnPackedBuffer,
		   const int iUnPackedXSize,
		   const int iUnPackedYSize,
		   const int iUnPackedZSize,		   
		   const int iMaskId=0,
		   int iUnpackedMasksPerByte=8);
	// PrashantChopra 4/14/06 - Specific for WS


	virtual ~AqMask() {};

	bool Set(int iX, int iY, int iZ, unsigned int iValue=1);
	bool UnSet(int iX, int iY, int iZ);
	bool IsSet(int iX, int iY, int iZ);


	

	void SetVoxelInfo(int	iVoxelSizeX, int iVoxelSizeY, unsigned int iBitsPerPixel);
	

	inline unsigned int GetVoxelSizeX() const {	return m_voxelSizeX;}
	inline unsigned int GetVoxelSizeY() const {	return m_voxelSizeY;}
	inline unsigned int BitsPerPixel() const {return m_bitsPerPixel;}

	// SH, 2006-07-14
	// this is a temporary code
	// what we need is pack slice by slice instead of supplying whole unpacked volume
	static bool _packSlice(char *iUnpackedSlice, int iUnPackedSliceSize, 
		char *oPackedSlice, int iPackedSliceSize, int iMaskId=0, bool iReverseMask = 0,
		int iUnpackedMasksPerByte=8);

protected:
	// external data volume sizes
	int					m_voxelSizeX;
	int					m_voxelSizeY;
	unsigned int		m_bitsPerPixel;

	bool _unpackSlice(char *oUnpackedSlice, int iUnPackedSliceSize, 
		char *iPackedSlice, int iPackedSliceSize, int iMaskId=0, 
		int iUnpackedMasksPerByte=8);



};

#endif // !defined(AFX_AQMASK_H__A343CCE5_C143_441C_8A03_028D50C57589__INCLUDED_)
