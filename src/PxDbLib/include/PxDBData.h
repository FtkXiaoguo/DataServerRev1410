/***********************************************************************
 *---------------------------------------------------------------------
 *-------------------------------------------------------------------
 */
#ifndef	__PX_FXDBData_h__
#define	__PX_FXDBData_h__
#pragma warning (disable: 4786)
#pragma warning (disable: 4503)

#include <vector>

#include "RTVDICOMDef.h"

#define USE_HEADER_DIRECT  // do not use dll
/////////////////////////////////////////////////////////////////////
#ifdef USE_HEADER_DIRECT
#define FxDefDllCls_FxDbDataLib 
#else
#ifdef MakeDll_FxDbLib
	#define FxDefDllCls_FxDbDataLib __declspec(dllexport)
#else 
	#define FxDefDllCls_FxDbDataLib __declspec(dllimport)
#endif
#endif
/////////////////////////////////////////////////////////////////////

#define ASTRNCPY(t, s) {strncpy(t, s, sizeof(t)); t[sizeof(t)-1] = 0;}

const enum {
	kStudyLevel		= (1<<0),
	kSeriesLevel	= (1<<1),
	kInstanceLevel	= (1<<2)
};

const enum {
   // deals with read/unread status 
	kDBIsUnknown		= 0,
	kDBIsUnread			= (1<<0),
	kDBIsPartiallyRead	= (1<<1),
	kDBIsRead			= (1<<2),
	kDBIsHiden			= (1<<3),
	kDBAllVisable		= (kDBIsUnread | kDBIsRead | kDBIsPartiallyRead ),
	kDBAll				= (kDBIsUnread | kDBIsRead | kDBIsPartiallyRead | kDBIsHiden)
};

//enum for instance level status
const enum {
	kImageFormatInDCM	= 0,
	kImageFormatInDCP	= (1<<0),
	kImageFormatError	= (1<<1),
	kImageProcessError	= (1<<2)
};


//enum for prebuild types
const enum eAqObjectType{
	kTypeNull			= 0,
	kTypeImageServer	= 1,
	kTypeDICOMServer	= 2,
	kTypeDatabaseServer	= 3,
	kTypeStorageServer	= 4,
	kTypeWebServer		= 5,
	kTypeProcessServer	= 6,
	kTypeClient			= 7,
	kTypeLocalAE		= 8,
	kTypeRemoteAE		= 9,
	kTypePrinter		= 10,
	kTypeDomain			= 11,
	kTypeOrganization	= 12,
	kTypeUserGroup		= 13,
	kTypeUserAccount	= 14
};

struct AqObjectType
{
	AqObjectType(){Clear();};
	~AqObjectType(){};

	void Clear() {memset(this, 0, sizeof(*this));};

	int		m_ID;
	char	m_TypeName[64+1]; // UNIQUE
	char	m_ViewName[64+1];
	char	m_Description[256+1];
};



struct AqObjectInterface
{

	AqObjectInterface(){};
	virtual ~AqObjectInterface(){};
	
	virtual void		Clear()= 0;
	virtual int&		GetID() =0;

	virtual int			GetType() const =0;
	virtual void		SetType(int iValue) {};

	virtual const char*	GetEntityName() const =0;
	virtual void		SetEntityName(const char* iValue) =0;

	virtual const char*	GetFullName() const =0;
	virtual void		SetFullName(const char* iValue) =0;

	virtual const char*	GetHostname() const =0;
	virtual void		SetHostname(const char* iValue) =0;

	virtual const char*	GetAddress() const =0;
	virtual void		SetAddress(const char* iValue) =0;

	virtual int			GetPort() const =0;
	virtual void		SetPort(int iValue) {};

	virtual const char*	GetDomainName() const =0;
	virtual void		SetDomainName(const char* iValue) {};

	virtual const char*	GetDescription() const =0;
	virtual void		SetDescription(const char* iValue) {};
	
	virtual bool	HasValidLink() const =0;
};


struct AqObject : public AqObjectInterface
{
	AqObject(){Clear();};
	virtual ~AqObject(){};

	virtual void Clear() {
		m_ID = 0;
		m_Type = 0;
		m_EntityName[0] = 0;
		m_FullName[0] = 0;
		m_Hostname[0] = 0;
		m_Address[0] = 0;
		m_Port = 0;
		m_DomainName[0] = 0;
		m_Description[0] = 0;
	};

	virtual int&		GetID() {return m_ID;}
	
	virtual int			GetType() const {return m_Type;};
	virtual void		SetType(int iValue) {m_Type=iValue;};

	virtual const char*	GetEntityName() const {return m_EntityName;};
	virtual void		SetEntityName(const char* iValue) {ASTRNCPY(m_EntityName, iValue);};

