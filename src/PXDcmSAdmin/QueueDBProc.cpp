#pragma warning (disable: 4503)

#include "PxNetDB.h"
#include "PxDcmDbManage.h"

 #include <QCoreApplication>

#include "QueueDBProc.h"
#include "PxQueue.h"

#include "QtHelper.h"

 
bool QueuViewItem::isBrokenEntry(const QueuViewItem &entry)
{
	time_t cur_time = time(0);//CPxQueueProc::getCurTime() ;
	int diff_time = cur_time - entry.m_accessTime;
	if(diff_time<0)  diff_time=-diff_time;

#if 0 //for debug	#26
	{

		struct tm tm_temp_1 = *localtime(&entry.m_accessTime);
		struct tm tm_temp_2 = *localtime(&cur_time);

		FILE *fp = fopen("dbg_time_diff.txt","wt");
		fprintf(fp,"time1=%lx",entry.m_accessTime);
		fprintf(fp,"time2=%lx ", cur_time);
		fprintf(fp,"\n");

		fprintf(fp," time1 %04d/%02d/%02d %02d:%02d:%02d ",
			tm_temp_1.tm_year + 1900,
			tm_temp_1.tm_mon + 1,
			tm_temp_1.tm_mday,
			tm_temp_1.tm_hour,
			tm_temp_1.tm_min,
			tm_temp_1.tm_sec
			);
		fprintf(fp," time2 %04d/%02d/%02d %02d:%02d:%02d ",
			tm_temp_2.tm_year + 1900,
			tm_temp_2.tm_mon + 1,
			tm_temp_2.tm_mday,
			tm_temp_2.tm_hour,
			tm_temp_2.tm_min,
			tm_temp_2.tm_sec
			);

		fclose(fp);

	}
#endif

	if(diff_time > 120){ // 2Min
		return true;
	}else{
		return false;
	}
}

std::string CQueueDBProc::m_HomeDir;
void CQueueDBProc::getHomeDir()
{
	m_HomeDir = QString2Str(QCoreApplication::applicationDirPath());
	int pos = m_HomeDir.find_last_of("/");
	m_HomeDir = m_HomeDir.substr(0,pos);
}
void CQueueDBProc::init()
{
	getHomeDir();
	///
	CPxQueueProc::Global_init();
}
CQueueDBProc::CQueueDBProc(void)
{
	if(m_HomeDir.size()<1){
		getHomeDir();
	}
	m_DBQueueName = m_HomeDir +"/data/" ;
	m_DBQueueName = m_DBQueueName + SQLITE_DB_FILTE;

	//
	CPxQueueProc::setupQueueDBName(m_DBQueueName);
	//
	std::string queue_entry_folder = m_HomeDir +"/data/entry/";
	CPxQueueProc::setupQueueEntryFolder(queue_entry_folder);
	
}

CQueueDBProc::~CQueueDBProc(void)
{
}
std::string CQueueDBProc::getStlyeFileName()
{
	if(m_HomeDir.size()<1){
		getHomeDir();
	}
	std::string style_file_name = m_HomeDir +"/config/";
	style_file_name = style_file_name+"PXDcmSAdmin.qss";

	return style_file_name;
}

