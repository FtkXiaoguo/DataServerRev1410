/***********************************************************************
 * AutoRoutingAEMan.cpp
 *---------------------------------------------------------------------
 *		Copyright, PreXion 2012, All rights reserved.
 *
 *  
 *-------------------------------------------------------------------
 */
#pragma warning (disable: 4786)
#pragma warning (disable: 4503)

#include "AutoRoutingAEMan.h"
 
#include "AqCore/AqString.h"

#include "Globals.h"
#include "PxNetDB.h"
#include "PxDBUtil.h"
#include "PxQueue.h"

#include "rtvsutil.h"
#include "PxDicomImage.h"
#include <algorithm> 
#include "AqCore/TRCriticalsection.h"

#include "IDcmLib.h"
#include "IDcmLibApi.h"
using namespace XTDcmLib;


AutoRoutingAEMan::AutoRoutingAEMan()
{
	m_stopFlag = false;
	m_ImageFileBlockSize = 32;

	m_WorkQueueInstance = new CPxWorkQueue ;
	m_cs = new TRCriticalSection ;
	m_db = new PxDBUtil::CPxDBUtil;
	///
	m_entrExtInfo = new CPxQueueEntryExt;
	reset();
}
AutoRoutingAEMan::~AutoRoutingAEMan()
{
	reset();
	delete m_WorkQueueInstance;
	delete m_cs;
	delete m_db;
	//
	delete m_entrExtInfo;
}
void AutoRoutingAEMan::reset()
{
	m_AEList.clear();
	m_filterEnable = false;
	m_isRoutingOnSchedule = false;
	//
	m_currentImageFileIndex = 0;
	m_ImageFileList.clear();
	m_lastCheckImageFileSize = 0;
	m_seriesFolder="";
	m_runCount = 0;
	m_waitFlag = true;
	 
}
bool AutoRoutingAEMan::isRoutingOnSchedule(CPxDicomImage* img)
{
	if(!m_filterEnable){
		if(!updateAEList(img)){
			return false;
		}
	}
	return m_isRoutingOnSchedule;
}
//bool AutoRoutingAEMan::getAEList(std::vector<std::string> &AEList,CPxDicomImage* img)
//#48
bool AutoRoutingAEMan::getAEList(std::vector<AutoRoutingTarget> &AEList,CPxDicomImage* img)
{
	if(!m_filterEnable){
		if(!updateAEList(img)){
			return false;
		}
	}
	AEList.clear();

	int size = m_AEList.size();
	if(size>0){
		AEList.resize(size);
		for(int i=0;i<size;i++){
			AEList[i] = m_AEList[i];
		}
	}
	return true;
}
bool AutoRoutingAEMan::updateAEList(CPxDicomImage* img)
{
	TRCSLock L(m_cs);

	bool isJPEGGateWay = AppComConfiguration::GetJpegGatewayFlag();

	m_filterEnable = false;
	m_isRoutingOnSchedule = false;
	m_AEList.clear();

	///
	m_SeriesInstanceUID = img->GetSeriesInstanceUID();
	m_StudyInstanceUID = img->GetStudyInstanceUID();

	{
		/*
		* 注意
		* DB登録時のCPxDicomMessage::FillPatientInfo と一致すること。
		* safetyCheckの時 重要
		*/
		char str_buff[kMaxPN];
	//#142_NoPatientName_NoComment
	//	img->GetValue(MC_ATT_PATIENTS_NAME,str_buff);
	//	m_entrExtInfo->m_PatientName	= str_buff;
		//
		img->GetValue(MC_ATT_PATIENT_ID,str_buff);
		m_entrExtInfo->m_PatientID		= str_buff;
		//
		str_buff[0]= 0;
		img->GetValue(MC_ATT_SERIES_DATE,str_buff);
		if(strlen(str_buff) ==0){//#79 2014/08/21 K.Ko
			img->GetValue(MC_ATT_STUDY_DATE,str_buff);
		}
		m_entrExtInfo->m_Date = str_buff;
		//
		str_buff[0]= 0;
		img->GetValue(MC_ATT_SERIES_TIME,str_buff);
		if(strlen(str_buff) ==0){//#79 2014/08/21 K.Ko
			img->GetValue(MC_ATT_STUDY_TIME,str_buff);
		}
		m_entrExtInfo->m_Date = m_entrExtInfo->m_Date + std::string(" ") + str_buff;
		///////
		
		img->GetValue(MC_ATT_SERIES_NUMBER,&(m_entrExtInfo->m_SeriesNumber));
		// do not set here
		//m_entrExtInfo->m_EntryImages;
		//m_entrExtInfo->m_ImagesInSeries;
		//
	//#142_NoPatientName_NoComment
	//	img->GetValue(MC_ATT_SERIES_DESCRIPTION,str_buff);
	//	m_entrExtInfo->m_Comment = str_buff;
		
	}

	if(m_db->isRoutingOnSchedule(AUTO_ROUTING_NAME)){
		;//OK
	}else{
		m_isRoutingOnSchedule = false;
		m_filterEnable = true;
		return true;
	};

	std::vector<int> TagFilterIDList;
	if(!getTagFilterIDFromRoutingSchedule( img,TagFilterIDList))
	{
		return false;
	}
	//#51 
	// 自動転送先に複数AEタイトルを設定した場合、１つしか転送できていない。複数AE
 	// へ転送できるようにする。

	std::map<std::string/*AE*/,AutoRoutingTarget> RoutingAEList_temp;//#51

	int tagFilter_size = TagFilterIDList.size();
	if(tagFilter_size>0){
	//	std::map<int,std::string> RoutingAEList_temp;
		////#48
//		std::map<int,AutoRoutingTarget> RoutingAEList_temp;//#51
		std::vector<RoutingAEInfo> routingAEsSubtotal;
		for(int i=0;i<tagFilter_size;i++){
			routingAEsSubtotal.clear();
			int filterID = TagFilterIDList[i];
			int status = m_db->GetRoutingAEInfos(filterID, routingAEsSubtotal);
			if (status != kOK && status != kNoResult)
			{
				GetAqLogger()->LogMessage("ERROR: db error %d while trying to get routingAEs for filterID = %d\n", status, filterID);
				GetAqLogger()->FlushLog();
				continue;
			}if (routingAEsSubtotal.size() <= 0)
			{
				GetAqLogger()->LogMessage(kDebug, "DEBUG: no routingAE's for filterID = %d\n", filterID);
				GetAqLogger()->FlushLog();
				continue;
			}
			//
			//	We got some, so add them to the total list
			if (routingAEsSubtotal.size() > 0) 
			{
				GetAqLogger()->LogMessage(kDebug, "DEBUG: Routing to: ");
				
			}

			for(int i = 0; i < routingAEsSubtotal.size(); i++)
			{
				std::string target_AE = routingAEsSubtotal[i].m_AE.m_AETitle;//#51
//			 	RoutingAEList_temp[filterID].m_AE = routingAEsSubtotal[i].m_AE.m_AETitle;
				RoutingAEList_temp[target_AE].m_AE = routingAEsSubtotal[i].m_AE.m_AETitle;//#51
				//#48
				std::string dest_ip_addr;
				if(routingAEsSubtotal[i].m_AE.GetAddress()!=0){
					dest_ip_addr = routingAEsSubtotal[i].m_AE.GetAddress();
				}
		//		RoutingAEList_temp[filterID].m_procType = AutoRoutingTarget::AutoRoutingType_DICOM;//defaultはDICOM転送
				RoutingAEList_temp[target_AE].m_procType = AutoRoutingTarget::AutoRoutingType_DICOM;//defaultはDICOM転送 //#51
				if(isJPEGGateWay){
					if(	(routingAEsSubtotal[i].m_AE.GetPort() == 0) &&
						(dest_ip_addr == "0.0.0.0")) {
						//RoutingAEList_temp[filterID].m_procType = AutoRoutingTarget::AutoRoutingType_JPEG;
						RoutingAEList_temp[target_AE].m_procType = AutoRoutingTarget::AutoRoutingType_JPEG;//#51
					}
				}
				 
				GetAqLogger()->LogMessage(kDebug, "[%s, %s, %s, %d] ", routingAEsSubtotal[i].m_AE.m_AETitle, 
					routingAEsSubtotal[i].m_AE.m_hostName, routingAEsSubtotal[i].m_AE.m_IPAddress, routingAEsSubtotal[i].m_AE.m_port);
				
			}

			if (routingAEsSubtotal.size() > 0) 
			{
				GetAqLogger()->LogMessage("\n");
				
			}

		}
		 
			

	//	std::map<int,AutoRoutingTarget>::iterator iter;
	std::map<std::string/*AE*/,AutoRoutingTarget>::iterator iter;//#51
		for(iter = RoutingAEList_temp.begin(); iter != RoutingAEList_temp.end(); iter++)
		{
		//	int filterID = iter->first;
		 
			AutoRoutingTarget AE_item;
			AE_item = iter->second;
			m_AEList.push_back(AE_item);
				 
		}
		 
	}

	m_filterEnable = true;
	m_isRoutingOnSchedule = (m_AEList.size()>0 );
	return true;
}
bool AutoRoutingAEMan::getTagFilterIDFromRoutingSchedule( CPxDicomImage* img,std::vector<int> &TagFilterIDList)
{
	TagFilterIDList.clear();

	////
	std::map<int, int> tmpFilterMap;

	////
	//	Get the list of Tag Filter Rules from the database
	std::vector<TagFilterRule> ruleV;
	int status = m_db->GetTagFilterRules(ruleV);
	if (status != kOK || ruleV.size() <= 0) 
	{
		return false;
	}

	char valueFromDicomBuff[kVR_LT];
	int ruleIsTrue = 0;
	for(int i = 0; i < ruleV.size(); i++)
	{
		

		int filterID				= ruleV[i].m_filterID;
		unsigned long tagFromRule	= ruleV[i].m_tag;
		int comparator				= ruleV[i].m_comparatorID;
		char* valueFromRule			= ruleV[i].m_value;
		//
		valueFromDicomBuff[0] = 0;
		int dcmStatus = img->GetValue(tagFromRule, valueFromDicomBuff, kVR_LT);
		if (dcmStatus != kNormalCompletion)
		{
			if (dcmStatus == kInvalidTag ||
				dcmStatus == kEmptyValue ||
				dcmStatus == kNullValue)
			{
				valueFromDicomBuff[0] = 0;
			}
			else
			{
				 
				GetAqLogger()->LogMessage("ERROR: Failed to get Tag %X , returned error: %d[%s]\n", tagFromRule, 
					dcmStatus, MC_Error_Message((MC_STATUS)dcmStatus));
				GetAqLogger()->FlushLog();

				tmpFilterMap[filterID] = 0;
				continue;
			}
		}
	 
	////
	//	Trim white space from both ends
		 
		iRTVDeSpaceDe(valueFromRule);

		iRTVDeSpaceDe(valueFromDicomBuff);

		std::string Str_valueFromRule = valueFromRule;
		std::transform(Str_valueFromRule.begin(), Str_valueFromRule.end(),Str_valueFromRule.begin(), ::toupper); 

		//
		std::string Str_valueFromDicom = valueFromDicomBuff;
		std::transform(Str_valueFromDicom.begin(), Str_valueFromDicom.end(),Str_valueFromDicom.begin(), ::toupper); 


		ruleIsTrue = 0;
		switch(comparator)
		{
		case kTagIs:
			ruleIsTrue = Str_valueFromRule == Str_valueFromDicom;
			GetAqLogger()->LogMessage(kDebug,"DEBUG: RULE [filterID = %d, rule# = %d, tag = 0x%08x]: %s (file) IS %s (rule) = %d\n", 
				filterID, i, tagFromRule, valueFromDicomBuff, valueFromRule, ruleIsTrue != 0);
			
			break;

		case kTagIsNot:
			ruleIsTrue =  !(Str_valueFromRule == Str_valueFromDicom);
			GetAqLogger()->LogMessage(kDebug,"DEBUG: RULE [filterID = %d, rule# = %d, tag = 0x%08x]: %s (file) IS NOT %s (rule) = %d\n", 
				filterID, i, tagFromRule, valueFromDicomBuff, valueFromRule, ruleIsTrue != 0);
			
			break;

		case kTagContains:
			ruleIsTrue = Str_valueFromDicom.find(Str_valueFromRule)>=0;
			GetAqLogger()->LogMessage(kDebug,"DEBUG: RULE [filterID = %d, rule# = %d, tag = 0x%08x]: %s (file) CONTAINS %s (rule) = %d\n", 
				filterID, i, tagFromRule, valueFromDicomBuff, valueFromRule, ruleIsTrue != 0);
			
			break;
 
		case kTagDoesNotContain:
			ruleIsTrue = Str_valueFromDicom.find(Str_valueFromRule)<0;
			GetAqLogger()->LogMessage(kDebug,"DEBUG: RULE [filterID = %d, rule# = %d, tag = 0x%08x]: %s (file) DOES NOT CONTAIN %s (rule) = %d\n", 
				filterID, i, tagFromRule, valueFromDicomBuff, valueFromRule, ruleIsTrue != 0);
			
			break;
 
		default:
			GetAqLogger()->LogMessage("ERROR: HandledCompletedSeries::GetListOfApplicableTagFilterIDs() - invalid comparator %d contained in rule:\n", comparator);
			GetAqLogger()->LogMessage("ERROR: HandledCompletedSeries::GetListOfApplicableTagFilterIDs() -    [%d, %d, %d, %s]\n", 
				filterID, tagFromRule, comparator, valueFromRule);
			
			tmpFilterMap[filterID] = 0;
			continue;
		};
		GetAqLogger()->FlushLog();

		//	RULE1 AND RULE2 AND ... RULEN.
		if (!ruleIsTrue)
		{
			//	if any rule evals to false, then the filter is false
			tmpFilterMap[filterID] = 0;
		}
		else
		{
			if (tmpFilterMap.find(filterID) == tmpFilterMap.end())
			{
				//	It's not there, so add it as true
				tmpFilterMap[filterID] = 1;
			}
		}
	////
	}
	
	//	Compose final output map only from rules that passed.  Just don't add the failed ones
	//		That way, oFilterIDMap.size() can tell us how many rules passed.
	std::map<int, int>::iterator iter;
	for(iter = tmpFilterMap.begin(); iter != tmpFilterMap.end(); iter++)
	{
		int filterID = iter->first;
		int filterPassed = iter->second;
		if (filterPassed){
			TagFilterIDList.push_back(filterID);
		}
	}
	return true;
}