	virtual const char*	GetFullName() const {return m_FullName;};
	virtual void		SetFullName(const char* iValue) {ASTRNCPY(m_FullName, iValue);};


	virtual const char*	GetHostname() const {return m_Hostname;};
	virtual void		SetHostname(const char* iValue) {ASTRNCPY(m_Hostname, iValue);};


	virtual const char*	GetAddress() const {return m_Address;};
	virtual void		SetAddress(const char* iValue) {ASTRNCPY(m_Address, iValue);};

	virtual int			GetPort() const {return m_Port;};
	virtual void		SetPort(int iValue) {m_Port = iValue;};

	virtual const char*	GetDomainName() const {return m_DomainName;};
	virtual void		SetDomainName(const char* iValue) {ASTRNCPY(m_DomainName, iValue);};

	virtual const char*	GetDescription() const {return m_Description;};
	virtual void		SetDescription(const char* iValue) {ASTRNCPY(m_Description, iValue);};
	
	
	virtual bool	HasValidLink() const {return (m_ID > 0 || (
		m_ID == 0 && m_Type == 0 && m_EntityName[0] == 0));};

	int		m_Type;
	char	m_EntityName[64+1];
	char	m_FullName[128+1];
	char	m_Hostname[64+1];
	char	m_Address[64+1];
	int		m_Port;
	char	m_DomainName[128+1];
	char	m_Description[256+1];
	//CONSTRAINT AqObject_Unique UNIQUE(Type, EntityName, FullName, Hostname, Address, Port, DomainName)

private:
	int		m_ID;

};


//enum for prebuild types
const enum eLogAction{
	kDoLogin	= 1,
	kDoLogout	,
	kDoCreate	,
	kDoDelete	,
	kDoReject	,
	kDoReceive	,
	kDoSend		,
	kDoRead		,
	kDoWrite	,
	kDoJoin		,
	kDoLeave	,
	kDoStart	,
	kDoPause	,
	kDoStop		,
	kDoCancel	,
	kDoResume	,
	kDoChange	,
	kDoAnonymize,
	kDoExport	,
	kDoImport	,
	kDoAssign	,
	kDoUnassign	,
	kDoFilm		,
	kDoPrint	,
	kDoRetrieve

};

struct Actions
{
	Actions(){Clear();};
	~Actions(){};

	void Clear() {memset(this, 0, sizeof(*this));};

	int		m_ID;
	char	m_ActionName[64+1];
	char	m_Description[256+1];
	//CREATE UNIQUE INDEX ActionName_index on Actions(ActionName)
};

const enum LogEventType {
	kSystemEventLog	= 0,
	kSeriesEventLog	= 1,
	kStudyEventLog	= 2
} ;


struct EventLog
{
	EventLog(){Clear();};
	~EventLog(){};

	void Clear() {memset(this, 0, sizeof(*this));};

	int		m_ID;
	int		m_Actor;
	int		m_Activity;
	int		m_ActOn;
	int		m_Requestor;
	int		m_ActionFrom;
	int		m_ActionAt;
	int		m_TransferTo;
	double	m_TimeOfAction;
	char	m_Description[256+1];
	int		m_Status; //0 means normal, otherwise anomal and should put detail in description

	LogEventType m_eventType;
};


struct DICOMData
{
	DICOMData(){Clear();};
	~DICOMData(){Clear();};

	void Clear() {memset(this, 0, sizeof(*this));};

	char	m_studyInstanceUID[ kVR_UI ];		
	char	m_patientsName[ kVR_PN ];			
	char	m_patientID[ kVR_LO ];				
	char	m_patientsBirthDate[ kVR_DA ];		
	char	m_patientsSex[ kVR_CS ];			
	long	m_patientsAge;
	char	m_studyDate[ kVR_DA ];				
	char	m_studyTime[ kVR_TM ];				
	char	m_accessionNumber[ kVR_SH ];		
	char	m_studyID[ kVR_SH ];				
	char	m_radiologistName[ kVR_PN ];
	char	m_referringPhysiciansName[ kVR_PN ];
	char	m_modalitiesInStudy[ kVR_CS ];		
	char	m_studyDescription[ kVR_LO ];		
	long	m_numberOfStudyRelatedSeries;
	long	m_numberOfStudyRelatedInstances;
	char	m_characterSet[ 257 ];
	long	m_accessTime;
	