std::string CQueueDBProc::getLogFileFolder()
{
	if(m_HomeDir.size()<1){
		getHomeDir();
	}
	std::string logfile_dir = m_HomeDir +"/log/";

	return logfile_dir;
}
bool CQueueDBProc::getSendQueueList(std::vector<QueuViewItem> &list)
{
	char _str_num_buff[64];
	list.clear();

	CPxWorkQueue QueueProc;
	QueueProc.initRes();
//	QueueProc.readQueueExt(CPxQueueEntry::PxQueuePriority_All);
	QueueProc.readQueueAll();

	std::vector<CPxQueueEntry> &entry_list = QueueProc.getEntryList();
	int size = entry_list.size();
	if(size>0){
		list.resize(size);
		for(int i=0;i<size;i++){
			int send_level = entry_list[i].m_SendLevel;
			//
			list[i].m_QueueID		= entry_list[i].m_QueueID;
			list[i].m_Level			= send_level;
			list[i].m_JobID			= entry_list[i].m_JobID;
			list[i].m_studyUID		= entry_list[i].m_StudyInstanceUID;
			list[i].m_seriesUID		= entry_list[i].m_SeriesInstanceUID;
 
			list[i].m_destinationAE	= entry_list[i].m_DestinationAE;
			//
			list[i].m_retryCount	= entry_list[i].m_RetryCount;
			list[i].m_accessTime	= entry_list[i].m_AccessTime;
			
			list[i].m_status		= entry_list[i].m_Status;
			if(!getInfoFromMainDB(list[i],&(entry_list[i]))){
				return false;
			}
 
		}
	}

	return true;
}
bool CQueueDBProc::getResultQueueList(std::vector<QueuViewItem> &list,bool failedOnly)
{
	list.clear();

	CPxResultQueue ResultQueueProc;
	ResultQueueProc.initRes();
	if(failedOnly){
		ResultQueueProc.readQueueExt(CPxQueueEntry::PxQueuePriority_All,
								CPxQueueEntry::PxQueueStatus_Failed);
	}else{
		ResultQueueProc.readQueueExt(CPxQueueEntry::PxQueuePriority_All,
								CPxQueueEntry::PxQueueStatus_Finished,
								CPxQueueEntry::PxQueueStatus_Failed);
	}
	std::vector<CPxQueueEntry> &entry_list = ResultQueueProc.getEntryList();
	int size = entry_list.size();
	if(size>0){
		list.resize(size);
		for(int i=0;i<size;i++){
			list[i].m_QueueID		= entry_list[i].m_QueueID;
			//
			list[i].m_Level			= entry_list[i].m_SendLevel;
			list[i].m_JobID			= entry_list[i].m_JobID;
			list[i].m_studyUID		= entry_list[i].m_StudyInstanceUID;
			list[i].m_seriesUID		= entry_list[i].m_SeriesInstanceUID;
			//
			list[i].m_destinationAE	= entry_list[i].m_DestinationAE;
			//
			list[i].m_retryCount	= entry_list[i].m_RetryCount;
			list[i].m_accessTime	= entry_list[i].m_AccessTime;
			//
			list[i].m_status		= entry_list[i].m_Status;
			if(!getInfoFromMainDB(list[i],&(entry_list[i]))){
				return false;
			}
		}
	}

	return true;
}
bool CQueueDBProc::getInfoFromMainDB(QueuViewItem &disp_entry,const CPxQueueEntry *src_entry)
{

	char _str_num_buff[64];

#if 1
	std::string str_temp;
	int send_level = src_entry->m_SendLevel;

	////#25 20012/06/11 K.Ko 
//	CPxDcmDbManage::ReformatJapaneseDicom(src_entry->m_extInfo.m_PatientName, str_temp );
//	disp_entry.m_patientName	= src_entry->m_extInfo.m_PatientName ;	//#142_NoPatientName_NoComment
	disp_entry.m_patientID		= src_entry->m_extInfo.m_PatientID ;
	disp_entry.m_dispDate		= src_entry->m_extInfo.m_Date;
	//
	sprintf(_str_num_buff,"%d",src_entry->m_extInfo.m_SeriesNumber);
	disp_entry.m_seriesNumber	= _str_num_buff;
	//
	if(send_level == CPxQueueEntry::PxQueuelevel_EntryFile){ 
		sprintf(_str_num_buff,"%d",src_entry->m_extInfo.m_EntryImages);
		disp_entry.m_images		= src_entry->m_SOPInstanceUID+std::string("-")+_str_num_buff;
	}else{
		sprintf(_str_num_buff,"%d",src_entry->m_extInfo.m_ImagesInSeries);
		disp_entry.m_images		= _str_num_buff;
	}
#else
	std::string map_key ;
	if(disp_entry.m_Level == CPxQueueEntry::PxQueueLevel_Study){
		map_key = disp_entry.m_studyUID;
	}else{
		map_key = disp_entry.m_seriesUID;
	}
	QueuViewItem &Catch_Entry = m_SeriesDisplayInfoCatch[map_key];
	 
	if(Catch_Entry.valid!=0){
	}else{
		CPxDcmDB pxDb;
	//	std::string str_temp;

		std::vector<SeriesDisplayInfo> oVal;
		DICOMData seriesFilter;

		strcpy(seriesFilter.m_studyInstanceUID,		disp_entry.m_studyUID.c_str());
		if(disp_entry.m_Level != CPxQueueEntry::PxQueueLevel_Study){
			strcpy(seriesFilter.m_seriesInstanceUID,	disp_entry.m_seriesUID.c_str());
		}
		int status = pxDb.GetSeriesDisplayInfoOnServer(oVal, &seriesFilter);
		if (status != kOK){
			return false;
		}
		if(oVal.size()<1){
			return false;
		}
		Catch_Entry.valid			= 1;
		//
		Catch_Entry.m_patientID			= oVal[0].m_patientID;

		CPxDcmDbManage::ReformatJapaneseDicom(oVal[0].m_patientsName, Catch_Entry.m_patientName );
		//Catch_Entry.m_patientName		= oVal[0].m_patientsName;
		Catch_Entry.m_studyDate			= oVal[0].m_studyDate;
//		Catch_Entry.m_seriesDate		= oVal[0].m_seriesDate;
//		Catch_Entry.m_seriesTime		= oVal[0].m_seriesTime;
		Catch_Entry.m_dispDate			= std::string(oVal[0].m_seriesDate) + std::string(" ") + std::string(oVal[0].m_seriesTime);
		//
		sprintf(_str_num_buff,"%d",oVal[0].m_numberOfSeriesRelatedInstances);
		Catch_Entry.m_images			= _str_num_buff;
		sprintf(_str_num_buff,"%d",oVal[0].m_seriesNumber);
		Catch_Entry.m_seriesNumber		= _str_num_buff;
	 
	}
	disp_entry.m_patientID	= Catch_Entry.m_patientID;
	disp_entry.m_patientName= Catch_Entry.m_patientName;
	disp_entry.m_studyDate	= Catch_Entry.m_studyDate;
//	disp_entry.m_seriesDate	= Catch_Entry.m_seriesDate;
//	disp_entry.m_seriesTime	= Catch_Entry.m_seriesTime;
	disp_entry.m_dispDate	= Catch_Entry.m_dispDate;
	disp_entry.m_images		= Catch_Entry.m_images;
	disp_entry.m_seriesNumber= Catch_Entry.m_seriesNumber;
#endif
	return true;
}