//////////////////
//#20 Series毎にAutoRoutingを行う　2012/05/23　K.KO
 
bool AutoRoutingAEMan::tryAutoRoutingOneSeries()
{
	if(!m_isRoutingOnSchedule){
		return true;
	}
	int dest_AE_size = m_AEList.size();
	if(dest_AE_size<1){
		return true;
	}
	 
	if(!m_WorkQueueInstance){
		return false;
	}

	if(!m_WorkQueueInstance->isValid()){
//		std::string db_file_name = std::string(gConfig.m_DB_FolderName) + "\\";
//		db_file_name = db_file_name + SQLITE_DB_FILTE;
//		m_WorkQueueInstance->setupDBName(db_file_name);
		m_WorkQueueInstance->initRes();
	}
	
//	CPxWorkQueue QueueProc;

 
	CPxQueueEntry new_entry;
	new_entry.m_SendLevel = CPxQueueEntry::PxQueueLevel_Series;
	//////////
	new_entry.m_StudyInstanceUID	= m_StudyInstanceUID;
	new_entry.m_SeriesInstanceUID	= m_SeriesInstanceUID;
 
	//////////

	///////////
	//extInfo
	{
	// m_entrExtInfoから情報取得できるが
	// m_numberOfSeriesRelatedInstances を取得するため。
	// 安全設計も
		CPxDcmDB pxDb;
	 
	 	std::vector<SeriesDisplayInfo> oVal;
 
		DICOMData seriesFilter;

		strcpy(seriesFilter.m_studyInstanceUID,		m_StudyInstanceUID.c_str());
		strcpy(seriesFilter.m_seriesInstanceUID,	m_SeriesInstanceUID.c_str());
		 
	 
		int status = pxDb.GetSeriesDisplayInfoOnServer(oVal, &seriesFilter);
	 
		if (status != kOK){
			return false;
		}
		if(oVal.size()<1){
			return false;
		}
	//#142_NoPatientName_NoComment
	//	new_entry.m_extInfo.m_PatientName	= oVal[0].m_patientsName;
		new_entry.m_extInfo.m_PatientID		= oVal[0].m_patientID;
//		new_entry.m_extInfo.m_Date			= oVal[0].m_seriesDate + std::string(" ") + oVal[0].m_seriesTime;
//#79 2014/08/21 K.Ko
		new_entry.m_extInfo.m_Date			=	  std::string(strlen(oVal[0].m_seriesDate)==0 ? oVal[0].m_studyDate :  oVal[0].m_seriesDate)
												+ std::string(" ") 
												+ std::string(strlen(oVal[0].m_seriesTime)==0 ? oVal[0].m_studyTime : oVal[0].m_seriesTime);
		new_entry.m_extInfo.m_ImagesInSeries= oVal[0].m_numberOfSeriesRelatedInstances;
		new_entry.m_extInfo.m_SeriesNumber	= oVal[0].m_seriesNumber;
//#142_NoPatientName_NoComment
//		new_entry.m_extInfo.m_Comment		= oVal[0].m_seriesDescription;
	}

	
#if 1
	for(int i=0;i<dest_AE_size;i++){
		new_entry.m_DestinationAE		= m_AEList[i].m_AE;
		//#48
		new_entry.m_cmdID				= m_AEList[i].m_procType==AutoRoutingTarget::AutoRoutingType_JPEG ? 
											CPxQueueEntry::PxQueueCmd_JPEG : CPxQueueEntry::PxQueueCmd_DICOM;
		//////////
		new_entry.m_Priority			= 1;
		new_entry.m_Status				= 0;//don't care here
		new_entry.m_CreateTime			= time(0);
		new_entry.m_AccessTime			= time(0);

		//#19 2012/05/21 K.Ko
		m_WorkQueueInstance->createJobID(new_entry,
							false /*newFlag*/ //同じSeries/AEの場合は同一JobID使用
							  );



		AutoRoutingAEMan::ReformatJapaneseDicom(new_entry);//#25 20012/06/11 K.Ko
		m_WorkQueueInstance->addQueue(new_entry);
		{
			if(gConfig.m_traceDicomLevel >=1){  
	//#142_NoPatientName_NoComment
	//			GetAqLogger()->LogMessage( "INFO:[C%08d]  AutoRouting Queue : patientName:%s, id:%s, sereisUID:%s comment:%s\n", 
					GetAqLogger()->LogMessage("INFO:[C%08d]  AutoRouting Queue : id:%s, sereisUID:%s \n",
					DicomServInfor_AutoRoutingInfo,
	//				new_entry.m_extInfo.m_PatientName.c_str(),
					new_entry.m_extInfo.m_PatientID.c_str(),
					new_entry.m_SeriesInstanceUID.c_str() 
	//				new_entry.m_extInfo.m_Comment.c_str()
					 
					);
				GetAqLogger()->FlushLog();
			}
		}

	}
#endif

	return true;
}