	char	m_seriesInstanceUID[ kVR_UI ];		
	long	m_seriesNumber;						
	char	m_seriesDescription[ kVR_LO ];		
	char	m_modality[ kVR_CS ];				
	char	m_bodyPartExamined[ kVR_CS ];		
	char	m_viewPosition[ kVR_CS ];			
	long	m_numberOfSeriesRelatedInstances;
	char	m_stationName[ kVR_SH ];
	int		m_offlineFlag;
	int		m_IsQRData;
	long	m_seriesModifyTime;
	long	m_seriesHoldToDate;
	char	m_seriesDate[ kVR_DA ];
	char	m_seriesTime[ kVR_TM ];
	

	// instance level
	char m_SOPInstanceUID[kVR_UI];
	char m_SOPClassUID[kVR_UI];
	
	int	m_transferSyntax;

	int m_instanceNumber;
	short m_rows;
	short m_columns;
	short m_numberOfFrames;
	
	char m_imageTypeTokens[kVR_UI];
	char m_bitsAllocated;
	char m_bitsStored;
	char m_highBit;
	char m_pixelRepresentation;
	char m_photometricInterpretation; // (use enum)
	short m_planarConfiguration; 

	float m_windowWidth;
	float m_windowCenter;
	int m_smallestPixelValue;
	int m_largestPixelValue;
	char m_samplesPerPixel;

	float m_pixelSpacing[2];
	float m_aspectRatio;
	double m_rescaleSlope;
	double m_rescaleIntercept;
	char m_patientOrientation[ kVR_CS ];
	float m_slicePosition;
	float m_sliceThickness;
	float m_imagePosition[3];
	double m_imageOrientation[6];

	int m_pixelOffset;
	int m_dataSize;
	char m_referencedSOPInstanceUID[kVR_UI];

	int     m_readStatus;			// can be used as mask if not -1

	// In output, it stores the query level ID such as StudylevelID, seriesLevelID, 
	// and InstanceLevelID. In input as filter, it is used as constrain for start ID 
	// If it is 0, no constrain applied.
	int m_recordID;
	
	//If it is not 0, it will be used as top records constrain.
	//User need to check the returned record size to decide if to make another
	// call to get more records.

	unsigned long m_maxRecords;

	int m_studyIndex;
	int m_seriesIndex;

	char m_imageDate[ kVR_DA ];
	char m_imageTime[ kVR_TM ];
	char m_wasLossyCompressed;

	char m_scanOptions[kVR_CS];
	char m_manufacturer[kVR_LO];

};


struct DICOMPatient
{
	DICOMPatient(){Clear();};
	~DICOMPatient(){Clear();};

	void Clear() {memset(this, 0, sizeof(*this));};

	char	m_patientsName[ kVR_PN ];				
	char	m_patientID[ kVR_LO ];					
	char	m_patientsBirthDate[ kVR_DA ];			
	char	m_patientsSex[ kVR_CS ];				
};

struct DICOMStudy
{
	DICOMStudy(){Clear();};
	~DICOMStudy(){Clear();};

	void Clear() {memset(this, 0, sizeof(*this));};

	char	m_studyInstanceUID[ kVR_UI ];			
	char	m_patientsName[ kVR_PN ];				
	char	m_patientID[ kVR_LO ];					
	char	m_patientsBirthDate[ kVR_DA ];			
	char	m_patientsSex[ kVR_CS ];				
	long	m_patientsAge;
	char	m_studyDate[ kVR_DA ];					
	char	m_studyTime[ kVR_TM ];					
	char	m_accessionNumber[ kVR_SH ];			
	char	m_studyID[ kVR_SH ];
	char	m_radiologistName[ kVR_PN ];
	char	m_referringPhysiciansName[ kVR_PN ];	
	char	m_modalitiesInStudy[ kVR_CS ];			
	char	m_studyDescription[ kVR_LO ];			
	long	m_numberOfStudyRelatedSeries;
	long	m_numberOfStudyRelatedInstances;
	char	m_characterSet[ 257 ];
	int		m_status;
	
};

struct DICOMSeries
{
	DICOMSeries(){Clear();};
	~DICOMSeries(){Clear();};

	void Clear() {memset(this, 0, sizeof(*this));};

	char	m_studyInstanceUID[ kVR_UI ];
	char	m_seriesInstanceUID[ kVR_UI ];			
	long	m_seriesNumber;							
	char	m_seriesDescription[ kVR_LO ];			
	char	m_modality[ kVR_CS ];					
	char	m_bodyPartExamined[ kVR_CS ];			
	char	m_viewPosition[ kVR_CS ];				
	long	m_numberOfSeriesRelatedInstances;
	char	m_stationName[ kVR_SH ];
	int		m_offlineFlag;
	int		m_IsQRData;
	long	m_seriesModifyTime;
	long	m_seriesHoldToDate;
	int		m_status;
	// -- 2006.02.01
	char	m_seriesDate[kVR_DA];
	char	m_seriesTime[kVR_TM];

};

