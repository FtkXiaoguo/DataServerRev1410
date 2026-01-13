
#include "PxDcmStudysManage.h"

#include "PxNetDB.h"

#include "AppComUtil.h"
#include "PxDicomImage.h"
///
#include <functional>
#include <algorithm>
#include <vector>
#include <iostream>

#include <QtCore/qstring.h>
#include <QChar>


#include "AddPushDicomRequest.h"
#include "JISToSJISMS.h"
#include "QtHelper.h"

//#140_search_Japanese_JIS_UTF8
void CPxDcmStudysManage::InitCharacterLib(void)
{
	CPxDB::InitCharacterLib();
}
CPxDcmStudysManage::CPxDcmStudysManage(const std::string &charSet):
m_CharacterSet(charSet)
{
	
}
CPxDcmStudysManage::~CPxDcmStudysManage(void)
{
	 
}

//#61 2013/07/29
inline void convertJISFilterString(const std::string &in_str, std::string &out_str)
{
	std::string org_in_str = in_str;

	//#61 2013/07/29
	//後ろ必ず ＊をつける//ClientViewerと同じ仕様
	if(org_in_str.find_last_of('*') == (org_in_str.size()-1)){
		//OK
	}else{
		org_in_str += std::string("*");
	};
		
//	CPxDcmDbManage::ConvertSJToJCodeOnly(org_in_str, out_str);
	CPxDcmDbManage::ConvertSJToJCodeOnlyForSQL(org_in_str, out_str);//#62 2013/07/30

	//#61 2013/07/29
	//日本語の場合、JISコードのため先頭必ず ＊をつける
	if(out_str != org_in_str){//ClientViewerと同じ仕様
		out_str = std::string("*")+out_str;
	}

}
void CPxDcmStudysManage::setupQueryFilter(const StudyQuery &query,pRTVSDicomInformation *filerQuery)
{
	memset(filerQuery,0,sizeof(pRTVSDicomInformation));
	filerQuery->m_validFields				= 94205;


	//std::string str_temp;
	

	if(query.m_patientName.size()<1){
		strcpy(filerQuery->m_patientName,		"**");
	}else{
	// 	CPxDcmDbManage::ConvertSJToJCodeOnly(query.m_patientName, str_temp);
	//#61 2013/07/29
	 	//convertJISFilterString(query.m_patientName, str_temp);
		//#140_input_str_as_UTF8
		//QString strTemp = Str2QString(query.m_patientName);// Str2QString(str_temp);
		//strncpy(filerQuery->m_characterSet, "ISO_IR 192", sizeof(filerQuery->m_characterSet));
		//strncpy(filerQuery->m_patientName, strTemp.toUtf8(), kpRTVSMaxDICOMPNameLength);

		//#140_search_Japanese_JIS_UTF8
		strncpy(filerQuery->m_characterSet, m_CharacterSet.c_str(), sizeof(filerQuery->m_characterSet));
		strncpy(filerQuery->m_patientName, query.m_patientName.c_str(), kpRTVSMaxDICOMPNameLength);
	}
	//
	if(query.m_patientID.size()<1){
		filerQuery->m_patientID[0]			= 0;
	}else{
		strncpy(filerQuery->m_patientID,		query.m_patientID.c_str(),	kpRTVSMaxDICOMUIDLength);
	}

	//sutdy date
	if(query.m_studyDate.size()<1){
		filerQuery->m_studyDate[0]			=0;	// can contain ranges yyyymmdd-yyyymmdd
	}else{
		strncpy(filerQuery->m_studyDate,		query.m_studyDate.c_str(),	kpRTVSMaxDICOMDateRangeLength);
	}

	//comment
	if(query.m_studyDescription.size()<1){
		filerQuery->m_studyDescription[0]	=0;	
	}else{
	//	CPxDcmDbManage::ConvertSJToJCodeOnly(query.m_studyDescription, str_temp);
		//#61 2013/07/29
	 	//convertJISFilterString(query.m_studyDescription, str_temp);
		//#140_input_str_as_UTF8
		//QString strTemp = Str2QString(str_temp);
		//strncpy(filerQuery->m_characterSet, "ISO_IR 192", sizeof(filerQuery->m_characterSet));
		//strncpy(filerQuery->m_studyDescription, strTemp.toUtf8(), kpRTVSMaxDICOMNameLength);
		//strncpy(filerQuery->m_studyDescription,		str_temp.c_str(),	kpRTVSMaxDICOMNameLength);
		
		//#140_search_Japanese_JIS_UTF8
		strncpy(filerQuery->m_characterSet, m_CharacterSet.c_str(), sizeof(filerQuery->m_characterSet));
		strncpy(filerQuery->m_studyDescription, query.m_studyDescription.c_str(), kpRTVSMaxDICOMNameLength);
	}
	 

	 
	filerQuery->m_patientSex[0]				=0;
	filerQuery->m_patientBirthDate[0]		=0;
	filerQuery->m_studyCount[0]				=0;

	filerQuery->m_studyUID[0]				=0;
	filerQuery->m_studyID[0]				=0;	// SH 16 char max
	
	filerQuery->m_studyTime[0]				=0;
	filerQuery->m_accessionNumber[0]		=0;   // SH 16 max
	filerQuery->m_radiologistName[0]		=0;
	filerQuery->m_physicianName[0]			=0;
//	strcpy(filerQuery->m_modalitiesInStudy,		"CT");
	filerQuery->m_modalitiesInStudy[0]		= 0;

	filerQuery->m_seriesCount[0]			=0;
	filerQuery->m_imagesInStudy[0]			=0;
	//#140_input_str_as_UTF8
//	filerQuery->m_characterSet[0]			=0;

	filerQuery->m_seriesUID[0]				=0;
	filerQuery->m_seriesDate[0]				=0;	// yyyymmdd
	filerQuery->m_seriesTime[0]				=0;	// hhmmss+6
	filerQuery->m_seriesNumber[0]			=0;	// IS 12 max
	filerQuery->m_modality[0]				=0;
	filerQuery->m_imageCount[0]				=0;
	filerQuery->m_seriesDescription[0]		=0;

	filerQuery->m_bodyPartExamined[0]		=0;
	//
	filerQuery->m_source.m_type				=0;
	filerQuery->m_source.m_port				=0;

	filerQuery->m_auxDataInfo 				=0;  /* read, unread, interactiveReport etc */
 	filerQuery->m_readStatus 				=7;
	filerQuery->m_daysToLock				=0;   /* how many days for the lock (autodelete). -1 for forever */
 
}
bool CPxDcmStudysManage::queryStudys(const StudyQuery &query,PxDicomInforList &objs )
{
	CPxDcmDB pxDb;
 
	pRTVSDicomInformation query_filer;

	setupQueryFilter(query,&query_filer);
 
	std::vector<pRTVSDicomInformation>  studyList;

	  
	objs.clear();

	int status = pxDb.GetUserStudies(1, 1, studyList, query_filer);

	if (status != kOK)
	{
		printf(" ERROR: GetUserStudies \n");

		 
		return false;
	}

	//
	
	int size = studyList.size();

	std::string str_temp;
	for(int i=0;i<size;i++){
		PxDicomInfor new_item;
		
	 
		//
		ReformatJapaneseDicom(studyList[i].m_patientName, str_temp );
		new_item.m_patientName			= str_temp;
		//
		new_item.m_patientID			= studyList[i].m_patientID;
		new_item.m_patientSex			= studyList[i].m_patientSex;
		new_item.m_patientBirthDate		= studyList[i].m_patientBirthDate;
	 
		//
		new_item.m_studyCount			= studyList[i].m_studyCount			;;
										 
		new_item.m_studyUID				= studyList[i].m_studyUID				;
		new_item.m_studyID				= studyList[i].m_studyID				;	// SH 16 char max
		new_item.m_studyDate			= studyList[i].m_studyDate			;	// can contain ranges yyyymmdd-yyyymmdd
		new_item.m_studyTime			= studyList[i].m_studyTime			;
		new_item.m_accessionNumber		= studyList[i].m_accessionNumber		;   // SH 16 max

		//
		ReformatJapaneseDicom(studyList[i].m_radiologistName, str_temp );
		new_item.m_radiologistName			= str_temp;
	 
		//
		ReformatJapaneseDicom(studyList[i].m_physicianName, str_temp );
		new_item.m_physicianName			= str_temp;
	
		new_item.m_modalitiesInStudy	= studyList[i].m_modalitiesInStudy	;

		//
		ReformatJapaneseDicom(studyList[i].m_studyDescription, str_temp );
		new_item.m_studyDescription			= str_temp;
	
		new_item.m_seriesCount			= studyList[i].m_seriesCount			;
		new_item.m_imagesInStudy		= studyList[i].m_imagesInStudy		;
										 
		new_item.m_seriesUID			= studyList[i].m_seriesUID			;
		new_item.m_seriesDate			= studyList[i].m_seriesDate			;	// yyyymmdd
		new_item.m_seriesTime			= studyList[i].m_seriesTime			;	// hhmmss+6
		new_item.m_seriesNumber			= studyList[i].m_seriesNumber			;	// IS 12 max
		new_item.m_modality				= studyList[i].m_modalitiesInStudy				;
		new_item.m_imageCount			= studyList[i].m_imageCount			;

		//
		ReformatJapaneseDicom(studyList[i].m_seriesDescription, str_temp );
		new_item.m_seriesDescription			= str_temp;
											 
		new_item.m_bodyPartExamined		= studyList[i].m_bodyPartExamined		;
//		new_item.m_source				= studyList[i].m_source				;

		objs.push_back(new_item); 
	}
	return true;
}


