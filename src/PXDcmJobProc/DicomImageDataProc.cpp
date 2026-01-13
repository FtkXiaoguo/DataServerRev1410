/***********************************************************************
 * CStore.cpp
 *---------------------------------------------------------------------
 *	
 *-------------------------------------------------------------------
 */
#pragma warning (disable: 4786)
#pragma warning (disable: 4503)

#include "DicomImageDataProc.h"
 

#include <assert.h>
#include <sys/timeb.h>

#include "Globals.h"
 

 
  
#include "ijl.h"
 typedef IJLERR (__stdcall *IJL_FUN1)(JPEG_CORE_PROPERTIES*);
 typedef IJLERR (__stdcall *IJL_FUN2)(JPEG_CORE_PROPERTIES*,IJLIOTYPE);

 

IJL_FUN1 lpfnIJLInit = NULL;
IJL_FUN1 lpfnIJLFree = NULL;
IJL_FUN2 lpfnIJLRead = NULL;
IJL_FUN2 lpfnIJLWrite = NULL;
////////////////
#include "../../ipp/include/ipp.h"
#include "../../ipp/include/ippcore.h"

void* CDicomImageDataProc::m_hDll_ijl = 0;
bool CDicomImageDataProc::m_bInit_ijl = false;

//
typedef IppStatus (__stdcall *ippiResize_8u_C1R_FUN)(const Ipp8u* pSrc, IppiSize srcSize, int srcStep, IppiRect srcRoi,
                                      Ipp8u* pDst, int dstStep, IppiSize dstRoiSize,
                                      double xFactor, double yFactor, int interpolation);

ippiResize_8u_C1R_FUN ippiResize_8u_C1R_ptr = NULL;

void* CDicomImageDataProc::m_hDll_ipp_i = 0;
bool CDicomImageDataProc::m_bInit_ipp_i = false;
//-----------------------------------------------------------------------------
CDicomImageDataProc::CDicomImageDataProc ()
 
{
	
}

//-----------------------------------------------------------------------------
CDicomImageDataProc::~CDicomImageDataProc()
{
	 destroy();
}

bool CDicomImageDataProc::init( )
{
	m_hDll_ijl   = LoadLibrary( "ijl15.dll" );
//m_hDll_ijl   = LoadLibrary( "F:\\PXSDataServer\\DataServer\\src\\bin_v8\\debug\\ijl15.dll");
	if(!m_hDll_ijl){
		return false;
	}

	lpfnIJLInit = (IJL_FUN1)GetProcAddress((HMODULE)m_hDll_ijl, "ijlInit");   
	if(!lpfnIJLInit){
		return false;
	}

	lpfnIJLFree	= (IJL_FUN1)GetProcAddress((HMODULE)m_hDll_ijl, "ijlFree");   
	if(!lpfnIJLFree){
		return false;
	}
 
    lpfnIJLRead = (IJL_FUN2)GetProcAddress((HMODULE)m_hDll_ijl, "ijlRead"); 
	if(!lpfnIJLRead){
		return false;
	}
  
    lpfnIJLWrite = (IJL_FUN2)GetProcAddress((HMODULE)m_hDll_ijl, "ijlWrite");   
	if(!lpfnIJLWrite){
		return false;
	}
 
 	m_bInit_ijl = true;

	//
	m_hDll_ipp_i   = LoadLibrary( "ippi-5.1.dll" );
	if(!m_hDll_ipp_i){
		return false;
	}

	ippiResize_8u_C1R_ptr = (ippiResize_8u_C1R_FUN)GetProcAddress((HMODULE)m_hDll_ipp_i, "ippiResize_8u_C1R");   
	if(!ippiResize_8u_C1R_ptr){
		return false;
	}
	m_bInit_ipp_i = true;

	return true;
}
 void CDicomImageDataProc::destroy()
 {
	 if(m_hDll_ijl){
		FreeLibrary( (HMODULE)m_hDll_ijl );
		m_hDll_ijl = 0;
		m_bInit_ijl = false;
	 }
//	 if(!m_hDll_ipp_i){
//		FreeLibrary( (HMODULE)m_hDll_ijl );
	 if(m_hDll_ipp_i){//2013/07/26
		FreeLibrary( (HMODULE)m_hDll_ipp_i ); //2013/07/26
		m_hDll_ipp_i = 0;
		m_bInit_ipp_i = false;
	}
 

 }

