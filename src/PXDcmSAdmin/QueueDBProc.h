#pragma once

#include "string"
#include "vector"

class CPxQueueEntry;
class QueuViewItem
{
public:
	QueuViewItem() { valid=0;};
	static bool isBrokenEntry(const QueuViewItem &entry);
	///
	int valid;

	int			m_QueueID;//for management
	int			m_Level;
	std::string m_patientName;
	std::string m_patientID;
	std::string m_studyDate;
	std::string m_dispDate;
//	std::string m_seriesTime;
	__int64		m_accessTime;
	int			m_retryCount;
	//
	std::string m_destinationAE;
	//
	std::string m_images;
	std::string m_seriesNumber;
	//
	std::string m_studyUID;
	std::string m_seriesUID;

	std::string m_JobID;
	int			m_status;
};
class CQueueDBProc  
{
public:
	CQueueDBProc(void);
	~CQueueDBProc(void);

	static void init();
 
	bool getSendQueueList(std::vector<QueuViewItem> &list);
	bool getResultQueueList(std::vector<QueuViewItem> &list,bool failedOnly=false);

	bool deleteSendQueueList(const std::vector<QueuViewItem> &del_list);
	bool deleteResultQueueList(const std::vector<QueuViewItem> &del_list);
	//
	bool resendResultQueueList(const std::vector<QueuViewItem> &del_list);
	//
	bool restoreSendQueueList(const std::vector<QueuViewItem> &status_list,const std::vector<QueuViewItem> &priority_list);

	static std::string getStlyeFileName();
	static std::string getLogFileFolder();
protected:
	static void getHomeDir();
	 bool getInfoFromMainDB(QueuViewItem &disp_entry,const CPxQueueEntry *src_entry);
	
	std::string m_DBQueueName;
static	std::string m_HomeDir;

	std::map<std::string /*seriesUID*/,QueuViewItem> m_SeriesDisplayInfoCatch;
};