bool CPxDcmStudysManage::querySeries(const std::string &studyUID,PxDicomInforList &objs)
{
	CPxDcmDB pxDb;

	DICOMData studyFilter;

	studyFilter.Clear();
	std::vector<DICOMSeries> series_list;

	strcpy(studyFilter.m_studyInstanceUID, studyUID.c_str());

	objs.clear();

 	int status = pxDb.GetSeriesList(series_list,&studyFilter);
 

	 
	if (status != kOK)
	{
		printf(" ERROR: listupStudy  GetSeriesList   studyUID=[%s]\n",studyUID.c_str());
		return false;
	}
	//
	
	int size = series_list.size();

	std::string str_temp;
	char _str_num_buff[128];
	for(int i=0;i<size;i++){
		PxDicomInfor new_item;
		
	 
		//
	 
		new_item.m_studyUID				= series_list[i].m_studyInstanceUID;
		  
										 
		new_item.m_seriesUID			= series_list[i].m_seriesInstanceUID;
		new_item.m_seriesDate			= series_list[i].m_seriesDate			;	// yyyymmdd
		new_item.m_seriesTime			= series_list[i].m_seriesTime			;	// hhmmss+6
		
		sprintf(_str_num_buff,"%d",series_list[i].m_seriesNumber);
		new_item.m_seriesNumber			= _str_num_buff			;	// IS 12 max
		new_item.m_modality				= series_list[i].m_modality				;

		sprintf(_str_num_buff,"%d",series_list[i].m_numberOfSeriesRelatedInstances);
		new_item.m_imageCount			= _str_num_buff			;

		//
		ReformatJapaneseDicom(series_list[i].m_seriesDescription, str_temp );
		new_item.m_seriesDescription			= str_temp;
		 
										 
		new_item.m_bodyPartExamined		= series_list[i].m_bodyPartExamined		;
//		new_item.m_source				= studyList[i].m_source				;

		objs.push_back(new_item); 
	}

	//
	 	
 					
	 ;			
 	/*
	char	m_viewPosition[ kVR_CS ];				
 
	char	m_stationName[ kVR_SH ];
	int		m_offlineFlag;
	int		m_IsQRData;
	long	m_seriesModifyTime;
	long	m_seriesHoldToDate;
	int		m_status;
	 
	 */

	return true;
}

