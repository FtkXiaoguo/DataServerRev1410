#include "stdafx.h"
#include "DicomProc.h"

#include "AqCore/TRLogger.h"

#include "dcmlib/include/IDcmLib.h"
#include "dcmlib/include/IDcmLibApi.h"
 

using namespace XTDcmLib;
#define MyDcmMessage ((DcmXTDicomMessage *)m_dcm_instance)

extern TRLogger gLogger;

///////////////////////////////////////
#define MyRootUID "1.2.392.200036.9163"   //PreXion DICOM root UID


 #define  sRTVRootUID	   MyRootUID ".1001" //2.16.840.1.113669.632.21"
//sRTVRootUID //"2.16.840.1.113669.632.21"
void generateNewStudyUID(char *strBuf,int size)
{
	char str_buff[64];
//	sprintf(str_buff,"1%d",::GetTickCount());//,::GetCurrentThreadId() ,::GetCurrentProcessId());
	//#1377 2012/02/23 K.Ko   マイナス符号が表示されることがある
	DWORD time = ::GetTickCount();
	if(time<1) time = 1;
	sprintf(str_buff,"%lu",time);//,::GetCurrentThreadId() ,::GetCurrentProcessId());
	sprintf(strBuf,"%s.%s",sRTVRootUID,str_buff);
	///
	sprintf(strBuf,"%s.%s",sRTVRootUID,str_buff);
}

/////////////////////////////////////

void CDicomProc::initDcmLib()
{
	IDcmLibApi::DcmLibInitialization(
								NULL ,
								NULL ,
								NULL)   ;
}
void CDicomProc::releaseDcmLib()
{
	IDcmLibApi::DcmLibRelease();
}

CDicomProc::CDicomProc(void)
{
	m_dcm_instance = 0;
	m_OBOWbuffer  = 0;
}

CDicomProc::~CDicomProc(void)
{
	closeDicom();
}
bool CDicomProc::logoutDicomInfo(const std::string &org_dicom_file)
{
	if(!openDicom(org_dicom_file)) return false;
	char _patientID[128];
	if(!MyDcmMessage->Get_Value(0x00100020, _patientID,128 )){ 
		return false;
	} 
	 
	//(0008,0020) DA    [20131107]                             #     8,  1  StudyDate
	char _study_data[128];
	if(!MyDcmMessage->Get_Value(0x00080020, _study_data,128 )){ 
		return false;
	}
	//(0008,0030) TM    [143148]                               #     6,  1  StudyTime   
	char _study_time[128];
	if(!MyDcmMessage->Get_Value(0x00080030, _study_time,128 )){ 
		return false;
	}

	//(0008,0031) TM    [144357]                               #     6,  1  SeriesTime
	char _series_time[128];
	if(!MyDcmMessage->Get_Value(0x00080031, _series_time,128 )){ 
		return false;
	}


	//(0020,0011) IS    [105]                                  #     4,  1  SeriesNumber
	char _SeriesNumber[128];
	if(!MyDcmMessage->Get_Value(0x00200011, _SeriesNumber,128 )){ 
		return false;
	} 
	gLogger.LogMessage(">>PatientID [%s]  Date[%s] Time[%s] -- SeriesNumber [%s] Time[%s]\n",_patientID,_study_data,_study_time,_SeriesNumber,_series_time);
	gLogger.FlushLog();

	return closeDicom();

}
bool CDicomProc::FlipHori(const std::string &org_dicom_file,const std::string &dest_dicom_file,bool LA_except,bool NewUIDFlag)
{

	if(!openDicom(org_dicom_file,true /*readPixel*/)) return false;

	char _str_buff[256];

 
	//SOPClassUID
	if(!MyDcmMessage->Get_Value(0x00080016, _str_buff,256 )){ 
		return false;
	}else{
		//SC 1.2.840.10008.5.1.4.1.1.7
		std::string str_temp = _str_buff;
		if(str_temp == "1.2.840.10008.5.1.4.1.1.7"){
			return false;
		}
	}

	//BodyPartExamined
	if(!MyDcmMessage->Get_Value(0x00180015, _str_buff,256 )){ 
		 
		return false;
	}else{
		CString str_temp = _str_buff;
		str_temp.MakeUpper();
		if(	(str_temp==_T("HEAD")) ||
			(str_temp==_T("WRIST"))
			)
		{
			// ok do it
		}else{
			 
			return false;
		}
	}

	if(LA_except){
		
		//Patient Orientation 
		if(!MyDcmMessage->Get_Value(0x00200020, _str_buff,256,true /*Sequence*/ )){ 
			 
			return false;
		}else{
			CString str_temp = _str_buff;
			str_temp.MakeUpper();
			//is LA
			if( (str_temp==_T("A\\F")) ||
				(str_temp==_T("P\\F")))
			{
				::CopyFile(org_dicom_file.c_str(),dest_dicom_file.c_str(),FALSE);
		 		return true;
				 
			}
		}
	}

	 
	//do flip ---
	int outSizeX = 0;
	int outSizeY = 0;
	//(0028,0011) US    1880                                   #     2,  1  Columns
	MyDcmMessage->Get_Value(0x00280011,outSizeX);
	//(0028,0010) US    2040                                   #     2,  1  Rows
	MyDcmMessage->Get_Value(0x00280010,outSizeY);
	if(2*outSizeX*outSizeY !=  m_OBOWSize){
	//シーンデータ
		return false;
	}

	int center_size = outSizeX/2;
	unsigned short *pixel_data = ( unsigned short *)m_OBOWbuffer;
	for(int y_i=0;y_i<outSizeY;y_i++){
		 
		unsigned short *dataLinePtr			= pixel_data			+ y_i*outSizeX;

		for(int x_i=0;x_i<center_size;x_i++){
			unsigned short data_temp = dataLinePtr[outSizeX-1-x_i];
			dataLinePtr[outSizeX-1-x_i] = dataLinePtr[x_i];
			dataLinePtr[x_i] = data_temp;
		}
	}

	if(NewUIDFlag){
		char _char_buff[128];
		generateNewStudyUID(_char_buff,128);
		std::string str_study_uid_temp = _char_buff;

		//StudyInstanceUID
		if(!MyDcmMessage->Set_Value( 0x0020000d, _char_buff )){ 
			return false;
		}
		//SeriesInstanceUID
		 
		if(!MyDcmMessage->Set_Value( 0x0020000e, (str_study_uid_temp+std::string(".1102")).c_str() )){ 
			return false;
		}
		//SOPInstanceUID
		if(!MyDcmMessage->Set_Value( 0x00080018, (str_study_uid_temp+std::string(".1102.1")).c_str() )){ 
			return false;
		}

	}

	//Pixel data
	if(!MyDcmMessage->Set_ArrayValue( 0x7FE00010, m_OBOWbuffer, 2*outSizeX*outSizeY )){ 
		return false;
	}
	if(!MyDcmMessage->writeFile(dest_dicom_file.c_str())){
		return false;
	}
	return true;
}

