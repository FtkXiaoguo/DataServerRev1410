/***********************************************************************
 * CStore.h
 *---------------------------------------------------------------------
 *	
 *
 *-------------------------------------------------------------------
 */
#ifndef C_DICOM_IMAGE_DATA__PROC_H
#define C_DICOM_IMAGE_DATA__PROC_H
 
#include <string>
  
class CDicomImageDataProc 
{
public:
	CDicomImageDataProc ();
	
	~CDicomImageDataProc();
	
	static bool init();

	static bool writeJPEG(const std::string &jpegFileName,const unsigned char *imageData,int sizeX,int sizeY,int writeSizeX,int writeSizeY,int quality);
	static bool resizeImage(/* input */ const unsigned char *SrcImageData,int sizeX,int sizeY,
							int src_LineAlocSize,
							/* output*/ unsigned char *DestImageData,int outSizeX,int outSizeY,
							int dest_LineAlocSize);
protected:
	void destroy();
static void* m_hDll_ijl;
static bool m_bInit_ijl;
//
static void* m_hDll_ipp_i;
static bool m_bInit_ipp_i;
};

//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
#endif // C_DICOM_IMAGE_DATA__PROC_H
