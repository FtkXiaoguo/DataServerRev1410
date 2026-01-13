// ImptDicomFile.h: CImptDicomFile クラスのインターフェイス
//
//////////////////////////////////////////////////////////////////////

#if !defined(AFX_IMPTDICOMFILE_H__3A35D1FD_20F6_4E68_AEE5_E4DE7F58F6DB__INCLUDED_)
#define AFX_IMPTDICOMFILE_H__3A35D1FD_20F6_4E68_AEE5_E4DE7F58F6DB__INCLUDED_

 
#include <string>

#define USE_LOCAL_DICOMMESSAGE

#ifdef USE_LOCAL_DICOMMESSAGE
class CMyDicomFile;
#else
class VLIDicomImage;
#endif

class TRCriticalSection;
class AQNetDB;
struct DICOMData;
class SimpleDicomInfo;
class CImptDicomFile  
{
public:
	static bool initDcmTK();
	CImptDicomFile();
	virtual ~CImptDicomFile();

	void resetResource();

	bool scanDicomFile(const std::string &fileName, bool importDB = true, TRCriticalSection *cs=0);
	bool doImportDB();
//	bool readDicomFile(const std::string &fileName);

	void setDB(AQNetDB *db) { m_db = db;};

	void resetTimeCounter();
	void reportTime();

	DICOMData	*getDICOMDATAInfo(){ return m_dbData; };

	bool getStudyDate(const std::string &DicomFile,SimpleDicomInfo *dcmInfo);
protected:
	bool CoerceSOPInstanceUID() ; // see CStore
	bool HandleTerareconSpecific();// see CStore
	bool theProcess();// see CStore

	std::string m_fileName;
#ifdef USE_LOCAL_DICOMMESSAGE
	CMyDicomFile *	  m_pImage;
#else
	VLIDicomImage*	  m_pImage;
#endif
//
	AQNetDB		*m_db;
	DICOMData	*m_dbData;

	double m_readDicomTimeSum;
	double m_saveDBTimeSum;
	double m_runCount;
 
};

#endif // !defined(AFX_IMPTDICOMFILE_H__3A35D1FD_20F6_4E68_AEE5_E4DE7F58F6DB__INCLUDED_)