class MyDcmXTMessageCallback : public DcmXTMessageCallback
{
public:
	MyDcmXTMessageCallback(CDicomProc *dicomProc){
		m_dicomProc = dicomProc;
	}
	~MyDcmXTMessageCallback(){};
virtual bool readPixelData(int msgID, CallbackType CBtype,
										unsigned long* dataSizePtr,void** dataBufferPtr,
										int isFirst,int* isLastPtr)
{
	if(CBtype == CB_DataLength){
		if(m_dicomProc){
			m_dicomProc->createPixelBuffer(*dataSizePtr);
		}
	}else if(CBtype == CB_Data){
		m_dicomProc->copyPixelData(*dataBufferPtr,*dataSizePtr);
	}
	return true;
};
protected:
	CDicomProc *m_dicomProc;
};

void CDicomProc::createPixelBuffer(int size)
{
	if(m_OBOWbuffer) delete [] m_OBOWbuffer;
	m_OBOWSize = size;
	m_OBOWbuffer = new unsigned  char[m_OBOWSize];
}
void CDicomProc::copyPixelData(void *data,int size)
{
	::memcpy(m_OBOWbuffer,data,size);
}
bool CDicomProc::openDicom(const std::string &dicom_file,bool readPixel)
{
	if(!IDcmLibApi::Open_Message(&m_messsageID)){
		return false;
	}

	m_dcm_instance = IDcmLibApi::get_DcmMessage(m_messsageID);
	 
	MyDcmXTMessageCallback pixel_callback(this);
	if(readPixel){
		MyDcmMessage->Set_Callback(&pixel_callback);
	}
	return MyDcmMessage->readFile(dicom_file.c_str());
	 
}
bool CDicomProc::closeDicom()
{
	if(m_dcm_instance){
		IDcmLibApi::Free_Message(m_messsageID);
		m_dcm_instance = 0;
	}
	if(m_OBOWbuffer) delete [] m_OBOWbuffer;
	return true;
}

 
