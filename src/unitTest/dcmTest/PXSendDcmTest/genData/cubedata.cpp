/*********************************************************************/
/*                                                                   */
/*                         TrCbct LIBRARY                            */
/*                                                                   */
/*                                                                   */
/*     v1.00 2007/03/07 N.Furutsuki  add  saveVoxelFile  VPro format */                             
/*                                                                   */
/*********************************************************************/

// ITrCtCbct.cpp: ITrCtCbct クラスのインプリメンテーション
//
//////////////////////////////////////////////////////////////////////

#include "stdafx.h"

//#include <stdio.h>

//#define USE_VALLoc
#ifdef USE_VALLoc
#include "OctSliceDsp.h"
#endif

#include "cubedata.h"
 

//////////////////////////////////////////////////////////////////////
// 構築/消滅
//////////////////////////////////////////////////////////////////////

#if 0
/*
#define ITrCtCbct_error  _TeraReconMessReptInstance->disperror

#define ReconParam_VCMask		0x01fff1
#define DetectorParam_VCMask	0x02fff2
#define CubeParam_VCMask		0x03fff3
#define FilterParam_VCMask		0x04fff4
#define PostProcParam_VCMask	0x05fff5
#define PreProcParam_VCMask		0x06fff6
#define ReconEXParam_VCMask		0x07fff7
*/
#endif
//////////////////////////////////////
void TrCtCbctCubeData::clearBuffer()
{
#define AllocDataType double
	int i;
	void *slice_buffer =  new AllocDataType[m_SliceSizeBytes/sizeof(AllocDataType)+1];
	int slice_size = m_PixelX*m_PixelY;

	if(slice_size <1) return ;

	switch(m_DataType){	 
		case DataType_INT:
		case DataType_UINT:	
			{
				int *data_p = (int*)slice_buffer;
				for(i=0;i<slice_size;i++){
					data_p[i] = 0;
				}
			}
			break;							
		case DataType_SHORT:
		case DataType_USHORT:
			{
				short *data_p = (short*)slice_buffer;
				for(i=0;i<slice_size;i++){
					data_p[i] = 0;
				}
			}
			break;								
		case DataType_FLOAT:					
			{
				float *data_p = (float*)slice_buffer;
				for(i=0;i<slice_size;i++){
					data_p[i] = 0;
				}
			}
			break;								
		case DataType_DOUBLE:					
			{
				double *data_p = (double*)slice_buffer;
				for(i=0;i<slice_size;i++){
					data_p[i] = 0;
				}
			}
			break;	
		case DataType_CHAR:					
			{
				char *data_p = (char*)slice_buffer;
				for(i=0;i<slice_size;i++){
					data_p[i] = 0;
				}
			}
			break;
		default:
			break;
	}
	for( i=0 ;i<m_PixelZ ;i++){
		memcpy(getSliceZ(i),slice_buffer,m_SliceSizeBytes);
	}

	AllocDataType *del_data_p = (AllocDataType *)slice_buffer;
	delete [] del_data_p;
}

void *  TrCtCbctCubeData::getRingSliceZ(int Z) const
{
	int ringZ = getRingIndex(Z);
	return m_SlicesBuffer[ringZ];

}
//////////////////////////////////////
//Cube data
TrCtCbctCubeData::TrCtCbctCubeData()
{
	m_AttachBufferFlag	= false;
	m_AttachMemoryFlag	= false;
	m_FlatMemory		= false;
	m_PixelX = m_PixelY = m_PixelZ = 0;
	m_SlicesBuffer		= NULL;
	m_DataMemory		= NULL;
	m_DataType			= DataType_UNKNOWN;

	setOrgPos(0,0,0);
	setPitch(0.1,0.1,0.1);

	m_SliceSizeBytes	= 0;
}
TrCtCbctCubeData::TrCtCbctCubeData(int sizeX,int sizeY,int sizeZ, DATA_TYPE type)
{
	m_AttachBufferFlag	= false;
	m_AttachMemoryFlag	= false;
	m_FlatMemory		= false;

	m_PixelX			= sizeX;
	m_PixelY			= sizeY;
	m_PixelZ			= sizeZ;

	m_SlicesBuffer		= NULL;
	m_DataMemory		= NULL;
	m_DataType			= type;

	setOrgPos(0,0,0);
	setPitch(0.1,0.1,0.1);

	m_SliceSizeBytes	= 0;
}

void TrCtCbctCubeData::setOrgPos(int x0,int y0,int z0)
{
	m_orgPos[0]	= x0;
	m_orgPos[1]	= y0;
	m_orgPos[2]	= z0;
}
void TrCtCbctCubeData::getOrgPos(int &x0,int &y0,int &z0)  const 
{
	x0 = m_orgPos[0]	 ;
	y0 = m_orgPos[1]	 ;
	z0 = m_orgPos[2]	 ;
}

