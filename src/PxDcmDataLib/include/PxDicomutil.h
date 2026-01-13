/***********************************************************************
 * FxDICOMUtil.h
 *---------------------------------------------------------------------
 *   
 *-------------------------------------------------------------------
 */
#ifndef FX_DICOM_UTIL_H
#define FX_DICOM_UTIL_H

#include <vector>
#include <map>
#include "RTVDICOMDef.h"

/////////////////////////////////////////////////////////////////////

/////////////////////////////////////////////////////////////////////

#define BINARY_READ		"rb" // Not "r" otherwise fread() will not work.

//-----------------------------------------------------------------------------
//
//Following variables added for DICOMDIR Suppport
#define PATH_LENGTH		100					/* Maximum length of pathname to place DICOMDIR into */
#define FSUAETITLE		"TR_DCMDIR_FSU"      /* AE title when writting to DICOM media */
#define UI_LENGTH		64					/* Maximum length of UI VR DICOM attributes + padding chars */
#define DIRECTORY_SLASH_STRING "\\"

enum
{
	kCBERRSuccess   = 0,
	kCBERROpenFile  = 1,
	kCBERRReadFile  = 2,
	kCBERRCloseFile = 4,
	kCBERRBadArg    = 8
};

//-----------------------------------------------------------------------------
//
typedef struct CALLBACKINFO
{
	FILE* fp;
	int messageID;
	char* serviceName;
	char prefix[30];
	unsigned long dataSize;

	CALLBACKINFO() { fp = 0; dataSize = 0; } // 09/26/2002 T.C. Zhao
	~CALLBACKINFO() { if (fp) { fclose(fp); fp = 0; }}
} CBinfo;

//-----------------------------------------------------------------------------
//
typedef struct MEDIA_CALLBACKINFO
{
    FILE*         fp;
    char          buffer[16*1024];
    unsigned long bytesRead;
	unsigned long dataSize;
	int			  errCode;

	MEDIA_CALLBACKINFO() { fp = 0; dataSize = 0; errCode = 0;}	// 09/26/2002 T.C. Zhao
	~MEDIA_CALLBACKINFO() { if (fp) { fclose(fp); fp = 0; }}
} MediaCBinfo;

//-----------------------------------------------------------------------------
//
typedef struct	MEMORY_CALLBACKINFO
{
	const char *  memoryPointerForRead;
	char *		  memoryPointerForWrite;
    char          buffer[16*1024];
    unsigned long bytesRead;
	unsigned long dataSize;

	MEMORY_CALLBACKINFO() { }		//8.11.2004, S.H.
	~MEMORY_CALLBACKINFO() { }
} MemoryCBinfo;

//
//-----------------------------------------------------------------------------
//
//  DICOMDIR Structure defination (Added for DICOMDIR Support)

typedef struct dicomdirinfo
{
    int   dirID;                    /* ID of the DICOMDIR object */
    int   entityID;                 /* ID of the current directory entity */
    int   recID;                    /* ID of the current directory record */
    int   nextRecID;                /* ID of the next directory record */
    int   lowerRecID;				/* ID of the first record in the lower level entity */
    char* type;						/* pointer to a  string with the current record type */
    int   direxists;				/* TRUE if the dicomdir already exists on media */
    int   memory;                   /* TRUE if dicomdir saved in memory */
    char  pathname[PATH_LENGTH];    /* path the dicomdir is to be located */
    char  dirname[PATH_LENGTH+10];  /* length of DICOMDIR name + slash */
    int   fsuappID;                 /* ID of the application */
    int   scpappID;                 /* ID of the application */
    int   count;                    /* Number of elements in a DICOMDIR */
    int   entityCount;              /* Number of elements in an entity */
} TRDicomDirInfo;

//-----------------------------------------------------------------------------
/*
 *  Enum used when filling in attributes within a directory record.  The
 *  enum specifies value types of the attributes being set in the DICOMDIR
 *  to know if a failure has occured.
 */
typedef enum
{
    Type_1 = 1,
    Type_2,
    Type_3
} DICOM_TYPE;

//-----------------------------------------------------------------------------
//
class TRLogger;
class  TRDICOMUtil
{
//DICOMDIR Relevant variables 

public:
	
#if 0
	static bool InitialDICOM(const char* iLicense, int iLogLevel=0, const char* iLogFile=0,
								  int iLogMaxSize=0, const char* iCaptureFile=0, int iNumCaptureFiles=2);
#else
	static bool InitialDICOM(const char* iLicense, int iLogLevel=0, TRLogger *LoggerFile=0,
								  int iLogMaxSize=0, const char* iCaptureFile=0, int iNumCaptureFiles=2);
#endif
	//#8 2012/03/09 K.KO
	//ServiceListŽw’èŽd‘g‚Ý‚Ì’Ç‰Á
	static bool initServiceList(const char *ServiceName,bool isPropose=false);
	static bool addSOPClassUID(const char * sopUID,const char *ServiceName,bool isPropose=false);

	static void ReleaseDICOM();

	static int AddGroup2Elements(
		int               A_fileID,
		int				  A_transSyntax, 
		const char*		  localAE = "");


	static int SendResponseMessage(
		int iRespStatus, 
		int iAssociationID);

	//	Rob Lewis - 05/22/02
	//	iLogLevel has the following meaning:
	//		0 -> Errors
	//		1 -> Errors + Warnings
	//		2 -> Errors + Warnings + Info
	//		3 -> Errors + Warnings + Info + Debug
	//		4 -> Errors + Warnings + Info + Debug + Trace
	static void SetLogBits(int iLogLevel, int iRotateLogs = 1, int iNumLogs = 2);

	static int EnableNetworkCapture(char* iCapFile, 
									      bool iRewrite = 1, 
									      int iNumFiles = 2, 
									      long iMaxSize = 204800);

	static int DisableNetworkCapture();

	static bool AssociationIsDead(int iStatus);

	static void DumpMessage(int iMsgID, const char* iService, int iCommand, const char* iRemoteAE = 0);

	static bool IsMainLevelObject(int iMessageID);

	static int ExtractTransferSyntax(int iMsgID, int& oTransferSyntax, bool& oIsCompressed);

	static int GetPixelDataFileOffset(int iMessageID, const char* iFilename, int iSyntax, unsigned long& oPixelDataSize, unsigned long& oOffset);

	static int GetPixelDataFileOffset(const char* iFileName /*DCM File only*/, unsigned long& oPixelOffset, unsigned long& oPixelSize, bool& isCompressed);

	static int GetPixelDataFileOffset(int iMessageID, unsigned long& oPixelOffset, unsigned long& oPixelSize, bool& isCompressed);

	static int GenerateServiceList(const char* iSOPClassUID, int iStoredSyntax, int iProposedSyntax, std::string& oServiceListName);

	static const char* GetVersionString(void);

	static bool GetTagName(unsigned long iTag, std::string& oName);//#49


	static std::string CalculateLocalAETitle(void);

	static std::string CalculateInboundLocalAETitle(void);

	//Added by Sumeet Khandelwal for DICOMDIR support
	static int CreateDICOMDIR(std::string iRootDirPath, std::string iDICOMDirPath, 
								std::map<std::string, std::string> mangledFileNameMap);
	static bool IsDICOMDirMessage( int iMessageID );
};
#endif // FX_DICOM_UTIL_H