void AutoRoutingAEMan::Kick(void)
{
	m_lastActiveTime = GetTickCount();
};
//-----------------------------------------------------------------------------------------------------
//
bool AutoRoutingAEMan::IsTimeOver(DWORD TickCount)
{

	if(!m_waitFlag){
		return true;
	}
	long overtime = (TickCount-m_lastActiveTime)/1000 -  gConfig.m_seriesCompleteAutoRoutingTimeout;

	return (overtime > 0);
 

}
 
//-----------------------------------------------------------------------------------------------------
//
void AutoRoutingAEMan::ForceTimeOut(void)
{	
//	TerareconCacheWriter::CloseCache(m_cacheDir);
}


//-----------------------------------------------------------------------------------------------------
//


int AutoRoutingAEMan::Process(void)
{
	if(gConfig.m_AutoRoutingTrig == AutoRouringTirg_BlockSize){
		tryAutoRoutingBlock(false/*dolast*/);
		m_lastActiveTime = GetTickCount();
	}
	return 0;
}
bool AutoRoutingAEMan::doLast()
{
	return  tryAutoRoutingBlock(true/*dolast*/);
}
bool AutoRoutingAEMan::tryAutoRoutingBlock(bool dolast)
{
	std::vector<std::string> image_file_list;
	int size = getImageFileNameBlock(image_file_list,dolast);
	if(size<1){
		m_waitFlag = true;
		return true;
	}else{
		m_waitFlag = false;
		if(!m_isRoutingOnSchedule){
			return true;
		}
		 
		if(!m_WorkQueueInstance){
			return false;
		}

		int dest_AE_size = m_AEList.size();
		if(dest_AE_size<1){
			return true;
		}

		char _str_buff[64];
		sprintf(_str_buff,"%d",m_runCount++);
		if(!m_WorkQueueInstance->isValid()){
	//		std::string db_file_name = std::string(gConfig.m_DB_FolderName) + "\\";
	//		db_file_name = db_file_name + SQLITE_DB_FILTE;
//			m_WorkQueueInstance->setupDBName(db_file_name);
			m_WorkQueueInstance->initRes();
		}
		CPxQueueEntry new_entry;
		new_entry.m_SendLevel = CPxQueueEntry::PxQueuelevel_EntryFile;
		//////////
		new_entry.m_StudyInstanceUID	= m_StudyInstanceUID;
		new_entry.m_SeriesInstanceUID	= m_SeriesInstanceUID;
		new_entry.m_SOPInstanceUID		= _str_buff;//entry No.

		///////////
		//extInfo
		{
		// m_entrExtInfoから情報取得
		// DBアクセスしない
	//#142_NoPatientName_NoComment			 
	//		new_entry.m_extInfo.m_PatientName	= m_entrExtInfo->m_PatientName; 
			new_entry.m_extInfo.m_PatientID		= m_entrExtInfo->m_PatientID;
			new_entry.m_extInfo.m_Date			= m_entrExtInfo->m_Date;
			new_entry.m_extInfo.m_ImagesInSeries= 0;
			new_entry.m_extInfo.m_EntryImages	= size;
			new_entry.m_extInfo.m_SeriesNumber	= m_entrExtInfo->m_SeriesNumber;
	//		new_entry.m_extInfo.m_Comment		= m_entrExtInfo->m_Comment;
		}
		for(int i=0;i<dest_AE_size;i++){
			new_entry.m_DestinationAE		= m_AEList[i].m_AE;
			//#48
			new_entry.m_cmdID				= m_AEList[i].m_procType==AutoRoutingTarget::AutoRoutingType_JPEG ? 
											CPxQueueEntry::PxQueueCmd_JPEG : CPxQueueEntry::PxQueueCmd_DICOM;
			//////////
			new_entry.m_Priority			= 1;
			new_entry.m_Status				= 0;//don't care here
			new_entry.m_CreateTime			= time(0);
			new_entry.m_AccessTime			= time(0);

			//#19 2012/05/21 K.Ko
			m_WorkQueueInstance->createJobID(new_entry,
								false /*newFlag*/ //同じSeries/AEの場合は同一JobID使用
								  );
			////////
			bool write_bin_b = writeQueueBinFile(&new_entry,image_file_list);
			if(!write_bin_b){
				return false;
			}
			////////
			AutoRoutingAEMan::ReformatJapaneseDicom(new_entry);//#25 20012/06/11 K.Ko
			m_WorkQueueInstance->addQueue(new_entry);
			//
			 
			{
				if(gConfig.m_traceDicomLevel >=1){  
			//#142_NoPatientName_NoComment
			//		GetAqLogger()->LogMessage( "INFO:[C%08d]  AutoRoutingBlock Queue : patientName:%s, id:%s, sereisUID:%s images[%d] JobID[%s]-%s\n", 
					GetAqLogger()->LogMessage("INFO:[C%08d]  AutoRoutingBlock Queue : id:%s, sereisUID:%s images[%d] JobID[%s]-%s\n",
						DicomServInfor_AutoRoutingInfo,
			//			new_entry.m_extInfo.m_PatientName.c_str(),
						new_entry.m_extInfo.m_PatientID.c_str(),
						new_entry.m_SeriesInstanceUID.c_str(),
						new_entry.m_extInfo.m_EntryImages,
						new_entry.m_JobID.c_str(),
						new_entry.m_SOPInstanceUID.c_str()
						);
					GetAqLogger()->FlushLog();
				}
			}
		}


	}
	return true;
}
int AutoRoutingAEMan::getImageFileNameBlock(std::vector<std::string> &list,bool dolast)
{
 	TRCSLock L(m_cs);
	int file_list_size = m_ImageFileList.size();

	int proc_size = 0 ;
	if(dolast){
		proc_size = file_list_size-m_currentImageFileIndex;
	}else{
		//
		// ファイルリストのサイズが変らなかったら、
		// シリーズ終了かもしれない、
		bool doRestFlag = file_list_size == m_lastCheckImageFileSize;

		m_lastCheckImageFileSize = file_list_size;

		int rest_size = file_list_size-m_currentImageFileIndex-m_ImageFileBlockSize;

		
		if(rest_size>=0){
			proc_size = m_ImageFileBlockSize;
		}else{
			if(doRestFlag){
				proc_size = file_list_size-m_currentImageFileIndex;
			}
		}
	}

	if(proc_size>0){
		 
		list.clear();
		list.resize(proc_size);
		for(int i=0;i<proc_size;i++){
			list[i] = m_ImageFileList[m_currentImageFileIndex+i];
		}
		m_currentImageFileIndex += proc_size;

		return proc_size;
	}else{
		
		return 0;
	}
 
	
}
void AutoRoutingAEMan::addImageFileName(const std::string &fileName)
{
	TRCSLock L(m_cs);
	m_ImageFileList.push_back(fileName);
}
void AutoRoutingAEMan::setupSeriesFolder(const std::string &folder)
{
	if(m_seriesFolder.size()>0) return;
	m_seriesFolder = folder;
}
 