void TrCtCbctCubeData::setUserSize(int sizeX,int sizeY,int sizeZ)
{
	m_userPixel[0]	= sizeX;
	m_userPixel[1]	= sizeY;
	m_userPixel[2]	= sizeZ;
}
void TrCtCbctCubeData::getUserSize(int &sizeX,int &sizeY,int &sizeZ)  const
{
	sizeX	= m_userPixel[0];
	sizeY	= m_userPixel[1];
	sizeZ	= m_userPixel[2];
}


TrCtCbctCubeData::~TrCtCbctCubeData()
{
	freeDataBuffer();
}

bool TrCtCbctCubeData::makeDataBuffer(bool allocMem)
{
	if(m_PixelX < 1 || m_PixelY <1 || m_PixelZ<1 ) return false;

	int slice_size = m_PixelX*m_PixelY;
 
	int slice_size_bytes;
	
	if(allocMem){
		m_FlatMemory = true;
	}
	switch(m_DataType){	 
		case DataType_INT:	
			slice_size_bytes = slice_size*sizeof(unsigned int);
			if(allocMem){
//				m_DataMemory = new unsigned int[slice_size*m_PixelZ];
				m_DataMemory = MALLOC_Int(slice_size*m_PixelZ); 
			}
			break;
		case DataType_UINT:	
			slice_size_bytes = slice_size*sizeof(int);
			if(allocMem){
//				m_DataMemory = new int[slice_size*m_PixelZ]; 
				m_DataMemory = MALLOC_Int(slice_size*m_PixelZ); 
			}
			break;								
		case DataType_SHORT:
			slice_size_bytes = slice_size*sizeof(short);
			if(allocMem){
//				m_DataMemory = new short[slice_size*m_PixelZ]; 
				m_DataMemory = MALLOC_Short(slice_size*m_PixelZ); 
			}
			break;
		case DataType_USHORT:
			slice_size_bytes = slice_size*sizeof(unsigned short);
			if(allocMem){
//				m_DataMemory = new unsigned short[slice_size*m_PixelZ]; 
				m_DataMemory = MALLOC_Short(slice_size*m_PixelZ); 
			}
			break;								
		case DataType_FLOAT:					
			slice_size_bytes = slice_size*sizeof(float);
			if(allocMem){
	//			m_DataMemory = new float[slice_size*m_PixelZ];
				m_DataMemory = MALLOC_Float(slice_size*m_PixelZ);
			}
			break;								
		case DataType_DOUBLE:					
			slice_size_bytes = slice_size*sizeof(double);
			if(allocMem){
				m_DataMemory = new double[slice_size*m_PixelZ];	
			}
			break;	
		case DataType_CHAR:					
			slice_size_bytes = slice_size*sizeof(char);
			if(allocMem){
//				m_DataMemory = new char[slice_size*m_PixelZ];
				m_DataMemory = MALLOC_Char(slice_size*m_PixelZ);
			}
			break;
		default:
			break;
	}
	if(allocMem){
		if(m_DataMemory == NULL){
			return false;
		}
	}
 

	m_SliceSizeBytes = slice_size_bytes;

	for(int i=0 ;i<m_PixelZ ;i++){
		m_SlicesBuffer[i] = (char*)m_DataMemory + i*slice_size_bytes;
	}

	return true;
}
bool TrCtCbctCubeData::allocDataBuffer(bool clearup)
{
	if(m_PixelX < 1 || m_PixelY <1 || m_PixelZ<1 ) return false;
		
	 m_SlicesBuffer = new void*[m_PixelZ] ;
	
	bool ret = makeDataBuffer(true);
	if(!ret) return false;

	if(clearup){
		clearBuffer();
	}
	return true;
 
}
 
