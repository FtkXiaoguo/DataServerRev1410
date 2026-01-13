#pragma once

#include "vector"

class CStlPolygon
{
public:
	float m_normal[3];
	float m_point1[3];
	float m_point2[3];
	float m_point3[3];
};
inline void Indata2PxStlData(const CStlPolygon &data_in,CStlPolygon &data_out){
		memcpy(data_out.m_point1,data_in.m_point1,sizeof(float)*3);
		memcpy(data_out.m_point2,data_in.m_normal,sizeof(float)*3);
		memcpy(data_out.m_normal,data_in.m_point2,sizeof(float)*3);
		memcpy(data_out.m_point3,data_in.m_point3,sizeof(float)*3);
}
 
class CStlReadWrite
{
public:	
class DataRange3D
{
public:
	float m_rangeX[2];
	float m_rangeY[2];
	float m_rangeZ[2];
};
	enum FileType {
		FileTypeUnknown = 0,
		FileTypeBinary,
		FileTypeText
	};


	CStlReadWrite(void);
	~CStlReadWrite(void);

	bool readStl(const char *fileName);
	bool writeStlText(const char *fileName);
	bool writeStlBinary(const char *fileName);
	bool writeStlPXBinary(const char *fileName);//#1590 use PreXion format

	std::vector<CStlPolygon> &getPolygon() { return m_polygonPts;};

	void profile(int &polysize,DataRange3D *polygonRange);
protected:
	FileType DetectFileType(const char *filename,
                            unsigned long length = 256, 
                            double percent_bin = 0.05);
	bool readStlBinary(const char *fileName);
	bool readStlText(const char *fileName);

	bool readStlPXBinary(const char *fileName);//#1590 use PreXion format
	bool checkPxFormat(const char *fileName);//#1590 use PreXion format
	std::vector<CStlPolygon> m_polygonPts;
};