struct DICOMInstance
{
	DICOMInstance(){Clear();};
	~DICOMInstance(){};

	void Clear() {memset(this, 0, sizeof(*this));};

	char m_studyInstanceUID[ kVR_UI ];
	char m_seriesInstanceUID[ kVR_UI ];
	char m_SOPInstanceUID[ kVR_UI ];				
	char m_SOPClassUID[ kVR_UI ];				
	int	 m_instanceNumber;			
	char m_imageTypeTokens[ kVR_UI ];					

};
	
struct DICOMInstanceX
{
	DICOMInstanceX(){Clear();};
	~DICOMInstanceX(){};

	void Clear() {memset(this, 0, sizeof(*this));};

	char m_studyInstanceUID[ kVR_UI ];
	char m_seriesInstanceUID[ kVR_UI ];
	
	char m_SOPInstanceUID[kVR_UI];
	char m_SOPClassUID[kVR_UI];
	
	int	m_transferSyntax;

	int m_instanceNumber;
	short m_rows;
	short m_columns;
	short m_numberOfFrames;
	
	char m_imageTypeTokens[kVR_UI];
	char m_bitsAllocated;
	char m_bitsStored;
	char m_highBit;
	char m_pixelRepresentation;
	char m_photometricInterpretation; // (use enum)
	short m_planarConfiguration; 

	float m_windowWidth;
	float m_windowCenter;
	int m_smallestPixelValue;
	int m_largestPixelValue;
	char m_samplesPerPixel;

	float m_pixelSpacing[2];
	float m_aspectRatio;
	double m_rescaleSlope;
	double m_rescaleIntercept;
	char m_patientOrientation[ kVR_CS ];
	float m_slicePosition;
	float m_sliceThickness;
	float m_imagePosition[3];
	double m_imageOrientation[6];

	int m_pixelOffset;
	int m_dataSize;
	char m_referencedSOPInstanceUID[kVR_UI];

	int		m_status;

	char m_imageDate[ kVR_DA ];
	char m_imageTime[ kVR_TM ];
	char m_wasLossyCompressed;

	char m_scanOptions[kVR_CS];
	char m_manufacturer[kVR_LO];
};


struct StudyInfo
{
	StudyInfo(){Clear();};
	~StudyInfo(){Clear();};

	void Clear() {memset(this, 0, sizeof(*this));};

	char	m_studyInstanceUID[ kVR_UI ];			
	char	m_patientsName[ kVR_PN ];				
	char	m_patientID[ kVR_LO ];					
	char	m_patientsBirthDate[ kVR_DA ];			
	char	m_patientsSex[ kVR_CS ];				
	long	m_patientsAge;
	char	m_studyDate[ kVR_DA ];					
	char	m_studyTime[ kVR_TM ];					
	char	m_accessionNumber[ kVR_SH ];			
	char	m_studyID[ kVR_SH ];					
	char	m_referringPhysiciansName[ kVR_PN ];	
	char	m_modalitiesInStudy[ kVR_CS ];			
	char	m_studyDescription[ kVR_LO ];			
	long	m_numberOfStudyRelatedSeries;
	long	m_numberOfStudyRelatedInstances;
	//char	m_characterSet[ kVR_CS ];				

	int     m_readStatus;
	int		m_hasAuxData;

};


struct SeriesDisplayInfo
{
	SeriesDisplayInfo() { Clear();};
	~SeriesDisplayInfo() {Clear();};
	
	void Clear() {memset(this, 0, sizeof(*this));};

	char m_studyInstanceUID[ kVR_UI ];			
	char m_patientsName[ kVR_PN ];				
	char m_patientID[ kVR_LO ];					
	char m_patientsBirthDate[ kVR_DA ];			
	char m_patientsSex[ kVR_CS ];				
    char m_studyDate[ kVR_DA ];					
	char m_studyID[ kVR_SH ];					
	char m_seriesInstanceUID[ kVR_UI ];		
	long m_seriesNumber;						
	char m_seriesDescription[ kVR_LO ];		
	char m_modality[ kVR_CS ];				
	long m_numberOfSeriesRelatedInstances;
	int	 m_offlineFlag;
    int  m_readStatus;
	long m_seriesModifyTime;
	int m_daysToKeep;
	char m_seriesDate[kVR_DA];
	char m_seriesTime[kVR_TM];
	char m_studyTime[kVR_TM];//2014/08/21 added by K.Ko

}; 

//-----------------------------------------------------------------------------
//
struct PresentationContext
{
	PresentationContext() { Clear();};
	~PresentationContext() {};
	
	void Clear() {memset(this, 0, sizeof(*this));};