void TrCtCbctCubeData::freeDataBuffer()
{
	 
	if(m_PixelX < 1 || m_PixelY <1 || m_PixelZ<1 ) return ;

	m_SliceSizeBytes = 0;

	if(m_AttachMemoryFlag){
		
		delete [] m_SlicesBuffer;
		
		m_SlicesBuffer = NULL;
		m_DataMemory = NULL;

		m_AttachBufferFlag	= false;
		m_AttachMemoryFlag	= false;
		m_FlatMemory		= false;

		return ;
	}
	if(m_AttachBufferFlag)
	{
		m_SlicesBuffer = NULL;
		m_DataMemory = NULL;
	
		m_AttachBufferFlag	= false;
		m_AttachMemoryFlag	= false;
		m_FlatMemory		= false;

		return ;
	}

	m_AttachBufferFlag	= false;
	m_AttachMemoryFlag	= false;
	m_FlatMemory		= false;

	if(m_SlicesBuffer == NULL) return ;
	if(m_DataMemory == NULL) return ;

	 
	switch(m_DataType){	 
	case DataType_INT:	
		{
#ifndef USE_VALLoc
	 		unsigned int *p_mem = (unsigned int *)m_DataMemory;
	 		if(p_mem) delete [] p_mem;
#else
			if(m_DataMemory) MALLOC_FREE(m_DataMemory);
#endif
			delete [] m_SlicesBuffer;
		 
		}
		break;
	case DataType_UINT:	
		{
#ifndef USE_VALLoc
 			int *p_mem = ( int *)m_DataMemory;
 			if(p_mem) delete [] p_mem;
#else
			if(m_DataMemory) MALLOC_FREE(m_DataMemory);
#endif
			delete [] m_SlicesBuffer;
		 
		}
		break;								
	case DataType_SHORT:
		{
#ifndef USE_VALLoc
 			short *p_mem = ( short *)m_DataMemory;
 			if(p_mem) delete [] p_mem;
#else
			if(m_DataMemory) MALLOC_FREE(m_DataMemory);
#endif
			delete [] m_SlicesBuffer;
		 
		}
		break;
	case DataType_USHORT:
		{
#ifndef USE_VALLoc
 			unsigned short *p_mem = ( unsigned short *)m_DataMemory;
 			if(p_mem) delete [] p_mem;
#else
			if(m_DataMemory) MALLOC_FREE(m_DataMemory);
#endif
			delete [] m_SlicesBuffer;
		 
		}
		break;								
	case DataType_FLOAT:
		{
#ifndef USE_VALLoc
 			float *p_mem = ( float *)m_DataMemory;
 			if(p_mem) delete [] p_mem;
#else
			if(m_DataMemory) MALLOC_FREE(m_DataMemory);
#endif
			delete [] m_SlicesBuffer;
		 
		}
		break;								
	case DataType_DOUBLE:	
		{
			double *p_mem = ( double *)m_DataMemory;
			if(p_mem) delete [] p_mem;

			delete [] m_SlicesBuffer;
		 
		}
		break;	
	case DataType_CHAR:	
		{
#ifndef USE_VALLoc
 			char *p_mem = ( char *)m_DataMemory;
 			if(p_mem) delete [] p_mem;
#else
			if(m_DataMemory) MALLOC_FREE(m_DataMemory);
#endif
			delete [] m_SlicesBuffer;
		 
		}
		break;
	default:
		break;
	}

	m_SlicesBuffer = NULL;
	m_DataMemory = NULL;
}
 
void TrCtCbctCubeData::setDim(int PixelX,int PixelY,int PixelZ)
{
	freeDataBuffer();
	m_PixelX	= PixelX;
	m_PixelY	= PixelY;
	m_PixelZ	= PixelZ;

	setOrgPos(0,0,0);
	setUserSize(m_PixelX,m_PixelY,m_PixelZ);
}
void TrCtCbctCubeData::getDim(int &PixelX,int &PixelY,int &PixelZ)  const 
{

	PixelX = m_PixelX ;
	PixelY = m_PixelY ;
	PixelZ = m_PixelZ ;
}
void TrCtCbctCubeData::setDataType(DATA_TYPE type)
{
	freeDataBuffer();
	m_DataType = type;
}
DATA_TYPE TrCtCbctCubeData::getDataType()  const 
{
	return m_DataType;
}

void *  TrCtCbctCubeData::getSliceZ(int Z) const
{
	if(Z <0 || Z >=m_PixelZ) return NULL;
	return m_SlicesBuffer[Z];
}
void ** TrCtCbctCubeData::getSlicesBuffer() const
{
	return m_SlicesBuffer;
}
void * 	TrCtCbctCubeData::getDataMemory() const
{
	return m_DataMemory;
}

void TrCtCbctCubeData::attachSlices(void **p)
{ 
	m_AttachBufferFlag	= true;
	m_AttachMemoryFlag  = false;
	m_FlatMemory		= false;

	m_SlicesBuffer		= p;
	m_DataMemory		= NULL;
};
 
void TrCtCbctCubeData::attachDataMemory(void *p)
{
	m_AttachBufferFlag	= false;
	m_AttachMemoryFlag  = true;
	m_FlatMemory		= true;

	m_DataMemory		= p;

	m_SlicesBuffer = new void*[m_PixelZ] ;

	makeDataBuffer(false);

}
 
//v1.00 --- > begin [({ 
void TrCtCbctCubeData::setPitch(double px, double py, double pz)
{
	PitchX	= px;
	PitchY	= py;
	PitchZ	= pz;
}
void TrCtCbctCubeData::getPitch(double &px, double &py, double &pz)  const 
{

	px = PitchX ;
	py = PitchY ;
	pz = PitchZ ;
}

//v1.00 --- < end })]