class PxImageInforInstanceNumberSort
{
public:
    bool operator()( const PxImageInfor& lhs, const PxImageInfor& rhs ) const
    {
        return lhs.m_instanceNumber  < rhs.m_instanceNumber;
    }
};


bool CPxDcmStudysManage::queryImages(const std::string &studyUID,const std::string &seriesUID,PxImageInforList &objs)
{
	CPxDcmDB pxDb;

	DICOMData  Filter;

	Filter.Clear();
	std::vector<DICOMInstance> image_list;

	DICOMStudy  iStudy;
	strcpy(Filter.m_studyInstanceUID, studyUID.c_str());
	strcpy(Filter.m_seriesInstanceUID, seriesUID.c_str());

	objs.clear();

	 

	int status = pxDb.GetInstanceList(image_list,&Filter,seriesUID.c_str());

 
	 
	if (status != kOK)
	{
		printf(" ERROR: listupStudy  GetSeriesList   studyUID=[%s]\n",studyUID.c_str());
		return false;
	}
	//
	
	int size = image_list.size();

//	objs.resize(size);

	char _str_num_buff[128];

	 
	for(int i=0;i<size;i++){
		PxImageInfor new_item;
		
	 
		//
	 
		new_item.m_studyInstanceUID		= studyUID;
		new_item.m_seriesInstanceUID	= seriesUID;
		//
		new_item.m_instanceNumber		= image_list[i].m_instanceNumber;
		new_item.m_SOPInstanceUID		= image_list[i].m_SOPInstanceUID;
		  					 
	 
		 
//		objs[new_item.m_instanceNumber-1] = new_item;

 		objs.push_back(new_item); 
	}
	std::sort(objs.begin(),objs.end(),PxImageInforInstanceNumberSort());
	return true;
}


