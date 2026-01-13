#pragma once

#include "string"

 
class CDicomProc
{
public:
	CDicomProc(void);
	~CDicomProc(void);

	static void initDcmLib();
	static void releaseDcmLib();
	bool FlipHori(const std::string &org_dicom_file,const std::string &dest_dicom_file,bool LA_except=true,bool NewUIDFlag=false);

	bool logoutDicomInfo(const std::string &org_dicom_file);

	void createPixelBuffer(int size);
	void copyPixelData(void *data,int size);
protected:
	
	bool openDicom(const std::string &dicom_file,bool readPixel=false);
	bool closeDicom();
	void *m_dcm_instance;
	int m_messsageID;
	unsigned char *m_OBOWbuffer;
	int m_OBOWSize;
};