	char m_sopClassUID[ kVR_UI ];
	int m_transferSyntax;
};

//-----------------------------------------------------------------------------
//
struct AuxReference
{
	AuxReference() { Clear();};
	~AuxReference() {};
	
	void Clear() {memset(this, 0, sizeof(*this));};

    char m_referencedSeriesInstanceUID[ kVR_UI ];
    char m_referencedStudyInstanceUID[ kVR_UI ];
	char m_volumeID[kVR_UI];
};

inline bool operator ==(const AuxReference& s1, const AuxReference& s2)	{ return memcmp(&s1, &s2, sizeof AuxReference) == 0; }
inline bool operator !=(const AuxReference& s1, const AuxReference& s2)	{ return memcmp(&s1, &s2, sizeof AuxReference) != 0; }




//-----------------------------------------------------------------------------
//
struct AuxDataInfo
{
	enum
	{
		kNone					= 0,
		kTemplate				= (1 << 0),
		kScene					= (1 << 1),
		kCaScore				= (1 << 2),
		kCaReport				= (1 << 3),
		kInteractiveReport		= (1 << 4),
		kCustom					= (1 << 5),
		kNetScene				= (1 << 6),
		kFindings				= (1 << 7),

		// COF Format (tcz 2005.11.04)
		kParametricMapEnhancingRatio		= (1 << 8),
		kMask					= (1 << 9),
		kCenterline				= (1 << 10),
		kCandidates				= (1 << 11),
		kRigidTransform			= (1 << 12),
		kIsAux					= (1 << 13),  // this should not be changed
		kParametricMapUptake	= (1 << 14),

		kCustom3rdParty1		= (1 << 15), // -- 2006.05.04

		kResampledVolume			= (1 << 16), // SH, 2006.09.25
	
		// sentinel (zhao aug-15-2002)
		kAuxBits				= 30
	};

	AuxDataInfo() { Clear();};
	~AuxDataInfo() {};

	void Clear() {memset(this, 0, sizeof(*this));};

	//	Describes this binary data object
	char m_auxStudyInstanceUID[ kVR_UI ];		// Which study?
	char m_auxSeriesInstanceUID[ kVR_UI ];		// Which series?
	char m_auxSOPInstanceUID[ kVR_UI ];			// Which file did it come from?
	int m_type;									// What kind of binary data?
	char m_name[ kVR_UI ];						// What's it called?
	char m_auxSeriesDate[ kVR_DA ];				// YYYYMMDD
	char m_auxSeriesTime[ kVR_TM ];				// HHMMSS
	char m_subtype[kVR_UI];						// old m_typeString. Same as anatomy
	// for the new COF tcz 2005.11.07
	char m_processType[kVR_UI];
	char m_processName[kVR_UI];
	char m_volumesHash[kVR_UI];
	char m_parameterHash[kVR_UI];
	char m_volumeID[kVR_UI];					// volume ID from PrivateDataReference

	int m_key;									// Primary key from database
};

struct PatientAuxDataInfo
{
	DICOMStudy	m_study;
	DICOMSeries	m_series;
	AuxDataInfo	m_auxdata;

};


/********************************************************************/
/* MeidaPoint Data class                                            */
/********************************************************************/
struct MediaPoint
{
	MediaPoint(){Clear();};
	~MediaPoint(){};

	void Clear() {memset(this, 0, sizeof(*this));};

	char	m_mediaPoint[256+1];
	char	m_mediaLabel[256+1];
	char	m_mediaType[16+1];
	int		m_highWaterMark;
	int		m_lowWaterMark;
	int		m_ID;

};


// This kind class uses memset(this, 0, sizeof(*this) to clear member variables.  It only works
// for class that has no virtual table.
struct Organization
{
	Organization(){Clear();};
	~Organization() {};

	void Clear() {memset(this, 0, sizeof(*this));};

	int     m_organizationUID;
	char	m_name[128+1];
	char	m_address[256+1];
	char	m_phone[32+1];
	char	m_fax[32+1];
	char	m_description[128+1];

};

struct Domain
{
	Domain() {Clear();};
	~Domain() {};
	
	void Clear() {memset(this, 0, sizeof(*this));};

	int     m_domainID;
	char	m_name[128 + 1];
	char	m_description[128+1];
	int		m_organizationID;
	int		m_type;
};

struct UserAccount : public AqObjectInterface
{
	UserAccount(){Clear();};
	virtual~UserAccount() {};