bool AutoRoutingAEMan::writeQueueBinFile(const CPxQueueEntry *entry,const std::vector<std::string> &fileList)
{
	std::string entry_file_name = CPxQueueProc::getQueueEntryFileName(*entry);//std::string(gConfig.m_DB_FolderName) + "\\entry\\" + entry->m_JobID;

	CPxQueueBinFile QueueBinFile;
	QueueBinFile.writeQueueBinFileEx(entry_file_name,m_seriesFolder,fileList);
	return true;
}
 
//#25 20012/06/11 K.Ko
#include "JISToSJISMS.h"
void AutoRoutingAEMan::ReformatJapaneseDicom( const std::string &org, std::string &conv)
{
	CJISToSJISMS::ConvertJISToSJIS( org, conv );
	CJISToSJISMS::ReformatPatientName( conv, cPNStandard );
}
void AutoRoutingAEMan::ReformatJapaneseDicom( CPxQueueEntry &entry )
{
	std::string str_temp;
	//m_PatientName
//#142_NoPatientName_NoComment
#if 0
	if(entry.m_extInfo.m_PatientName.size()>0){
		
		AutoRoutingAEMan::ReformatJapaneseDicom(entry.m_extInfo.m_PatientName, str_temp);
		entry.m_extInfo.m_PatientName	= str_temp;
	}

	//m_Comment
	if(entry.m_extInfo.m_Comment.size()>0){
		
		AutoRoutingAEMan::ReformatJapaneseDicom(entry.m_extInfo.m_Comment, str_temp);
		entry.m_extInfo.m_Comment		= str_temp;
	}
#endif
}