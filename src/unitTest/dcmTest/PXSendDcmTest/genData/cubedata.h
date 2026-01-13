/******************************************************************************
 *
 *    Copyright 2006 - 2008 by TeraRecon, Inc.
 *    All Rights Reserved.
 *
 *****************************************************************************/
//////////////////////////////////////////////////////////////////////

#if !defined(AFX_ITrCtCbct_H__9DDE90CB_3C0B_4523_929B_8EAFBAA31889__INCLUDED_)
#define AFX_ITrCtCbct_H__9DDE90CB_3C0B_4523_929B_8EAFBAA31889__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000


class ITrCtCbctDirect;
//---------------------------------------
//  define data type
//---------------------------------------

	 enum DATA_TYPE 
	 {
		DataType_UNKNOWN	= 0 ,
		DataType_USHORT		= 1 ,		
		DataType_SHORT		= 2 ,		
		DataType_UINT		= 3 ,		
		DataType_INT		= 4 ,
		DataType_FLOAT		= 5 ,
		DataType_DOUBLE		= 6	,
		DataType_CHAR		= 7 ,
	};



//---------------------------------------
//  define array arameter 
//---------------------------------------
template <class ArrayT> class  ParamArray 
{
public:
	ParamArray(){
		m_ArraySize	= 0;
		m_Array		= NULL;
	};
	ParamArray(int size){ m_Array = NULL;setArraySize(size);}
	
	virtual	~ParamArray(){
		if(m_Array	!= NULL) delete [] m_Array;
	};
	//
	void setArraySize(int size){
		m_ArraySize	= size;
		if(m_Array	!= NULL) delete [] m_Array;
		m_Array		= new ArrayT[size];
	};
	//
	int getArraySize() const { return m_ArraySize; };
	//
	ArrayT  *getArray()const { return m_Array; };
	//
	template <class T> 
		void setData( int index, T data)
		{
			if(index >= m_ArraySize) return ;
			m_Array[index] = (ArrayT)data;
		}
	//
	void copyArray( int size, const ArrayT* Array)
	{
		if(size < 0 ) return ;
		if(m_Array != NULL) delete [] m_Array;
		m_ArraySize	= size;
		m_Array = new ArrayT[m_ArraySize];
		memcpy(m_Array,Array,sizeof(ArrayT)*size);
	}
	//
	template <class T> 
		void setArray( int size, const T* Array)
		{
			int i;
			if(size < 0 ) return ;
			if(m_Array != NULL) delete [] m_Array;
			m_ArraySize	= size;
			m_Array = new ArrayT[m_ArraySize];
			for(i=0 ;i <size; i++){
				m_Array[i] = (ArrayT)(Array[i]);
			}
		}
protected:
	int		m_ArraySize;
	ArrayT	*m_Array;
};

typedef ParamArray<float>	FloatParamArray		;
typedef ParamArray<double>	DoubleParamArray	;
typedef ParamArray<int>		IntParamArray		;
typedef ParamArray<short>	ShortPramArray		; 
typedef ParamArray<unsigned short>	UShortPramArray		; 

//#define USE_VALLoc //

#ifdef USE_VALLoc
void* VAlloc( int size );
void VFree( void* p );

#define INF_USE_VALLoc

//
#define MALLOC_Float(size) (float*)COctSliceDsp::mm_new(sizeof(float)*size)
#define MALLOC_Char(size) (char*)COctSliceDsp::mm_new(sizeof(char)*size)
#define MALLOC_Int(size) (int*)COctSliceDsp::mm_new(sizeof(int)*size)
#define MALLOC_Short(size) (short*)COctSliceDsp::mm_new(sizeof(short)*size)

#define MALLOC_FREE( p) COctSliceDsp::mm_del(p)
//

#else
#define MALLOC_Float(size) new float[(size)]
#define MALLOC_Char(size) new char[(size)]
#define MALLOC_Int(size) new int[(size)]
#define MALLOC_Short(size) new short[(size)]

#define MALLOC_FREE( p) delete [] (p)
#endif
//---------------------------------------
//  reconstructed result cube data
//---------------------------------------
class  TrCtCbctCubeData 
{
public:
	int			getSliceSizeBytes() { return m_SliceSizeBytes;};
	int			getRingIndex(int i) const { return (i+m_PixelZ)%m_PixelZ; };
	void *		getRingSliceZ(int Z) const;

	TrCtCbctCubeData();
	TrCtCbctCubeData(int sizeX,int sizeY,int sizeZ, DATA_TYPE type);
	virtual	~TrCtCbctCubeData();
	void		setDim(int PixelX,int PixelY,int PixelZ);
	void		getDim(int &PixelX,int &PixelY,int &PixelZ) const;
	void		setDataType(DATA_TYPE type);
	DATA_TYPE	getDataType() const;;
	bool		allocDataBuffer(bool clearup=false);
	void		freeDataBuffer();
	void		clearBuffer();
	void		attachSlices(void **p);
	void		attachDataMemory(void *p);
	void *		getSliceZ(int Z) const; 
	void **		getSlicesBuffer() const;
	void * 		getDataMemory() const;
	void		setOrgPos(int x0,int y0,int z0) ;
	void		getOrgPos(int &x0,int &y0,int &z0)  const;
	void		setPitch(double  px, double  py, double  pz);
	void		getPitch(double &px, double &py, double &pz) const;
 
	//valid cube data size  
	void		setUserSize(int sizeX,int sizeY,int sizeZ) ;
	void		getUserSize(int &sizeX,int &sizeY,int &sizeZ)  const;

//	bool		saveVoxelFile(const char * fname) const ;
	
//	void		setVoxelBits(int bits);
//	int			getVoxelBits(void);
protected:
	bool		makeDataBuffer(bool allocMem);
	DATA_TYPE		m_DataType;

	int				m_SliceSizeBytes;
	int				m_orgPos[3];
	int				m_userPixel[3];
	bool			m_AttachBufferFlag	;
	bool			m_AttachMemoryFlag	;
	bool			m_FlatMemory;
	int				m_PixelX	;		//reconstructed cube size(piexels) X 
	int				m_PixelY	;		//reconstructed cube size(piexels) Y 
	int				m_PixelZ	;		//reconstructed cube size(piexels) Z 

	double			PitchX ;	//reconstructed cube Voxel pitch X [mm]
	double			PitchY ;	//reconstructed cube Voxel pitch Y [mm]
	double			PitchZ ;	//reconstructed cube Voxel pitch Z [mm]
	
//	int				m_VoxelBits;	// Voxel bit size

	void**			m_SlicesBuffer;		//2D array m_DataBuffer[PixelZ][PixelX*PixelY]
	void*			m_DataMemory;		//flat continue memory , if m_FlatMemory is true
};

class CVoxelFileCubeData : public TrCtCbctCubeData
{
public:
CVoxelFileCubeData();
CVoxelFileCubeData(int sizeX,int sizeY,int sizeZ, DATA_TYPE type);
~CVoxelFileCubeData();
bool	saveVoxelFile(const char * fname) const ;

void	setVoxelBits(int bits);
int		getVoxelBits(void);
protected:
int				m_VoxelBits;	// Voxel bit size
};

#endif // !defined(AFX_ITrCtCbct_H__9DDE90CB_3C0B_4523_929B_8EAFBAA31889__INCLUDED_)