	virtual void Clear() {
		m_ID = 0;
		m_accountID = 0;;
		m_username[0] = 0;
		m_password[0] = 0;
		m_homeDirectory[0] = 0;
		m_lastName[0] = 0;
		m_middleName[0] = 0;
		m_firstName[0] = 0;
		m_title[0] = 0;
		m_address[0] = 0;
		m_city[0] = 0;
		m_state[0] = 0;
		m_zip[0] = 0;
		m_country[0] = 0;
		m_phone[0] = 0;
		m_cell[0] = 0;
		m_fax[0] = 0;
		m_pager[0] = 0;
		m_email[0] = 0;
		m_status[0] = 0;
		m_description[0] = 0;
		m_pwdExpireTime [0] = 0;
		m_lastLoginTime [0] = 0;
		m_roamingProfile = 0;
		m_loginRetry = 0; 
		m_DomainName[0] = 0;
		m_domainID = 0;
	};

	virtual int&		GetID() {return m_ID;}
	
	virtual int			GetType() const;
	//virtual void		SetType(int iValue) {m_Type=iValue;};

	virtual const char*	GetEntityName() const {return m_username;};
	virtual void		SetEntityName(const char* iValue) {ASTRNCPY(m_username, iValue);};

	virtual const char*	GetFullName() const {return m_lastName;};
	virtual void		SetFullName(const char* iValue) {ASTRNCPY(m_lastName, iValue);};

	virtual const char*	GetHostname() const {return m_firstName;};
	virtual void		SetHostname(const char* iValue) {ASTRNCPY(m_firstName, iValue);};

	virtual const char*	GetAddress() const {return m_email;};
	virtual void		SetAddress(const char* iValue) {ASTRNCPY(m_email, iValue);};

	virtual int			GetPort() const {return 0;};
	//virtual void		SetPort(int iValue) {m_Port = iValue;};

	virtual const char*	GetDomainName() const {return m_DomainName;};
	virtual void		SetDomainName(const char* iValue) {ASTRNCPY(m_DomainName, iValue);};

	virtual const char*	GetDescription() const {return m_description;};
	virtual void		SetDescription(const char* iValue) {ASTRNCPY(m_description, iValue);};

	virtual bool	HasValidLink() const {return (m_ID > 0 || (
		m_ID == 0 && m_username[0] == 0));};

	int		m_accountID;
	char	m_username[20 + 1];
	char	m_password[32 + 1];
	char	m_homeDirectory[256 + 1];	// HomeDir in DB UserAccount table
	char	m_lastName[32 + 1];
	char	m_middleName[32 + 1];
	char	m_firstName[32 + 1];
	char	m_title[64 + 1];
	char	m_address[256 + 1];
	char	m_city[64 + 1];
	char	m_state[64 + 1];
	char	m_zip[10 + 1];
	char	m_country[64 + 1];
	char	m_phone[32 + 1];
	char	m_cell[32 + 1];
	char	m_fax[32 + 1];
	char	m_pager[32 + 1];
	char	m_email[64 + 1];
	char	m_status[8 + 1];
	char	m_description[128+1];
	char	m_pwdExpireTime [kVR_DA];
	char	m_lastLoginTime [kVR_DA];
	int		m_roamingProfile ;
	int		m_loginRetry ;
	char	m_DomainName[128+1];
	int		m_domainID;

	//Unique Username, LastName, FirstName, DomainName
private:
	int m_ID;
};   

struct DomainInfo
{
	DomainInfo() {Clear();};
	~DomainInfo() {};
	Domain m_domain;
	Organization m_organization;
	
	void Clear() {memset(this, 0, sizeof(*this));};
};

struct FxDefDllCls_FxDbDataLib UserGroup : public AqObjectInterface 
{
	UserGroup() {Clear();};
	~UserGroup() {};

	void Clear() {
		m_ID = 0;
		m_groupUID = 0;
		m_domainID = 0;
		m_name[0] = 0;
		m_privilege = 0;
		m_description[0] = 0;
	};

	virtual int&		GetID() {return m_ID;}

	virtual int			GetType() const;

	virtual const char*	GetEntityName() const {return m_name;};
	virtual void		SetEntityName(const char* iValue) {ASTRNCPY(m_name, iValue);};

	virtual int			GetPort() const {return m_privilege;};
	virtual void		SetPort(int iValue) {m_privilege = iValue;};

	virtual const char*	GetDescription() const {return m_description;};
	virtual void		SetDescription(const char* iValue) {ASTRNCPY(m_description, iValue);};

	virtual const char*	GetFullName() const {return "";}
	virtual void		SetFullName(const char* iValue) {}

	virtual const char*	GetHostname() const {return "";}
	virtual void		SetHostname(const char* iValue) {}

	virtual const char*	GetAddress() const {return "";}
	virtual void		SetAddress(const char* iValue) {}

	virtual const char*	GetDomainName() const {return m_DomainName;}
	virtual void		SetDomainName(const char* iValue) {};
 

