#pragma once

#ifdef MakeDicomDBComAPI 
#define IDicomDBComDllCls __declspec(dllexport)

#else 
#define IDicomDBComDllCls __declspec(dllimport)

#endif

#define DefSimpleDicomInfoDef 1
class SimpleDicomInfoDef
{
public:
	char m_studyUID[128];
	char m_patientName[128];
	char m_patientID[128];
	//
	char m_seriesUID[128];
	int	m_studyYear;
	int m_studyMon;
	int m_studyDay;
};

namespace DicomDBComAPI
{
	class CDicomDBComSearchFolderIf
	{
	public:

		virtual bool procStart(int pos, int size) = 0;
		virtual bool procSeries(const char*StudyUID, const char*seriesUID) = 0;
		virtual bool procStudy(const char*patientName, const char*patientID, const char*StudyUID) = 0;
		virtual bool dispInfo(const char*mess) = 0;
		virtual bool procFinished(const char*Folder, int status) = 0;

	};


	class DicomDBComLogger
	{
	public:

	 
		virtual void LogMessage(const char *fmt, ...) = 0;
		virtual void LogMessage(int iLevel, const char *fmt, ...) = 0;
		 
		virtual void FlushLog(void) = 0;
	};

	class DicomDBComInterface
	{
	public:
		virtual void destroy(void) = 0;
		virtual  bool initMain(void) = 0;
		virtual  void initDBCom(void) = 0;
		virtual  bool getMediaPointList(char **MediaPointListBuf, int &size) = 0;
		virtual  void getDBName(char *strBuff, int size) = 0;
		//
		virtual  void generatStudyFolder(const char *MediaPointTopFolder, const char *StudyUID, char *strBuff, int size) = 0;
		virtual  void generatSeriesFolder(const char *StudyFolder, const char *SeriesUID, char *strBuff, int size) = 0;
		virtual  void generatDicomFile(const char *SeriesFolder, const char *InstanceUID, int imageNum, char *strBuff, int size) = 0;
		//
		virtual  bool getStudyDate(const char *DicomFile, SimpleDicomInfoDef *dcmInfo) = 0;
		virtual  bool doImportDicomfile(const char *DicomFile) = 0; 
		virtual  bool findSeriesUIDFromDB(const char *seriesUID) = 0;
		virtual  bool doConform(const char* folder, CDicomDBComSearchFolderIf *callback, bool &toCancelFlag) = 0;
	};


class IDicomDBComDllCls  CDicomDBComAPILib
{
public:
	CDicomDBComAPILib(void);
	~CDicomDBComAPILib(void);
	static void setupLogger(DicomDBComLogger *logger);
	static void getVersion(unsigned int &major, unsigned int &minor);

	/////////////
	/// @brief ビューアアプリとのインタフェースを取得し。
	///        PluginGUI中からビューアアプリにアクセスする。
	/// 
	/// @return  正常：　CPxViewerClientInfoのインタフェース, 失敗: 0
	static DicomDBComInterface *createInterface(void);

	 

};

}