bool CPxDcmStudysManage::loadImage(const PxImageInfor &Image,PxImagePixel &pixelData,int &ret_saftyCheck,PxDicomInfor *dicom_info/*for safty check*/,bool rawData ) //#27 2012/06/14
{

	std::string fileName = getImageFileName( Image);

	CPxDicomImage dicom;
	if(kNormalCompletion !=dicom.Load(fileName.c_str())){
		return false;
	}

	ret_saftyCheck = SaftyCheckTag_None;

	if(dicom_info){//#27 2012/06/14
		std::string str_temp;
		char _str_buff[256];
		_str_buff[0]=0;
		//
		QString str1;
		QString str2;

#if 1
		//#27 2012/06/14
		//patient Name
#if 0 //#140
		if(dicom_info->m_patientName.size()>0){
			_str_buff[0]=0;
			dicom.GetValue(kVLIPatientsName, _str_buff, sizeof(_str_buff));
			ReformatJapaneseDicom(_str_buff, str_temp);
	//		CJISToSJISMS::ConvertJISToSJIS( _str_buff, str_temp );

			str1 = Str2QString(str_temp);
			str2 = Str2QString(dicom_info->m_patientName);
			if(str1.trimmed()  != str2.trimmed() ){
				ret_saftyCheck = SaftyCheckTag_PatientName;
//				return false;
			}
		}
#endif
		//patientID
		if(dicom_info->m_patientID.size()>0){
			_str_buff[0]=0;
			dicom.GetValue(kVLIPatientId, _str_buff, sizeof(_str_buff));
			 
			str1 = QString(_str_buff);
			str2 = Str2QString(dicom_info->m_patientID);
			if(str1.trimmed()  != str2.trimmed() ){
				ret_saftyCheck = SaftyCheckTag_PatientID;
	//			return false;
			}
		}

		//patientBirthDate
		if(dicom_info->m_patientBirthDate.size()>0){
			_str_buff[0]=0;
			dicom.GetValue(kVLIPatientsBirthDate, _str_buff, sizeof(_str_buff));
	//		CJISToSJISMS::ConvertJISToSJIS( _str_buff, str_temp );

			str1 = QString(_str_buff);
			str2 = Str2QString(dicom_info->m_patientBirthDate);
			if(str1.trimmed()  != str2.trimmed() ){
				ret_saftyCheck = SaftyCheckTag_PatientBirthDate;
	//			return false;
			}
		}
		static bool dbg_flag = false;
		if(dbg_flag){
			return false;
		}
		 
		 
#endif
		 
	}

#if 1
	bool ret_b = setupImagePixel(&dicom,pixelData,rawData);
	return ret_b;
#else
	int sizeY = dicom.GetNumberOfRows();
    int sizeX = dicom.GetNumberOfColumns();
    int frames = dicom.GetNumberOfFrames();

	//
	double RescaleIntercept = dicom.GetRescaleIntercept();
	double RescaleSlope = dicom.GetRescaleSlope();

	//
	double wl = dicom.GetWindowCenter();
	double ww = dicom.GetWindowWidth();
	//
	int BitsAllocated = dicom.GetBitsAllocated();
    int BitsStored = dicom.GetBitsStored();
    int HighBit = dicom.GetHighBit();
    int SamplesPerPixel = dicom.GetSamplesPerPixel();

 

	unsigned char*	image_pixels = dicom.GetImagePixels();
	if(!image_pixels){
		return false;
	}
	// 
	int sliceSize = sizeX*sizeY;
	if(sliceSize <1) return false;
	pixelData.m_sizeX = sizeX;
	pixelData.m_sizeY = sizeY;
	int adjSamplePerPixel = SamplesPerPixel;
	if(adjSamplePerPixel == 3){
		adjSamplePerPixel = 4;
	}
	pixelData.m_samplesPerPixel = adjSamplePerPixel ;
	if(pixelData.m_pixelData){
		delete [] pixelData.m_pixelData;
		
	}
	pixelData.m_pixelData = new unsigned char[sliceSize*adjSamplePerPixel];
	
	
	double offset;
	double gain ;
	if(BitsAllocated == 16){
		if(wl == 0){
			wl = 1200;
		}
		if(ww == 0){
			ww = 3000;
		}
		 offset =   RescaleIntercept -(wl-ww/2.0)  ;
		 gain = 256.0/ww;

		short *data_ptr = (short*)image_pixels;
		for(int i=0;i<sliceSize;i++){
			short s_temp = (RescaleSlope*data_ptr[i] + offset)*gain  ;
			if(s_temp<0) s_temp = 0;
			if(s_temp>255) s_temp = 255;
			pixelData.m_pixelData[i] = s_temp;
		}
		
	}else 
	if(BitsAllocated == 8){
		if(wl == 0){
			wl = 128;
		}
		if(ww == 0){
			ww = 256;
		}

		offset =   RescaleIntercept -(wl-ww/2.0)  ;
		 gain = 256.0/ww;

		unsigned char *data_ptr = (unsigned char *)image_pixels;
		 
		 
		if(adjSamplePerPixel == 4){
			
			for(int i=0;i<sliceSize;i++){
				 
				int index = 3*i;
				int des_index = adjSamplePerPixel*i;
				short s_temp = (RescaleSlope*data_ptr[index + 0] + offset)*gain  ;
				if(s_temp<0) s_temp = 0;
				if(s_temp>255) s_temp = 255;
				pixelData.m_pixelData[des_index + 2] = s_temp;
				 
				//+1
				s_temp = (RescaleSlope*data_ptr[index + 1] + offset)*gain  ;
				if(s_temp<0) s_temp = 1;
				if(s_temp>255) s_temp = 255;
				pixelData.m_pixelData[des_index + 1] = s_temp;
				 
				//+2
				s_temp = (RescaleSlope*data_ptr[index + 2] + offset)*gain  ;
				if(s_temp<0) s_temp = 1;
				if(s_temp>255) s_temp = 255;
				pixelData.m_pixelData[des_index + 0] = s_temp;
				 
				//+3
			// 	pixelData.m_pixelData[des_index + 3]  = sum_pixel/3;
			}
		}else{
			for(int i=0;i<sliceSize;i++){
				short s_temp = (RescaleSlope*data_ptr[i] + offset)*gain  ;
				if(s_temp<0) s_temp = 0;
				if(s_temp>255) s_temp = 255;
				pixelData.m_pixelData[i] = s_temp;
			}
		}
	}
	return true;
#endif
}