	virtual bool	HasValidLink() const {return (m_ID > 0 || (
		m_ID == 0 && m_groupUID > 0));};

	int     m_groupUID;
	int		m_domainID;
	char	m_name[64 + 1];
	int		m_privilege;
	char	m_description[128+1];
	char	m_DomainName[128+1];

private: 
	int		m_ID;
};

//----------09/29/03 junnan Wu start---------------
struct UserGroupInfo
{
	UserGroupInfo() {Clear();};
	~UserGroupInfo() {};

	void Clear() {memset(this, 0, sizeof(*this));};
	UserGroup m_userGroup;
	DomainInfo m_domainInfo;
 
};

 
struct UserAccountInfo
{
	UserAccountInfo() {Clear();};
	~UserAccountInfo() {Clear();};

	void Clear() {m_userAccount.Clear(); m_userGroupInfo.clear();};

	std::vector<UserGroupInfo> m_userGroupInfo;
	UserAccount m_userAccount;
 
};

//-----------09/29/03 junnan Wu end---------------//

struct UserGroupAssignment
{
	UserGroupAssignment() {Clear();};
	~UserGroupAssignment() {};
	
	void Clear() {memset(this, 0, sizeof(*this));};
	
	int		m_uID;
	int     m_groupID;
	int     m_accountID;
};

struct FxDefDllCls_FxDbDataLib ApplicationEntity : public AqObjectInterface
{
	ApplicationEntity() {Clear();};
	virtual ~ApplicationEntity() {};

	virtual void Clear() {
		m_ID = 0;
		m_AEID = 0;
		m_AEName [0] = 0;
		m_AETitle [0] = 0;
		m_hostName [0] = 0;
		m_IPAddress[0] = 0;
		m_port = 0;;
		m_level = 0;;
		m_priority = 0;;
		m_description [0] = 0;
		m_IsLocalAE = true;
	};

	virtual int&		GetID() {return m_ID;}

	virtual int			GetType() const;
	virtual void		SetType(int iValue);

	virtual const char*	GetEntityName() const {return m_AETitle;};
	virtual void		SetEntityName(const char* iValue) {ASTRNCPY(m_AETitle, iValue);};

	virtual const char*	GetFullName() const {return m_AEName;};
	virtual void		SetFullName(const char* iValue) {ASTRNCPY(m_AEName, iValue);};

	virtual const char*	GetHostname() const {return m_hostName;};
	virtual void		SetHostname(const char* iValue) {ASTRNCPY(m_hostName, iValue);};

	virtual const char*	GetAddress() const {return m_IPAddress;};
	virtual void		SetAddress(const char* iValue) {ASTRNCPY(m_IPAddress, iValue);};

	virtual int			GetPort() const {return m_port;};
	virtual void		SetPort(int iValue) {m_port = iValue;};

	virtual const char*	GetDomainName() const {return "";};
	//virtual void		SetDomainName(const char* iValue);

	virtual const char*	GetDescription() const {return m_description;};
	virtual void		SetDescription(const char* iValue) {ASTRNCPY(m_description, iValue);};

	virtual bool	HasValidLink() const {return (m_ID > 0 || (
		m_ID == 0 && m_AETitle[0] == 0));};


	int  m_AEID;
	char m_AEName [64 +1];
	char m_AETitle [64 +1];
	char m_hostName [128 + 1];
	char m_IPAddress[32+1];
	int  m_port;
	int  m_level;
	int  m_priority;
	char m_description [128 + 1];

	bool m_IsLocalAE;

	//Unique IsLocalAE, m_AETitle, m_hostName, m_IPAddress, m_port
private: 
	int m_ID;

};

//	-- - 10/25/04 - Changed from (1,2,4) to (0,1,2)
//		It used to be this way, so compatible w/ old database entries
const enum
{
	kCompressionNone			= 0,
	kCompressionLosslessJ2K		= 1,
	kCompressionLossyJ2K		= 2
};

struct RoutingPatternEntry
{
	RoutingPatternEntry() {Clear();};
	~RoutingPatternEntry() {};

	int m_routingPatternID;
	char m_tagFilterName[32+1];
	int m_tagFilterID;
	char m_storeTargetName[128+1];
	int m_storeTargetID;
	int m_compressionMethod;
	int m_compressionFactor;
	
	void Clear() {memset(this, 0, sizeof(*this));};
};

const enum
{
	kTagIs				= 1,
	kTagIsNot			= 2,
	kTagContains		= 3,
	kTagDoesNotContain	= 4
};

struct TagFilterRule
{
	TagFilterRule() {Clear();};
	~TagFilterRule() {};

