#pragma once

#include "cubedata.h"

class TrCtCbctCubeData;
class COctDatBase
{
public:
	 
	COctDatBase(void);
	~COctDatBase(void);
	void setDim(int sizeX,int sizeY,int sizeZ){ m_sizeX=sizeX;m_sizeY=sizeY;m_sizeZ=sizeZ;};
	void setPitch(double pitchX,double pitchY,double pitchZ){ m_pitchX=pitchX;m_pitchY=pitchY;m_pitchZ=pitchZ;};
	void genData(DATA_TYPE dataType,int patID=0,float ref_val1=0.0,float ref_val2=0.0);

 
	void setOffset(int offset){ m_offset = offset;};

	void *getDataPtr(int frame) const;  
protected:
 
	int m_sizeX;
	int m_sizeY;
	int m_sizeZ;
	double m_pitchX;
	double m_pitchY;
	double m_pitchZ;
	int m_offset;

	TrCtCbctCubeData *m_dataBuff;


};
