/*----------------------------------------------------------------------
 * FxDicomServer.h
 *---------------------------------------------------------------------
 *----------------------------------------------------------------------*/
#ifndef FX_DICOM_SERVER_H
#define FX_DICOM_SERVER_H

#pragma warning(disable: 4251) 


#define kMaxLocalAELen (16 + 4)
#define kMaxHostNameLen 128
#define kMaxPathNameLen 512
#define kInvalidAssociationID -1



#include <list>

//const char* kDefaultLocalAETitle = "VPNET";

// made dicomserver a proper class TC Zhao Dec. 2001
class DicomServer
{
public:
	DicomServer(const char* AE=0, const char* Host=0, int iPort=105)
	{
		strncpy(m_applicationEntityTitle, AE ? AE:"",kMaxLocalAELen);
		strncpy(m_hostname, Host?Host:"", kMaxHostNameLen);
		m_applicationEntityTitle[kMaxLocalAELen-1] = '\0';
		m_hostname[kMaxHostNameLen-1] = '\0';
		m_port = iPort;
		m_associationID = -1;
	}

	~DicomServer(void) {}
	char	m_applicationEntityTitle[kMaxLocalAELen];
	char	m_hostname[kMaxHostNameLen];
	int		m_port;
	int		m_associationID;
	float	m_transferRate;		// Avg (mean) number of MB transferred / minute during last retrieve

	bool operator==(const DicomServer& B)
	{
		return strcmp(m_applicationEntityTitle, B.m_applicationEntityTitle) == 0 &&
			   _stricmp(m_hostname, B.m_hostname) == 0 && m_port == B.m_port;
	}

	//	RL 12/12/01
	DicomServer& operator=(const DicomServer& iServer)
	{
		if (this != &iServer)
		{
			strncpy(m_applicationEntityTitle, iServer.m_applicationEntityTitle, kMaxLocalAELen);
			strncpy(m_hostname, iServer.m_hostname, kMaxHostNameLen);
			m_applicationEntityTitle[kMaxLocalAELen-1] = '\0';
			m_hostname[kMaxHostNameLen-1] = '\0';
			m_port = iServer.m_port;
			m_associationID = iServer.m_associationID;
		}
		return *this;
	}

};

typedef std::list<DicomServer> DicomServerList;
typedef DicomServerList::iterator DicomServerListIter;

typedef enum 
{
	kUnknownSource = 0, // moved Unknown to 0 (was 3) so empty datasource is unknown (TC 9/30/00)
	kDicomServer,
	kLocalFile
} eDataSourceType;



class  DicomDataSource 
{
public:
	DicomDataSource(const DicomServer& iS) {SetServer(iS);}
	DicomDataSource(void) { type = kUnknownSource; m_fileName[0] = '\0';}
	void SetServer(const DicomServer& iS) 
	{ 
		m_server = iS; 
		// T.C. Zhao 2005.10.19 Source is now much more complex, can't assume it's always good
		type = (iS.m_applicationEntityTitle[0]&& iS.m_port>0) ? kDicomServer:kUnknownSource;
	}

	eDataSourceType type;
	char m_fileName[kMaxPathNameLen];
	DicomServer m_server;
};

// add Jan 2002 T.C. Zhao
class VLICachedInfo 
{
public:
	VLICachedInfo(const DicomDataSource &iSource) { m_dataSource = iSource; strcpy(m_modality,"?");}
	void	SetModality(const char* modality)	 { strncpy(m_modality,  modality, sizeof m_modality -1); m_modality[sizeof m_modality -1] = '\0';}
	void	SetDataSource(const DicomDataSource &iSource) 
			{ 
				m_dataSource.type = iSource.type; 
				strncpy(m_dataSource.m_fileName, iSource.m_fileName, sizeof m_dataSource.m_fileName -1);
				m_dataSource.m_server = iSource.m_server;
			}
	DicomDataSource& GetDataSource() { return m_dataSource; }

public:
	DicomDataSource	m_dataSource;
	char			m_modality[8];
};

#endif // FX_DICOM_SERVER_H