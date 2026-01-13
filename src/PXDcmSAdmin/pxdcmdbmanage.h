#pragma once

#include <string>
#include <map>
#include <vector>
 
class MediaPointInfo
{
public:
	std::string m_name;
	float m_total; //MByte
	float m_free;
};

class CPxDcmDbManage
{
public:
	CPxDcmDbManage(void);
virtual ~CPxDcmDbManage(void);
 
	 
static void initDispSpaceManager();
static bool initDB();
static void initDcmtk();
static void ReformatJapaneseDicom( const std::string  &iOrg, std::string  &oConv );
static void	ConvertSJToJCodeOnly( const std::string &iOrg,  std::string  &oConv );
static void	ConvertSJToJCodeOnlyForSQL( const std::string &iOrg,  std::string  &oConv );//#62 2013/07/30

static std::string getDBSeriesFolder(const std::string &studyUID,const std::string &seriesUID,const char *SOPInstanceUID=0);

static void RemoveSeriesFromDisk(const char* iSeriesUID, const char* iStudyUID, std::string& originalDir);
static void RemoveAllDiskFiles(std::vector<std::string>& iAllSeries, const char* studyUID, int iKeepOrphaned);


static int	CPxDcmDbManage::DeletePrivateSeries(const char* seriesUID);

static bool verifyUserAccount(const std::string &userName, const std::string &passwd);
//
static bool getMediaPointInfo(std::vector<MediaPointInfo> &MpList);
static std::string getTagName(unsigned long iTag);//#49
protected:
	 
};
 