bool CDicomImageDataProc::writeJPEG(const std::string &jpegFileName,const unsigned char *imageData,int sizeX,int sizeY,int writeSizeX,int writeSizeY,int quality)
{
	if(!m_bInit_ijl) return false;

	JPEG_CORE_PROPERTIES image;

	ZeroMemory( &image, sizeof( JPEG_CORE_PROPERTIES ) );
	bool ret_b = true;
	try{
// 		if( ijlInit( &image ) != IJL_OK )
 		if( lpfnIJLInit( &image ) != IJL_OK )
			
		{
			ret_b = false;
			throw(-1);
		}
 
	 
		image.DIBWidth         = sizeX;
		image.DIBHeight        = sizeY  ;
		image.DIBBytes         = ((LPBYTE)imageData);
		
		// Setup JPEG
	//        image.JPGFile          = (lpszPathName);
		image.JPGFile          = jpegFileName.c_str();
		image.JPGWidth         = writeSizeX;
		image.JPGHeight        = writeSizeY;

		//
		image.DIBColor       = IJL_G;
		image.DIBChannels    = 1;
		image.DIBPadBytes    = IJL_DIB_PAD_BYTES(image.DIBWidth,1);
	
		image.JPGColor       = IJL_G;
		image.JPGChannels    = 1;
		image.JPGSubsampling = IJL_NONE;
		image.jquality		 = quality;//gConfig.m_OutputJpegQuality;
		int status;
			
//	 	if( (status = ijlWrite( &image, IJL_JFILE_WRITEWHOLEIMAGE )) != IJL_OK )
		if( (status = lpfnIJLWrite( &image, IJL_JFILE_WRITEWHOLEIMAGE )) != IJL_OK )
		
		{
			ret_b = false;
		}
	}catch(...){
		ret_b = false;
	}
	
//  if( ijlFree( &image ) != IJL_OK )
	if( lpfnIJLFree( &image ) != IJL_OK )
    {
		ret_b = false;
    }
	return ret_b;
}


bool CDicomImageDataProc::resizeImage(	/* input */ const unsigned char *SrcImageData,int src_sizeX,int src_sizeY,
										int src_LineAlocSize,
										/* output*/ unsigned char *DestImageData,int dest_sizeX,int dest_sizeY,
										int dest_LineAlocSize)
{
	 
	if(!m_bInit_ipp_i){
		return false;
	}
	double xFactor = (double)dest_sizeX/src_sizeX;
	double yFactor = (double)dest_sizeY/src_sizeY; 

	//src_data
	IppiSize srcSize;
	srcSize.width	= src_sizeX;
	srcSize.height	= src_sizeY;
	int srcStep = src_LineAlocSize;//src_sizeX;
	//
	IppiRect srcRoi;
	srcRoi.x = 0;
	srcRoi.y = 0;
	srcRoi.width = src_sizeX;
	srcRoi.height = src_sizeY;
	//
	///////////
	//dest_data
	int dstStep = dest_LineAlocSize;//dest_sizeX;
	IppiSize dstRoiSize;
	dstRoiSize.width = dest_sizeX;
	dstRoiSize.height = dest_sizeY;

 
	Ipp8u *src_data		= (Ipp8u *)SrcImageData;
	Ipp8u *dest_data	= (Ipp8u *)DestImageData;
	IppStatus ret =  ippiResize_8u_C1R_ptr(	src_data,  srcSize,		srcStep,	srcRoi,
											dest_data,				dstStep,	dstRoiSize,
											xFactor , yFactor,
											IPPI_INTER_LINEAR/*interpolation*/);

	return ippStsNoErr == ret;
}