bool CQueueDBProc::deleteSendQueueList(const std::vector<QueuViewItem> &del_list)
{
	CPxWorkQueue QueueProc;
	QueueProc.initRes();

	int size = del_list.size();
	for(int i=0;i<size;i++){
//		QueueProc.changeStatus(del_list[i].m_QueueID,CPxQueueEntry::PxQueueStatus_Empty);
		QueueProc.deleteQueue(del_list[i].m_QueueID);
		
	}
	return true;
}
bool CQueueDBProc::restoreSendQueueList(const std::vector<QueuViewItem> &status_list,const std::vector<QueuViewItem> &priority_list)
{
	CPxWorkQueue QueueProc;
	QueueProc.initRes();

	{
		int size = status_list.size();
		for(int i=0;i<size;i++){
			QueueProc.changeStatus(status_list[i].m_QueueID,CPxQueueEntry::PxQueueStatus_Standby);
		}
	}
	//
	{
		int size = priority_list.size();
		for(int i=0;i<size;i++){
			QueueProc.changePriority(priority_list[i].m_QueueID,CPxQueueEntry::PxQueuePriority_Default,
									CPxQueueEntry::PxQueueStatus_Standby,
									0/*RetryCount*/);
		}
	}

	return true;
}
bool CQueueDBProc::deleteResultQueueList(const std::vector<QueuViewItem> &del_list)
{
	CPxResultQueue ResultQueueProc;
	ResultQueueProc.initRes();

	int size = del_list.size();
	for(int i=0;i<size;i++){
	//	ResultQueueProc.changeStatus(del_list[i].m_QueueID,CPxQueueEntry::PxQueueStatus_Empty);
		ResultQueueProc.deleteQueue(del_list[i].m_QueueID);
	}
	return true;
}
bool CQueueDBProc::resendResultQueueList(const std::vector<QueuViewItem> &del_list)
{
	 
	CPxWorkQueue QueueProc;
	QueueProc.initRes();
 
	int size = del_list.size();
	for(int i=0;i<size;i++){
		QueueProc.resendQueue(del_list[i].m_QueueID);
		 
	}

	return true;
}