std::string CPxDcmStudysManage::getImageFileName(const PxImageInfor &Image) const
{
	char str_buff[120];
	 
	const char *iSOP = Image.m_SOPInstanceUID.c_str();

	std::string seriesFolder = CPxDcmDbManage::getDBSeriesFolder(Image.m_studyInstanceUID ,Image.m_seriesInstanceUID,iSOP);

#if 0
	std::string fileName  ;

	if(seriesFolder.size()>0){
		sprintf(str_buff,"%05d_%s.dcm", Image.m_instanceNumber ,iSOP);
		fileName = seriesFolder + str_buff;
	}else{
		;
	}

	return fileName;
#else
	return AppComUtil::getDicomFileName(	seriesFolder,Image.m_SOPInstanceUID,
										Image.m_instanceNumber);
#endif
}

std::string CPxDcmStudysManage::getSeriesFolder(const std::string &studyUID,const std::string &seriesUID ) const
{
	std::string seriesFolder = CPxDcmDbManage::getDBSeriesFolder(studyUID ,seriesUID);
	return seriesFolder;
}

bool CPxDcmStudysManage::delSeries(const std::string &studyUID,const std::string &seriesUID)
{
	CPxDcmDB pxDb;
	if( kOK!=pxDb.DeleteSeries( seriesUID.c_str()))
	{
		return false;
	}
	if( kOK!=DeletePrivateSeries(seriesUID.c_str()))
	{
		;
	}

	std::string  originalDir;

	CPxDcmDbManage::RemoveSeriesFromDisk(seriesUID.c_str(), studyUID.c_str(), originalDir);

	return true ;
}

