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

#include "cubedata.h"
 
#include "VGVoxFile.h"

CVoxelFileCubeData::CVoxelFileCubeData()
{
	m_VoxelBits = 16;
}
CVoxelFileCubeData::CVoxelFileCubeData(int sizeX,int sizeY,int sizeZ, DATA_TYPE type) 
{
	m_VoxelBits = 16;
}
CVoxelFileCubeData::~CVoxelFileCubeData()
{
}

void CVoxelFileCubeData::setVoxelBits(int bits)
{
	m_VoxelBits = bits;
}
int CVoxelFileCubeData::getVoxelBits(void)
{
	return m_VoxelBits;
}
bool CVoxelFileCubeData::saveVoxelFile(const char * fname) const
{
	VGVoxFile voxFile;
	char copyright[256] = "No copyright claimed";

	bool retcd = true;

	int outVoxelSizeInBytes = 2;

	switch(m_DataType) {
	case DataType_INT:	
		outVoxelSizeInBytes = sizeof(unsigned int);	break;
	case DataType_UINT:	
		outVoxelSizeInBytes = sizeof(int);break;
	case DataType_SHORT:
		outVoxelSizeInBytes = sizeof(short);break;
	case DataType_USHORT:
		outVoxelSizeInBytes = sizeof(unsigned short);break;
	case DataType_FLOAT:					
		outVoxelSizeInBytes = sizeof(float);break;
	case DataType_DOUBLE:					
		outVoxelSizeInBytes = sizeof(double);
		return false;
		break;
	case DataType_CHAR:					
		outVoxelSizeInBytes = sizeof(char);
		return false;
		break;
	default:
		return false;
		break;
	}


	//Create a new voxFile
	retcd = voxFile.CreateNewFile (fname);
	if (retcd == false) return false;

	while(1) {
		retcd = voxFile.BeginFileHeader(1);
		if (retcd == false) break;
		retcd = voxFile.PutTitle(fname);
		if (retcd == false) break;
		retcd = voxFile.PutCopyright(copyright);
		if (retcd == false) break;
		retcd = voxFile.EndFileHeader();
		if (retcd == false) break;
		//-------------First volume
		retcd = voxFile.BeginVolumeDescriptor('L');
		if (retcd == false) break;
		retcd = voxFile.PutVolumeSize(m_PixelX, m_PixelY, m_PixelZ);
		if (retcd == false) break;
		retcd = voxFile.PutVoxelSize(8 * outVoxelSizeInBytes);
		if (retcd == false) break;
		retcd = voxFile.PutVolumePosition(0, 0, 0);
		if (retcd == false) break;
		retcd = voxFile.PutVolumeScale((float)PitchX, (float)PitchY, (float)PitchZ);
		if (retcd == false) break;

		switch(m_DataType) {
		case DataType_INT:	
		case DataType_SHORT:
//			retcd = voxFile.PutVoxelField (0, 0, outVoxelSizeInBytes * 8, "data", "si");
			retcd = voxFile.PutVoxelField (0, 0, m_VoxelBits, "data",0, "si");
			break;
		case DataType_UINT:	
		case DataType_USHORT:
//			retcd = voxFile.PutVoxelField (0, 0, outVoxelSizeInBytes * 8, "data", "ui");
			retcd = voxFile.PutVoxelField (0, 0, m_VoxelBits, "data",0, "ui");
			break;
		case DataType_FLOAT:					
			retcd = voxFile.PutVoxelField (0, 0, outVoxelSizeInBytes * 8, "data",0, "f");
			break;
		}
		if (retcd == false) break;

		VGFloat matrix[16] = { 0.0 }; // Initialize entire matrix to 0
		matrix[0]=1; matrix[5] = 1.0; matrix[10] = 1.0; matrix[15] = 1.0;

		retcd = voxFile.PutVolumeModelMatrix(matrix);
		if (retcd == false) break;
		retcd = voxFile.EndVolumeDescriptor();
		if (retcd == false) break;

		// WriteFile
		int sliceSize = m_PixelX * m_PixelY * outVoxelSizeInBytes;
	
		for(int i = 0; i < m_PixelZ; i++) {
			bool retcd = voxFile.PutDataSet((const char *)getSliceZ(i), sliceSize);
			if (retcd == false) break;
		}
		break;
	}

	voxFile.CloseNewFile();
	return retcd;
}


//v1.00 --- < end })]