	int m_filterID;
	unsigned long m_tag;
	int m_comparatorID;
	char m_value[64 + 1];

	void Clear() {memset(this, 0, sizeof(*this));};
};

struct TagRule
{
	TagRule() {Clear();};
	~TagRule() {};
	
	void Clear() {memset(this, 0, sizeof(*this));};

	int m_id;
	int DicomTagID;
	int ComparatorID;
	char m_value[64+1]; 
};


struct DateRange
{
	DateRange() {Clear();};
	~DateRange() {};

	char m_startDate [kVR_DA];
	char m_endDate [kVR_DA];
	int m_repeat;
	void Clear() {memset(this, 0, sizeof(*this));};
};

struct TimeRange
{
	TimeRange() {Clear();};
	~TimeRange() {};
	char m_startTime [5];	//"hhmm" 
	char m_endTime [5];		//"hhmm"
	void Clear() {memset(this, 0, sizeof(*this));};
};

struct RoutingAEInfo
{
	RoutingAEInfo() {Clear();};
	~RoutingAEInfo() {};

	ApplicationEntity  m_AE;
	int m_compressionMethod;
	int m_compressionFactor;
	
	void Clear() {m_AE.Clear(); m_compressionMethod = m_compressionFactor;};

};

struct PrinterInfo
{
	PrinterInfo() {Clear();}
	~PrinterInfo() {}

	void Clear() {memset(this, 0, sizeof(*this));}

	int  m_ID;
	char m_name[32+1];
	char m_AETitle[16+1];
	char m_IPAddress[32+1];
	int  m_port;
	int  m_color;
	char m_mediaSize[16+1];
 	char m_destType[16+1];
	char m_magType[16+1];
	char m_mediaType[16+1];
	char m_orientation[16+1];
	char m_manufacturer[32+1];
	char m_model[32+1];
	char m_location[32+1];
};

struct PrinterGroupAssignment
{
	PrinterGroupAssignment() {Clear();};
	~PrinterGroupAssignment() {};
	
	void Clear() {memset(this, 0, sizeof(*this));};
	
	int		m_uID;
	int     m_PrinterID;
	int     m_groupID;
};

struct RemoteAESpecification
{
	RemoteAESpecification() {Clear();};
	~RemoteAESpecification() {};
	
	void Clear() {memset(this, 0, sizeof(*this));};
	
	bool m_isStoreTargetAE;
	bool m_isQRSourceAE;
	bool m_isQRAllowedAE;
};

struct WebConfiguration
{
	WebConfiguration() {Clear();};
	~WebConfiguration() {};
	
	void Clear() {memset(this, 0, sizeof(*this));};

	int m_pwdExpiryInterval;
	int m_lockTimeOut;
	int m_maxloginRetry;
	int m_inactiveDaysAllowed;
	int m_sessionTimeOut;
	int m_itemsPerPage;
	int m_logItemsPerPage;
};

struct PrefetchPatternEntry
{
	PrefetchPatternEntry() {Clear();};
	~PrefetchPatternEntry() {};

	void Clear() {memset(this, 0, sizeof(*this));};
	int m_ID;
	int m_tagFilterID;
	char m_tagFilterName[32+1];
	char m_modality[64+1];
	int m_studyNotOlderThan;
	int m_dayUnitType;
	int m_maxNumberResults;
};

struct FilmingPatternEntry
{
	FilmingPatternEntry() {Clear();};
	~FilmingPatternEntry() {};

	void Clear() {memset(this, 0, sizeof(*this));};
	char m_printerName[32+1];
	char m_displayMode[16+1];
	int m_skipN;
	
};



struct DataProcessor
{
	DataProcessor() {Clear();};
	~DataProcessor() {};
	void Clear() {memset(this, 0, sizeof(*this));};

	int		m_id;
	char	m_processName[129];
	char	m_handler[129];
	char	m_description[129];
};


struct DataProcessJob
{
	DataProcessJob() {};
	~DataProcessJob() {};

	int		m_id;
	char	m_jobName[65];
	char	m_description[129];

	std::vector<DataProcessor> m_processors;
};


struct DBSizeInfo
{
	DBSizeInfo() {Clear();};
	~DBSizeInfo() {};

	void Clear() {memset(this, 0, sizeof(*this));};
	
	char m_DBName[32+1];

	double m_database_size_KB;
	double m_unallocated_space_KB;
	double m_reserved_KB;
	double m_data_size_KB;
	double m_index_size_KB;
	double m_unused_KB;
	long   m_Instance_Rows;
	double m_Instance_data_KB;
	double m_Instance_index_size_KB;

	long m_instanceFragmentWarningSize;
	long m_instanceActualSize;
	
};



#endif	/* __PX_FXDBData_h__ */