bool CPxDcmStudysManage::delStudy(const std::string &studyUID)
{
	CPxDcmDB pxDb;

	//
	std::vector<std::string>  oSeries;

	if(kOK!=pxDb.GetSeries(studyUID.c_str(), oSeries)){
		return false;
	};

	if( kOK!=pxDb.DeleteStudy( studyUID.c_str()))
	{
		return false;
	}
	
	CPxDcmDbManage::RemoveAllDiskFiles(oSeries, studyUID.c_str(), 0/*iKeepOrphaned*/);
	return true;
}

//
//bool CPxDcmStudysManage::pushDICOMSeries(const std::string AETitle,const std::string &studyUID,const std::string &seriesUID)
//#48
bool CPxDcmStudysManage::pushDICOMSeries(const AEItemData *AETitle,const std::string &studyUID,const std::string &seriesUID)
{

	CAddPushDicomRequest addReq;
	addReq.pushDICOMSeries(AETitle,studyUID,seriesUID);
	return true;
}
//bool CPxDcmStudysManage::pushDICOMStudy(const std::string AETitle,const std::string &studyUID)
//#48
bool CPxDcmStudysManage::pushDICOMStudy(const AEItemData *AETitle,const std::string &studyUID)
{

	CAddPushDicomRequest addReq;
	addReq.pushDICOMStudy(AETitle,studyUID);

	return true;
}
bool CPxDcmStudysManage::setupImagePixel(CPxDicomImage *pDicom,PxImagePixel &pixelData,bool rawData)
{
	

	int sizeY = pDicom->GetNumberOfRows();
    int sizeX = pDicom->GetNumberOfColumns();
    int frames = pDicom->GetNumberOfFrames();

	//
	double RescaleIntercept = pDicom->GetRescaleIntercept();
	double RescaleSlope = pDicom->GetRescaleSlope();

	//
	double wl = pDicom->GetWindowCenter();
	double ww = pDicom->GetWindowWidth();
	//
	int BitsAllocated = pDicom->GetBitsAllocated();
    int BitsStored = pDicom->GetBitsStored();
    int HighBit = pDicom->GetHighBit();
    int SamplesPerPixel = pDicom->GetSamplesPerPixel();

 

	unsigned char*	image_pixels = pDicom->GetImagePixels();
	if(!image_pixels){
		return false;
	}
	// 
	int sliceSize = sizeX*sizeY;
	if(sliceSize <1) return false;
	pixelData.m_sizeX = sizeX;
	pixelData.m_sizeY = sizeY;
	int adjSamplePerPixel = SamplesPerPixel;
	if(adjSamplePerPixel == 3){
		adjSamplePerPixel = 4;
	}
	pixelData.m_samplesPerPixel = adjSamplePerPixel ;

	if(rawData){
		pixelData.m_bits				= BitsStored;
		pixelData.m_rescaleIntercept	= RescaleIntercept;
		pixelData.m_rescaleSlope		= RescaleSlope;
 
		pixelData.m_ww					= ww;
		pixelData.m_wl					= wl;
	 
		pixelData.m_pixelRepresentation	= pDicom->GetPixelRepresentation(); //0: unsigned short, 1: singned short
		if(kCT == pDicom->GetModality()){
			pixelData.m_hu_offset			= 1024.0f;
		}else{
			pixelData.m_hu_offset			= 0.0f;
		}
	}else{
		pixelData.m_bits				= 8;
		pixelData.m_rescaleIntercept	= 0.0f;
		pixelData.m_rescaleSlope		= 1.0f;
		pixelData.m_ww					= 255;
		pixelData.m_wl					= 128;
		//
		pixelData.m_pixelRepresentation	= 0; //0: unsigned short, 1: singned short
		pixelData.m_hu_offset			= 0.0f;
	}
	if(pixelData.m_pixelData){
		delete [] pixelData.m_pixelData;
		
	}
	int bpp = 1;
	if(pixelData.m_bits>8){
		bpp = 2;
	}
	int pixel_buff_len = sliceSize*adjSamplePerPixel*bpp;
//	pixelData.m_pixelData = new unsigned char[pixel_buff_len];
	pixelData.m_pixelData = (unsigned char*)(new unsigned short[pixel_buff_len/2+1]);
	
	
	if(rawData){
		if(adjSamplePerPixel == 4){
			
			pixelData.m_hu_offset			= 0.0f;
			unsigned char *data_ptr = (unsigned char *)image_pixels;

			for(int i=0;i<sliceSize;i++){
					 
				int index = 3*i;
				int des_index = adjSamplePerPixel*i;
				 
				pixelData.m_pixelData[des_index + 2] = data_ptr[index + 0];
				 
				//+1
				 
				pixelData.m_pixelData[des_index + 1] = data_ptr[index + 1];
				 
				//+2
				 
				pixelData.m_pixelData[des_index + 0] = data_ptr[index + 2];
				 
				//+3
			// 	pixelData.m_pixelData[des_index + 3]  = sum_pixel/3;
			}
		}else{
			memcpy(pixelData.m_pixelData,image_pixels,pixel_buff_len);
		}
	}else{

		double offset;
		double gain ;
		if(BitsAllocated == 16){
			if(wl == 0){
				wl = 1200;
			}
			if(ww == 0){
				ww = 3000;
			}
			 offset =   RescaleIntercept -(wl-ww/2.0)  ;
			 gain = 256.0/ww;

			short *data_ptr = (short*)image_pixels;
			for(int i=0;i<sliceSize;i++){
				short s_temp = (RescaleSlope*data_ptr[i] + offset)*gain  ;
				if(s_temp<0) s_temp = 0;
				if(s_temp>255) s_temp = 255;
				pixelData.m_pixelData[i] = s_temp;
			}
			
		}else 
		if(BitsAllocated == 8){
			if(wl == 0){
				wl = 128;
			}
			if(ww == 0){
				ww = 256;
			}

			offset =   RescaleIntercept -(wl-ww/2.0)  ;
			 gain = 256.0/ww;

			unsigned char *data_ptr = (unsigned char *)image_pixels;
			 
			 
			if(adjSamplePerPixel == 4){
				
				for(int i=0;i<sliceSize;i++){
					 
					int index = 3*i;
					int des_index = adjSamplePerPixel*i;
					short s_temp = (RescaleSlope*data_ptr[index + 0] + offset)*gain  ;
					if(s_temp<0) s_temp = 0;
					if(s_temp>255) s_temp = 255;
					pixelData.m_pixelData[des_index + 2] = s_temp;
					 
					//+1
					s_temp = (RescaleSlope*data_ptr[index + 1] + offset)*gain  ;
					if(s_temp<0) s_temp = 1;
					if(s_temp>255) s_temp = 255;
					pixelData.m_pixelData[des_index + 1] = s_temp;
					 
					//+2
					s_temp = (RescaleSlope*data_ptr[index + 2] + offset)*gain  ;
					if(s_temp<0) s_temp = 1;
					if(s_temp>255) s_temp = 255;
					pixelData.m_pixelData[des_index + 0] = s_temp;
					 
					//+3
				// 	pixelData.m_pixelData[des_index + 3]  = sum_pixel/3;
				}
			}else{
				for(int i=0;i<sliceSize;i++){
					short s_temp = (RescaleSlope*data_ptr[i] + offset)*gain  ;
					if(s_temp<0) s_temp = 0;
					if(s_temp>255) s_temp = 255;
					pixelData.m_pixelData[i] = s_temp;
				}
			}
		}

	}
	return true;
}
bool CPxDcmStudysManage::loadImage(const std::string dicomFile,PxImagePixel &pixelData ,bool rawData,PxDicomInfor *dicom_info  )
{
	 
	CPxDicomImage dicom;
	if(kNormalCompletion !=dicom.Load(dicomFile.c_str())){
		return false;
	}

	bool ret_b = setupImagePixel(&dicom,pixelData,rawData);
 
	if(ret_b){
	//2012/12/18
		if(dicom_info){
			std::string str_temp;
			char _str_buff[256];
			_str_buff[0]=0;
			//
		
			//patient Name
			_str_buff[0]=0;
			dicom.GetValue(kVLIPatientsName, _str_buff, sizeof(_str_buff));
			ReformatJapaneseDicom(_str_buff, str_temp);
			dicom_info->m_patientName = str_temp;
				 
			//patientID
			_str_buff[0]=0;
			dicom.GetValue(kVLIPatientId, _str_buff, sizeof(_str_buff));
			dicom_info->m_patientID = _str_buff;
				 
			//patientBirthDate
			_str_buff[0]=0;
			dicom.GetValue(kVLIPatientsBirthDate, _str_buff, sizeof(_str_buff));
			dicom_info->m_patientBirthDate = _str_buff ;
				 
			//patientSex
			_str_buff[0]=0;
			dicom.GetValue(kVLIPatientsSex, _str_buff, sizeof(_str_buff));
			dicom_info->m_patientSex = _str_buff ;
			 
			//studyDescription
			_str_buff[0]=0;
			dicom.GetValue(kVLIStudyDescription, _str_buff, sizeof(_str_buff));
			ReformatJapaneseDicom(_str_buff, str_temp);
			dicom_info->m_studyDescription = str_temp ;

			//seriesDescription
			_str_buff[0]=0;
			dicom.GetValue(kVLISeriesDescription, _str_buff, sizeof(_str_buff));
			ReformatJapaneseDicom(_str_buff, str_temp);
			dicom_info->m_seriesDescription = str_temp ;

			//studyDate
			_str_buff[0]=0;
			dicom.GetValue(kVLIStudyDate, _str_buff, sizeof(_str_buff));
			dicom_info->m_studyDate = _str_buff ;

			//studyTime
			_str_buff[0]=0;
			dicom.GetValue(kVLIStudyTime, _str_buff, sizeof(_str_buff));
			dicom_info->m_studyTime = _str_buff ;

			//seriesDate
			_str_buff[0]=0;
			dicom.GetValue(kVLISeriesDate, _str_buff, sizeof(_str_buff));
			dicom_info->m_seriesDate = _str_buff ;

			//seriesTime
			_str_buff[0]=0;
			dicom.GetValue(kVLISeriesTime, _str_buff, sizeof(_str_buff));
			dicom_info->m_seriesTime = _str_buff ;
			
			//physicianName
			_str_buff[0]=0;
			dicom.GetValue(kVLIReferringPhysiciansName, _str_buff, sizeof(_str_buff));
			ReformatJapaneseDicom(_str_buff, str_temp);
			dicom_info->m_physicianName = str_temp ;

			//modalitiesInStudy
			_str_buff[0]=0;
			dicom.GetValue(kVLIModality, _str_buff, sizeof(_str_buff));
			dicom_info->m_modalitiesInStudy = _str_buff ;
 
			//KVP
			_str_buff[0]=0;
			dicom.GetValue(kVLIKvp, _str_buff, sizeof(_str_buff));
			dicom_info->m_KVP = _str_buff ;

			//ExposureTime
			_str_buff[0]=0;
			dicom.GetValue(kVLIExposureTime, _str_buff, sizeof(_str_buff));
			dicom_info->m_ExposureTime = _str_buff ;
 
			//XRayTubeCurrent
			_str_buff[0]=0;
			dicom.GetValue(kVLIXRayTubeCurrent, _str_buff, sizeof(_str_buff));
			dicom_info->m_XRayTubeCurrent = _str_buff ;
			
			//SeriesNumber
			_str_buff[0]=0;
			dicom.GetValue(kVLISeriesNumber, _str_buff, sizeof(_str_buff));
			dicom_info->m_seriesNumber =  _str_buff ;

			//ManufacturerModelName
			_str_buff[0]=0;
			dicom.GetValue(kVLIManufacturersModelName, _str_buff, sizeof(_str_buff));
			dicom_info->m_ManufacturerModelName =  _str_buff ;

			//StationName
			_str_buff[0]=0;
			dicom.GetValue(kVLIStationName, _str_buff, sizeof(_str_buff));
			dicom_info->m_StationName =  _str_buff ;
		 
			//InstitutionName
			_str_buff[0]=0;
			dicom.GetValue(kVLIInstitutionName, _str_buff, sizeof(_str_buff));
			ReformatJapaneseDicom(_str_buff, str_temp);
			dicom_info->m_InstitutionName =  str_temp ;
 
	 ;


		}
	}
	return ret_b;
}