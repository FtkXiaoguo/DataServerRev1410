/***********************************************************************
 *---------------------------------------------------------------------
 *-------------------------------------------------------------------
 */


#include "PxDB.h"

#include "PXAppComLib/rtvPoolAccess.h"

#include "CheckMemoryLeak.h"

using namespace std;

//#140_search_Japanese_JIS_UTF8
#include "PxDBCharacterLib.h"
JISCharacterLib *g_JisCharLib = nullptr;
int	CPxDB::InitCharacterLib(void)
{
	if (g_JisCharLib != nullptr) return 0;
	g_JisCharLib = new JISCharacterLib;
	if (!g_JisCharLib->loadLib()){
		delete g_JisCharLib;
		g_JisCharLib = nullptr;
		return -1;
	}
	return 0;
}

static const char	cTemplateStudyUID[]	= "2.16.840.1.114053.2100.9.2";
static const int	cDriveListMax	= 16;
static const HKEY kDefaultKey       = HKEY_LOCAL_MACHINE;
//static const char* kDefaultLocation = "Software\\TeraRecon\\Aquarius\\AQNet\\1.0";
//static const char* kDefaultAQCommonLocation = "Software\\TeraRecon\\Aquarius\\Common";
static const char* kDefaultLocation = "Software\\PreXion\\PXApp\\DataServer\\1.0";//#46
static const char* kDefaultAQCommonLocation = "Software\\PreXion\\PXApp\\Common";//#46
static double cPagesperMB = -1;

char CPxDB::c_sharedServerDir[256] = "C:\\";
char CPxDB::c_dbServerName[64] = "";
static char c_dbUsername[64] = "";
static char c_dbUserPassword[64] = "";


int	 CPxDB::c_admGroupID = -1;
UserGroup CPxDB::c_publicGroup;
int  CPxDB::c_AQNetDomainID = -1;

static int	c_sharedGroupID = -1;

vector<MediaPoint> CPxDB::c_mediaPoints;

static TRCriticalSection gInitLock;

//#62 2013/07/30
inline void sqlCharReplace(AqString &str_data)
{
	int len = str_data.GetBufferSize(); 
	bool escap_flag = false;
	for(int i=0;i<(len-1);i++){
		if(str_data[i] == '['){
			escap_flag = true;
		}else{
			bool replace_flag = true;
			if(escap_flag){
				if(str_data[i+1] == ']'){
					replace_flag = false;
				}
			}
			escap_flag = false;
			//
			if(replace_flag){
				if(str_data[i] == '*'){
					str_data.SetAt('%',i);
				}else
				if(str_data[i] == '?'){
					str_data.SetAt('_',i);
				}
			}
		}
		 
	}
}
//#136 2021/01/12 N.Furutsuki unicode version
inline void sqlUCharReplace(AqUString &str_data)
{
	int len = str_data.GetBufferSize();
	bool escap_flag = false;
	for (int i = 0; i<(len - 1); i++){
		if (str_data[i] == L'['){
			escap_flag = true;
		}
		else{
			bool replace_flag = true;
			if (escap_flag){
				if (str_data[i + 1] == L']'){
					replace_flag = false;
				}
			}
			escap_flag = false;
			//
			if (replace_flag){
				if (str_data[i] == L'*'){
					str_data.SetAt(L'%', i);
				}
				else
					if (str_data[i] == L'?'){
						str_data.SetAt(L'_', i);
					}
			}
		}

	}
}
inline void sqlCharReplace1(AqString &str_data)
{
	//replace "'" -> "''"
	int len = str_data.GetBufferSize(); 
	bool escap_flag = false;

	AqString str_temp;
	for(int i=0;i<(len-1);i++){
		str_temp += str_data[i];
		if(str_data[i] == '['){
			escap_flag = true;
		}else{
			bool replace_flag = true;
			if(escap_flag){
				if(str_data[i+1] == ']'){
					replace_flag = false;
				}
			}
			escap_flag = false;
			//
			if(replace_flag){
				if(str_data[i] == 0x27){  	// "'"(シングルクォート)
					 str_temp += "'";
				}
			}
		}
		 
	}
	str_data = str_temp;
}

// object <-> ID cache support 
/*
struct UAqObject
{
	UAqObject(const AqObject& iObj)
	{
		m_Type = iObj.m_Type;
		m_EntityName = iObj.m_EntityName;
		m_FullName = iObj.m_FullName;
		m_Hostname = iObj.m_Hostname;
		m_Address = iObj.m_Address;
		m_Port = iObj.m_Port;
		m_DomainName = iObj.m_DomainName;
		m_Description = iObj.m_Description;
	};
	
	UAqObject(const ApplicationEntity& iObj)
	{
		m_Type = 0;
		m_EntityName = iObj.m_AETitle;
		m_FullName = iObj.m_AEName;
		m_Hostname = iObj.m_hostName;
		m_Address = iObj.m_IPAddress;
		m_Port = iObj.m_port;
		m_DomainName = "";
		m_Description = iObj.m_description;
	};

	UAqObject(const UserAccount& iObj)
	{
		m_Type = 0;
		m_EntityName = iObj.m_username;
		m_FullName = iObj.m_lastName;
		m_Hostname = iObj.m_firstName;
		m_Address = "";
		m_extra = iObj.m_email; // put email in m_extra to avoid it in constrain
		m_Port = 0;
		m_DomainName = iObj.m_DomainName;
		m_Description = iObj.m_description;
	};

	bool  operator < (const UAqObject& iObj) const
	{
		if(m_Type != iObj.m_Type)
			return (m_Type < iObj.m_Type);
		else if (m_EntityName != iObj.m_EntityName)
			return (m_EntityName < iObj.m_EntityName);
		else if (m_FullName != iObj.m_FullName)
			return (m_FullName < iObj.m_FullName);
		else if (m_Hostname != iObj.m_Hostname)
			return (m_Hostname < iObj.m_Hostname);
		else if (m_Address != iObj.m_Address)
			return (m_Address < iObj.m_Address);
		else if (m_Port != iObj.m_Port)
			return (m_Port < iObj.m_Port);
		else 
			return (m_DomainName< iObj.m_DomainName);
		
	}
	
	bool  operator == (const UAqObject& iObj) const
		{
			return (m_Type == iObj.m_Type &&
				m_EntityName == iObj.m_EntityName &&
				m_FullName == iObj.m_FullName &&
				m_Hostname == iObj.m_Hostname &&
				m_Address == iObj.m_Address &&
				m_Port == iObj.m_Port &&
				m_DomainName == iObj.m_DomainName);
		}
	

	//int  operator ==(const AqObject& iObj) const;	

	//int		m_ID;
	int		m_Type;
	string	m_EntityName;//char	m_EntityName[64+1];
	string	m_FullName;	//char	m_FullName[128+1];
	string	m_Hostname;	//char	m_Hostname[64+1];
	string	m_Address;	//char	m_Address[64+1];
	int		m_Port;
	string	m_DomainName;		//char	m_DomainName[128+1];
	string	m_Description; //char	m_Description[256+1];
	string	m_extra;
	//CONSTRAINT AqObject_Unique UNIQUE(Type, EntityName, FullName, Hostname, Address, Port, DomainName)
};
typedef RTVMapAccess<UAqObject, int> MapAqObjectToID;
static MapAqObjectToID gMapAqObject;

*/

typedef RTVMapAccess<string, int> MapStringToID;
static MapStringToID gMapAqObjectType;

static MapStringToID gMapActions;

/////////////////////////////////////////////////////////////////////////////
static void Crypt(TCHAR *inp, DWORD inplen)
{
    //we will consider size of sbox 256 bytes
    //(extra byte are only to prevent any mishep just in case)
    TCHAR Sbox[257], Sbox2[257];
    unsigned long i, j, t, x;

    static TCHAR  OurKey[128] = "";
	static int OurKeyLen = 0;
	if(OurKey[0] == 0)
	{
		unsigned long nlen = 98;
		GetComputerName(OurKey, &nlen);
		strcat(OurKey, "www.aqnet.com");
		OurKeyLen = strlen(OurKey);
	}

    
    TCHAR temp , k;
    i = j = k = t =  x = 0;
    temp = 0;

    //always initialize the arrays with zero
    ZeroMemory(Sbox, sizeof(Sbox));
    ZeroMemory(Sbox2, sizeof(Sbox2));

    //initialize sbox i
    for(i = 0; i < 256U; i++)
    {
//        Sbox[i] = (TCHAR)i;
		Sbox[i] = ((TCHAR)i < 0)?i +128:i;
    }

    j = 0;
    //initialize the sbox2 with our key
    for(i = 0; i < 256U ; i++)
    {
        if(j == OurKeyLen)
        {
            j = 0;
        }
        Sbox2[i] = OurKey[j++];
    }


    j = 0 ; //Initialize j
    //scramble sbox1 with sbox2
    for(i = 0; i < 256; i++)
    {
        j = (j + (unsigned long) Sbox[i] + (unsigned long) Sbox2[i]) % 256U ;
        temp =  Sbox[i];                    
        Sbox[i] = Sbox[j];
        Sbox[j] =  temp;
    }

    i = j = 0;
    for(x = 0; x < inplen; x++)
    {
        //increment i
        i = (i + 1U) % 256U;
        //increment j
        j = (j + (unsigned long) Sbox[i]) % 256U;

        //Scramble SBox #1 further so encryption routine will
        //will repeat itself at great interval
        temp = Sbox[i];
        Sbox[i] = Sbox[j] ;
        Sbox[j] = temp;

        //Get ready to create pseudo random  byte for encryption key
        t = ((unsigned long) Sbox[i] + (unsigned long) Sbox[j]) %  256U ;

        //get the random byte
        k = Sbox[t];

		if( (inp[x] ^ k) != 0 )
		    //xor with the data and done
	        inp[x] = (inp[x] ^ k);
    }    
}


#define kREGISTRYMaxPath 1024
//-----------------------------------------------------------------------------
// This function gets key set in the registry
static bool GetRegistryKey (const char* iKeyName, const char* iLocation, std::string& oKeyValue, bool iCrypt=false)
{
	HKEY	hKey;
	DWORD	size;
	DWORD	type;
	unsigned char	str[kREGISTRYMaxPath] = { 0 };

	oKeyValue = "";

	if( RegOpenKey (kDefaultKey, iLocation, &hKey) != ERROR_SUCCESS )
		return false;

	if(RegQueryValueEx (hKey, iKeyName, NULL, &type, NULL, &size) != ERROR_SUCCESS || size >= kREGISTRYMaxPath)
	{
		RegCloseKey (hKey);
		return false;
	}

	if(RegQueryValueEx (hKey, iKeyName, NULL, &type, str, &size) != ERROR_SUCCESS)
	{
		RegCloseKey( hKey );
		return false;
	}
	RegCloseKey( hKey );


	int len = strlen((char*)str);
	if(iCrypt && len > 0 && len < 256 )
	{
		Crypt((char*)str, len);
		
		char buf[256];
		unsigned long nSize = sizeof(buf)-1;
		GetComputerName(buf, &nSize);

		int lenb = strlen(buf);
		if(lenb < 3)
			lenb = 3;
		str[2+len-lenb] = 0;
		oKeyValue = (char*)(str+2);
	}
	else
		oKeyValue = (char*)str;

	return true;
}


static bool SetRegistryKey (const char* iKeyName, const char* iLocation, const char* iKeyValue, bool iCrypt=false)
{
	
	HKEY	hKey;
	int		retcd;
	DWORD	result;

	retcd = RegCreateKeyEx (kDefaultKey, iLocation, 0, "", REG_OPTION_NON_VOLATILE,
			                KEY_ALL_ACCESS, NULL, &hKey, &result);
	if( retcd != ERROR_SUCCESS )
		return false;
	

	int len = strlen(iKeyValue);
	if(iCrypt && len > 0 && len < 256 )
	{
		char buf[256];
		unsigned long nSize = sizeof(buf)-1;
		GetComputerName(buf, &nSize);

		if(strlen(buf) < 3)
			ASTRNCPY(buf, "xxx");

		AqString tStr;
		tStr.Format("%c%c%s%s", buf[0], buf[1], iKeyValue, (buf+2));
		ASTRNCPY(buf, tStr);

		len = strlen(buf);
		Crypt(buf, len);
		retcd = RegSetValueEx (hKey, iKeyName, 0, REG_SZ, (BYTE*)buf, strlen(buf));
	}
	else
	{
		retcd = RegSetValueEx (hKey, iKeyName, 0, REG_SZ, (BYTE*)iKeyValue, strlen(iKeyValue));
	}

	RegCloseKey( hKey );

	return ( retcd != ERROR_SUCCESS )?false:true;
}

//-----------------------------------------------------------------------------
// This function gets key set in the registry
static bool GetRegistryKey (const char* iKeyName, const char* iLocation, int& oKeyValue)
{
	HKEY	hKey;
	DWORD	size;
	DWORD	type;
	DWORD   value = 0;

	oKeyValue = 0;

	if( RegOpenKey (kDefaultKey, iLocation, &hKey) != ERROR_SUCCESS )
		return false;

	if(RegQueryValueEx (hKey, iKeyName, NULL, &type, NULL, &size) != ERROR_SUCCESS || type != 4)
	{
		RegCloseKey (hKey);
		return false;
	}

	if(RegQueryValueEx (hKey, iKeyName, NULL, &type, (BYTE*)&value, &size) != ERROR_SUCCESS)
	{
		RegCloseKey( hKey );
		return false;
	}
	RegCloseKey( hKey );

	oKeyValue = value;
	return true;

}

//-----------------------------------------------------------------------------
// This function gets key set in the registry
static bool SetRegistryKey (const char* iKeyName, const char* iLocation, int iKeyValue)
{
	HKEY	hKey;
	int		retcd;
	DWORD	result;

	retcd = RegCreateKeyEx (kDefaultKey, iLocation, 0, "", REG_OPTION_NON_VOLATILE,
			                KEY_ALL_ACCESS, NULL, &hKey, &result);
	if( retcd != ERROR_SUCCESS )
		return false;
	

	retcd = RegSetValueEx (hKey, iKeyName, 0, REG_DWORD, (BYTE*)&iKeyValue, sizeof (iKeyValue));

	RegCloseKey( hKey );

	return ( retcd != ERROR_SUCCESS )?false:true;

}


CPxDB::CPxDB()
{
}

CPxDB::~CPxDB()
{
}

void CPxDB::SetLogger(AqLoggerInterface* iLogger)
{
		SetAqLogger(iLogger);
}
bool CPxDB::SetWatermark(int iMediaPointID, int iHigh, int iLow)
{
	if(iMediaPointID < 0 || iMediaPointID >= 16)
		return false;


	AqString regPath;
	regPath.Format("%s\\DriveList\\%02X", kDefaultAQCommonLocation, iMediaPointID);
	if(iHigh > 0)
	{
		if(!SetRegistryKey ("DiskHighWaterMark", regPath, iHigh))
			return false;
		TRCSLock fplock(&gInitLock);
		if(iMediaPointID < c_mediaPoints.size())
			c_mediaPoints[iMediaPointID].m_highWaterMark = iHigh;

	}

	if(iLow > 0)
	{
		if(!SetRegistryKey ("DiskLowWaterMark", regPath, iLow))
			return false;
		TRCSLock fplock(&gInitLock);
		if(iMediaPointID < c_mediaPoints.size())
			c_mediaPoints[iMediaPointID].m_lowWaterMark = iLow;

	}
	
	return true;
}

int CPxDB::InitMediaPoints(bool iRedo/*=false*/)
{
	TRCSLock fplock(&gInitLock);
	if (!iRedo && c_mediaPoints.size() > 0)
		return kOK;
	std::string	result;
	int	retcd = 0;
	MediaPoint	data;
	int index = 0;
	AqString tmpStr;
	int i;


    int highWaterMarkInMB = 10000;
	int lowWaterMarkInMB = 500;
	int spaceInMB;

	if(GetRegistryKey ("DiskHighWaterMark",kDefaultLocation, spaceInMB))
		highWaterMarkInMB = spaceInMB;
		

	if(GetRegistryKey ("DiskLowWaterMark",kDefaultLocation, spaceInMB))
		lowWaterMarkInMB = spaceInMB;


	

	c_mediaPoints.resize(cDriveListMax);

	for(i=0; i<cDriveListMax; i++ )
	{
		data.m_ID = i;
        tmpStr.Format("%s\\DriveList\\%02X", kDefaultAQCommonLocation, data.m_ID);
		
		if(GetRegistryKey ("DiskHighWaterMark", tmpStr, spaceInMB))
			data.m_highWaterMark = spaceInMB;
		else
			data.m_highWaterMark = highWaterMarkInMB;

		if(GetRegistryKey ("DiskLowWaterMark", tmpStr, spaceInMB))
			data.m_lowWaterMark = spaceInMB;
		else
			data.m_lowWaterMark = lowWaterMarkInMB;


		if( !GetRegistryKey( "Label", tmpStr, result ) || result.empty())
			break;
		ASTRNCPY(data.m_mediaLabel, result.c_str());

		if( !GetRegistryKey( "Type", tmpStr, result ) || result.empty())
			break;
		ASTRNCPY(data.m_mediaType, result.c_str());

		if( !GetRegistryKey( "Path", tmpStr, result ) || result.empty())
			break;

		tmpStr = result.c_str();
		tmpStr.Replace('/', '\\');
		// Make sure you always have a trailing slash
		if(tmpStr.Right(1) != "\\" )
			tmpStr += '\\';

		ASTRNCPY(data.m_mediaPoint, tmpStr);


		c_mediaPoints[index++] = data;

	}

	c_mediaPoints.resize(index);
	if(index < 1)
	{
		GetAqLogger()->LogMessage("CPxDB::InitMediaPoints no media points found\n");
		c_mediaPoints.clear();
		return kNoResult;
	}
	
	return kOK;
}

const std::vector<MediaPoint>& CPxDB::GetMediaPoints()
{ 
	TRCSLock fplock(&gInitLock);
	if(c_mediaPoints.size() == 0)
		InitMediaPoints();

	return c_mediaPoints; 
}

const MediaPoint* CPxDB::GetMediaPoint(const char* iDir)
{ 
	TRCSLock fplock(&gInitLock);
	if(!iDir || !iDir[0])
		return 0;

	const std::vector<MediaPoint>& mpV = GetMediaPoints();
	for (int i=0; i<mpV.size(); i++)
	{
		if(strstr(iDir, mpV[i].m_mediaPoint) == iDir)
			return &(mpV[i]);
	}
	return 0;
}

const MediaPoint* CPxDB::GetMediaPoint(int iID)
{ 
	TRCSLock fplock(&gInitLock);
	const std::vector<MediaPoint>& mpV = GetMediaPoints();
	if(iID >= 0 && iID <mpV.size())
		return &(mpV[iID]);
	else
		return 0;
}

int CPxDB::GetRegDBInfo(std::string& oDBName, std::string& oSharedServerDir, std::string& oUser, std::string& oPassword)
{
	// get connection string
	
	oSharedServerDir = "C:\\";
	oDBName = "";
	oUser = "";
	oPassword = "";

	GetRegistryKey("SharedServerDir", kDefaultAQCommonLocation, oSharedServerDir );
	if(oSharedServerDir.size() < 2)
		oSharedServerDir = "C:\\";

	GetRegistryKey("DBName", kDefaultAQCommonLocation, oDBName );
	GetRegistryKey("User", kDefaultAQCommonLocation, oUser);
	GetRegistryKey("Password", kDefaultAQCommonLocation, oPassword, true );
	return kOK;
}

int CPxDB::SetRegDBInfo(const char* iDBName, const char* iSharedServerDir, const char* iUser, const char* iPassword)
{
	
	SetRegistryKey( "SharedServerDir", kDefaultAQCommonLocation, 
		(iSharedServerDir && iSharedServerDir[0])?iSharedServerDir:"C:\\" );
	SetRegistryKey( "DBName", kDefaultAQCommonLocation, (iDBName)?iDBName:"" );
	SetRegistryKey( "DBName", kDefaultAQCommonLocation, (iDBName)?iDBName:"" );
	SetRegistryKey( "User", kDefaultAQCommonLocation, (iUser)?iUser:"" );
	SetRegistryKey( "Password", kDefaultAQCommonLocation, (iPassword)?iPassword:"", true );

	return kOK;
}

bool CPxDB::InCluster()
{
	bool clusterOn = false;

	std::string dbName, sharedServerDir, user, password;
	GetRegDBInfo(dbName, sharedServerDir, user, password);

	if(!dbName.empty() || sharedServerDir != "C:\\" || !user.empty() || !password.empty())
			clusterOn = true;

	return clusterOn;

}

//InitDBServerName(const char* iDBName=0, const char* iSharedServerDir=0, const char* iUname=0, const char* iPswd=0);

void CPxDB::InitDBServerName(const char* iDBName, const char* iSharedServerDir, const char* iUname, const char* iPswd)
{
	TRCSLock fplock(&gInitLock);
	if(iDBName && iDBName[0])
	{
		if(iSharedServerDir && iSharedServerDir[0])
		{
			ASTRNCPY(c_sharedServerDir, iSharedServerDir);
		}
		else
		{
			ASTRNCPY(c_sharedServerDir, "C:\\")
		}

		if(c_dbServerName[0])
		{
			if(stricmp(c_dbServerName, iDBName) == 0)
				return;
		}

		ASTRNCPY(c_dbServerName, iDBName);

		if(iUname)
		{
			ASTRNCPY(c_dbUsername, iUname);
		}
		else
		{
			c_dbUsername[0] = 0;
		}

		if(iPswd)
		{
			ASTRNCPY(c_dbUserPassword, iPswd);
		}
		else
		{
			c_dbUserPassword[0] = 0;
		}
	}
	else
	{

		std::string sSharedServerDir, sDBName, sUser, sPassword;
		if(GetRegDBInfo(sDBName, sSharedServerDir, sUser, sPassword) != kOK || sDBName.empty())
		{
			ASTRNCPY(c_sharedServerDir, "C:\\");

			unsigned long nSize = sizeof(c_dbServerName)-1;
			GetComputerName(c_dbServerName, &nSize);
			c_dbUsername[0] = 0;
			c_dbUserPassword[0] = 0;
		}
		else
		{
			ASTRNCPY(c_sharedServerDir, sSharedServerDir.c_str())
			ASTRNCPY(c_dbServerName, sDBName.c_str());
			ASTRNCPY(c_dbUsername, sUser.c_str());
			ASTRNCPY(c_dbUserPassword, sPassword.c_str());
		}
	}
	
	//	Make upper case for SQL Server 
	for(int i = 0; i < sizeof(c_dbServerName) && c_dbServerName[i] != 0; i++ )
	{
		if(islower(c_dbServerName[i]))
			c_dbServerName[i] = _toupper(c_dbServerName[i]);
	}
	if( c_admGroupID > 0 )
		CPxDB::InitDatabaseInfo(true, 10);

}


bool CPxDB::InitDatabaseInfo(bool iRedo, int iretry)
{
	GetAqLogger()->LogMessage(kDebug, "DEBUG: -CPxDB::InitDatabaseInfo start\n");
	
	if( c_admGroupID < 0 || iRedo)
	{
		TRCSLock fplock(&gInitLock);
		
		if(	InitMediaPoints(iRedo) != kOK)
			return false;

		if(c_dbServerName[0] == 0) 
			InitDBServerName();
		
		AqUString strSQL;
		
		switch(m_GlobalDBType){ // add SQLite 2011/09/08 K.Ko
		case kDBType_MSSQL:
		{
			if(c_dbUsername[0] == 0)
			{
				strSQL.Format(L"Provider=SQLOLEDB;Data Source=%S;Integrated Security=SSPI;Initial Catalog=PxDcmDB;APPLICATION NAME=%S", 
					c_dbServerName, GetCurrentProcessName());
			}
			else
			{
				if(c_dbUserPassword[0])
				{
					strSQL.Format(L"Provider=SQLOLEDB;Data Source=%S;User ID=%S;Password=%S;Initial Catalog=PxDcmDB;APPLICATION NAME=%S", 
						c_dbServerName, c_dbUsername, c_dbUserPassword, GetCurrentProcessName());
				}
				else
				{
					strSQL.Format(L"Provider=SQLOLEDB;Data Source=%S;User ID=%S;Initial Catalog=PxDcmDB;APPLICATION NAME=%S", 
						c_dbServerName, c_dbUsername, GetCurrentProcessName());

				}
			}
		}
		break;
		case kDBType_SQLite:
		{
		 	strSQL.Format(L"Provider=SQLITE;Data Source=%S",c_dbServerName);
		//	strSQL.Format(L"%S",c_dbServerName);
		}
		break;

		}


		SetDBConectionInfo(strSQL);

		CPxDB db;
		if( db.TestConnectionInfo(iretry)  != kOK )
		{
			c_admGroupID = -1;
			return false;
		}
		gMapAqObjectType.Clear();
		gMapActions.Clear();

		c_AQNetDomainID = db.GetDomainID("AQNet");
		// remember special user group ID of super, scan, and shared
		int retcd = db.GetUserGroupID("Administrators", c_AQNetDomainID);
		if( retcd > 0)
			c_admGroupID = retcd;
		else
			return false;
		
		retcd = db.GetUserGroupID("shared", c_AQNetDomainID);
		if(retcd > 0 )
			c_sharedGroupID = retcd;

		c_publicGroup.m_groupUID = 0;
		retcd = db.GetUserGroupID("AqNET_Public", c_AQNetDomainID);
		if(retcd > 0 )
		{
			c_publicGroup.m_groupUID = retcd;
			retcd = db.GetUserGroup (c_publicGroup.m_groupUID, c_publicGroup);
			if(retcd != kOK)
				c_publicGroup.m_groupUID = 0;
		}
		
		//else
		//	return false;
		if(cPagesperMB < 0.0)
		{
			int psize;
			retcd = db.SQLGetInt("select low from master.dbo.spt_values where number = 1 and type = 'E'", psize);
			if(retcd == kOK && psize > 0)
				cPagesperMB = 1048576 / psize;
		}
		
	}
	GetAqLogger()->LogMessage(kDebug, "DEBUG: -CPxDB::InitDatabaseInfo end\n");
	return true;
}

int CPxDB::TestConnectionInfo(int iretry)
{
	//TRACE( "connectStr : %s\n", GetDBConectionInfo() );

	SQA sqa(getLocalDBType());
	int retcd;
	bool logerror = false;
	do
	{
		if(iretry == 1)
			logerror = true;

		retcd = openDatabase(sqa, logerror);
		closeDatabase( sqa );
		if(retcd == kOK || m_cancelFlag)
			break;

		Sleep(5000);
	} 
	while(iretry-- > 0);

	
	return retcd;
}

//-------------------------------------------------------------------------
// 
int	CPxDB::SQLGetString(const char* iQueryStr, string& oVal)
{
	SQA sqa(getLocalDBType());
	oVal.empty();
	
	GetAqLogger()->LogMessage(kDebug, "DEBUG: -CPxDB::SQLGetString with: %s\n", iQueryStr);
	sqa.SetCommandText(iQueryStr);
	int retcd = SQLExecuteBegin(sqa);
	if(retcd != kOK)  return retcd;

	retcd = sqa.MoveFirst(); if(retcd != kOK)  return retcd;
	// return here without SQLExecuteEnd(sqa) is OK.
	// When sqa is out scope, all connections will be closed (with rollback)

	oVal = sqa.getDataS();
	SQLExecuteEnd(sqa);

	return kOK;
}

//-------------------------------------------------------------------------
// 
int	CPxDB::SQLGetInt(const char* iQueryStr, int& oVal)
{
	SQA sqa(getLocalDBType());
	oVal=0;
	GetAqLogger()->LogMessage(kDebug, "DEBUG: -CPxDB::SQLGetInt with: %s\n", iQueryStr);
	sqa.SetCommandText(iQueryStr);
	int retcd = SQLExecuteBegin(sqa);
	if(retcd != kOK)  return retcd;

	retcd = sqa.MoveFirst(); if(retcd != kOK)  return retcd;

	oVal = sqa.getDataInt();
	SQLExecuteEnd(sqa);

	return kOK;
}

int	CPxDB::SQLMakeGetID(const char* iQueryStr, int& oVal)
{
	int rcode;

	// try three times  for make type stored procedure
	rcode = SQLGetInt(iQueryStr, oVal);
	if (rcode == kOK)
		return rcode;

	rcode = SQLGetInt(iQueryStr, oVal);
	if (rcode == kOK)
		return rcode;

	return SQLGetInt(iQueryStr, oVal);

}


int	CPxDB::SQLGet2Int(const char* iQueryStr, int& oVal1, int& oVal2)
{
	SQA sqa(getLocalDBType());
	oVal1=oVal2=0;
	GetAqLogger()->LogMessage(kDebug, "DEBUG: -CPxDB::SQLGetInt with: %s\n", iQueryStr);
	sqa.SetCommandText(iQueryStr);
	int retcd = SQLExecuteBegin(sqa);
	if(retcd != kOK)  return retcd;

	retcd = sqa.MoveFirst(); if(retcd != kOK)  return retcd;

	oVal1 = sqa.getDataInt();
	oVal2 = sqa.getDataInt();
	SQLExecuteEnd(sqa);

	return kOK;
}

int	CPxDB::SQLMakeGet2ID(const char* iQueryStr, int& oVal1, int& oVal2)
{
	int rcode;

	// try three times  for make type stored procedure
	rcode = SQLGet2Int(iQueryStr, oVal1, oVal2);
	if (rcode == kOK)
		return rcode;

	rcode = SQLGet2Int(iQueryStr, oVal1, oVal2);
	if (rcode == kOK)
		return rcode;

	return SQLGet2Int(iQueryStr, oVal1, oVal2);

}



//-------------------------------------------------------------------------
// 
int	CPxDB::SQLQuery(const char* iQueryStr, std::vector<KVP_MAP>& oVal)
{
	SQA sqa(getLocalDBType());
	int i, size, index = 0;
	AqString name, value;
	map<string, string> oneRow;

	GetAqLogger()->LogMessage(kDebug, "DEBUG: -DBCore::SQLQuery start\n");
	sqa.SetCommandText(iQueryStr);
	int retcd = SQLExecuteBegin(sqa);
	oVal.clear(); 
	if(retcd != kOK) 
		return retcd;

	size = sqa.GetRecordCount(); 
	if(size < 1) 
		return kNoResult;
	oVal.resize(size);

	retcd = sqa.MoveFirst(); 
	if(retcd != kOK)  
		return retcd;

	int fieldCount = sqa.GetFieldCount();
	AqUString nameU, valueU;
	while( retcd == kOK )
	{
		oneRow.clear();
		for(i = 0; i < fieldCount; i++)
		{
			sqa.getDataST(nameU, valueU);
			
			oneRow[name.ConvertUTF8(nameU)] = value.ConvertUTF8(valueU); 
		}

		oVal[index++] = oneRow;
		retcd = sqa.MoveNext();
	}

	SQLExecuteEnd(sqa);
	GetAqLogger()->LogMessage(kDebug, "DEBUG: -DBCore::SQLQuery end\n");
	return kOK;
}

//-------------------------------------------------------------------------
// 
int	CPxDB::SQLStringValues(const char* iQueryStr, vector<string>& oVal)
{
	GetAqLogger()->LogMessage(kDebug, "DEBUG: -DBCore::SQLStringValues start\n");
	SQA sqa(getLocalDBType());

	sqa.SetCommandText(iQueryStr);
	int retcd = SQLExecuteBegin(sqa);
	oVal.clear(); if(retcd != kOK) return retcd;
	int size = sqa.GetRecordCount(); if(size < 1) return kNoResult;
	oVal.resize(size);

	retcd = sqa.MoveFirst(); if(retcd != kOK)  return retcd;
	int index = 0;
	while( retcd == kOK )
	{
		oVal[index++] = sqa.getDataS();
		retcd = sqa.MoveNext();
	}

	SQLExecuteEnd(sqa);
	GetAqLogger()->LogMessage(kDebug, "DEBUG: -DBCore::SQLStringValues end\n");
	return kOK;
}

//-------------------------------------------------------------------------
// 
int	CPxDB::SQLIntValues(const char* iQueryStr, vector<int>& oVal)
{
	GetAqLogger()->LogMessage(kDebug, "DEBUG: -DBCore::SQLIntValues start\n");
	SQA sqa(getLocalDBType());
	sqa.SetCommandText(iQueryStr);
	int retcd = SQLExecuteBegin(sqa);
	oVal.clear(); if(retcd != kOK) return retcd;
	int size = sqa.GetRecordCount(); if(size < 1) return kNoResult;
	oVal.resize(size);
	int index = 0;
	retcd = sqa.MoveFirst(); if(retcd != kOK)  return retcd;
	while( retcd == kOK )
	{
		oVal[index++] = sqa.getDataInt();
		retcd = sqa.MoveNext();
	}

	SQLExecuteEnd(sqa);
	GetAqLogger()->LogMessage(kDebug, "DEBUG: -DBCore::SQLIntValues end\n");
	return kOK;
}

long CPxDB::GetMBLimit()
{
	SQA sqa(getLocalDBType());
	sqa.SetCommandText("select @@VERSION");
	int retcd = SQLExecuteBegin(sqa);
	if(retcd != kOK)  return 0;

	retcd = sqa.MoveFirst(); if(retcd != kOK)  return 0;

	AqString queryStr = sqa.getDataS();
	SQLExecuteEnd(sqa);
	
	int idx = queryStr.Find("Desktop Engine");

	// -1 means unlimit
	return (idx < 0)?-1:2000;
}

//default to get main dataabse, "" to total size
double CPxDB::GetTotalDataMBSize(const char* dbname)
{
	vector<AqString> dbnames;
	SQA sqa(getLocalDBType());

	if(!dbname)
	{
		dbnames.push_back("PxDcmDB");
	}
	else if(!dbname[0])
	{
		dbnames.push_back("PxDcmDB");
		dbnames.push_back("PxDcmDB2");
		dbnames.push_back("PxDcmDB3");
		dbnames.push_back("PxDcmHistDB");
	}
	else
	{
		dbnames.push_back(dbname);
	}

	double total = 0;
	for(int i=0; i<dbnames.size(); i++)
	{
		sqa.FormatCommandText("select sum(convert(dec(15),size))/%f from %s.dbo.sysfiles", cPagesperMB, dbnames[i]);
		if(SQLExecuteBegin(sqa) == kOK && sqa.MoveFirst() == kOK)
		{
			total += sqa.getDataD();
		}

	}
	SQLExecuteEnd(sqa);
	
	
	return total;
}

double CPxDB::GetDBDateTime()
{
	SQA sqa(getLocalDBType());
	double dateTime = 0.0;
	sqa.SetCommandText("select GETDATE()");	
	int retcd = SQLExecuteBegin(sqa);
	if(retcd != kOK)  return 0.0;

	retcd = sqa.MoveFirst(); if(retcd != kOK)  return 0.0;
	// return here without SQLExecuteEnd(sqa) is OK.
	// When sqa is out scope, all connections will be closed (with rollback)

	dateTime = sqa.getDataDate();
	SQLExecuteEnd(sqa);

	return dateTime;

}


// UserAccount
int CPxDB::MakeImportUser(const char * iUsername, const char * iUserDomain, const char * iLastname, 
					  const char * iFirstname, const char * iEmail, int& oUserID, int& oDomainID)
{
	AqString strSQL; 
	strSQL.Format("EXEC PxDcmDB.dbo.MakeImportUserAccount '%s', '%s', '%s', '%s', '%s'", 
		iUsername, iUserDomain, iLastname, iFirstname, iEmail);
	
	return SQLGet2Int(strSQL, oUserID, oDomainID);
}



int CPxDB::GetUserID(const char * iUsername, const char * iUserDomain)
{
	AqString strSQL; 
	strSQL.Format("SELECT AccountID FROM UserAccount u JOIN  DomainT d ON u.DomainID = d.DomainID "
		" WHERE Username='%s' AND d.NAME='%s'", iUsername, iUserDomain);
	
	int id=0;
	SQLGetInt(strSQL, id);
	return id;


}

int CPxDB::GetUser2ID(const char * iUsername, const char * iUserDomain, int& oUserID, int& oDomainID)
{
	AqString strSQL; 
	strSQL.Format("SELECT AccountID, u.DomainID FROM UserAccount u JOIN  DomainT d ON u.DomainID = d.DomainID "
		" WHERE Username='%s' AND d.NAME='%s'", iUsername, iUserDomain);
	
	return SQLGet2Int(strSQL, oUserID, oDomainID);

}

int CPxDB::GetUserAccount(int iUserID, UserAccount& oUserAccount)
{
	AqString	whereFilter; 
	whereFilter.Format("WHERE AccountID = %d", iUserID);

	return QueryUserAccount(whereFilter, oUserAccount);
}


int CPxDB::GetAqNETUserAccount(const char* iUsername, UserAccount& oUserAccount)
{
	AqString	whereFilter; 
	whereFilter.Format("WHERE Username='%s' AND DomainID=%d ", iUsername, c_AQNetDomainID);

	return QueryUserAccount(whereFilter, oUserAccount);
}

bool CPxDB::HasThisAqNETUser(const char* iUsername)
{
	AqString	strSQL; 
	strSQL.Format ("SELECT AccountID FROM UserAccount WHERE Username='%s' AND DomainID=%d",
		iUsername, c_AQNetDomainID);
	int id;
	SQLGetInt(strSQL, id);
	return (id > 0);
}

//-------------------------------------------------------------------------
// 
int	CPxDB::QueryUserAccount(const char* iWhereFilter, UserAccount& oUserAccount)
{
	GetAqLogger()->LogMessage(kDebug, "DEBUG: -CPxDB::QueryUserAccount start\n");
	if(iWhereFilter == 0)
		return kParameterError;

	const char* selectedItems  = "AccountID, Username, Password, HomeDir, LastName, MiddleName, FirstName, "
		"Title, Address, City, State, Zip, Country, Phone, Cell, Fax, Pager, Email, Status, Description, "
		"convert(varchar(17),  PwdExpireTime, 101), convert(varchar(17),  LastLoginTime, 101), "
		"RoamingProfile,LoginRetry, DomainID";

	SQA sqa(getLocalDBType());
	sqa.FormatCommandText("SELECT %s FROM UserAccount %s", selectedItems, iWhereFilter);	
	int retcd = SQLExecuteBegin(sqa);
	if(retcd != kOK) return retcd;

	oUserAccount.Clear();
	retcd = sqa.MoveFirst(); 
	if(retcd != kOK)  return retcd;
	if(retcd == kOK)
	{	
		SQL_GET_INT(oUserAccount.m_accountID, sqa);
		SQL_GET_STR(oUserAccount.m_username, sqa);
		SQL_GET_STR(oUserAccount.m_password, sqa);
		SQL_GET_STR(oUserAccount.m_homeDirectory, sqa);
		SQL_GET_STR(oUserAccount.m_lastName, sqa);
		SQL_GET_STR(oUserAccount.m_middleName, sqa);
		SQL_GET_STR(oUserAccount.m_firstName, sqa);
		SQL_GET_STR(oUserAccount.m_title, sqa);
		SQL_GET_STR(oUserAccount.m_address, sqa);
		SQL_GET_STR(oUserAccount.m_city, sqa);
		SQL_GET_STR(oUserAccount.m_state, sqa);
		SQL_GET_STR(oUserAccount.m_zip, sqa);
		SQL_GET_STR(oUserAccount.m_country, sqa);
		SQL_GET_STR(oUserAccount.m_phone, sqa);
		SQL_GET_STR(oUserAccount.m_cell, sqa);
		SQL_GET_STR(oUserAccount.m_fax, sqa);
		SQL_GET_STR(oUserAccount.m_pager, sqa);
		SQL_GET_STR(oUserAccount.m_email, sqa);
		SQL_GET_STR(oUserAccount.m_status, sqa);
		SQL_GET_STR(oUserAccount.m_description, sqa);
		SQL_GET_STR(oUserAccount.m_pwdExpireTime, sqa);
		SQL_GET_STR(oUserAccount.m_lastLoginTime, sqa);
		SQL_GET_INT(oUserAccount.m_roamingProfile, sqa);
		SQL_GET_INT(oUserAccount.m_loginRetry, sqa);
		SQL_GET_INT(oUserAccount.m_domainID, sqa);
	}
	
	SQLExecuteEnd(sqa);
	GetAqLogger()->LogMessage(kDebug, "DEBUG: -CPxDB::QueryUserAccount end\n");

	return kOK;
}

//-------------------------------------------------------------------------
// PwdExpireTime = 0 means that password will never expire --->set password only.
// otherwise, set both password and pwdExpireTime
//
int	CPxDB::UpdateUserPassword(int iUserAccountID, const char* iPassword, int iInterval)
{
	AqString iSQLStr;
	iSQLStr.Format("UPDATE UserAccount SET Password='%s' WHERE AccountID=%d "
		"UPDATE UserAccount SET  PwdExpireTime = GETDATE() + %d WHERE AccountID=%d AND PwdExpireTime <>0", 
		iPassword, iUserAccountID, iInterval, iUserAccountID);
	
	GetAqLogger()->LogMessage(kDebug, "DEBUG: -CPxDB::UpdateUserPassword with: %s\n", iSQLStr);
	return SQLExecute(iSQLStr);
}

int CPxDB::DisableUserAccount(int iUserAccountID)
{
	AqString strSQL;
	strSQL.Format("UPDATE UserAccount SET Status='DISABLED' WHERE AccountID=%d ", iUserAccountID);
	GetAqLogger()->LogMessage(kDebug, "DEBUG: -CPxDB::DisableUserAccount with: %s\n", strSQL);
	return SQLExecute(strSQL);
}
//-------------------------------------------------------------------------
//  Only disable user accounts other than administrator and shared group users under build in domain "AQNET" 
//
int CPxDB::DisableInactiveAccounts(int iInactiveIntervalInDays, std::vector<int>& oUserIDAffected )
{
	AqString strSQL;
	strSQL.Format("Select ua.AccountID FROM UserAccount ua INNER JOIN UserAssignedGroupView ug "
		"ON ua.AccountID = ug.AccountID AND ug.GroupID <> %d AND ug.GroupID <> %d WHERE GETDATE() "
		"> ua.LastLoginTime + %d AND ua.Status <>'DISABLED' AND ua.domainID = %d ", 
		c_admGroupID, c_sharedGroupID, iInactiveIntervalInDays, c_AQNetDomainID);

	std::vector<int> oVal; 
 	int retcd = SQLIntValues(strSQL, oVal);
	if( retcd != kOK) return retcd;
	int length = oVal.size();
	if(length <= 0) return kNoResult;

	int count = 0;

	AqString whereSQL = "";
	for (int i=0; i<length-1; i++)
	{
		strSQL.Format("%d,", oVal[i]);

	 	whereSQL += strSQL;
 	}

	if(length > 1)
		strSQL.Format("UPDATE UserAccount SET Status='DISABLED' WHERE AccountID in (%s,%d)", 
			whereSQL, oVal[length-1]);
	else
		strSQL.Format("UPDATE UserAccount SET Status='DISABLED' WHERE AccountID in (%d)", oVal[0]);

	retcd = SQLExecute(strSQL);
	if( retcd != kOK) return retcd;
	
	oUserIDAffected =oVal; 
	return kOK;
}


//-------------------------------------------------------------------------
//
int CPxDB::SetUserLoginTime(int iAccountID)
{
	AqString strSQL;
	strSQL.Format("UPDATE UserAccount SET LastLoginTime = GETDATE() WHERE AccountID = %d ", iAccountID);
	GetAqLogger()->LogMessage(kDebug, "DEBUG: -CPxDB::SetUserLoginTime with: %s\n", strSQL);
	return SQLExecute(strSQL);

}
//-------------------------------------------------------------------------
// PwdExpireTime = 0 means that password will nerver expire
//
bool CPxDB::IsPasswordExpired(int iAccountID)
{
	AqString strSQL;
	strSQL.Format("Select AccountID FROM userAccount WHERE AccountID=%d AND GETDATE() - "
		"PwdExpireTime >=0 AND PwdExpireTime <> 0 ", iAccountID);
	int id;
	int retcd = SQLGetInt(strSQL, id);
	return (retcd == kOK && id > 0);
}
//------------------------------------------------------------------------------------------------
//
int	CPxDB::AssignUsertoGroups(int iUserAccountID, const vector<int>& iGroupIDs)
{
	GetAqLogger()->LogMessage(kDebug, "DEBUG: -CPxDB::AssignUsertoGroups() start\n");
 
	SQA sqa(getLocalDBType());
	// remove all entries from User Group Assignment based on user account UID
	sqa.FormatCommandText("DELETE FROM UserDefaultGroup WHERE AccountID = %d", iUserAccountID);
	int retcd = SQLExecuteBegin(sqa);
	if(retcd != kOK) 
		return retcd;

	sqa.FormatCommandText("DELETE FROM UserOtherGroup WHERE AccountID = %d", iUserAccountID);
	retcd = SQLExecuteBegin(sqa);
	if(retcd != kOK) 
		return retcd;
	
	// Add updated entries to User Group Assignment tables (get UserGroupUID from Usergroup table based on name 

    int size = iGroupIDs.size();
	for(int i=0; i<size; i++)
	{
	 
		if(i == 0) // the first one it default group
			sqa.FormatCommandText("INSERT UserDefaultGroup (AccountID, GroupID) VALUES (%d, %d)", 
			iUserAccountID, iGroupIDs[i]);
		else
			sqa.FormatCommandText("INSERT UserOtherGroup (AccountID, GroupID) VALUES (%d, %d)", 
			iUserAccountID, iGroupIDs[i]);

		retcd = SQLExecuteBegin(sqa);
		if(retcd != kOK) 
			return retcd;
	} 

 	SQLExecuteEnd(sqa);	
 	GetAqLogger()->LogMessage(kDebug, "DEBUG: -CPxDB::AssignUsertoGroups()  end\n");
	return retcd;

}

//-------------------------------------------------------------------------
// 
int	CPxDB::AssignUsertoGroups(int iUserID, const std::vector< AqString >& iGroupsSID)
{
	int nGroup = iGroupsSID.size();
	if(nGroup == 0)
		return kParameterError;

	
	AqString strSQL = "SELECT UserGroupID FROM UserGroup WHERE SID IN (";
	
	strSQL += iGroupsSID[0].GetString();
	if(nGroup > 1)
	{
		AqString delmitor = ",";
		for(int i=1; i< nGroup; i++)
			strSQL += delmitor+iGroupsSID[i].GetString();
	}
	strSQL += ")";

	vector<int> groupIDs;
	
	int ret = SQLIntValues(strSQL, groupIDs);
	
	if(ret != kOK)
		return ret;
	
	return AssignUsertoGroups(iUserID, groupIDs);
	
}


bool CPxDB::IsUserInGroup(int iUserID, int iGroupID)
{
	AqString strSQL;
	strSQL.Format("SELECT AccountID FROM UserAssignedGroupView WHERE groupID=%d AND "
				  "AccountID=%d", iGroupID, iUserID);
	
	int id;
	int retcd = SQLGetInt(strSQL, id);
	return (retcd == kOK && id > 0);
}

//-------------------------------------------------------------------------
// 
int	CPxDB::GetUserAssignedGroups(int iAccountID, vector<UserGroup>& oUserGroups)
{
	UserGroup defaultGroup;
	defaultGroup.Clear();

	// get the default group
	AqString	whereFilter ;
	whereFilter.Format(" WHERE UserGroupID IN (SELECT GroupID FROM UserDefaultGroup "\
		" WHERE AccountID =  %d )", iAccountID);

	int rcode = QueryUserGroup(oUserGroups, whereFilter);
	if( rcode != kOK) 
		return rcode;
	//store the default group
	if(oUserGroups.size() > 0)
		defaultGroup = oUserGroups[0];
	
	// get other groups
	whereFilter.Format(" WHERE UserGroupID IN (SELECT GroupID FROM UserOtherGroup "\
		" WHERE AccountID =  %d ) ORDER BY UserGroup.Name", iAccountID);

	rcode = QueryUserGroup(oUserGroups, whereFilter);

	// put the default group in the first location
	if(defaultGroup.m_groupUID > 0)
	{
		if(oUserGroups.size() > 0)
		{
			UserGroup tmpGroup = oUserGroups[0];
			oUserGroups[0] = defaultGroup;
			oUserGroups.push_back(tmpGroup);
		}
		else
		{
			oUserGroups.push_back(defaultGroup);
		}
	}

	if(c_publicGroup.m_groupUID != 0)
		oUserGroups.push_back(c_publicGroup);

	if(rcode == kNoResult) // other user groups may no exist
		rcode = kOK;
	
	return rcode;
}

int	CPxDB::GetUserAssignedGroupIDs(int iAccountID, std::vector<int>& oUserGroupIDs, bool& oHasShared)
{

	SQA sqa(getLocalDBType());
	sqa.FormatCommandText("SELECT GroupID, PxDcmDB.dbo.CanGroupAccessAllData(GroupID) FROM "
		" dbo.UserDefaultGroup WHERE AccountID=%d UNION ALL SELECT GroupID, "
		" PxDcmDB.dbo.CanGroupAccessAllData(GroupID) FROM dbo.UserOtherGroup where AccountID=%d", 
		iAccountID, iAccountID);

	int retcd = SQLExecuteBegin(sqa);
	oHasShared = false;	oUserGroupIDs.clear(); 
	if(retcd != kOK) return retcd;
	int size = sqa.GetRecordCount(); if(size < 1) return kNoResult;
	oUserGroupIDs.resize(size);
	int index = 0;
	retcd = sqa.MoveFirst(); if(retcd != kOK)  return retcd;
	while( retcd == kOK )
	{
		oUserGroupIDs[index++] = sqa.getDataInt();
		if(!oHasShared && sqa.getDataInt() == 1)
		{
			oHasShared = true;
		}

		retcd = sqa.MoveNext();
	}

	SQLExecuteEnd(sqa);

	if(c_publicGroup.m_groupUID != 0)
		oUserGroupIDs.push_back(c_publicGroup.m_groupUID);

		
	return kOK;

}


int CPxDB::DeleteUserAccount(int iUserID)
{
	
	AqString strSQL;
	strSQL.Format("DELETE FROM UserAccount WHERE AccountID=%d", iUserID);

	GetAqLogger()->LogMessage(kDebug, "DEBUG: -CPxDB::DeleteUserAccount with: %s\n", strSQL);
	return SQLExecute(strSQL);

}

// Organization
//------------------------------------------------------------------------------------------------
//  Get Organization Names
//
int	CPxDB::GetOrganizationNames (vector <string>&oVal)
{
	return SQLStringValues("SELECT Name FROM Organization", oVal);
}

//------------------------------------------------------------------------------------------------
//  Get Organization Names
//
int	CPxDB::GetOrganization (int iOrganizationID, Organization &oVal)
{
	oVal.Clear();
	AqString whereFilter;
	whereFilter.Format(" WHERE OrganizationID = %d ",  iOrganizationID);
	vector<Organization>  tmp;
	int rcode = QueryOrganization( tmp, whereFilter);
	if (rcode != kOK) return rcode;

	if(tmp.size() > 0)
		oVal = tmp[0];
	return kOK;
}

//------------------------------------------------------------------------------------------------
//  Get Organization Names
//
int	CPxDB::GetOrganization (const char* iName, Organization &oVal)
{
	oVal.Clear();
	if(!iName) return kParameterError;
	AqString whereFilter;
	whereFilter.Format(" WHERE Name = '%s'",  iName);
	vector<Organization>  tmp;
	int rcode = QueryOrganization( tmp, whereFilter);
	if (rcode != kOK) return rcode;

	if(tmp.size() > 0)
		oVal = tmp[0];
	return kOK;
}

//------------------------------------------------------------------------------------------------
//  Get Organization Names
//
int	CPxDB::QueryOrganization(vector<Organization>& oVal, const char* iWhereFilter)
{
	AqString strSQL = "SELECT OrganizationID,Name,Address,Phone,Fax,Description FROM Organization ";
	if(iWhereFilter) strSQL += iWhereFilter;
	strSQL += " ORDER BY Name ";
		
	SQA sqa(getLocalDBType());
	GetAqLogger()->LogMessage(kDebug, "DEBUG: -CPxDB::QueryOrganization with: %s\n", strSQL);
	sqa.SetCommandText(strSQL);
	int retcd = SQLExecuteBegin(sqa);
	oVal.clear(); if(retcd != kOK) return retcd;
	int size = sqa.GetRecordCount(); if(size < 1) return kNoResult;
	oVal.resize(size);

	Organization* pOrganization;
	int index = 0;
	retcd = sqa.MoveFirst(); if(retcd != kOK)  return retcd;
	while( retcd == kOK && index < size )
	{
		pOrganization = &(oVal[index++]);

		SQL_GET_INT(pOrganization->m_organizationUID, sqa);
		SQL_GET_STR(pOrganization->m_name, sqa);
		SQL_GET_STR(pOrganization->m_address, sqa);
		SQL_GET_STR(pOrganization->m_phone, sqa);
		SQL_GET_STR(pOrganization->m_fax, sqa);
		SQL_GET_STR(pOrganization->m_description, sqa);
			
		retcd = sqa.MoveNext();
	}

	SQLExecuteEnd(sqa);
		 
	return kOK;
}

//------------------------------------------------------------------------------------------------
//  Get Domain Names
//
int	CPxDB::GetDomainID(const char* iDomainName)
{
	AqString strSQL;
	strSQL.Format("SELECT DomainID FROM DomainT WHERE Name='%s'", iDomainName);

	int oVal;
	SQLGetInt(strSQL, oVal);
	return oVal;
}

int	CPxDB::GetDomainNames(vector<string>& oVal)
{
	AqString strSQL;
	strSQL.Format("SELECT Name FROM DomainT");

	return SQLStringValues(strSQL, oVal);
}


//------------------------------------------------------------------------------------------------
//
int	CPxDB::GetDomainInfo(int iDomainID, DomainInfo &oVal)
{
	AqString	strSQL;
	strSQL.Format("SELECT d.DmainID, d.Name, d.OrganizationID, d.Description, d.type, o.OrganizationID, o.Name, "
		"Address, Phone, Fax, o.Description FROM DomainT d JOIN Organization o ON "
		"d.OrganizationId = o.OrganizationID WHERE d.DomainID = %d", iDomainID);
 
	SQA sqa(getLocalDBType());
	GetAqLogger()->LogMessage(kDebug, "DEBUG: -CPxDB::GetDomainInfo with: %s\n", strSQL);
	sqa.SetCommandText(strSQL);
	int retcd = SQLExecuteBegin(sqa);
	if(retcd != kOK) return retcd;
 
	oVal.Clear();
	retcd = sqa.MoveFirst(); if(retcd != kOK)  return retcd;
	if(retcd == kOK)
	{
		SQL_GET_INT(oVal.m_domain.m_domainID, sqa);
		SQL_GET_STR(oVal.m_domain.m_name, sqa);
		SQL_GET_INT(oVal.m_domain.m_organizationID, sqa);
		SQL_GET_STR(oVal.m_domain.m_description, sqa);
		SQL_GET_INT(oVal.m_domain.m_type, sqa);

		SQL_GET_INT(oVal.m_organization.m_organizationUID, sqa);
		SQL_GET_STR(oVal.m_organization.m_name, sqa);
		SQL_GET_STR(oVal.m_organization.m_address, sqa);
		SQL_GET_STR(oVal.m_organization.m_phone, sqa);
		SQL_GET_STR(oVal.m_organization.m_fax, sqa);
		SQL_GET_STR(oVal.m_organization.m_description, sqa);
	}
		
	SQLExecuteEnd(sqa);
	return kOK;
}

//------------------------------------------------------------------------------------------------
//  Get group of Domain information
//
int	CPxDB::GetDomainInfo(const char* iDomainName , vector<DomainInfo> &oVal)
{
	if(!iDomainName)
	{
		oVal.clear();
		return kParameterError;
	}

	AqString whereFilter ;
	whereFilter.Format(" WHERE  DomainT.Name = '%s' ", iDomainName);
 
	return QueryDomain( whereFilter, oVal);
}

int	CPxDB::QueryDomain(const char* iWhereFilter, vector<Domain>& oVal)
{
	AqString	strSQL = "SELECT DomainT.DomainID,DomainT.Name, DomainT.OrganizationId,DomainT.Description, DomainT.Type "\
		"FROM DomainT ";
	
	if(iWhereFilter) strSQL += iWhereFilter;
	
	SQA sqa(getLocalDBType());
	GetAqLogger()->LogMessage(kDebug, "DEBUG: -CPxDB::QueryDomain with: %s\n", strSQL);
	sqa.SetCommandText(strSQL);
	int retcd = SQLExecuteBegin(sqa);
	oVal.clear(); if(retcd != kOK) return retcd;
	int size = sqa.GetRecordCount(); if(size < 1) return kNoResult;
	oVal.resize(size);

	Domain* pDomain;
	int index = 0;
	retcd = sqa.MoveFirst(); if(retcd != kOK)  return retcd;
	while( retcd == kOK && index < size )
	{	 
		pDomain = &(oVal[index++]);
		SQL_GET_INT(pDomain->m_domainID, sqa);
		SQL_GET_STR(pDomain->m_name, sqa);
		SQL_GET_INT(pDomain->m_organizationID, sqa);
		SQL_GET_STR(pDomain->m_description, sqa);
		SQL_GET_INT(pDomain->m_type, sqa);
		
		retcd = sqa.MoveNext();
	}

	SQLExecuteEnd(sqa);	 
	return kOK;
	
}

int	CPxDB::QueryDomain(const char* iWhereFilter, vector<DomainInfo> &oVal)
{
	AqString	strSQL = "SELECT DomainT.DomainID,DomainT.Name, DomainT.OrganizationID,DomainT.Description, DomainT.Type, "\
		"Organization.OrganizationID,Organization.Name, Organization.Address, "\
		"Organization.Phone, Organization.Fax, Organization.Description "\
		"FROM DomainT JOIN Organization ON "\
		"DomainT.OrganizationId = Organization.OrganizationID ";// Safe here to have a space
	
	if(iWhereFilter) strSQL += iWhereFilter;
	
	SQA sqa(getLocalDBType());
	GetAqLogger()->LogMessage(kDebug, "DEBUG: -CPxDB::QueryDomains with: %s\n", strSQL);
	sqa.SetCommandText(strSQL);
	int retcd = SQLExecuteBegin(sqa);
	oVal.clear(); if(retcd != kOK) return retcd;
	int size = sqa.GetRecordCount(); if(size < 1) return kNoResult;
	oVal.resize(size);

	DomainInfo* pDomainInfo;
	int index = 0;
	retcd = sqa.MoveFirst(); if(retcd != kOK)  return retcd;
	while( retcd == kOK && index < size )
	{
		pDomainInfo = &(oVal[index++]);
	 
		SQL_GET_INT(pDomainInfo->m_domain.m_domainID, sqa);
		SQL_GET_STR(pDomainInfo->m_domain.m_name, sqa);
		SQL_GET_INT(pDomainInfo->m_domain.m_organizationID, sqa);
		SQL_GET_STR(pDomainInfo->m_domain.m_description, sqa);
		SQL_GET_INT(pDomainInfo->m_domain.m_type, sqa);

		SQL_GET_INT(pDomainInfo->m_organization.m_organizationUID, sqa);
		SQL_GET_STR(pDomainInfo->m_organization.m_name, sqa);
		SQL_GET_STR(pDomainInfo->m_organization.m_address, sqa);
		SQL_GET_STR(pDomainInfo->m_organization.m_phone, sqa);
		SQL_GET_STR(pDomainInfo->m_organization.m_fax, sqa);
		SQL_GET_STR(pDomainInfo->m_organization.m_description, sqa);
			
		retcd = sqa.MoveNext();
	}

	SQLExecuteEnd(sqa);
	 
	return kOK;
	
}

//------------------------------------------------------------------------------------------------
int	CPxDB::GetUserGroupID (const char* iGroupName, int iDomainID)
{
	AqString	strSQL;
	strSQL.Format("SELECT UserGroupID FROM UserGroup WHERE Name='%s' and DomainID=%d", 
		iGroupName, iDomainID);
 
	int userGroupID = 0;
	SQLGetInt(strSQL, userGroupID);		 
	return userGroupID;
}

//------------------------------------------------------------------------------------------------
//  Get All UserGroup objects  based on iUserGroupName .
//
int	CPxDB::QueryUserGroup( vector<UserGroup>& oVal, const char* iWhereFilter)
{
 	GetAqLogger()->LogMessage(kDebug, "DEBUG: -CPxDB::QueryUserGroup start\n");
	AqString	strSQL;
	strSQL.Format("SELECT UserGroupID, DomainId, Name, Privilege, Description FROM UserGroup ");
	
	if(iWhereFilter != 0)
		strSQL += iWhereFilter;

	SQA sqa(getLocalDBType());
	
	GetAqLogger()->LogMessage(kDebug, "DEBUG: -CPxDB::QueryUserGroup with: %s\n", strSQL);
	sqa.SetCommandText(strSQL);
	int retcd = SQLExecuteBegin(sqa);
	oVal.clear(); if(retcd != kOK) return retcd;
	int size = sqa.GetRecordCount(); if(size < 1) return kNoResult;
	oVal.resize(size);

	UserGroup* pInfo;
	int index = 0;
	retcd = sqa.MoveFirst(); if(retcd != kOK)  return retcd;
	while( retcd == kOK && index < size )
	{
		pInfo = &(oVal[index++]);
		
		SQL_GET_INT(pInfo->m_groupUID, sqa);
		SQL_GET_INT(pInfo->m_domainID, sqa);
		SQL_GET_STR(pInfo->m_name, sqa);
		SQL_GET_INT(pInfo->m_privilege, sqa);
		SQL_GET_STR(pInfo->m_description, sqa);
		
		retcd = sqa.MoveNext();
	}
	SQLExecuteEnd(sqa);
	GetAqLogger()->LogMessage(kDebug, "DEBUG: -CPxDB::QueryUserGroup end\n");
	 
	return kOK;
}

//------------------------------------------------------------------------------------------------
//
int	CPxDB::GetUserGroup (int iUserGroupID, UserGroup &oVal)
{
	AqString whereFilter ;
	whereFilter.Format(" WHERE UserGroupID = %d ", iUserGroupID);

	oVal.Clear();
	vector<UserGroup> tmp ;
	int rcode = QueryUserGroup(tmp, whereFilter);
	if( rcode != kOK) return rcode;
	if(tmp.size() > 0)
		oVal = tmp[0];
	return kOK;
}

//------------------------------------------------------------------------------------------------
//  Get All UserGroup object  based on iUserGroupName .
//
int	CPxDB::GetUserGroup (const char* iUserGroupName, int iDomainID, UserGroup &oVal)
{
	oVal.Clear();
	if(!iUserGroupName) return kParameterError;
	AqString whereFilter ;

	whereFilter.Format(" WHERE Name = '%s' and DomainID=%d", iUserGroupName, iDomainID);

	vector<UserGroup> tmp ;
	int rcode = QueryUserGroup(tmp, whereFilter);
	if( rcode != kOK ) return rcode;
	if(tmp.size() > 0)
		oVal = tmp[0];
	return kOK;

}


//------------------------------------------------------------------------------------------------
int	CPxDB::GetGroupRights(int iGroupID, std::vector<AqStringKV>& oRights )
{
	
	SQA sqa(getLocalDBType());
	sqa.FormatCommandText("SELECT RightsName, RightsValue FROM ViewGroupRights WHERE GroupID=%d", iGroupID);
	int retcd = SQLExecuteBegin(sqa);
	if(retcd != kOK) return retcd;
	int size = sqa.GetRecordCount(); if(size < 1) return kNoResult;
	oRights.resize(size);

	AqStringKV* pRights;
	int index = 0;
	retcd = sqa.MoveFirst(); if(retcd != kOK)  return retcd;
	while( retcd == kOK && index < size )
	{
		pRights = &(oRights[index++]);
	
		pRights->Key = sqa.getDataS();
		pRights->Value = sqa.getDataS();

		retcd = sqa.MoveNext();
	}
	SQLExecuteEnd(sqa);	 

	return kOK;

}

//------------------------------------------------------------------------------------------------
int	CPxDB::SetGroupRights(int iGroupID, const char* iKey, const char* iValue)
{
	AqString strSQL;
	strSQL.Format("EXEC SetGroupRights %d, '%s', '%s' ", iGroupID, iKey, iValue);
	return SQLExecute(strSQL);
}

//------------------------------------------------------------------------------------------------
int	CPxDB::RemoveGroupRights(int iGroupID, const char* iKey)
{
	AqString strSQL;
	strSQL.Format("EXEC RemoveGroupRights %d, '%s'", iGroupID, iKey);
	return SQLExecute(strSQL);
}

//------------------------------------------------------------------------------------------------
bool CPxDB::GroupHasTheRights(int iGroupID, const char* iKey, const char* iValue)
{
	AqString strSQL;
	strSQL.Format("Select PxDcmDB.dbo.GroupHasTheRights(%d, '%s', '%s') ", iGroupID, iKey, iValue);
	int flg = 0;
	int ret = SQLGetInt(strSQL, flg);

	return (ret== kOK && flg == 1);
}


//------------------------------------------------------------------------------------------------
bool CPxDB::CanGroupAccessAllData(int iGroupID)
{
	AqString strSQL;
	if(getLocalDBType() == kDBType_SQLite){
		strSQL.Format("Select * from ViewGroupRights where GroupID='%d' and RightsName='AccessAllData' and RightsValue='1'",iGroupID);
		 
	}else{
		strSQL.Format("Select PxDcmDB.dbo.CanGroupAccessAllData(%d) ", iGroupID);
	}
	int flg = 0;
	int ret = SQLGetInt(strSQL, flg);

	return (ret== kOK && flg == 1);
	 
}


//------------------------------------------------------------------------------------------------
// iDayofWeek fixed from 0 to 6
//
int CPxDB::GetRoutingTimeRanges(int iScheduleID, int iDayofWeek, std::vector<TimeRange>& oVal)
{
	if(iDayofWeek >6 || iDayofWeek <0) return kParameterError;

	AqString	strSQL;
	strSQL.Format("SELECT StartTime, EndTime FROM TimeRange WHERE ScheduleID = %d AND DayofWeek = %d ORDER BY StartTime ", iScheduleID, iDayofWeek);

	
	SQA sqa(getLocalDBType());
	GetAqLogger()->LogMessage(kDebug, "DEBUG: -CPxDB::GetRoutingTimeRanges with: %s\n", strSQL);	
	sqa.SetCommandText(strSQL);
	int retcd = SQLExecuteBegin(sqa);
	if(retcd != kOK) return retcd;
	int size = sqa.GetRecordCount(); if(size < 1) return kNoResult;
	oVal.resize(size);

	TimeRange* pTmp;
	int index = 0;
	retcd = sqa.MoveFirst(); if(retcd != kOK)  return retcd;
	while( retcd == kOK && index < size )
	{
		pTmp = &(oVal[index++]);
	 
		SQL_GET_STR(pTmp->m_startTime, sqa);
		SQL_GET_STR(pTmp->m_endTime, sqa);
		retcd = sqa.MoveNext();
	}
	SQLExecuteEnd(sqa);	 

	GetAqLogger()->LogMessage(kDebug, "DEBUG: -CPxDB::GetRoutingTimeRanges end\n");
	return kOK;

}

//------------------------------------------------------------------------------------------------
//
int	CPxDB::GetTempRoutingAEInfos(int iTagFilterID, vector<RoutingAEInfo>& oVal, int& oSuspendOthers)
{
	int suspendOthersAccumulator = 0;

	GetAqLogger()->LogMessage(kDebug, "DEBUG: -CPxDB::GetTempRoutingAEInfos start\n");
	
	oVal.clear();  
	AqString tagFilterWhere;
	tagFilterWhere.Format("    WHERE TagFilterID = %d AND ", iTagFilterID);

	AqString	tmpSQL = "";

	tmpSQL += "SELECT DISTINCT SuspendOthers FROM TempRoutingSchedule WHERE SuspendOthers = 1 AND ";
	tmpSQL += "(EndDate is NULL OR GETDATE() <= (TempRoutingSchedule.EndDate + 1)) ";
	SQA sqa(getLocalDBType());

	GetAqLogger()->LogMessage(kDebug, "DEBUG: -CPxDB::GetTempRoutingAEInfos with: %s\n", tmpSQL);	
	sqa.SetCommandText(tmpSQL);
	int retcd = SQLExecuteBegin(sqa);
	if(retcd != kOK) 
		return retcd;

	oSuspendOthers = (sqa.GetRecordCount() > 0);
	
	SQLExecuteEnd(sqa);	 

	AqString strSQL = "";
	strSQL += "SELECT DISTINCT RemoteAE.*, TagBasedRoutingPatternEntry.CompressionMethod, TagBasedRoutingPatternEntry.CompressionFactor ";
	strSQL += "FROM TagBasedRoutingPatternEntry ";
	strSQL += "JOIN RemoteAE ON RemoteAE.ID = TagBasedRoutingPatternEntry.StoreTargetID ";
	strSQL += "JOIN TempRoutingSchedule ON TempRoutingSchedule.RoutingPatternID = TagBasedRoutingPatternEntry.RoutingPatternID ";
	strSQL += "WHERE TagBasedRoutingPatternEntry.RoutingPatternID IN ";
	strSQL += "( ";
	strSQL += "	SELECT RoutingPatternID FROM TempRoutingSchedule ";
	strSQL += tagFilterWhere;
	strSQL += "	(EndDate is NULL OR GETDATE() <= (TempRoutingSchedule.EndDate + 1)) ";
	strSQL += ") ";

	GetAqLogger()->LogMessage(kDebug, "DEBUG: -CPxDB::GetTempRoutingAEInfos with: %s\n", strSQL);	
	sqa.SetCommandText(strSQL);
	retcd = SQLExecuteBegin(sqa);
	if(retcd != kOK) return retcd;
	int size = sqa.GetRecordCount(); if(size < 1) return kNoResult;
	oVal.resize(size);

	RoutingAEInfo* pAEInfo;
	int index = 0;
	retcd = sqa.MoveFirst(); if(retcd != kOK)  return retcd;
	while( retcd == kOK && index < size )
	{
		pAEInfo = &(oVal[index++]);
	
		SQL_GET_INT(pAEInfo->m_AE.m_AEID, sqa);
		SQL_GET_STR(pAEInfo->m_AE.m_AEName, sqa);
		SQL_GET_STR(pAEInfo->m_AE.m_AETitle, sqa);
		SQL_GET_STR(pAEInfo->m_AE.m_hostName, sqa);
		SQL_GET_STR(pAEInfo->m_AE.m_IPAddress, sqa);
		SQL_GET_INT(pAEInfo->m_AE.m_port, sqa);
	 	SQL_GET_INT(pAEInfo->m_AE.m_level, sqa);
		SQL_GET_INT(pAEInfo->m_AE.m_priority, sqa);
		SQL_GET_STR(pAEInfo->m_AE.m_description, sqa);
		SQL_GET_INT(pAEInfo->m_compressionMethod, sqa);
		SQL_GET_INT(pAEInfo->m_compressionFactor, sqa);

		if (pAEInfo->m_compressionMethod < 0 || pAEInfo->m_compressionMethod > 2)
		{
			pAEInfo->m_compressionMethod = 0;
		}
		if (pAEInfo->m_compressionFactor < 0 || pAEInfo->m_compressionFactor > 1000)
		{
			pAEInfo->m_compressionFactor = 0;
		}

		if (pAEInfo->m_compressionMethod == 1)
		{
			pAEInfo->m_compressionFactor = 1;
		}

		retcd = sqa.MoveNext();
	}
	SQLExecuteEnd(sqa);	 

	GetAqLogger()->LogMessage(kDebug, "DEBUG: -CPxDB::GetTempRoutingAEInfos end\n");
	return kOK;
}

#include <time.h>
//------------------------------------------------------------------------------------------------
//
int	CPxDB::GetRoutingAEInfos(int iTagFilterID, vector<RoutingAEInfo>& oVal)
{
	GetAqLogger()->LogMessage(kDebug, "DEBUG: -CPxDB::GetRoutingAEInfos start\n");
	
	oVal.clear();  

	int suspendOthers = 0;
	int retcd = GetTempRoutingAEInfos(iTagFilterID, oVal, suspendOthers);
	if (suspendOthers)
	{
		GetAqLogger()->LogMessage("CPxDB::GetRoutingAEInfos: Routing by temporary patterns - suspending regular schedule\n");
		GetAqLogger()->LogMessage(kDebug, "DEBUG: -CPxDB::GetRoutingAEInfos end\n");
		return retcd;
	}

	AqString TagFilterWhere;
	TagFilterWhere.Format("    WHERE TagFilterID = %d AND ", iTagFilterID);

	time_t t = time(0);
	struct tm tm = *localtime(&t);

	AqString StartTimeWhere, EndTimeWhere;
	StartTimeWhere.Format(" StartTime <= %02d%02d ", tm.tm_hour, tm.tm_min);
	EndTimeWhere.Format(" EndTime >= %02d%02d ", tm.tm_hour, tm.tm_min);

	AqString	strSQL = "";

	strSQL += "SELECT DISTINCT RemoteAE.*, TagBasedRoutingPatternEntry.CompressionMethod, TagBasedRoutingPatternEntry.CompressionFactor ";
	strSQL += "FROM TagBasedRoutingPatternEntry ";
	strSQL += "JOIN RemoteAE ON RemoteAE.ID = TagBasedRoutingPatternEntry.StoreTargetID ";
	strSQL += "WHERE RoutingPatternID IN ";
	strSQL += "( ";
	strSQL += "	 SELECT RoutingPatternID FROM RoutingSchedule LEFT OUTER JOIN TimeRange ";
	strSQL += "	 ON RoutingSchedule.ScheduleID = TimeRange.ScheduleID ";
	strSQL += TagFilterWhere; //"    WHERE TagFilterID = %d AND "
	strSQL += "	 ( ";
	strSQL += "	   ((select count(*) FROM TimeRange WHERE ScheduleID = RoutingSchedule.ScheduleID) = 0) ";
	strSQL += "	   OR ";
	strSQL += "	   ( ";
	strSQL += "        DayOfWeek = (DATEPART(dw,GETDATE()) - 1) AND ";	// RL 2004.04.11 -1 (after DATEPART) to make day range 0-6 or or it will wrap around
	strSQL += StartTimeWhere; // " StartTime <= %02d%02d "
	strSQL += "        AND ";
	strSQL += EndTimeWhere; // " EndTime >= %02d%02d "
	strSQL += "    ) ";
	strSQL += "  ) ";
	strSQL += "  AND ";
	strSQL += "	 ((StartDate IS NULL OR StartDate <= GETDATE()) AND (EndDate IS NULL OR EndDate >= GETDATE())) ";
	strSQL += "	 AND ";
	strSQL += "  ( ";
	strSQL += "     (StartDate IS NULL) OR ";
	strSQL += "     (Repeat > 0) OR ";
	strSQL += "     (DATEDIFF(dd, StartDate, GETDATE()) < 7) ";
	strSQL += "  ) ";
	strSQL += ") ";

	SQA sqa(getLocalDBType());
	GetAqLogger()->LogMessage(kDebug, "DEBUG: -CPxDB::GetRoutingAEInfos with: %s\n", strSQL);	
	sqa.SetCommandText(strSQL);
	retcd = SQLExecuteBegin(sqa);
	if (retcd != kOK) 
		return retcd;

	int size = sqa.GetRecordCount(); 
	if (size < 1)
	{
		if (oVal.size() < 1) 
			return kNoResult;
		else
			return kOK;
	}

	int totalSize = size + oVal.size();
	int index = oVal.size();
	oVal.resize(totalSize);

	RoutingAEInfo* pAEInfo;
	retcd = sqa.MoveFirst(); if(retcd != kOK)  return retcd;
	while( retcd == kOK && index < totalSize )
	{
		pAEInfo = &(oVal[index++]);
	
		SQL_GET_INT(pAEInfo->m_AE.m_AEID, sqa);
		SQL_GET_STR(pAEInfo->m_AE.m_AEName, sqa);
		SQL_GET_STR(pAEInfo->m_AE.m_AETitle, sqa);
		SQL_GET_STR(pAEInfo->m_AE.m_hostName, sqa);
		SQL_GET_STR(pAEInfo->m_AE.m_IPAddress, sqa);
		SQL_GET_INT(pAEInfo->m_AE.m_port, sqa);
	 	SQL_GET_INT(pAEInfo->m_AE.m_level, sqa);
		SQL_GET_INT(pAEInfo->m_AE.m_priority, sqa);
		SQL_GET_STR(pAEInfo->m_AE.m_description, sqa);
		SQL_GET_INT(pAEInfo->m_compressionMethod, sqa);
		SQL_GET_INT(pAEInfo->m_compressionFactor, sqa);

		if (pAEInfo->m_compressionMethod < 0 || pAEInfo->m_compressionMethod > 2)
		{
			pAEInfo->m_compressionMethod = 0;
		}
		if (pAEInfo->m_compressionFactor < 0 || pAEInfo->m_compressionFactor > 1000)
		{
			pAEInfo->m_compressionFactor = 0;
		}

		if (pAEInfo->m_compressionMethod == 1)
		{
			pAEInfo->m_compressionFactor = 1;
		}

		retcd = sqa.MoveNext();
	}
	SQLExecuteEnd(sqa);	 

	GetAqLogger()->LogMessage(kDebug, "DEBUG: -CPxDB::GetRoutingAEInfos end\n");
	return kOK;

}

//------------------------------------------------------------------------------------------------
//  Get All ApplicationEntity objects based on different table type and whereFilter.
//
int	CPxDB::QueryApplicationEntity(AE_TYPE type ,vector <ApplicationEntity> &oVal,  const char* iWhereFilter )
{
	GetAqLogger()->LogMessage(kDebug, "DEBUG: -CPxDB::QueryApplicationEntity start\n");

	bool isLocalAE = false;

	oVal.clear();
	AqString strSQL = "SELECT ID,AEName,AETitle,HostName,IPAddress,Port,Level,Priority,Description ";

	if(type ==CPxDB::kQRAllowedAE)
		strSQL = strSQL + " FROM RemoteAE JOIN QRAllowedAE ON ID=AETitleID ";
	else if(type == CPxDB::kRemoteAE)
		strSQL = strSQL + " FROM RemoteAE ";
	else if(type == CPxDB::kStoreTargetAE)
		strSQL = strSQL + " FROM RemoteAE JOIN StoreTargetAE ON ID=AETitleID ";
	else if(type == CPxDB::kQRSourceAE)
		strSQL = strSQL + " FROM RemoteAE JOIN QRSourceAE ON ID=AETitleID ";
	else if(type == CPxDB::kLocalAE)
	{
		strSQL = strSQL + " FROM LocalAE ";
		isLocalAE = true;
	}
	else
		return kParameterError;

	if(iWhereFilter !=0 ) 
		strSQL += iWhereFilter;

	SQA sqa(getLocalDBType());
	GetAqLogger()->LogMessage(kDebug, "DEBUG: -CPxDB::QueryApplicationEntity with: %s\n", strSQL);	
	sqa.SetCommandText(strSQL);
	int retcd = SQLExecuteBegin(sqa);
	if(retcd != kOK) return retcd;
	int size = sqa.GetRecordCount(); if(size < 1) return kNoResult;
	oVal.resize(size);

	ApplicationEntity* pAE;
	int index = 0;
	retcd = sqa.MoveFirst(); if(retcd != kOK)  return retcd;
	while( retcd == kOK && index < size )
	{
		pAE = &(oVal[index++]);
	
		SQL_GET_INT(pAE->m_AEID, sqa);
		SQL_GET_STR(pAE->m_AEName, sqa);
		SQL_GET_STR(pAE->m_AETitle, sqa);
		SQL_GET_STR(pAE->m_hostName, sqa);
		SQL_GET_STR(pAE->m_IPAddress, sqa);
		SQL_GET_INT(pAE->m_port, sqa);
	 	SQL_GET_INT(pAE->m_level, sqa);
		SQL_GET_INT(pAE->m_priority, sqa);
		SQL_GET_STR(pAE->m_description, sqa);
		
		pAE->m_IsLocalAE = isLocalAE;

		retcd = sqa.MoveNext();
	}
	SQLExecuteEnd(sqa);	 

	GetAqLogger()->LogMessage(kDebug, "DEBUG: -CPxDB::QueryApplicationEntity end\n");
	return kOK;
}

//------------------------------------------------------------------------------------------------
//  Get All ApplicationEntity objects based on AE Title ID and different table type .
//
int  CPxDB::GetApplicationEntity(int iAETitleID, ApplicationEntity & oVal, AE_TYPE type)
{
	oVal.Clear();
	AqString whereFilter;
	whereFilter.Format(" WHERE ID=%d ", iAETitleID);

	vector <ApplicationEntity>  tmp;
	int ret = QueryApplicationEntity(type, tmp, whereFilter);
	if(ret != kOK) return ret;

	if(tmp.size() > 0)	oVal = tmp[0];
	return kOK;
}

//------------------------------------------------------------------------------------------------
//  Get All ApplicationEntity objects based on different table type.
//
int	CPxDB::GetAllApplicationEntity(vector <ApplicationEntity> &oVal, AE_TYPE type)
{
	return QueryApplicationEntity( type,  oVal, " ORDER BY AETitle ");
}

//------------------------------------------------------------------------------------------------
// Return true if AE exists in QRAllowedAE table.
//
bool CPxDB::IsRetrieveAE (const ApplicationEntity& iAE)
{
	AqString strSQL;
	//	-- 08/06/02	Not checking the port for now, because it creates a dependancy
	//		with the listening port setting in the config file
	//	strSQL.Format("Select  AETitle from QRAllowedAE WHERE AETitle = '%s' AND "\
	//		"IPAddress = '%s' AND Port = %d",iAE.AETitle(), iAE.IPAddress(), iAE.Port());
	
	strSQL.Format("SELECT ID FROM RemoteAE JOIN QRAllowedAE ON ID=AETitleID "
				  "WHERE AETitle='%s' AND IPAddress='%s'", iAE.m_AETitle, iAE.m_IPAddress);
	int id;
	int retcd = SQLGetInt(strSQL, id);
	return (retcd == kOK && id > 0);
}

//------------------------------------------------------------------------------------------------
// Get active local AETitles from LocalAE table. Map is used for quick search. 
//
int	CPxDB::GetAllLocalAE(int iPort, map< string, int >& localAEs, char* iHostname)
{
	AqString strSQL;
	
	if(iHostname && iHostname[0])
	{
		strSQL.Format( "SELECT distinct AETitle FROM LocalAE WHERE Level <> 0 AND HostName = '%s'" ,
			iHostname);
	}
	else
	{
		strSQL = "SELECT distinct AETitle FROM LocalAE WHERE Level <> 0";
	}

	//don't check port yet
	//strSQL.Format("SELECT distinct AETitle FROM LocalAE WHERE Level <> 0 AND PORT=%d", iPort);
	
	SQA sqa(getLocalDBType());
	GetAqLogger()->LogMessage(kDebug, "DEBUG: -CPxDB::GetAllLocalAE with: %s\n", strSQL);	
	sqa.SetCommandText(strSQL);
	int retcd = SQLExecuteBegin(sqa);
	if(retcd != kOK) 
	{
		localAEs.clear();
		return retcd;
	}
	
	retcd = sqa.MoveFirst(); if(retcd != kOK)  return retcd;
	while( retcd == kOK )
	{
		localAEs[sqa.getDataS()] = 1;  
		retcd = sqa.MoveNext();
	}
	SQLExecuteEnd(sqa);
	return kOK;
}

//------------------------------------------------------------------------------------------------
//
int	CPxDB::GetLocalAENames(int iPort, vector<string>&oVal)
{
	AqString	strSQL;
	strSQL.Format("SELECT AEName FROM LocalAE WHERE Port = %d ORDER BY AEName ", iPort);
	oVal.clear();
 
	SQA sqa(getLocalDBType());
	GetAqLogger()->LogMessage(kDebug, "DEBUG: -CPxDB::GetLocalAENames with: %s\n", strSQL);	
	return SQLStringValues(strSQL, oVal);

}


//------------------------------------------------------------------------------------------------
//
int	CPxDB::GetEnabledLocalAE(std::vector<ApplicationEntity>& oAEList, int iRetry)

{
	int ret;
	do
	{
		ret = QueryApplicationEntity( CPxDB::kLocalAE,  oAEList, " Where Level > 0");
		if(ret == kOK || m_cancelFlag)
			break;
		else
		{
			iRetry--;
			Sleep(5000);
		}
	} 
	while( iRetry >= 0);

	return ret;
}


//------------------------------------------------------------------------------------------------
// Return true get AE info if AE exists in StoreTargetAE  table.
//
bool CPxDB::IsStoreTargetAE (const char* iAETitle, ApplicationEntity& oAE)
{
	if(!iAETitle  ) 
	{
		return false;
	}
	
	AqString whereFilter;
	whereFilter.Format(" WHERE AETitle = '%s'", iAETitle);
	vector<ApplicationEntity>  oVal;
	int ret =  QueryApplicationEntity( CPxDB::kStoreTargetAE, oVal,whereFilter);
	if(ret != kOK)
		return false;

	if(oVal.size() < 1)
		return false;

	oAE = oVal[0]; // only one 
	return true;
}
 
 
//------------------------------------------------------------------------------------------------
// Get QRSourceAE based on group ID.
//
int	CPxDB::GetQRSourceAEs(int iGroupID, vector<ApplicationEntity> &oVal)
{
 	AqString whereFilter = "";
	if(!CanGroupAccessAllData(iGroupID))
	{
		whereFilter.Format(" JOIN  QRSourceAEGroupAssignment ON QRSourceAEID=ID"
						   " WHERE GroupID = %d", iGroupID);
	}
	return QueryApplicationEntity(CPxDB::kQRSourceAE, oVal,whereFilter);
}


// make filter helper
static void ConvertDate(const char* iDate, AqString& oDate1, AqString& oDate2, bool forTime=false)
{
	//	The date field can be in one of four configurations:
	//		1. yyyymmdd				only studies on this date
	//		2. -yyyymmdd			studies on or before this date
	//		2. yyyymmdd-			studies on or after this date
	//		4. yyyymmdd-yyyymmdd	studies on or between the two dates
	
	// DICOM time format: hhmmss.f

	char Date_buff_[128];
	strcpy(Date_buff_,iDate);

	char* pSeperator = strchr(Date_buff_, '-');
	if(pSeperator == 0)
	{
		oDate1 = Date_buff_; oDate1.TrimLeft(); oDate1.TrimRight();
		oDate2 = "";
	}
	else
	{
		*pSeperator = 0; // cut to two string
		oDate1 = Date_buff_; oDate1.TrimLeft(); oDate1.TrimRight();
		oDate2 = (pSeperator+1); oDate2.TrimLeft(); oDate2.TrimRight();
		*pSeperator = '-'; // restore it
		
		if(oDate1.IsEmpty())
		{
			if(forTime)
				oDate1 = "000001.00";
			else
				oDate1 = "19000101";
		}

		if(oDate2.IsEmpty())
		{
			SYSTEMTIME dt; GetSystemTime(&dt);
			if(forTime)
				oDate2.Format("%2d%2d%2d.6d", dt.wHour, dt.wMinute, dt.wSecond, dt.wMilliseconds);
			else
				oDate2.Format("%4d%2d%2d", dt.wYear, dt.wMonth, dt.wDay);
		}
	}

}


static AqString  ToSQLString(const char* inStr, bool iDoTrim=false)
{
	const char* p = strstr(inStr, "'");
	if(p == 0 && !iDoTrim)
		return inStr;

	AqString oStr;
	oStr = inStr;
	oStr.Replace("'", "''");
	if(iDoTrim)
		oStr.TrimRight();
	return oStr;

}

//#136 2021/01/12 N.Furutsuki unicode version
static AqUString  ToSQLUString(const wchar_t* inStr, bool iDoTrim = false)
{

	const wchar_t* p = wcsstr(inStr, L"'");
	if (p == 0 && !iDoTrim)
		return inStr;

	AqUString oStr;
	oStr = inStr;
	oStr.Replace(L"'", L"''");
	if (iDoTrim)
		oStr.TrimRight();
	return oStr;

}
static bool HasMatchChar(const char* inStr)
{

	return (inStr && (strstr(inStr, "*") != 0 || strstr(inStr, "?") != 0));
}

int CPxDB::MakeUserStudiesFilter(const DICOMData& iKey, AqString& cond, int iUserID)
{
	cond.Empty();
	AqString tmpStr;
	AqString range;
	AqString Date, Date2;

	std::map<const char*, const char*> mapConLike;
	std::map<const char*, const char*>::iterator iter;

	// setup key map
	mapConLike["PatientsName"] = iKey.m_patientsName;
	mapConLike["PatientID"] = iKey.m_patientID;
	mapConLike["StudyID"] = iKey.m_studyID;
	mapConLike["ReadingPhysiciansName"] = iKey.m_radiologistName;
	mapConLike["ReferringPhysiciansName"] = iKey.m_referringPhysiciansName;
	mapConLike["StudyDescription"] = iKey.m_studyDescription;
	mapConLike["SeriesDescription"] = iKey.m_seriesDescription;

	// walk throiugh map to generate query string
	for(iter = mapConLike.begin(); iter != mapConLike.end(); iter++)
	{
		if( iter->second[0] == 0)
			continue;

		if(!HasMatchChar(iter->second))
		{
			tmpStr.Format( "%s = '%s'", iter->first, ToSQLString(iter->second) );
			if(!cond.IsEmpty()) cond += " AND "; cond += tmpStr;
		}
		else if(strcmp(iter->second, "*") != 0) // skip match all condition
		{
			tmpStr.Format( "%s LIKE '%s'", iter->first, ToSQLString(iter->second, true) );
			//	Converting DICOM Wildcards to SQL Wildcards, GL
#if 0
			tmpStr.Replace('*', '%');
			tmpStr.Replace('?', '_');
#else
			//#62 2013/07/30
			sqlCharReplace(tmpStr);
#endif
			if(!cond.IsEmpty()) cond += " AND "; cond += tmpStr;
		}
	}

	if( iKey.m_modality[0] )
	{
		if(strstr(iKey.m_modality, "*") == 0 && strstr(iKey.m_modality, "?") == 0)
		{
			tmpStr.Format( "Modality = '%s'", iKey.m_modality);
			if(!cond.IsEmpty()) cond += " AND "; cond += tmpStr;
		}
		else if(strcmp(iKey.m_modality, "*") != 0) // skip match all condition
		{
			tmpStr.Format( "Modality LIKE '%s'", iKey.m_modality );
			//	Converting DICOM Wildcards to SQL Wildcards, GL
#if 0
			tmpStr.Replace('*', '%');
			tmpStr.Replace('?', '_');
#else
			//#62 2013/07/30
			sqlCharReplace(tmpStr);
#endif
			if(!cond.IsEmpty()) cond += " AND "; cond += tmpStr;
		}
	}

	std::map<const char*, const char*> mapConDate;

	mapConDate["PatientsBirthDate"] = iKey.m_patientsBirthDate;
	//	-- 12/18/01 - This was mostly here - I just took out #ifdef 0
	//	and completed it.  Incoming StudyDate and StudyDate2 will always have
	//	actual dates (not strings "BEFORE" or "AFTER".  
	mapConDate["StudyDate"] = iKey.m_studyDate;
	mapConDate["StudyTime"] = iKey.m_studyTime;

	for(iter = mapConDate.begin(); iter != mapConDate.end(); iter++)
	{
		if( iter->second[0] == 0)
			continue;

		ConvertDate(iter->second, Date, Date2);
		if( Date2.IsEmpty() )
		{
			range.Format( "%s='%s'", iter->first, Date );
		} 
		else
		{
			if( Date == Date2 )
				range.Format( "%s='%s'", iter->first, Date );
			else
				range.Format( "%s BETWEEN '%s' AND '%s'", iter->first, Date, Date2 );
		}
		if(!cond.IsEmpty()) cond += " AND "; cond += range;

	}


	std::map<const char*, const char*> mapConExact;
	mapConExact["PatientsSex"] = iKey.m_patientsSex;
	//mapConExact["PatientAge"] = iKey.m_patientAge;
	mapConExact["StudyInstanceUID"] = iKey.m_studyInstanceUID;
	mapConExact["SeriesInstanceUID"] = iKey.m_seriesInstanceUID;

	for(iter = mapConExact.begin(); iter != mapConExact.end(); iter++)
	{
		if( iter->second[0] == 0)
			continue;

		tmpStr.Format( "%s='%s'", iter->first, iter->second );
		if(!cond.IsEmpty()) cond += " AND "; cond += tmpStr;

	}
	
	//-----< Study Info >-----//
	if( iKey.m_accessionNumber[0]  != 0)
	{
		tmpStr.Format( "AccessionNumber= '%s'", iKey.m_accessionNumber );
		if(!cond.IsEmpty()) cond += " AND "; cond += tmpStr;
	}

	if( iKey.m_modalitiesInStudy[0] )
	{
		AqString Modality = iKey.m_modalitiesInStudy;
		int len = Modality.GetLength();
	//	int num = len / 3;
		int num = (len+2) / 3; // -- 05/13/2002


		AqString tStr = "(";
		for( int i=0; i<num; i++ )
		{
			tStr += "ModalitiesInStudy LIKE '%" + Modality.Mid( i*3, 2 ) + "%' OR ";
		}
		len = tStr.GetLength();
		if(!cond.IsEmpty()) cond += " AND "; cond += tStr.Left( len-4 ) + ")";
	}
	

	if(iUserID > 0 && iKey.m_readStatus >= 0)
	{
		AqString StatusStr, StatusCon;

		// short cut for all status
		if(iKey.m_readStatus == kDBAll)
		{
			StatusStr.Empty();
		}
		else if(iKey.m_readStatus == kDBAllVisable)
		{
			StatusStr.Format("(ReadStatus <> %d)", kDBIsHiden);
		}
		else
		{
			if(iKey.m_readStatus & kDBIsUnread)
			{
				StatusStr.Format("(ReadStatus = 0 OR ReadStatus = %d)", kDBIsUnread);
			}
			
			if(iKey.m_readStatus & kDBIsPartiallyRead)
			{
				StatusCon.Format("ReadStatus = %d", kDBIsPartiallyRead);
				StatusStr = (StatusStr.IsEmpty())?StatusCon:(StatusStr+" OR "+StatusCon);
			}

			if(iKey.m_readStatus & kDBIsRead)
			{
				StatusCon.Format("ReadStatus = %d", kDBIsRead);
				StatusStr = (StatusStr.IsEmpty())?StatusCon:(StatusStr+" OR "+StatusCon);
			}

			if(iKey.m_readStatus & kDBIsHiden)
			{
				StatusCon.Format("ReadStatus = %d", kDBIsHiden);
				StatusStr = (StatusStr.IsEmpty())?StatusCon:(StatusStr+" OR "+StatusCon);
			}
		}
		
		if(!StatusStr.IsEmpty())
		{
			StatusCon.Format("(UserID=%d AND (%s))", iUserID, StatusStr);
		}
		else
		{
			StatusCon.Empty();
		}
		
		if(!cond.IsEmpty() && !StatusCon.IsEmpty()) 
			cond += " AND "; 
		cond += StatusCon;
	}

	return kOK;
}


//------------------------------------------------------------------------------------------------
//
int CPxDB::SaveAuxDataInfo (AuxDataInfo& iAuxData, const vector<AuxReference>& iReferences)
{
	int i;
	GetAqLogger()->LogMessage(kDebug, "DEBUG: -CPxDB::SaveAuxDataInfo start\n");

	iAuxData.m_key = 0;
	
	//make date string
	char t[6]; //hhmmss 
	for ( i= 0; i<6; i++)
	{
		if (iAuxData.m_auxSeriesTime[i] == 0) t[i] = '0';
			else t[i] = iAuxData.m_auxSeriesTime[i];
	}
	AqString dateTime;
	dateTime.Format("%s %c%c:%c%c:%c%c", iAuxData.m_auxSeriesDate, t[0],t[1], t[2],t[3], t[4],t[5]);
	
	AqString	strSQL, QuerySQL ;

	// take out date constrain	
	//	id check sql
	// for ParametricMap store the first instance per series only
	if (iAuxData.m_type == AuxDataInfo::kParametricMapEnhancingRatio ||
			 iAuxData.m_type == AuxDataInfo::kParametricMapUptake ||
			 iAuxData.m_type == AuxDataInfo::kRigidTransform  ||
			 iAuxData.m_type == AuxDataInfo::kResampledVolume ) // SH, 2006.09.25
	{
		QuerySQL.Format("SELECT ID FROM PrivateData WHERE AuxSeriesUID='%s' AND Type=%d AND Name='%s' AND Subtype='%s' "
				"AND ProcessName='%s' AND ProcessType='%s' AND VolumesHash='%s' AND ParameterHash='%s' ",
				iAuxData.m_auxSeriesInstanceUID, iAuxData.m_type, ToSQLString(iAuxData.m_name), iAuxData.m_subtype,
				iAuxData.m_processName, iAuxData.m_processType, iAuxData.m_volumesHash, iAuxData.m_parameterHash);
	}
	else
	{
		QuerySQL.Format("SELECT ID FROM PrivateData WHERE AuxSOPUID='%s' AND Type=%d AND Name='%s' AND Subtype='%s' "
				"AND ProcessName='%s' AND ProcessType='%s' AND VolumesHash='%s' AND ParameterHash='%s'", 
				iAuxData.m_auxSOPInstanceUID, iAuxData.m_type, ToSQLString(iAuxData.m_name), iAuxData.m_subtype,
				iAuxData.m_processName, iAuxData.m_processType, iAuxData.m_volumesHash, iAuxData.m_parameterHash);
	}

	// aux data make sql
	strSQL.Format("IF NOT EXISTS (%s) INSERT PrivateData (AuxStudyUID, AuxSeriesUID, AuxSOPUID, "
			"Type, Name, Date, Subtype, ProcessName, ProcessType, VolumesHash, ParameterHash) values "
			"('%s', '%s', '%s', %d, '%s', '%s', '%s', '%s', '%s', '%s', '%s')", QuerySQL, 
			iAuxData.m_auxStudyInstanceUID, iAuxData.m_auxSeriesInstanceUID,iAuxData.m_auxSOPInstanceUID, 
			iAuxData.m_type, ToSQLString(iAuxData.m_name), dateTime, iAuxData.m_subtype, 
			iAuxData.m_processName, iAuxData.m_processType, iAuxData.m_volumesHash, iAuxData.m_parameterHash);

	SQA sqa(getLocalDBType());
	sqa.setOptions(kDBLockExecute|kDBExecuteNoRecords|kDBNoLogOnIntegrityViolation);
	sqa.SetCommandText(strSQL);

	int retcd = SQLExecuteBegin(sqa);
	if(retcd != kOK && retcd != kDBException) 
		return retcd;

	// get ID
	sqa.setOptions(0);
	sqa.SetCommandText(QuerySQL);
	retcd = SQLExecuteBegin(sqa);
	if(retcd != kOK) 
	{
		GetAqLogger()->LogMessage("ERROR: -CPxDB::SaveAuxDataInfo failed to get PK for AuxSOPInstanceUID = '%s'\n",
		iAuxData.m_auxSOPInstanceUID);
		return retcd;
	}

	retcd = sqa.MoveFirst(); 
	if(retcd != kOK)
		return retcd;
	
	iAuxData.m_key = sqa.getDataInt();

	int size = iReferences.size();
	
	sqa.setOptions(kDBLockExecute|kDBExecuteNoRecords|kDBNoLogOnIntegrityViolation);

	sqa.SetCommandText("INSERT PrivateDataReference "
				  "(PrivateDataID, AuxRefStudyUID, AuxRefSeriesUID, VolumeID) values (?,?,?,?)");

	sqa.AddParameter(iAuxData.m_key);
	sqa.AddParameter("");
	sqa.AddParameter("");
	sqa.AddParameter("");

	for (i = 0; i< size; i++)
	{
		sqa.SetParameter(iReferences[i].m_referencedStudyInstanceUID, 1);
		sqa.SetParameter(iReferences[i].m_referencedSeriesInstanceUID, 2);
		sqa.SetParameter(iReferences[i].m_volumeID, 3);

		SQLExecuteBegin(sqa);
	}
	
	SQLExecuteEnd(sqa);	 
	

	GetAqLogger()->LogMessage(kDebug, "DEBUG: -CPxDB::SaveAuxDataInfo end\n");
	return kOK; 
 }

//---------------------------------------------------------------------------------------
// -- 2006.05.26
int CPxDB::GetAuxDataInfo(int iAuxID, AuxDataInfo& oAuxInfo)
{
	GetAqLogger()->LogMessage(kDebug, "DEBUG: -CPxDB::GetAuxDataInfos start\n");
	AqString strSQL ;
	int retcd, size;
	
	const char* fields="AuxStudyUID,AuxSeriesUID,AuxSOPUID,Type,Name,Date,Subtype,ProcessName,ProcessType, VolumesHash, ParameterHash ";
	
	strSQL.Format("SELECT %s From PrivateData WHERE ID=%d",fields, iAuxID);
	
	SQA sqa(getLocalDBType());
	GetAqLogger()->LogMessage(kDebug, "DEBUG: -CPxDB::GetAuxDataInfos with: %s\n", strSQL);	
	sqa.SetCommandText(strSQL);

	if ((retcd = SQLExecuteBegin(sqa)) != kOK)
		return retcd;

	if ((size = sqa.GetRecordCount()) < 1)
	   return kNoResult;
	
	if ((retcd = sqa.MoveFirst()) != kOK)  
		return retcd;

	double tf;
	SYSTEMTIME st;

	oAuxInfo.m_key = iAuxID;
	SQL_GET_STR(oAuxInfo.m_auxStudyInstanceUID, sqa);
	SQL_GET_STR(oAuxInfo.m_auxSeriesInstanceUID, sqa);
	SQL_GET_STR(oAuxInfo.m_auxSOPInstanceUID, sqa);
	SQL_GET_INT(oAuxInfo.m_type,sqa);
	SQL_GET_STR(oAuxInfo.m_name,sqa);
	tf	= sqa.getDataDate();
	VariantTimeToSystemTime(tf,&st);
	sprintf(oAuxInfo.m_auxSeriesDate,"%d%02d%02d",st.wYear,st.wMonth,st.wDay);
	sprintf(oAuxInfo.m_auxSeriesTime,"%02d%02d%02d",st.wHour,st.wMinute,st.wSecond);
	SQL_GET_STR(oAuxInfo.m_subtype, sqa);
	SQL_GET_STR(oAuxInfo.m_processName,sqa);
	SQL_GET_STR(oAuxInfo.m_processType,sqa);
	SQL_GET_STR(oAuxInfo.m_volumesHash,sqa);
	SQL_GET_STR(oAuxInfo.m_parameterHash,sqa);
	SQLExecuteEnd(sqa);	
	
	return kOK;
}

//#include <afxdisp.h>
#include <time.h>
//------------------------------------------------------------------------------------------------
//  
int	CPxDB::GetAuxDataInfos(const char* iSeriesInstanceUID, vector<AuxDataInfo>&oVal )
{
	GetAqLogger()->LogMessage(kDebug, "DEBUG: -CPxDB::GetAuxDataInfos start\n");
	AqString strSQL ;

	const char* auxItmes = "ID, AuxStudyUID, AuxSeriesUID, AuxSOPUID, Type,	Name, Date, Subtype, ProcessName, ProcessType, VolumesHash, ParameterHash ";


	if (strcmp(iSeriesInstanceUID, "Template") == 0)
	{
		// 08/04/2002 -- 
		// a hack to get Template work
		strSQL.Format("SELECT %s FROM PrivateData WHERE Type=%d", auxItmes, AuxDataInfo::kTemplate);   
	}
	else
	{

		// iSeriesInstanceUID is refere to Referenced SeriesInstance UID 
		// OR the aux data SeriesInstance UID

		strSQL.Format("SELECT %s FROM PrivateData LEFT JOIN PrivateDataReference ON "
					  "ID=PrivateDataID WHERE AuxRefSeriesUID='%s' OR AuxSeriesUID='%s' ", 
					  auxItmes, iSeriesInstanceUID, iSeriesInstanceUID);
	}

	SQA sqa(getLocalDBType());
	GetAqLogger()->LogMessage(kDebug, "DEBUG: -CPxDB::GetAuxDataInfos with: %s\n", strSQL);	
	sqa.SetCommandText(strSQL);
	int retcd = SQLExecuteBegin(sqa);
	oVal.clear(); if(retcd != kOK) return retcd;
	int size = sqa.GetRecordCount(); if(size < 1) return kNoResult;
	oVal.resize(size);

	AuxDataInfo* pAuxDataInfo;
	int index = 0;
	double tf;
	SYSTEMTIME st;

	retcd = sqa.MoveFirst(); if(retcd != kOK)  return retcd;
	while( retcd == kOK && index < size )
	{
		pAuxDataInfo = &(oVal[index++]);

		SQL_GET_INT(pAuxDataInfo->m_key, sqa);
		SQL_GET_STR(pAuxDataInfo->m_auxStudyInstanceUID, sqa);
		SQL_GET_STR(pAuxDataInfo->m_auxSeriesInstanceUID, sqa);
		SQL_GET_STR(pAuxDataInfo->m_auxSOPInstanceUID, sqa);
		SQL_GET_INT(pAuxDataInfo->m_type,sqa);
		SQL_GET_STR(pAuxDataInfo->m_name,sqa);
		tf	= sqa.getDataDate();
		VariantTimeToSystemTime(tf,&st);
		sprintf(pAuxDataInfo->m_auxSeriesDate,"%d%02d%02d",st.wYear,st.wMonth,st.wDay);
		sprintf(pAuxDataInfo->m_auxSeriesTime,"%02d%02d%02d",st.wHour,st.wMinute,st.wSecond);
		SQL_GET_STR(pAuxDataInfo->m_subtype, sqa);
		SQL_GET_STR(pAuxDataInfo->m_processName,sqa);
		SQL_GET_STR(pAuxDataInfo->m_processType,sqa);
		SQL_GET_STR(pAuxDataInfo->m_volumesHash,sqa);
		SQL_GET_STR(pAuxDataInfo->m_parameterHash,sqa);

		retcd = sqa.MoveNext();
	}
	SQLExecuteEnd(sqa);	 

	GetAqLogger()->LogMessage(kDebug, "DEBUG: -CPxDB::GetAuxDataInfos end\n");
	return kOK;
	
}


//------------------------------------------------------------------------------------------------
//	 
int	CPxDB::GetAuxRefererces(const char* iAuxSOPInstanceUID, vector<AuxReference>& oVal)
{
	GetAqLogger()->LogMessage(kDebug, "DEBUG: -CPxDB::GetAuxRefererces start\n");

	SQA sqa(getLocalDBType());
	sqa.FormatCommandText("SELECT  AuxRefStudyUID, AuxRefSeriesUID, VolumeID FROM PrivateDataReference JOIN "
		"PrivateData ON PrivateDataID=ID WHERE AuxSOPUID='%s' ORDER BY PKey ASC", iAuxSOPInstanceUID);
	int retcd = SQLExecuteBegin(sqa);
	oVal.clear(); if(retcd != kOK) return retcd;
	int size = sqa.GetRecordCount(); if(size < 1) return kNoResult;
	oVal.resize(size);

	AuxReference* pAuxReference;
	int index = 0;
	retcd = sqa.MoveFirst(); if(retcd != kOK)  return retcd;
	while( retcd == kOK && index < size )
	{
		pAuxReference = &(oVal[index++]);

		SQL_GET_STR(pAuxReference->m_referencedStudyInstanceUID,sqa);
		SQL_GET_STR(pAuxReference->m_referencedSeriesInstanceUID,sqa);
		SQL_GET_STR(pAuxReference->m_volumeID,sqa);
		
		 
		retcd = sqa.MoveNext();
	}
	SQLExecuteEnd(sqa);	 

	GetAqLogger()->LogMessage(kDebug, "DEBUG: -CPxDB::GetAuxRefererces end\n");
	return kOK;
}


//------------------------------------------------------------------------------------------------
//  
int	CPxDB::CheckAuxDataStudy(int iAuxID, const char* iRefStudyUID, std::string& oAuxStudyUID)
{
	AqString	strSQL ;

	strSQL.Format("SELECT TOP 1 AuxStudyUID FROM PrivateData JOIN PrivateDataReference ON "
				"ID=PrivateDataID WHERE AuxRefStudyUID='%s' AND ID=%d", iRefStudyUID, iAuxID);


	return SQLGetString(strSQL, oAuxStudyUID);
	
}


//------------------------------------------------------------------------------------------------
//
int	CPxDB::HasAuxData(const char* iOrigStudyInstanceUID, int& oMask) 
{
	GetAqLogger()->LogMessage(kDebug, "DEBUG: -CPxDB::HasAuxData start\n");
	
	oMask = 0;
	SQA sqa(getLocalDBType());

	sqa.FormatCommandText("SELECT Type FROM PrivateDataReference JOIN "
		"PrivateData ON PrivateDataID=ID WHERE AuxRefStudyUID='%s'", iOrigStudyInstanceUID);

	int retcd = SQLExecuteBegin(sqa);
	if(retcd != kOK) return retcd;

	if (sqa.GetRecordCount() <=0) 
	{
		SQLExecuteEnd(sqa);
		return kOK;
	}

	int auxDatatype;
	retcd = sqa.MoveFirst(); if(retcd != kOK)  return retcd;
	while( retcd == kOK )
	{
		SQL_GET_INT(auxDatatype, sqa);
		//oMask |= AuxDataTypeToMask(auxDatatype);
		oMask |= auxDatatype;
		retcd = sqa.MoveNext();
	}

	SQLExecuteEnd(sqa);	 

	GetAqLogger()->LogMessage(kDebug, "DEBUG: -CPxDB::HasAuxData end\n");
	return kOK;
}
   
//------------------------------------------------------------------------------------------------
//
int	CPxDB::GetSeriesAuxDataMask(const char* iSeriesInstanceUID, int& oMask)
{
	GetAqLogger()->LogMessage(kDebug, "DEBUG: -CPxDB::GetSeriesAuxDataMask start\n");
	
	oMask = 0;
	SQA sqa(getLocalDBType());
 
	sqa.FormatCommandText("SELECT Type FROM PrivateDataReference JOIN "
		"PrivateData ON PrivateDataID=ID WHERE AuxRefSeriesUID='%s'", iSeriesInstanceUID);

	int retcd = SQLExecuteBegin(sqa);
	if(retcd != kOK) return retcd;

	if (sqa.GetRecordCount() <=0) 
	{
		SQLExecuteEnd(sqa);
		return kOK;
	}

	int auxDatatype;
	retcd = sqa.MoveFirst(); if(retcd != kOK)  return retcd;
	while( retcd == kOK )
	{
		SQL_GET_INT(auxDatatype, sqa);
		//oMask |= AuxDataTypeToMask(auxDatatype);
		oMask |= auxDatatype;
		retcd = sqa.MoveNext();
	}

	SQLExecuteEnd(sqa);	 

	GetAqLogger()->LogMessage(kDebug, "DEBUG: -CPxDB::GetSeriesAuxDataMask end\n");
	return kOK;
}

//------------------------------------------------------------------------------------------------
//
bool CPxDB::IsAuxSeries(const char* iSeriesInstanceUID)
{
	GetAqLogger()->LogMessage(kDebug, "DEBUG: -CPxDB::IsAuxSeries start\n");
	SQA sqa(getLocalDBType());
	
	sqa.FormatCommandText("SELECT Type FROM PrivateData WHERE AuxSeriesUID='%s'", iSeriesInstanceUID);

	int retcd = SQLExecuteBegin(sqa);
	if(retcd != kOK) return false;

	retcd = sqa.GetRecordCount();
	SQLExecuteEnd(sqa);

	GetAqLogger()->LogMessage(kDebug, "DEBUG: -CPxDB::IsAuxSeries end\n");
	return (retcd > 0);
}

//------------------------------------------------------------------------------------------------
//
extern void MakeWhereFilter(const DICOMData* iFilter, AqString& cond,  const char* receiveDate1=0, const char*  receiveDate2=0, int iUserID=0);
void MakeWhereFilter(const DICOMData* iFilter, AqString& cond,  const char* receiveDate1, const char*  receiveDate2, int iUserID)
{
	AqString filter = "";
	if(iFilter)
		CPxDB::MakeUserStudiesFilter(*iFilter, filter, iUserID); // no userID is needed
 

	AqString receiveDateFilter = "";
	AqString startHour = "00:00:00.000";
	AqString endHour = "23:59:59.999";


	if( !receiveDate2 || !receiveDate2[0]) 
	{
		if (receiveDate1 && receiveDate1[0]) {
			SYSTEMTIME dt; GetSystemTime(&dt);
			AqString  date2;
			date2.Format("%4d%2d%2d", dt.wYear, dt.wMonth, dt.wDay);
			receiveDateFilter.Format(" ModifyTime BETWEEN '%s %s' AND '%s %s' ", receiveDate1, startHour,  date2, endHour);
		}
	}
	else 
	{
		if(!receiveDate1 || !receiveDate1[0]) receiveDate1 = "19000101";
		receiveDateFilter.Format( " ModifyTime BETWEEN '%s %s' AND '%s %s' ", receiveDate1, startHour,  receiveDate2, endHour);
	}

	AqString whereFilter ;
	if(filter != "")
	{
		if (receiveDateFilter != "")
			whereFilter.Format(" WHERE %s AND %s ", filter, receiveDateFilter); 
		else 
			whereFilter.Format(" WHERE %s ", filter); 
	}
	else 
	{
		if (receiveDateFilter != "")
			whereFilter.Format(" WHERE %s ", receiveDateFilter); 
	}
	cond = whereFilter;
	
}

//------------------------------------------------------------------------------------------------
// 
int CPxDB::GetTotalSeries2Display(const DICOMData* iFilter,int iQueryType, int  &oTotalItems, const char* receiveDate1, const char*  receiveDate2)
{
	AqString whereFilter, strSQL, tmp;
	MakeWhereFilter(iFilter,  whereFilter, receiveDate1,  receiveDate2);

	if(iQueryType == CPxDB::kQueryUnassignedOnly) 
	{

		tmp = " NOT EXISTS (SELECT SerieslevelID FROM GroupSeries g Where g.SerieslevelID = "
			"SeriesLevel.SerieslevelID and PxDcmDB.dbo.CanGroupAccessAllData(g.GroupID) <> 1)";


		if(whereFilter == "")
		{
			whereFilter.Format( " WHERE %s ", tmp ) ; 
		}
		else 
		{
			whereFilter = whereFilter + " AND  " + tmp;
		}
 	} 
	else 
	{
		if(iQueryType == CPxDB::kQueryTemplateOnly) {
			if(whereFilter == "")
			{
				whereFilter.Format( " WHERE StudyLevel.StudyInstanceUID = '%s'", cTemplateStudyUID ) ; 
			}
			else 
			{
				whereFilter = whereFilter + " AND StudyLevel.StudyInstanceUID = '" + cTemplateStudyUID + "'" ;
			}
		}
	}
	
	strSQL.Format("SELECT count(SeriesLevel.SerieslevelID) FROM SeriesLevel JOIN StudyLevel ON SeriesLevel.StudyLevelID = StudyLevel.StudyLevelID %s " ,whereFilter);
	GetAqLogger()->LogMessage(kDebug, "DEBUG: -CPxDB::GetTotalSeries2Display with: %s\n", strSQL);	
	return  SQLGetInt(strSQL,  oTotalItems);

}

//------------------------------------------------------------------------------------------------
// no addition c_publicGroupID for AqNet_Public group 
int CPxDB::GetTotalSeries2Display4Group(const DICOMData* iFilter, int iGroupID, int &oTotalItems, const char* receiveDate1, const char*  receiveDate2)
{
	AqString whereFilter,joinStr, whereFinal, strSQL;
	MakeWhereFilter(iFilter,  whereFilter, receiveDate1,  receiveDate2);
	joinStr = "JOIN GroupSeries ON GroupSeries.SeriesLevelID=SeriesLevel.SeriesLevelID"; 
 	if (whereFilter == "")  
 		whereFinal.Format(" %s WHERE GroupID= %d ", joinStr, iGroupID);
 	else 
 		whereFinal.Format(" %s %s AND (GroupID= %d OR GroupID = %d) ",joinStr, whereFilter, 
		iGroupID, (c_publicGroup.m_groupUID != 0)?c_publicGroup.m_groupUID:iGroupID);

	strSQL.Format("SELECT count(SeriesLevel.SerieslevelID) FROM SeriesLevel JOIN StudyLevel ON "
		" SeriesLevel.StudyLevelID = StudyLevel.StudyLevelID %s " ,whereFinal);
	GetAqLogger()->LogMessage(kDebug, "DEBUG: -CPxDB::GetTotalSeries2Display4Group with: %s\n", strSQL);	

 	return SQLGetInt(strSQL,  oTotalItems);

}

//------------------------------------------------------------------------------------------------
//
int CPxDB::GetTotalSeries2Display4User(const DICOMData* iFilter, int iUserID, int iStatusType, int &oTotalItems, const char* receiveDate1, const char*  receiveDate2)
{

	std::string owhereFilter;
	int ret = MakeUserSeriesFilter(iFilter,  owhereFilter, iUserID, iStatusType,  receiveDate1,  receiveDate2);
	if(ret != kOK) return ret;

	AqString strSQL;
	// need DISTINCT for query againt UserSeriesView 
	strSQL.Format("SELECT count(DISTINCT SerieslevelID) FROM UserSeriesView %s " ,owhereFilter.c_str());
	GetAqLogger()->LogMessage(kDebug, "DEBUG: -CPxDB::GetTotalSeries2Display4User with: %s\n", strSQL);	
	return SQLGetInt(strSQL,  oTotalItems);

}

//------------------------------------------------------------------------------------------------
// The caller has to release the memory.
//
int CPxDB::GetSeriesDisplayInfoOnServer(vector<SeriesDisplayInfo> &oVal, const char* iWhereFilter, int iSize)
{
	GetAqLogger()->LogMessage(kDebug, "DEBUG: -CPxDB::GetSeriesDisplayInfoOnServer start\n");
	AqString	strSQL;
	AqString topStr = "";
	if(iSize > 0) topStr.Format("TOP %d", iSize);

	// put StudyTime at end for order clause
	strSQL.Format("SELECT DISTINCT %s  StudyInstanceUID, PatientsName, PatientID, PatientsBirthDate,"
				  "PatientsSex, StudyDate, StudyID, SeriesInstanceUID, SeriesNumber,SeriesDescription,"
				  "Modality, NumberOfSeriesRelatedInstances, DATEDIFF(Day,GETDATE(),HoldToDate), "
				  "seriesDate, seriesTime, studyTime,  s.SeriesLevelID  FROM  SeriesView s "
				  "JOIN StudyLevel ON s.StudyLevelID = StudyLevel.StudyLevelID ", topStr);

	if(iWhereFilter != 0)
		strSQL += iWhereFilter;
 

	SQA sqa(getLocalDBType());
	GetAqLogger()->LogMessage(kDebug, "DEBUG: -CPxDB::GetSeriesDisplayInfoOnServer with: %s\n", strSQL);	
	sqa.SetCommandText(strSQL);
	int retcd = SQLExecuteBegin(sqa);
	oVal.clear(); if(retcd != kOK) return retcd;
	int size = sqa.GetRecordCount(); if(size < 1) return kNoResult;
	oVal.resize(size);

	SeriesDisplayInfo* pInfo;
	int index = 0;
	retcd = sqa.MoveFirst(); if(retcd != kOK)  return retcd;
	while( retcd == kOK && index < size )
	{
		pInfo = &(oVal[index++]);
 
		SQL_GET_STR(pInfo->m_studyInstanceUID, sqa);
		SQL_GET_STR(pInfo->m_patientsName, sqa);
		SQL_GET_STR(pInfo->m_patientID, sqa);
		SQL_GET_STR(pInfo->m_patientsBirthDate, sqa);
		SQL_GET_STR(pInfo->m_patientsSex, sqa);
		SQL_GET_STR(pInfo->m_studyDate, sqa);
		SQL_GET_STR(pInfo->m_studyID, sqa);
		SQL_GET_STR(pInfo->m_seriesInstanceUID, sqa);
		SQL_GET_INT(pInfo->m_seriesNumber, sqa);
		SQL_GET_STR(pInfo->m_seriesDescription, sqa);
		SQL_GET_STR(pInfo->m_modality, sqa);
		SQL_GET_INT(pInfo->m_numberOfSeriesRelatedInstances, sqa);
		SQL_GET_INT(pInfo->m_daysToKeep, sqa);
		SQL_GET_STR(pInfo->m_seriesDate, sqa);
		SQL_GET_STR(pInfo->m_seriesTime, sqa);
		SQL_GET_STR(pInfo->m_studyTime, sqa); //2014/08/21 added by K.Ko
 
		retcd = sqa.MoveNext();
	}
	SQLExecuteEnd(sqa);	 

	GetAqLogger()->LogMessage(kDebug, "DEBUG: -CPxDB::GetSeriesDisplayInfoOnServer end\n");
	return kOK;
}
 	 

int	CPxDB::GetSeriesDisplayInfoOnServer(vector<SeriesDisplayInfo> &oVal, const DICOMData* iFilter, 
									   const char* receiveDate1, const char* receiveDate2)
{
	AqString whereFilter;
	MakeWhereFilter(iFilter,  whereFilter, receiveDate1,  receiveDate2);

	whereFilter += " ORDER by StudyDate DESC, StudyTime";

	return GetSeriesDisplayInfoOnServer (oVal, whereFilter);
}
 
//------------------------------------------------------------------------------------------------
//
int	CPxDB::GetSeriesDisplayInfoOnServer4Page(vector<SeriesDisplayInfo> &oVal, const DICOMData* iFilter, int iQueryType,
									    int iItemsPerPage, int iPageNumber, const char* receiveDate1, const char* receiveDate2)
{
	int numStarts = iItemsPerPage*(iPageNumber-1);

	AqString whereFilter, tmp;
	MakeWhereFilter(iFilter,  whereFilter, receiveDate1,  receiveDate2);
 
	if(iQueryType == CPxDB::kQueryTemplateOnly) {
		if(whereFilter == "")
		{
			whereFilter.Format( " WHERE StudyLevel.StudyInstanceUID = '%s'", cTemplateStudyUID ) ; 
		}
		else 
		{
			whereFilter = whereFilter + " AND StudyLevel.StudyInstanceUID = '" + cTemplateStudyUID + "'" ;
		}
	}
	else if(iQueryType == CPxDB::kQueryUnassignedOnly) 
	{
		tmp = " NOT EXISTS (SELECT SerieslevelID FROM GroupSeries g Where g.SerieslevelID = "
			"s.SerieslevelID and PxDcmDB.dbo.CanGroupAccessAllData(g.GroupID) <> 1)";

		if(whereFilter == "")
		{
			whereFilter.Format( " WHERE %s ", tmp ) ; 
		}
		else 
		{
			whereFilter = whereFilter + " AND  " + tmp;
		}
 	} 
	 
	
	AqString subSqlStr;
 
	subSqlStr.Format("SELECT DISTINCT TOP %d s.SeriesLevelID FROM SeriesView s JOIN StudyLevel "
			  "ON s.StudyLevelID = StudyLevel.StudyLevelID %s ", numStarts,whereFilter );
	 

	if(whereFilter == "")
	{
		whereFilter = " WHERE " ; 
	}
	else 
	{
		whereFilter = whereFilter + " AND ";
	}


	AqString whereCond;

	whereCond.Format(" %s s.SeriesLevelID NOT IN ( %s ORDER BY s.SeriesLevelID ) ORDER BY s.SeriesLevelID",  whereFilter, subSqlStr );

	return GetSeriesDisplayInfoOnServer (oVal, whereCond, iItemsPerPage);
}

//------------------------------------------------------------------------------------------------
// no addition of c_publicGroupID
int	CPxDB::GetSeriesDisplayInfoOnServer4Group4Page(vector<SeriesDisplayInfo> &oVal, const DICOMData* iFilter,
		 int iItemsPerPage, int iPageNumber, int iGroupID,const char* receiveDate1, const char* receiveDate2)
{
	AqString whereFilter, whereFinal;
	MakeWhereFilter(iFilter,  whereFilter, receiveDate1,  receiveDate2);
	AqString joinStr = "JOIN GroupSeries ON GroupSeries.SeriesLevelID=s.SeriesLevelID"; 

 	if (whereFilter == "")  
 		whereFinal.Format(" %s WHERE GroupID= %d ", joinStr, iGroupID);
 	else 
 		whereFinal.Format(" %s %s AND GroupID= %d ",joinStr, whereFilter, iGroupID);
 
 
	int numStarts = iItemsPerPage*(iPageNumber-1);
	AqString subSqlStr;
	subSqlStr.Format("SELECT DISTINCT TOP %d s.SeriesLevelID FROM SeriesView s JOIN StudyLevel "
				  "ON s.StudyLevelID = StudyLevel.StudyLevelID %s ", numStarts,whereFinal );


	AqString whereCond;
	whereCond.Format(" %s AND s.SeriesLevelID NOT IN ( %s ORDER BY s.SeriesLevelID) ORDER BY s.SeriesLevelID ",  whereFinal, subSqlStr );

	return GetSeriesDisplayInfoOnServer (oVal, whereCond, iItemsPerPage);
}

//------------------------------------------------------------------------------------------------
//
int CPxDB::MakeUserSeriesFilter(const DICOMData* iFilter, std::string &oFilter, int iUserID,int iStatusType, const char* receiveDate1, const char* receiveDate2)
{

	AqString filter;
	MakeWhereFilter(iFilter,  filter, receiveDate1,  receiveDate2,iUserID);

	// check groups the user belongs to
	vector<int> oGroupIDs;
	bool hasShared = false;
	int	ret = GetUserAssignedGroupIDs(iUserID, oGroupIDs, hasShared);
	if(ret != kOK)
		return ret;
 
	int size = oGroupIDs.size();
	if(size <1) return kNoResult;

	AqString whereFilter, moreCond;
	if(!hasShared)
	{
		AqString groupfilter = "" ;
		AqString tmp = "" ;
		for (int i=0; i<size-1; i++)
		{
			tmp.Format("%d, ", oGroupIDs[i]);
			groupfilter += tmp;
		}
		if(size > 0)
			moreCond.Format(" AND GroupID IN (%s %d) ", groupfilter, oGroupIDs[size-1]); 
	}
	 
 	if(iStatusType == kDBIsUnread) {
		whereFilter.Format(" UserID=%d AND  (ReadStatus = 0 OR ReadStatus = %d ) %s", iUserID, kDBIsUnread, moreCond);
	}
	else if(iStatusType == kDBIsRead){
		whereFilter.Format(" UserID=%d AND  ReadStatus = %d %s", iUserID, iStatusType, moreCond);
	}
	else {
		whereFilter.Format(" UserID=%d  %s", iUserID, moreCond); //all read status
	}

	if(filter == "") whereFilter = " WHERE " + whereFilter;
	else whereFilter = filter + " AND " + whereFilter;
	oFilter = whereFilter;
	return kOK;
}

//------------------------------------------------------------------------------------------------
//
int	CPxDB::GetSeriesDisplayInfoOnServer4User4Page(vector<SeriesDisplayInfo> &oVal, const DICOMData* iFilter,
		 int iItemsPerPage, int iPageNumber, int iUserID, int iStatusType, const char* receiveDate1, const char* receiveDate2)
{
	
	std::string owhereFilter;
	int ret = MakeUserSeriesFilter(iFilter,  owhereFilter, iUserID, iStatusType,  receiveDate1,  receiveDate2);
	if(ret != kOK) return ret;

	int numStarts = iItemsPerPage*(iPageNumber-1);
 
	AqString subSqlStr;
	subSqlStr.Format("SELECT DISTINCT TOP %d  SeriesLevelID FROM UserSeriesView %s ", numStarts,owhereFilter.c_str());


	AqString whereCond;
	whereCond.Format(" %s AND  SeriesLevelID NOT IN ( %s ORDER BY  SeriesLevelID) ORDER BY SeriesLevelID ", owhereFilter.c_str(), subSqlStr);

	return GetUserSeriesInfo(oVal,iUserID, whereCond, iItemsPerPage);
}

int	CPxDB::GetGroupSeriesInfo(vector<SeriesDisplayInfo> &oVal, int iGroupID)
{
	AqString wFilter;
	wFilter.Format("JOIN GroupSeries ON GroupSeries.SeriesLevelID=s.SeriesLevelID"
				   " WHERE GroupID=%d OR GroupID=%d ORDER by StudyDate DESC, StudyTime", 
				   iGroupID, (c_publicGroup.m_groupUID != 0)?c_publicGroup.m_groupUID:iGroupID);

	return GetSeriesDisplayInfoOnServer(oVal, wFilter);

}
 

int	CPxDB::MakeSeriesPath(const char* iTopDir, const char* iStudyUID, const char* iSeriesUID, std::string &oSeriesPath)
{
	oSeriesPath = "";
	if(!iTopDir || !iStudyUID || !iSeriesUID)
		return kParameterError;
	
	AqString tmpStr;
	tmpStr = iTopDir;
	if(GetFileAttributes(tmpStr) == 0xffffffff)
	{
		if(!CreateDirectory(tmpStr, 0))
			return kDBOpenError;
	}

	tmpStr = tmpStr + "\\" + iStudyUID;
	if(GetFileAttributes(tmpStr) == 0xffffffff)
	{
		if(!CreateDirectory(tmpStr, 0))
			return kDBOpenError;
	}

	tmpStr = tmpStr + "\\" + iSeriesUID;
	if(GetFileAttributes(tmpStr) == 0xffffffff)
	{
		if(!CreateDirectory(tmpStr, 0))
			return kDBOpenError;
	}

	oSeriesPath = tmpStr;
	return kOK;
}


//-----------------------------------------------------------------------
bool CPxDB::FileExists(const char* iDir, const char* iPat)
{
	if(!iDir || !iDir[0])
		return false;

	AqString fileSpec;
	WIN32_FIND_DATA FindFileData;
	HANDLE handle;
	bool yes;
	
	fileSpec = iDir;
	
	if(iPat)
		fileSpec = fileSpec + "/" + iPat;
	
	handle = FindFirstFile(fileSpec, &FindFileData);
	
	if ((yes = (handle != INVALID_HANDLE_VALUE)))
		FindClose(handle);
	return yes;
}

//-----------------------------------------------------------------------
int	CPxDB::GetStudyPath(const char* iStudyUID, string &oStudyPath) 
{
	oStudyPath.empty();

	AqString studyDir;
	// search the series that has pixel file.
	for (int i = 0; i < c_mediaPoints.size(); i++)
	{
//		seriesDir.Format("%c:\\%s\\%s\\%s\\%s.px0", c_mediaPoints[i].m_drive, 
//			c_mediaPoints[i].m_mediaLabel, iStudyUID, iSeriesUID, iSeriesUID);
		// tcz 2004.10.07 changed to v1.5 path generation
		studyDir.Format("%s\\%s\\%s", c_mediaPoints[i].m_mediaPoint, 
			c_mediaPoints[i].m_mediaLabel, iStudyUID);

		if(GetFileAttributes(studyDir) != 0xffffffff)
		{
			oStudyPath = (const char*)studyDir;
			return kOK;
/*
			if(iPattern && iPattern[0])
			{
				if (FileExists(studyDir, iPattern))
				{
					return kOK;
				}
				else
				{
					oStudyPath.empty();
					return kNoResult;
				}

			}
			else
			{
				return kOK;
			}
			*/
		}
	}

	return kNoResult;
}

//-----------------------------------------------------------------------
int	CPxDB::GetSeriesPath(const char* iStudyUID, const char* iSeriesUID, string &oSeriesPath, 
						const char* iPattern,const char* iMediaLabel) 
{

	int rcode = CPxDB::InitMediaPoints();
	if(rcode != kOK)
		return rcode;

	AqString seriesDir;
	if(iPattern && iPattern[0] && oSeriesPath.size())
	{
		seriesDir.Format("%s\\%s", oSeriesPath.c_str(), iPattern);
		if(GetFileAttributes(seriesDir) != 0xffffffff)
			return kOK;
	}

	oSeriesPath.empty();
	// search the series that has pixel file.
	for (int i = 0; i < c_mediaPoints.size(); i++)
	{
//		seriesDir.Format("%c:\\%s\\%s\\%s\\%s.px0", c_mediaPoints[i].m_drive, 
//			c_mediaPoints[i].m_mediaLabel, iStudyUID, iSeriesUID, iSeriesUID);
		// tcz 2004.10.07 changed to v1.5 path generation
		AqString mediaLabel;
		if ( !iMediaLabel )
			mediaLabel = c_mediaPoints[i].m_mediaLabel;
		else
			mediaLabel = iMediaLabel;
		
		seriesDir.Format("%s\\%s\\%s\\%s", c_mediaPoints[i].m_mediaPoint, 
			mediaLabel, iStudyUID, iSeriesUID);

		if(GetFileAttributes(seriesDir) != 0xffffffff)
		{
			oSeriesPath = (const char*)seriesDir;
			if(iPattern && iPattern[0])
			{
				if (FileExists(seriesDir, iPattern))
				{
					return kOK;
				}
				else
				{
					oSeriesPath = "";
					return kNoResult;
				}

			}
			else
			{
				return kOK;
			}
		}
		
	}

	return kNoResult;
}


//-----------------------------------------------------------------------
// Added by shiying hu, 2006-02-02
// AqPE needs a working directory. Currently, this working directory will be under 
// the first cache directory
// add size needed when querying the cache dir.
// this is for online PE which will have a working dir under AqNetCache to dump byte mask
int	CPxDB::GetAqNetCacheDir(string &oCacheDir)
{
	oCacheDir.empty();

	int rcode = CPxDB::InitMediaPoints();
	if(rcode != kOK)
		return rcode;

	if ( c_mediaPoints.size() == 0 )
		return kNoResult;


	AqString cacheDir;
	cacheDir.Format("%s%s", c_mediaPoints[0].m_mediaPoint, "AQNetCache");
	oCacheDir = (const char*)cacheDir;

	return kOK;
}

//-----------------------------------------------------------------------
// Added by shiying hu, 2006-02-02
// AqPE needs AqNetImport directory to dump its private dicom file
int	CPxDB::GetAqNetImportDir(string &oImportDir)  
{
	oImportDir.empty();

	std::string cacheDir;
	int rcode = GetAqNetCacheDir(cacheDir);
	if(rcode != kOK)
		return rcode;
	
	AqString importDir;
	importDir.Format("%s\\%s", cacheDir.c_str(), "AQNetImport");
	oImportDir = (const char*)importDir;

	return kOK;
}



//-----------------------------------------------------------------------
int	CPxDB::GetSeriesPath(const char* iSeriesUID, string &oSeriesPath)
{
	oSeriesPath.empty();

	AqString	strSQL;
	strSQL.Format("SELECT DISTINCT StudyInstanceUID FROM StudyLevel JOIN SeriesLevel ON "
		"StudyLevel.StudyLevelID=SeriesLevel.StudyLevelID WHERE seriesInstanceUID='%s'", iSeriesUID);

	string studyUID;
	int rcode = SQLGetString(strSQL, studyUID);
	if(rcode != kOK)
		return rcode;
	return GetSeriesPath(studyUID.c_str(),iSeriesUID, oSeriesPath );
}


//------------------------------------------------------------------------------------------------
//  Return all UserSeriesInfo and status based on Filter
//
int	CPxDB::GetUserSeriesInfo (vector<SeriesDisplayInfo>& oVal, int iUserID, const char* iWhereFilter, int iTopSize)
{
	GetAqLogger()->LogMessage(kDebug, "DEBUG: -CPxDB::GetUserSeriesInfo start\n");
	AqString	strSQL;
 
	AqString topNStr = "";
	if(iTopSize > 0)
		topNStr.Format(" Top %d ", iTopSize);
 
	strSQL.Format( "SELECT DISTINCT %s SeriesNumber, NumberOfSeriesRelatedInstances, StudyInstanceUID, "
		"SeriesInstanceUID, SeriesDescription, Modality, ReadStatus, PatientsName, PatientID, "
		"PatientsBirthDate, PatientsSex, StudyDate, StudyID, DATEDIFF(Day,GETDATE(), HoldToDate), "
		"ModifyTime, seriesDate, seriesTime, StudyTime, seriesLevelID FROM UserSeriesView", topNStr);

	if(iWhereFilter) strSQL += iWhereFilter;
				  
	SQA sqa(getLocalDBType());
	GetAqLogger()->LogMessage(kDebug, "DEBUG: -CPxDB::GetUserSeriesInfo with: %s\n", strSQL);	
	sqa.SetCommandText(strSQL);
	int retcd = SQLExecuteBegin(sqa);
	oVal.clear(); if(retcd != kOK) return retcd;
	int size = sqa.GetRecordCount(); if(size < 1) return kNoResult;
	oVal.resize(size);

	SeriesDisplayInfo* pInfo;
	int index = 0;
	retcd = sqa.MoveFirst(); if(retcd != kOK)  return retcd;
	while( retcd == kOK && index < size )
	{
		pInfo = &(oVal[index++]);
		
		SQL_GET_INT(pInfo->m_seriesNumber, sqa);
		SQL_GET_INT(pInfo->m_numberOfSeriesRelatedInstances, sqa);
		SQL_GET_STR(pInfo->m_studyInstanceUID, sqa);
		SQL_GET_STR(pInfo->m_seriesInstanceUID, sqa);
		SQL_GET_STR(pInfo->m_seriesDescription, sqa);
		SQL_GET_STR(pInfo->m_modality, sqa);
		SQL_GET_INT(pInfo->m_readStatus, sqa);
		SQL_GET_STR(pInfo->m_patientsName, sqa);
		SQL_GET_STR(pInfo->m_patientID, sqa);
		SQL_GET_STR(pInfo->m_patientsBirthDate, sqa);
		SQL_GET_STR(pInfo->m_patientsSex, sqa);
		SQL_GET_STR(pInfo->m_studyDate, sqa);
		SQL_GET_STR(pInfo->m_studyID, sqa);
	//	SQL_GET_INT(pInfo->m_offlineFlag, sqa);
		SQL_GET_INT(pInfo->m_daysToKeep, sqa);
		pInfo->m_seriesModifyTime = VariantTimeToTime_t(sqa.getDataDate()); // not query able yet
		SQL_GET_STR(pInfo->m_seriesDate, sqa);
		SQL_GET_STR(pInfo->m_seriesTime, sqa);
		SQL_GET_STR(pInfo->m_studyTime, sqa); //2014/08/21 added by K.Ko
		// no StudyTime, no seriesLevelID, go to next row
		retcd = sqa.MoveNext();  
	}
	SQLExecuteEnd(sqa);	 

	GetAqLogger()->LogMessage(kDebug, "DEBUG: -CPxDB::GetUserSeriesInfo end\n");
	return kOK;
}

//----------------------------------------------------------------------------------
// Get all distinct seriesDisplayInfo objects from all groups the specific user belongs. 
//
int	CPxDB::GetUserSeriesInfo (int iUserID, vector<SeriesDisplayInfo>& oVal, const DICOMData* iFilter)
{
	GetAqLogger()->LogMessage(kDebug, "DEBUG: -CPxDB::GetUserSeriesInfo(int iUserID, vector<SeriesDisplayInfo>& oVal, DICOMData* iFilter) start\n");
	AqString filter = "";
	if(iFilter)
	{
		CPxDB::MakeUserStudiesFilter(*iFilter, filter, iUserID);
	}
	if(filter != "")
		filter = "AND " + filter;

	// check groups the user belongs to
	vector<int> oGroupIDs;
	bool hasShared = false;
	int	ret = GetUserAssignedGroupIDs(iUserID, oGroupIDs, hasShared);
	if(ret != kOK)
		return ret;
 
	int size = oGroupIDs.size();
	if(size <1) return kNoResult;

	AqString whereFilter = "" ;
	if(hasShared)
	{
		whereFilter.Format(" WHERE UserID=%d ", iUserID);
	}
	else 
	{
		AqString groupfilter = "" , tmp = "" ;
		
		for (int i=0; i<size-1; i++)
		{
			tmp.Format("%d, ", oGroupIDs[i]);
			groupfilter += tmp;
		}
	 
		whereFilter.Format(" WHERE UserID=%d AND GroupID IN ( %s %d) ", iUserID ,groupfilter, oGroupIDs[size-1] );
	}

	whereFilter +=  filter ;
 //	whereFilter += " ORDER by StudyDate DESC, StudyTime ";

	ret = GetUserSeriesInfo (oVal,iUserID, whereFilter);
	GetAqLogger()->LogMessage(kDebug, "DEBUG: -CPxDB::GetUserSeriesInfo(int iUserID, vector<SeriesDisplayInfo>& oVal, DICOMData* iFilter) end\n");
	return ret; 
} 


//----------------------------------------------------------------------------------
// Get all distinct seriesDisplayInfo objects from all groups the specific user belongs. 
//
int	CPxDB::GetUserSeriesInfo (int iUserID, vector<SeriesDisplayInfo>& oVal, int iStatus, const DICOMData* iFilter)
{
	GetAqLogger()->LogMessage(kDebug, "DEBUG: -CPxDB::GetUserSeriesInfo(int iUserID, vector<SeriesDisplayInfo>& oVal, DICOMData* iFilter) start\n");
	AqString filter = "";
	if(iFilter)
	{
		CPxDB::MakeUserStudiesFilter(*iFilter, filter, iUserID);
	}
	if(filter != "")
		filter = "AND " + filter;

	// check groups the user belongs to
	vector<int> oGroupIDs;
	bool hasShared = false;
	int	ret = GetUserAssignedGroupIDs(iUserID, oGroupIDs, hasShared);
	if(ret != kOK)
		return ret;
 
	int size = oGroupIDs.size();
	if(size <1) return kNoResult;

	AqString whereFilter = "" ; AqString moreCond;
	if(!hasShared)
	{
		AqString tmp = "", groupfilter = "" ;
		for (int i=0; i<size-1; i++)
		{
			tmp.Format("%d, ", oGroupIDs[i]);
			groupfilter += tmp;
		}
		if(size > 0)
			moreCond.Format(" AND GroupID IN (%s %d) ", groupfilter, oGroupIDs[size-1]); 
	}
	 

	if(iStatus == kDBIsUnread) {
		whereFilter.Format(" WHERE UserID=%d AND  (ReadStatus = 0 OR ReadStatus = %d ) %s", iUserID, kDBIsUnread, moreCond);
	}
	else {
		whereFilter.Format(" WHERE UserID=%d AND  ReadStatus = %d %s", iUserID, iStatus, moreCond);
	}

 
	whereFilter +=  filter ;
//	whereFilter += " ORDER by StudyDate DESC, StudyTime ";

	ret = GetUserSeriesInfo (oVal,iUserID, whereFilter);
	GetAqLogger()->LogMessage(kDebug, "DEBUG: -CPxDB::GetUserSeriesInfo(int iUserID, vector<SeriesDisplayInfo>& oVal, DICOMData* iFilter) end\n");
	return ret; 
} 

//------------------------------------------------------------------------------------------------
// Get series status for user. It can be 0=unread, 1= read, 2= unassigned  
//
int	CPxDB::GetUserSeriesStatus(int iUserID, const char* iSeriesUID, int& oStatus)
{
	oStatus = kDBIsUnread;
  	AqString	strSQL;
	// this function can not get QR series read status yet
	strSQL.Format("EXEC PxDcmDB.dbo.GetSeriesReadStatus '%s', '%s' ", iUserID, iSeriesUID);

	return SQLGetInt(strSQL, oStatus);
} 
 
//------------------------------------------------------------------------------------------------
// Update user series status based on username and seriesUID. 


// changed by shiying hu 6-13-2005 due to database implementation change
int	CPxDB::UpdateUserSeriesStatus(const char* iSeriesUID, const int iUserID, int iStatus)
{	 
	if(!iSeriesUID) 
		return kParameterError;

  	AqString	strSQL;
	strSQL.Format("EXEC SetUserReadStatus '%s', '%d', %d", iSeriesUID, iUserID, iStatus);

	return SQLExecute(strSQL);
}

 


/* UserSeriesStatus is kind of history record, never delete it
*/

// User group functions

// for scan manager (--, 04/30/2003)
//-------------------------------------------------------------------------------------
// basic series query based on studyUID
// -- (04/29/2003)
//
int CPxDB::GetSeries(const char* iStudyUID, vector<string>& oSeries)
{
    AqString strSQL;
	strSQL.Format("SELECT SeriesInstanceUID FROM seriesLevel JOIN studylevel ON "
		" seriesLevel.studylevelID = studylevel.studylevelID where StudyInstanceUID='%s'", iStudyUID);

	return SQLStringValues(strSQL,oSeries);
}

//-------------------------------------------------------------------------------------
// basic sop query based on seriesUID
// -- (04/30/2003)
//
int CPxDB::GetSOPUID(const char* iSeriesUID, vector<string>& oSOP)
{
	//must have the series UID, otherwise it will be very slow
    AqString strSQL;
	strSQL.Format("SELECT SOPInstanceUID FROM InstanceView JOIN SeriesLevel ON "
		"InstanceView.seriesLevelID = seriesLevel.seriesLevelID where seriesInstanceUID='%s'", iSeriesUID);
	return SQLStringValues(strSQL, oSOP);
}

//-------------------------------------------------------------------------------------
// Forcibly remove a study - ignoring permission
// -- (04/29/2003)
int CPxDB::DeleteStudy(const char* iStudyUID)
{
	AqString strSQL;
	SQA sqa(getLocalDBType());
	int ret;

	//strSQL.Format("DELETE From StudyLevel where StudyInstanceUID='%s'",iStudyUID);
	strSQL.Format("Delete SeriesLevel From SeriesLevel JOIN studyLevel On "
			"StudyLevel.StudyLevelID = SeriesLevel.StudyLevelID "
			"where StudyInstanceUID='%s'",iStudyUID);
	ret = SQLExecute(strSQL);
	return ret;
}

int CPxDB::GetNumberOfSeriesRelatedInstances(const char* iSeriesInstanceUID)
{
	AqString strSQL;
	//strSQL.Format("SELECT NumberOfSeriesRelatedInstances FROM SeriesLevel WHERE "
	//	"SeriesInstanceUID='%s'", iSeriesInstanceUID);

	strSQL.Format("SELECT count(SOPInstanceUID) FROM SeriesLevel JOIN InstanceView "
		"ON SeriesLevel.SeriesLevelID=InstanceView.SeriesLevelID WHERE "
		"SeriesInstanceUID='%s'", iSeriesInstanceUID);

	int nInstances = 0;
	
	if(SQLGetInt(strSQL, nInstances) != kOK)
		nInstances = 0;
	
	return nInstances;
}

int	CPxDB::GetNumberOfSeriesRelatedFrames(const char* iSeriesInstanceUID)
{
	AqString strSQL;
	strSQL.Format("SELECT SUM(NumberOfFrames) FROM SeriesLevel JOIN InstanceView "
		"ON SeriesLevel.SeriesLevelID=InstanceView.SeriesLevelID WHERE "
		"SeriesInstanceUID='%s'", iSeriesInstanceUID);

	int nFrames = 0;
	
	if(SQLGetInt(strSQL, nFrames) != kOK)
		nFrames = 0;
	
	return nFrames;
}

// DICOM data
bool CPxDB::HasThisInstance (const char* iSopInstanceUID, const char* iSeriesInstanceUID)
{
	if(!iSopInstanceUID || !iSeriesInstanceUID[0])
		return false;

	AqString strSQL;

	//strSQL.Format("Select SeriesLevelID from InstanceView Where sopInstanceUID='%s'",
	//		iSopInstanceUID);

	//must have the series UID, otherwise it will be very slow
	strSQL.Format("SELECT top 1 InstanceView.seriesLevelID FROM InstanceView JOIN SeriesLevel ON "
		"InstanceView.seriesLevelID = seriesLevel.seriesLevelID AND seriesInstanceUID='%s' where "
		"sopInstanceUID='%s'", iSeriesInstanceUID, iSopInstanceUID);

	int SeriesLevelID, rcode;
	rcode = SQLGetInt(strSQL, SeriesLevelID);
	return (SeriesLevelID > 0 && rcode == kOK);

}


// DICOM data
int	CPxDB::RemoveInstance( const char* iSeriesUID, const char* iSopInstanceUID)
{
	if(!iSeriesUID || !iSopInstanceUID[0])
		return kParameterError;


	//must have the series UID, otherwise it will be very slow
	AqString dbStr;
	SQA sqa(getLocalDBType());
	int instanceID;

	// detect which db the instance record is
	sqa.FormatCommandText("SELECT DBName, InstanceLevelID FROM InstanceView JOIN "
		"SeriesLevel ON InstanceView.seriesLevelID=seriesLevel.seriesLevelID "
		"WHERE seriesInstanceUID ='%s' AND SOPInstanceUID='%s'", iSeriesUID, iSopInstanceUID);

	dbStr.Empty();
	instanceID = 0;
	sqa.setOptions(0);
	
	int retcd = SQLExecuteBegin(sqa);
	if(retcd != kOK)  return retcd;

	if(sqa.GetRecordCount() <= 0)
		return kNoResult;

	retcd = sqa.MoveFirst(); if(retcd != kOK)  return retcd;
	dbStr = sqa.getDataS();
	SQL_GET_INT(instanceID, sqa);

	sqa.FormatCommandText("delete %s.dbo.InstanceLevel Where InstanceLevelID = %d", dbStr, instanceID);
	sqa.setOptions(kDBLockExecute|kDBExecuteNoRecords);
	retcd = SQLExecuteBegin(sqa);
	if(retcd != kOK)  return retcd;

	SQLExecuteEnd(sqa);


	sqa.FormatCommandText("EXEC OnSeriesCompleted '%s'", iSeriesUID);
	sqa.setOptions(kDBLockExecute|kDBExecuteNoRecords);
	retcd = SQLExecuteBegin(sqa);
	//if(retcd != kOK)  return retcd;

	SQLExecuteEnd(sqa);

	sqa.FormatCommandText("delete dbo.PrivateData Where AuxSeriesUID='%s' and AuxSOPUID='%s'", iSeriesUID, iSopInstanceUID);
	sqa.setOptions(kDBLockExecute|kDBExecuteNoRecords);
	retcd = SQLExecuteBegin(sqa);
	if(retcd != kOK)  return retcd;

	SQLExecuteEnd(sqa);


	return kOK;

}


#define USE_PARAMTER

int	CPxDB::SaveDICOMData(const DICOMData& dData, int iInstanceStatus)
{
	GetAqLogger()->LogMessage(kDebug, "DEBUG: -CPxDB::SaveDICOMData start\n");

	SQA sqa(getLocalDBType());
	int retcd;

	//#136 2021/01/12 N.Furutsuki unicode version
	unsigned int iCodePage = CPxDB::getCodePageFromCharatorSet(dData.m_characterSet);
	if (iCodePage == _Def_MyCodePage_JIS){
		//fixed 2023/03/15
		//write JIS to DB (not convertion)
		iCodePage = 1252;//Latin1  
	}
	// get series id first
	int seriesID=0;
	sqa.FormatCommandText("SELECT SeriesLevelID FROM SeriesLevel WHERE SeriesInstanceUID='%s'",
		dData.m_seriesInstanceUID);
	sqa.setOptions(0);

	retcd = SQLExecuteBegin(sqa);
	if(retcd == kOK && sqa.MoveFirst()==kOK) 
		seriesID = sqa.getDataInt();

	// make the series start from making study
	if(seriesID == 0)
	{
		//this a new series, add to history DB first
		//MakeHistoryPatientInfo(dData, false);
		
		int studyID=0;
		// save the study data and get it's ID
		sqa.FormatCommandText("SELECT StudyLevelID FROM StudyLevel WHERE StudyInstanceUID='%s'", 
			dData.m_studyInstanceUID);

		// try to get study ID first
		sqa.setOptions(0);
		retcd = SQLExecuteBegin(sqa);
		if(retcd == kOK && sqa.MoveFirst()==kOK) 
		{
			studyID = sqa.getDataInt();
		}
		else
		{

			//#136 2021/01/12 N.Furutsuki unicode version
			AqUString UPpatientsName, UPhysiciansName, UStudyDescription;
			UPpatientsName.Convert(dData.m_patientsName, iCodePage);
			UPhysiciansName.Convert(dData.m_referringPhysiciansName, iCodePage);
			UStudyDescription.Convert(dData.m_studyDescription, iCodePage);

			sqa.SetCommandText(L"EXEC MakeStudy ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?");
			sqa.AddParameter(dData.m_studyInstanceUID);
			//sqa.AddParameter(dData.m_patientsName);
			sqa.AddParameter(UPpatientsName);
			sqa.AddParameter(dData.m_patientID);
			sqa.AddParameter(dData.m_patientsBirthDate);
			sqa.AddParameter(dData.m_patientsSex);
			sqa.AddParameter(dData.m_patientsAge);
			sqa.AddParameter(dData.m_studyDate);
			sqa.AddParameter(dData.m_studyTime);
			sqa.AddParameter(dData.m_accessionNumber);
			sqa.AddParameter(dData.m_studyID);
			sqa.AddParameter(dData.m_radiologistName);
			//sqa.AddParameter(dData.m_referringPhysiciansName);
			sqa.AddParameter(UPhysiciansName);
			//sqa.AddParameter(dData.m_modalitiesInStudy);
			sqa.AddParameter(dData.m_modality); // use series modality because at this time only one series exists
			//sqa.AddParameter(dData.m_studyDescription);
			sqa.AddParameter(UStudyDescription);
			sqa.AddParameter(dData.m_numberOfStudyRelatedSeries);
			sqa.AddParameter(dData.m_numberOfStudyRelatedInstances);
			sqa.AddParameter(dData.m_characterSet);

			sqa.setOptions(kDBLockExecute|kDBNoLogOnIntegrityViolation|kDBExecuteNoRecords);
			retcd = SQLExecuteBegin(sqa);
			if(retcd != kOK && retcd != kDBException)
				return retcd;
			

			//SQLCommit(sqa); // for new study id
			//SQLNewTrans(sqa);

			//get id agine in case make failed because other thread did first
			sqa.FormatCommandText("SELECT StudyLevelID FROM StudyLevel WHERE StudyInstanceUID='%s'", 
				dData.m_studyInstanceUID);

			// get study id again
			sqa.setOptions(0);
			retcd = SQLExecuteBegin(sqa);
			if(retcd != kOK)
				return retcd;
			
			retcd = sqa.MoveFirst(); 
			if(retcd != kOK)
				return retcd;
		
			studyID = sqa.getDataInt();

		}
		
		AqUString  USerisDescription;
		USerisDescription.Convert(dData.m_seriesDescription, iCodePage);

		// make series
		//sqa.SetCommandText("EXEC MakeSeries ?, ?, ?, ?, ?, ?, ?, ?, ?,?,?");
		sqa.SetCommandText(L"EXEC MakeSeries ?, ?, ?, ?, ?, ?, ?, ?,?,?,?");

		sqa.AddParameter(studyID);
		sqa.AddParameter(dData.m_seriesInstanceUID);
		sqa.AddParameter(dData.m_seriesNumber);
		//sqa.AddParameter(dData.m_seriesDescription);
		sqa.AddParameter(USerisDescription);
		sqa.AddParameter(dData.m_modality);
		sqa.AddParameter(dData.m_bodyPartExamined);
		sqa.AddParameter(dData.m_viewPosition);
		//sqa.AddParameter(0);	//dData.m_numberOfSeriesRelatedInstances,
		sqa.AddParameter(dData.m_stationName);
		sqa.AddParameter(dData.m_seriesDate);
		sqa.AddParameter(dData.m_seriesTime);
		sqa.AddParameter(dData.m_manufacturer);
		
		sqa.setOptions(kDBLockExecute|kDBNoLogOnIntegrityViolation|kDBExecuteNoRecords);
		retcd = SQLExecuteBegin(sqa);
		if(retcd != kOK && retcd != kDBException)
			return retcd;
		

		//get id agine in case make failed because other thread did first
		sqa.FormatCommandText("SELECT SeriesLevelID FROM SeriesLevel WHERE SeriesInstanceUID='%s'",
			dData.m_seriesInstanceUID);

		sqa.setOptions(0);
		retcd = SQLExecuteBegin(sqa);
		if(retcd == kOK && sqa.MoveFirst()==kOK) 
			seriesID = sqa.getDataInt();
	}

	if(dData.m_SOPInstanceUID[0] == 0)
	{
		SQLExecuteEnd(sqa);
		return kOK;
	}
	
//	sqa.SetCommandText("EXEC MakeInstance ?,?,?,?,?,?,?,?,?,?,?,?,?,?,?,?,?,?,?,?,?,?,?,?,?,?,?,?,?,?,?,?,?,?,?,?,?,?,?,?,?,?,?,?,?");
//#1 2012/02/10 K.Ko reduce the instanceLevel's field
// changed to 11 field 
	sqa.SetCommandText("EXEC MakeInstance ?,?,?,?,?,?,?,?,?,?,?"); // 10 field
	
	sqa.AddParameter(dData.m_SOPInstanceUID);
	sqa.AddParameter(seriesID);
	sqa.AddParameter(dData.m_SOPClassUID);
	sqa.AddParameter(dData.m_transferSyntax);

	sqa.AddParameter(dData.m_instanceNumber);
	sqa.AddParameter(dData.m_rows);
	sqa.AddParameter(dData.m_columns);
	sqa.AddParameter(dData.m_numberOfFrames);
	sqa.AddParameter(dData.m_imageTypeTokens);
//#1 2012/02/10 K.Ko reduce the instanceLevel's field
/*
	sqa.AddParameter(dData.m_bitsAllocated);
	sqa.AddParameter(dData.m_bitsStored);
	sqa.AddParameter(dData.m_highBit);
	sqa.AddParameter(dData.m_pixelRepresentation);
	sqa.AddParameter(dData.m_photometricInterpretation);
	sqa.AddParameter(dData.m_planarConfiguration);
	
	sqa.AddParameter(dData.m_windowWidth);
	sqa.AddParameter(dData.m_windowCenter);
	sqa.AddParameter(dData.m_smallestPixelValue);
	sqa.AddParameter(dData.m_largestPixelValue);
	sqa.AddParameter(dData.m_samplesPerPixel);

	sqa.AddParameter(dData.m_pixelSpacing[0]);
	sqa.AddParameter(dData.m_pixelSpacing[1]);
	sqa.AddParameter(dData.m_aspectRatio);
	sqa.AddParameter(dData.m_rescaleSlope);
	sqa.AddParameter(dData.m_rescaleIntercept);

	sqa.AddParameter(dData.m_patientOrientation);
	sqa.AddParameter(dData.m_slicePosition);
	sqa.AddParameter(dData.m_sliceThickness);

	sqa.AddParameter(dData.m_imagePosition[0]);
	sqa.AddParameter(dData.m_imagePosition[1]);
	sqa.AddParameter(dData.m_imagePosition[2]);

	sqa.AddParameter(dData.m_imageOrientation[0]);
	sqa.AddParameter(dData.m_imageOrientation[1]);
	sqa.AddParameter(dData.m_imageOrientation[2]);
	sqa.AddParameter(dData.m_imageOrientation[3]);
	sqa.AddParameter(dData.m_imageOrientation[4]);
	sqa.AddParameter(dData.m_imageOrientation[5]);
*/
	sqa.AddParameter(dData.m_pixelOffset);
	sqa.AddParameter(dData.m_dataSize);
//#1 2012/02/10 K.Ko reduce the instanceLevel's field
/*
	sqa.AddParameter(dData.m_referencedSOPInstanceUID);
	sqa.AddParameter(iInstanceStatus);

	sqa.AddParameter(dData.m_imageDate);
	sqa.AddParameter(dData.m_imageTime);
	sqa.AddParameter(dData.m_wasLossyCompressed);
	sqa.AddParameter(dData.m_scanOptions);

*/
	sqa.setOptions(kDBLockExecute|kDBExecuteNoRecords);
	retcd = SQLExecuteBegin(sqa);
	// try one more time in case make SOPClassID clashed
	if(retcd != kOK)
	{
		SQLExecuteEnd(sqa, false);
		GetAqLogger()->LogMessage(kInfo,"INFO: -CPxDB::SaveDICOMData try insert instance again when first attempt fail\n");
		retcd = SQLExecuteBegin(sqa);
	}
	if(retcd != kOK)
		return retcd;

	SQLExecuteEnd(sqa);
	GetAqLogger()->LogMessage(kDebug, "DEBUG: -CPxDB::SaveDICOMData end\n");
	return kOK;

}

int	CPxDB::GetPatientList( vector<DICOMPatient>& oVal, const DICOMData*  iFilter)
{
	GetAqLogger()->LogMessage(kDebug, "DEBUG: -CPxDB::GetPatientList start\n");
	AqUString filter = L"";
	if(iFilter)
	{
		CPxDB::MakeUserStudiesFilterU(*iFilter, filter);
		if(!filter.IsEmpty()) 
		{
			filter = L"WHERE " + filter;
		}
	}

	SQA sqa(getLocalDBType());

	sqa.FormatCommandText(L"SELECT DISTINCT PatientsName, PatientID, PatientsBirthDate,"\
				  L"PatientsSex FROM StudyLevel %s", filter);

	sqa.setOptions(kDBAsyncExecute);

	int retcd = SQLExecuteBegin(sqa); // do AsyncExecute
	oVal.clear(); if(retcd != kOK) return retcd;
	int size = sqa.GetRecordCount(); if(size < 1) return kNoResult;
	oVal.resize(size);

	DICOMPatient* pInfo;
	int index = 0;
	retcd = sqa.MoveFirst(); if(retcd != kOK)  return retcd;
	while( retcd == kOK && index < size )
	{
		pInfo = &(oVal[index++]);
				
		SQL_GET_STR(pInfo->m_patientsName, sqa);
		SQL_GET_STR(pInfo->m_patientID, sqa);
		SQL_GET_STR(pInfo->m_patientsBirthDate, sqa);
		SQL_GET_STR(pInfo->m_patientsSex, sqa);

		retcd = sqa.MoveNext();
	}
	SQLExecuteEnd(sqa);
	 
	GetAqLogger()->LogMessage(kDebug, "DEBUG: -CPxDB::GetPatientList end\n");
	return kOK;
}

//#139_Viwer(#2216)_Read_From_DB_Alwasy_UTF8
// added: bCnvUTF8
int	CPxDB::GetStudyList(vector<DICOMStudy>& oVal, const DICOMData* iFilter, int TopN, bool iSort, bool bCnvUTF8)
{
	GetAqLogger()->LogMessage(kDebug, "DEBUG: -CPxDB::GetStudyList start\n");
	AqUString filter = L"";
	if(iFilter)
	{
		CPxDB::MakeUserStudiesFilterU(*iFilter, filter);
		if(!filter.IsEmpty()) 
		{
			filter = L"WHERE " + filter;
		}
	}

	AqUString topNStr = L"";
	if(TopN > 0)
		topNStr.Format(L" Top %d ", TopN);

	SQA sqa(getLocalDBType());

	sqa.FormatCommandText(L"SELECT DISTINCT %s StudyInstanceUID, PatientsName, PatientID, PatientsBirthDate,"
			L"PatientsSex, PatientsAge, StudyDate, StudyTime, AccessionNumber, StudyID,"
			L"ReadingPhysiciansName, ReferringPhysiciansName, ModalitiesInStudy, StudyDescription,"
			L"NumberOfStudyRelatedSeries, NumberOfStudyRelatedInstances, CharacterSet, Status "
			L"FROM StudyLevel %s %s", topNStr, filter, iSort ? L" Order by StudyInstanceUID " : L"");

	sqa.setOptions(kDBAsyncExecute);

	int retcd = SQLExecuteBegin(sqa); // do AsyncExecute
	oVal.clear(); if(retcd != kOK) return retcd;
	int size = sqa.GetRecordCount(); if(size < 1) return kNoResult;
	oVal.resize(size);

	DICOMStudy* pInfo;
	int index = 0;
	retcd = sqa.MoveFirst(); if(retcd != kOK)  return retcd;
	while( retcd == kOK && index < size )
	{
		pInfo = &(oVal[index++]);
				
		SQL_GET_STR(pInfo->m_studyInstanceUID, sqa);
		if (bCnvUTF8){
			SQL_GET_STR_UTF8(pInfo->m_patientsName, sqa);//#139_Viwer(#2216)_Read_From_DB_Alwasy_UTF8
		}
		else{
			SQL_GET_STR(pInfo->m_patientsName, sqa);
		}
		SQL_GET_STR(pInfo->m_patientID, sqa);
		SQL_GET_STR(pInfo->m_patientsBirthDate, sqa);
		SQL_GET_STR(pInfo->m_patientsSex, sqa);
		SQL_GET_INT(pInfo->m_patientsAge, sqa);
		SQL_GET_STR(pInfo->m_studyDate, sqa);
		SQL_GET_STR(pInfo->m_studyTime, sqa);
		SQL_GET_STR(pInfo->m_accessionNumber, sqa);
		SQL_GET_STR(pInfo->m_studyID, sqa);
		SQL_GET_STR(pInfo->m_radiologistName, sqa);
		if (bCnvUTF8){
			SQL_GET_STR_UTF8(pInfo->m_referringPhysiciansName, sqa);//#139_Viwer(#2216)_Read_From_DB_Alwasy_UTF8
		}
		else{
			SQL_GET_STR(pInfo->m_referringPhysiciansName, sqa);
		}
		SQL_GET_STR(pInfo->m_modalitiesInStudy, sqa);
		if (bCnvUTF8){
			SQL_GET_STR_UTF8(pInfo->m_studyDescription, sqa);//#139_Viwer(#2216)_Read_From_DB_Alwasy_UTF8
		}
		else{
			SQL_GET_STR(pInfo->m_studyDescription, sqa);
		}
		SQL_GET_INT(pInfo->m_numberOfStudyRelatedSeries, sqa);
		SQL_GET_INT(pInfo->m_numberOfStudyRelatedInstances, sqa);
		SQL_GET_STR(pInfo->m_characterSet, sqa);
		SQL_GET_INT(pInfo->m_status, sqa);

		retcd = sqa.MoveNext();
	}
	SQLExecuteEnd(sqa);
	 
	GetAqLogger()->LogMessage(kDebug, "DEBUG: -CPxDB::GetStudyList end\n");
	return kOK;

}
//#60 2013/07/03
/*
*  iSortStudyDate: 0: none ,1:ASC, 2: DESC
*/
//#139_Viwer(#2216)_Read_From_DB_Alwasy_UTF8
// added: bCnvUTF8
int	CPxDB::GetStudyListEx(vector<DICOMStudy>& oVal, const DICOMData* iFilter, int TopN, int iSortStudyDate, bool bCnvUTF8)
{
	GetAqLogger()->LogMessage(kDebug, "DEBUG: -CPxDB::GetStudyList start\n");
	AqUString filter = L"";
	if(iFilter)
	{
		CPxDB::MakeUserStudiesFilterU(*iFilter, filter);
		if(!filter.IsEmpty()) 
		{
			filter = L"WHERE " + filter;
		}
	}

	AqUString topNStr = L"";
	if(TopN > 0)
		topNStr.Format(L" Top %d ", TopN);

	SQA sqa(getLocalDBType());

	AqUString orderByStudyDate;
	switch(iSortStudyDate){
		case 1: //ASC
		orderByStudyDate.Format(L"Order By StudyDate ASC, StudyTime ASC");
		break;
		case 2: //DESC
		orderByStudyDate.Format(L"Order By StudyDate DESC, StudyTime DESC");
		break;
	}

	sqa.FormatCommandText(L"SELECT DISTINCT %s StudyInstanceUID, PatientsName, PatientID, PatientsBirthDate,"
			L"PatientsSex, PatientsAge, StudyDate, StudyTime, AccessionNumber, StudyID,"
			L"ReadingPhysiciansName, ReferringPhysiciansName, ModalitiesInStudy, StudyDescription,"
			L"NumberOfStudyRelatedSeries, NumberOfStudyRelatedInstances, CharacterSet, Status "
			L"FROM StudyLevel %s %s", topNStr, filter, orderByStudyDate.IsEmpty() ? L"" : orderByStudyDate);

	sqa.setOptions(kDBAsyncExecute);

	int retcd = SQLExecuteBegin(sqa); // do AsyncExecute
	oVal.clear(); if(retcd != kOK) return retcd;
	int size = sqa.GetRecordCount(); if(size < 1) return kNoResult;
	oVal.resize(size);

	DICOMStudy* pInfo;
	int index = 0;
	retcd = sqa.MoveFirst(); if(retcd != kOK)  return retcd;
	while( retcd == kOK && index < size )
	{
		pInfo = &(oVal[index++]);
				
		SQL_GET_STR(pInfo->m_studyInstanceUID, sqa);
		if (bCnvUTF8){
			SQL_GET_STR_UTF8(pInfo->m_patientsName, sqa);//#139_Viwer(#2216)_Read_From_DB_Alwasy_UTF8
		}
		else{
			SQL_GET_STR(pInfo->m_patientsName, sqa);
		}
		SQL_GET_STR(pInfo->m_patientID, sqa);
		SQL_GET_STR(pInfo->m_patientsBirthDate, sqa);
		SQL_GET_STR(pInfo->m_patientsSex, sqa);
		SQL_GET_INT(pInfo->m_patientsAge, sqa);
		SQL_GET_STR(pInfo->m_studyDate, sqa);
		SQL_GET_STR(pInfo->m_studyTime, sqa);
		SQL_GET_STR(pInfo->m_accessionNumber, sqa);
		SQL_GET_STR(pInfo->m_studyID, sqa);
		SQL_GET_STR(pInfo->m_radiologistName, sqa);
		if (bCnvUTF8){
			SQL_GET_STR_UTF8(pInfo->m_referringPhysiciansName, sqa);//#139_Viwer(#2216)_Read_From_DB_Alwasy_UTF8
		}
		else{
			SQL_GET_STR(pInfo->m_referringPhysiciansName, sqa);
		}
		SQL_GET_STR(pInfo->m_modalitiesInStudy, sqa);
		if (bCnvUTF8){
			SQL_GET_STR_UTF8(pInfo->m_studyDescription, sqa);//#139_Viwer(#2216)_Read_From_DB_Alwasy_UTF8
		}
		else{
			SQL_GET_STR(pInfo->m_studyDescription, sqa);
		}
		SQL_GET_INT(pInfo->m_numberOfStudyRelatedSeries, sqa);
		SQL_GET_INT(pInfo->m_numberOfStudyRelatedInstances, sqa);
		SQL_GET_STR(pInfo->m_characterSet, sqa);
		SQL_GET_INT(pInfo->m_status, sqa);

		retcd = sqa.MoveNext();
	}
	SQLExecuteEnd(sqa);
	 
	GetAqLogger()->LogMessage(kDebug, "DEBUG: -CPxDB::GetStudyList end\n");
	return kOK;

}
//#139_Viwer(#2216)_Read_From_DB_Alwasy_UTF8
// added: bCnvUTF8
int	CPxDB::GetSeriesList(vector<DICOMSeries>& oVal, const DICOMData* iFilter, bool iSort, bool bCnvUTF8)
{
	GetAqLogger()->LogMessage(kDebug, "DEBUG: -CPxDB::GetSeriesList start\n");

	AqUString filter = L"";
	if(iFilter)
	{
		CPxDB::MakeUserStudiesFilterU(*iFilter, filter);
		if(!filter.IsEmpty()) filter = L"WHERE " + filter;
	}

	SQA sqa(getLocalDBType());

	sqa.FormatCommandText(L"SELECT DISTINCT StudyInstanceUID, SeriesInstanceUID, SeriesNumber, "
				L"SeriesDescription, Modality, BodyPartExamined, ViewPosition, "
				L"NumberOfSeriesRelatedInstances, StationName, "
				L"OfflineFlag,	QRFlag,	ModifyTime,	HoldToDate,	s.Status, SeriesDate, SeriesTime "
				L"FROM SeriesView s JOIN StudyLevel ON StudyLevel.StudyLevelID = "
				L"s.StudyLevelID %s %s", filter, iSort ? L" Order by StudyInstanceUID " : L"");
	
	sqa.setOptions(kDBAsyncExecute);

	int retcd = SQLExecuteBegin(sqa);
	oVal.clear(); if(retcd != kOK) return retcd;
	int size = sqa.GetRecordCount(); if(size < 1) return kNoResult;
	oVal.resize(size);

	DICOMSeries* pInfo;
	int index = 0;
	retcd = sqa.MoveFirst(); if(retcd != kOK)  return retcd;
	while( retcd == kOK && index < size )
	{
		pInfo = &(oVal[index++]);
				
		SQL_GET_STR(pInfo->m_studyInstanceUID, sqa);
		SQL_GET_STR(pInfo->m_seriesInstanceUID, sqa);
		SQL_GET_INT(pInfo->m_seriesNumber, sqa);
		if (bCnvUTF8){
			SQL_GET_STR_UTF8(pInfo->m_seriesDescription, sqa);//#139_Viwer(#2216)_Read_From_DB_Alwasy_UTF8
		}
		else{
			SQL_GET_STR(pInfo->m_seriesDescription, sqa); 
		}
		SQL_GET_STR(pInfo->m_modality, sqa);
		SQL_GET_STR(pInfo->m_bodyPartExamined, sqa);
		SQL_GET_STR(pInfo->m_viewPosition, sqa);
		SQL_GET_INT(pInfo->m_numberOfSeriesRelatedInstances, sqa);
		SQL_GET_STR(pInfo->m_stationName, sqa);
		SQL_GET_INT(pInfo->m_offlineFlag, sqa);
		SQL_GET_INT(pInfo->m_IsQRData, sqa);
		pInfo->m_seriesModifyTime = VariantTimeToTime_t(sqa.getDataDate());
		pInfo->m_seriesHoldToDate = VariantTimeToTime_t(sqa.getDataDate());
		//SQL_GET_DATE(pInfo->m_seriesModifyTime, sqa);
		//SQL_GET_DATE(pInfo->m_seriesHoldToDate, sqa);
		SQL_GET_INT(pInfo->m_status, sqa);
		// -- 2006.02.01
		SQL_GET_STR(pInfo->m_seriesDate, sqa);
		SQL_GET_STR(pInfo->m_seriesTime, sqa);
	 
		retcd = sqa.MoveNext();
	}

	SQLExecuteEnd(sqa);
	GetAqLogger()->LogMessage(kDebug, "DEBUG: -CPxDB::GetSeriesList end\n");
	 
	return kOK;

}


void MakeInstanceFilter(const DICOMData& iKey, AqString& cond)
{
	AqString tmpStr;

	if( iKey.m_studyInstanceUID[0] )
	{
		tmpStr.Format( "StudyInstanceUID='%s'", iKey.m_studyInstanceUID);
		if(!cond.IsEmpty()) cond += " AND "; cond += tmpStr;
	}
	
	if( iKey.m_seriesInstanceUID[0] )
	{
		tmpStr.Format( "SeriesInstanceUID='%s'", iKey.m_seriesInstanceUID);
		if(!cond.IsEmpty()) cond += " AND "; cond += tmpStr;
	}

	if( iKey.m_SOPInstanceUID[0] )
	{
		tmpStr.Format( "sOPInstanceUID='%s'", iKey.m_SOPInstanceUID);
		if(!cond.IsEmpty()) cond += " AND "; cond += tmpStr;
	}

	if( iKey.m_SOPClassUID[0] )
	{
		tmpStr.Format( "SOPClassID='%s'", iKey.m_SOPClassUID);
		if(!cond.IsEmpty()) cond += " AND "; cond += tmpStr;
	}

	if( iKey.m_instanceNumber )
	{
		tmpStr.Format( "InstanceNumber=%d", iKey.m_instanceNumber);
		if(!cond.IsEmpty()) cond += " AND "; cond += tmpStr;
	}

	if( iKey.m_imageTypeTokens[0] )
	{
		if(strcmp(iKey.m_imageTypeTokens, "*") != 0) // skip match all condition
		{
			tmpStr.Format( "ImageType LIKE '%s'", iKey.m_imageTypeTokens );
			if(!cond.IsEmpty()) cond += " AND "; cond += tmpStr;
		}
	}

	//	Converting DICOM Wildcards to SQL Wildcards, GL
#if 0
	cond.Replace('*', '%');
	cond.Replace('?', '_');
#else
			//#62 2013/07/30
			sqlCharReplace(cond);
#endif
}


//------------------------------------------------------------------------------------------------
int	CPxDB::GetPathToInstanceList(const char* iStudyUID, const char* iSeriesUID, const vector<AqString>& iInstances, vector<string>& oPaths)
{
	vector<const char*> instances;
	for(int i=0; i<iInstances.size(); i++)
	{
		instances.push_back(iInstances[i].GetString());
	}
	return GetPathToInstanceList(iStudyUID, iSeriesUID, instances, oPaths);
}

//------------------------------------------------------------------------------------------------
int	CPxDB::GetPathToInstanceList(const char* iStudyUID, const char* iSeriesUID, const vector<string>& iInstances, vector<string>& oPaths)
{
	vector<const char*> instances;
	for(int i=0; i<iInstances.size(); i++)
	{
		instances.push_back(iInstances[i].c_str());
	}
	return GetPathToInstanceList(iStudyUID, iSeriesUID, instances, oPaths);
}


struct ltstr 
{ 
  bool operator()(const char* s1, const char* s2) const 
  { 
    return strcmp(s1, s2) < 0; 
  } 


}; 

//------------------------------------------------------------------------------------------------
//	This one uses db
//	 
int	CPxDB::GetPathToInstanceList(const char* iStudyUID, const char* iSeriesUID, const vector<const char*>& iInstances, vector<string>& oPaths)
{
	GetAqLogger()->LogMessage(kDebug, "DEBUG: -CPxDB::GetPathToInstanceList start\n");

	SQA sqa(getLocalDBType());
	sqa.setOptions(kDBAsyncExecute);

	if(iInstances.size() == 1)
	{
		sqa.FormatCommandText("SELECT DISTINCT SOPInstanceUID, InstanceNumber FROM SeriesLevel JOIN InstanceView ON "
			"SeriesLevel.SeriesLevelID=InstanceView.SeriesLevelID WHERE SeriesInstanceUID='%s' and SOPInstanceUID='%s'", 
			iSeriesUID, iInstances[0]);
	}
	else
	{
		sqa.FormatCommandText("SELECT DISTINCT SOPInstanceUID, InstanceNumber FROM SeriesLevel JOIN InstanceView ON "
			"SeriesLevel.SeriesLevelID=InstanceView.SeriesLevelID WHERE SeriesInstanceUID='%s'", iSeriesUID);
	}

	int retcd = SQLExecuteBegin(sqa);
	oPaths.clear(); if(retcd != kOK) return retcd;
	int nFoundInstances = sqa.GetRecordCount(); if(nFoundInstances < 1) return kNoResult;

	if(nFoundInstances > 100000)
		return kParameterError;

	retcd = sqa.MoveFirst(); if(retcd != kOK)  return retcd;
	
	int nInputInstances = iInstances.size();
	oPaths.resize( (nInputInstances > 0)? nInputInstances :nFoundInstances );


	map<const char*,int, ltstr> mapInstUID;
	
	// make instanceUID order look up map
	int index ;
	if(nInputInstances > 0)
	{
		for (index=0; index<nInputInstances; index++)
		{
			mapInstUID[iInstances[index]] = index;
		}
	}

	AqString sopUID, instName, fullpath;
	string seriesPath = ""; // nno hint
	map<const char*,int, ltstr>::iterator iter;
	int count=0, slot = -1;
	index = 0;
	while( retcd == kOK && index < nFoundInstances )
	{
		index ++;

		sopUID = sqa.getDataS();

		// find the same insert slot as input instance list
		if(nInputInstances > 0)
		{
			iter = mapInstUID.find(sopUID);
			if(iter == mapInstUID.end())
			{
				retcd = sqa.MoveNext();
				continue;  // skip not requested instance
			}

			slot = iter->second;

		}
		else
		{
			slot++;
		}
		
		// make instance file name
		instName.Format("%05d_%s.dcm", sqa.getDataInt(), sopUID);

		if (GetSeriesPath(iStudyUID, iSeriesUID, seriesPath, instName) != kOK)
			return kNoDiskSpace;

		fullpath.Format("%s\\%s", seriesPath.c_str(), instName);

		oPaths[slot] = fullpath;
		count++;

		if(nInputInstances > 0 && nInputInstances == count)
			break; // get all asked paths, short cut it

		// keep seriesPath content for next GetSeriesPath hint

		retcd = sqa.MoveNext();
	}

	SQLExecuteEnd(sqa);	 


	GetAqLogger()->LogMessage(kDebug, "DEBUG: -CPxDB::GetPathToInstanceList end\n");
	return kOK;
}

int CPxDB::GetInstanceList( vector<DICOMInstance>& oVal, const DICOMData* iFilter, const char* iSeriesInstanceUID)
{

	if (!iSeriesInstanceUID || !*iSeriesInstanceUID)
	{
		GetAqLogger()->LogMessage("ERROR: CPxDB::GetInstanceList() - missing required argument iSeriesInstanceUID\n");
		return kParameterError;
	}

	GetAqLogger()->LogMessage(kDebug, "DEBUG: -CPxDB::GetUserSeries start\n");
	AqString strSQL; 
	AqString topStr="";
	AqString filter = "";
	

	if(iFilter->m_maxRecords > 0)
		topStr.Format("TOP %d", iFilter->m_maxRecords);

	strSQL = "SELECT DISTINCT StudyInstanceUID, SeriesInstanceUID, SOPInstanceUID, SOPClassUID," 
			 "InstanceNumber, ImageType FROM StudyLevel JOIN SeriesLevel ON "
			 "StudyLevel.StudyLevelID=SeriesLevel.StudyLevelID JOIN InstanceView ON "
			 "SeriesLevel.SeriesLevelID=InstanceView.SeriesLevelID JOIN SOPClassUIDs "
			 "ON InstanceView.SOPClassID = SOPClassUIDs.SOPClassID ";
	
	//!!!must have filter and the series UID, otherwise it will be very slow
	if(iFilter)
	{
		MakeInstanceFilter(*iFilter, filter);
		if(filter != "")
			strSQL += " WHERE "+filter;
	}

	SQA sqa(getLocalDBType()); sqa.setOptions(kDBAsyncExecute);
	GetAqLogger()->LogMessage(kDebug, "DEBUG: -CPxDB::GetInstanceList with: %s\n", strSQL);
	sqa.SetCommandText(strSQL);
	int retcd = SQLExecuteBegin(sqa);
	oVal.clear(); if(retcd != kOK) return retcd;
	int size = sqa.GetRecordCount(); if(size < 1) return kNoResult;

	if(size > 100000)
		return kParameterError;

	oVal.resize(size);

	DICOMInstance* pInfo;
	int index = 0;
	retcd = sqa.MoveFirst(); if(retcd != kOK)  return retcd;
	while( retcd == kOK && index < size )
	{
		pInfo = &(oVal[index++]);
		
		SQL_GET_STR(pInfo->m_studyInstanceUID, sqa);
		SQL_GET_STR(pInfo->m_seriesInstanceUID, sqa);
		SQL_GET_STR(pInfo->m_SOPInstanceUID, sqa);
		SQL_GET_STR(pInfo->m_SOPClassUID, sqa);
		SQL_GET_INT(pInfo->m_instanceNumber, sqa);
		SQL_GET_STR(pInfo->m_imageTypeTokens, sqa);

		retcd = sqa.MoveNext();
	}
	SQLExecuteEnd(sqa);	 

	GetAqLogger()->LogMessage(kDebug, "DEBUG: -CPxDB::GetUserSeries end\n");
	return kOK;
}

int CPxDB::GetInstanceList( vector<DICOMInstanceX>& oVal, const DICOMData*  iFilter, const char* iSeriesInstanceUID, int iNInstace)
{

	if (!iSeriesInstanceUID || !*iSeriesInstanceUID)
	{
		GetAqLogger()->LogMessage("ERROR: CPxDB::GetInstanceList() - missing required argument iSeriesInstanceUID\n");
		return kParameterError;
	}

	GetAqLogger()->LogMessage(kDebug, "DEBUG: -CPxDB::GetUserSeries start\n");
	AqString	strSQL;

	AqString topStr= "";
	if(iNInstace > 0)
	{
		topStr.Format(" Top %d ", iNInstace);
	}

	strSQL.Format("SELECT DISTINCT %s StudyInstanceUID, SeriesInstanceUID, SOPClassUID, Manufacturer," 
			 "InstanceView.* FROM StudyLevel "
			 "JOIN SeriesLevel ON StudyLevel.StudyLevelID=SeriesLevel.StudyLevelID "
			 "JOIN InstanceView ON SeriesLevel.SeriesLevelID=InstanceView.SeriesLevelID "
			 "JOIN SOPClassUIDs ON InstanceView.SOPClassID = SOPClassUIDs.SOPClassID ", topStr);

	//!!!must have filter and the series UID, otherwise it will be very slow
	if(iFilter)
	{
		AqString filter = "";
		MakeInstanceFilter(*iFilter, filter);
		if(filter != "")
			strSQL += " WHERE "+filter;
	}

	SQA sqa(getLocalDBType()); sqa.setOptions(kDBAsyncExecute);
	GetAqLogger()->LogMessage(kDebug, "DEBUG: -CPxDB::GetInstanceList with: %s\n", strSQL);
	sqa.SetCommandText(strSQL);
	int retcd = SQLExecuteBegin(sqa);
	oVal.clear(); if(retcd != kOK) return retcd;
	int size = sqa.GetRecordCount(); if(size < 1) return kNoResult;
	
	if(size > 100000)
		return kParameterError;
	
	oVal.resize(size);

	DICOMInstanceX* pInfo;
	int index = 0;
	retcd = sqa.MoveFirst(); if(retcd != kOK)  return retcd;
	while( retcd == kOK && index < size )
	{
		pInfo = &(oVal[index++]);
		
		SQL_GET_STR(pInfo->m_studyInstanceUID, sqa);
		SQL_GET_STR(pInfo->m_seriesInstanceUID, sqa);
		SQL_GET_STR(pInfo->m_SOPClassUID, sqa);
		SQL_GET_STR(pInfo->m_manufacturer, sqa);
		sqa.SkipData(); //InstanceLevelID
		SQL_GET_STR(pInfo->m_SOPInstanceUID, sqa);

		sqa.SkipData(); //SeriesLevelID
		sqa.SkipData(); //SOPClassID

		SQL_GET_INT(pInfo->m_transferSyntax, sqa);
		SQL_GET_INT(pInfo->m_instanceNumber, sqa);
		SQL_GET_INT(pInfo->m_rows, sqa);
		SQL_GET_INT(pInfo->m_columns, sqa);
		SQL_GET_INT(pInfo->m_numberOfFrames, sqa);

		SQL_GET_STR(pInfo->m_imageTypeTokens, sqa);
		SQL_GET_INT(pInfo->m_bitsAllocated, sqa);
		SQL_GET_INT(pInfo->m_bitsStored, sqa);
		SQL_GET_INT(pInfo->m_highBit, sqa);
		SQL_GET_INT(pInfo->m_pixelRepresentation, sqa);
		SQL_GET_INT(pInfo->m_photometricInterpretation, sqa);
		SQL_GET_INT(pInfo->m_planarConfiguration, sqa);

		SQL_GET_FLOAT(pInfo->m_windowWidth, sqa);
		SQL_GET_FLOAT(pInfo->m_windowCenter, sqa);
		SQL_GET_INT(pInfo->m_smallestPixelValue, sqa);
		SQL_GET_INT(pInfo->m_largestPixelValue, sqa);
		SQL_GET_INT(pInfo->m_samplesPerPixel, sqa);

		SQL_GET_FLOAT(pInfo->m_pixelSpacing[0], sqa);
		SQL_GET_FLOAT(pInfo->m_pixelSpacing[1], sqa);
		SQL_GET_FLOAT(pInfo->m_aspectRatio, sqa);
		SQL_GET_FLOAT(pInfo->m_rescaleSlope, sqa);
		SQL_GET_FLOAT(pInfo->m_rescaleIntercept, sqa);

		SQL_GET_STR(pInfo->m_patientOrientation, sqa);
		SQL_GET_FLOAT(pInfo->m_slicePosition, sqa);
		SQL_GET_FLOAT(pInfo->m_sliceThickness, sqa);
		SQL_GET_FLOAT(pInfo->m_imagePosition[0], sqa);
		SQL_GET_FLOAT(pInfo->m_imagePosition[1], sqa);
		SQL_GET_FLOAT(pInfo->m_imagePosition[2], sqa);
		SQL_GET_FLOAT(pInfo->m_imageOrientation[0], sqa);
		SQL_GET_FLOAT(pInfo->m_imageOrientation[1], sqa);
		SQL_GET_FLOAT(pInfo->m_imageOrientation[2], sqa);
		SQL_GET_FLOAT(pInfo->m_imageOrientation[3], sqa);
		SQL_GET_FLOAT(pInfo->m_imageOrientation[4], sqa);
		SQL_GET_FLOAT(pInfo->m_imageOrientation[5], sqa);
		
		SQL_GET_INT(pInfo->m_pixelOffset, sqa);
		SQL_GET_INT(pInfo->m_dataSize, sqa);
		SQL_GET_STR(pInfo->m_referencedSOPInstanceUID, sqa);
		
		SQL_GET_INT(pInfo->m_status, sqa);

		SQL_GET_STR(pInfo->m_imageDate, sqa);
		SQL_GET_STR(pInfo->m_imageTime, sqa);
		SQL_GET_INT(pInfo->m_wasLossyCompressed, sqa);
		SQL_GET_STR(pInfo->m_scanOptions, sqa);
		


		retcd = sqa.MoveNext();
	}
	SQLExecuteEnd(sqa);	 

	GetAqLogger()->LogMessage(kDebug, "DEBUG: -CPxDB::GetUserSeries end\n");
	return kOK;
}

int CPxDB::GetInstanceList( const DICOMStudy& iStudy, const DICOMSeries& iSeries, 
						  vector<DICOMData>& oVal, const char* iRealSeriesUID)
{
	GetAqLogger()->LogMessage(kDebug, "DEBUG: -CPxDB::GetInstanceList start\n");
	AqString	strSQL;

	const char* seriesUIDFilter = iRealSeriesUID;
	if(!seriesUIDFilter || !seriesUIDFilter[0])
		seriesUIDFilter = iSeries.m_seriesInstanceUID;

	strSQL.Format("SELECT DISTINCT SOPClassUID, Manufacturer, InstanceView.* FROM InstanceView "
			 "JOIN SeriesLevel ON SeriesLevel.SeriesLevelID=InstanceView.SeriesLevelID "
			 "JOIN SOPClassUIDs ON InstanceView.SOPClassID = SOPClassUIDs.SOPClassID "
			 "Where SeriesInstanceUID = '%s'", seriesUIDFilter);
	
	SQA sqa(getLocalDBType()); sqa.setOptions(kDBAsyncExecute);
	GetAqLogger()->LogMessage(kDebug, "DEBUG: -CPxDB::GetInstanceList with: %s\n", strSQL);
	sqa.SetCommandText(strSQL);
	int retcd = SQLExecuteBegin(sqa);
	oVal.clear(); if(retcd != kOK) return retcd;
	int size = sqa.GetRecordCount(); if(size < 1) return kNoResult;
	
	if(size > 100000)
		return kParameterError;
	
	oVal.resize(size);

	DICOMData* pInfo;
	int index = 0;
	retcd = sqa.MoveFirst(); if(retcd != kOK)  return retcd;
	while( retcd == kOK && index < size )
	{
		pInfo = &(oVal[index++]);
		
		ASTRNCPY(pInfo->m_studyInstanceUID, iStudy.m_studyInstanceUID);
		ASTRNCPY(pInfo->m_patientsName, iStudy.m_patientsName);
		ASTRNCPY(pInfo->m_patientID, iStudy.m_patientID);
		ASTRNCPY(pInfo->m_patientsBirthDate, iStudy.m_patientsBirthDate);
		ASTRNCPY(pInfo->m_patientsSex, iStudy.m_patientsSex);
		pInfo->m_patientsAge = iStudy.m_patientsAge;
		ASTRNCPY(pInfo->m_studyDate, iStudy.m_studyDate);
		ASTRNCPY(pInfo->m_studyTime, iStudy.m_studyTime);
		ASTRNCPY(pInfo->m_accessionNumber, iStudy.m_accessionNumber);
		ASTRNCPY(pInfo->m_studyID, iStudy.m_studyID);
		ASTRNCPY(pInfo->m_radiologistName, iStudy.m_radiologistName);
		ASTRNCPY(pInfo->m_referringPhysiciansName, iStudy.m_referringPhysiciansName);
		ASTRNCPY(pInfo->m_modalitiesInStudy, iStudy.m_modalitiesInStudy);
		ASTRNCPY(pInfo->m_studyDescription, iStudy.m_studyDescription);
		pInfo->m_numberOfStudyRelatedSeries = iStudy.m_numberOfStudyRelatedSeries;
		pInfo->m_numberOfStudyRelatedInstances = iStudy.m_numberOfStudyRelatedInstances;
		ASTRNCPY(pInfo->m_characterSet, iStudy.m_characterSet);

		ASTRNCPY(pInfo->m_seriesInstanceUID, iSeries.m_seriesInstanceUID);
		pInfo->m_seriesNumber = iSeries.m_seriesNumber;
		ASTRNCPY(pInfo->m_seriesDescription, iSeries.m_seriesDescription);
		ASTRNCPY(pInfo->m_modality, iSeries.m_modality);
		ASTRNCPY(pInfo->m_bodyPartExamined, iSeries.m_bodyPartExamined);
		ASTRNCPY(pInfo->m_viewPosition, iSeries.m_viewPosition);
		pInfo->m_numberOfSeriesRelatedInstances = iSeries.m_numberOfSeriesRelatedInstances;
		ASTRNCPY(pInfo->m_stationName, iSeries.m_stationName);
		pInfo->m_offlineFlag = iSeries.m_offlineFlag;
		pInfo->m_IsQRData = iSeries.m_IsQRData;
		pInfo->m_seriesModifyTime = iSeries.m_seriesModifyTime;
		pInfo->m_seriesHoldToDate = iSeries.m_seriesHoldToDate;
	
		SQL_GET_STR(pInfo->m_SOPClassUID, sqa);
		SQL_GET_STR(pInfo->m_manufacturer, sqa);
		sqa.SkipData(); //InstanceLevelID
		SQL_GET_STR(pInfo->m_SOPInstanceUID, sqa);

		sqa.SkipData(); //SeriesLevelID
		sqa.SkipData(); //SOPClassID

		SQL_GET_INT(pInfo->m_transferSyntax, sqa);
		SQL_GET_INT(pInfo->m_instanceNumber, sqa);
		SQL_GET_INT(pInfo->m_rows, sqa);
		SQL_GET_INT(pInfo->m_columns, sqa);
		SQL_GET_INT(pInfo->m_numberOfFrames, sqa);

		SQL_GET_STR(pInfo->m_imageTypeTokens, sqa);
		SQL_GET_INT(pInfo->m_bitsAllocated, sqa);
		SQL_GET_INT(pInfo->m_bitsStored, sqa);
		SQL_GET_INT(pInfo->m_highBit, sqa);
		SQL_GET_INT(pInfo->m_pixelRepresentation, sqa);
		SQL_GET_INT(pInfo->m_photometricInterpretation, sqa);
		SQL_GET_INT(pInfo->m_planarConfiguration, sqa);

		SQL_GET_FLOAT(pInfo->m_windowWidth, sqa);
		SQL_GET_FLOAT(pInfo->m_windowCenter, sqa);
		SQL_GET_INT(pInfo->m_smallestPixelValue, sqa);
		SQL_GET_INT(pInfo->m_largestPixelValue, sqa);
		SQL_GET_INT(pInfo->m_samplesPerPixel, sqa);

		SQL_GET_FLOAT(pInfo->m_pixelSpacing[0], sqa);
		SQL_GET_FLOAT(pInfo->m_pixelSpacing[1], sqa);
		SQL_GET_FLOAT(pInfo->m_aspectRatio, sqa);
		SQL_GET_FLOAT(pInfo->m_rescaleSlope, sqa);
		SQL_GET_FLOAT(pInfo->m_rescaleIntercept, sqa);

		SQL_GET_STR(pInfo->m_patientOrientation, sqa);
		SQL_GET_FLOAT(pInfo->m_slicePosition, sqa);
		SQL_GET_FLOAT(pInfo->m_sliceThickness, sqa);
		SQL_GET_FLOAT(pInfo->m_imagePosition[0], sqa);
		SQL_GET_FLOAT(pInfo->m_imagePosition[1], sqa);
		SQL_GET_FLOAT(pInfo->m_imagePosition[2], sqa);
		SQL_GET_FLOAT(pInfo->m_imageOrientation[0], sqa);
		SQL_GET_FLOAT(pInfo->m_imageOrientation[1], sqa);
		SQL_GET_FLOAT(pInfo->m_imageOrientation[2], sqa);
		SQL_GET_FLOAT(pInfo->m_imageOrientation[3], sqa);
		SQL_GET_FLOAT(pInfo->m_imageOrientation[4], sqa);
		SQL_GET_FLOAT(pInfo->m_imageOrientation[5], sqa);
		
		SQL_GET_INT(pInfo->m_pixelOffset, sqa);
		SQL_GET_INT(pInfo->m_dataSize, sqa);
		SQL_GET_STR(pInfo->m_referencedSOPInstanceUID, sqa);
		
		//SQL_GET_INT(pInfo->m_status, sqa);
		
		sqa.SkipData(); //m_status
		SQL_GET_STR(pInfo->m_imageDate, sqa);
		SQL_GET_STR(pInfo->m_imageTime, sqa);
		SQL_GET_INT(pInfo->m_wasLossyCompressed, sqa);
		SQL_GET_STR(pInfo->m_scanOptions, sqa);


		retcd = sqa.MoveNext();
	}
	SQLExecuteEnd(sqa);	 

	GetAqLogger()->LogMessage(kDebug, "DEBUG: -CPxDB::GetInstanceList end\n");
	return kOK;
}

int	CPxDB::GetInstanceStatus( const char* iSeriesUID, const char* iSopInstanceUID, int& iStatus)
{
	//must have the series UID, otherwise it will be very slow
	AqString strSQL;
	strSQL.Format("SELECT status FROM InstanceView JOIN SeriesLevel ON InstanceView.seriesLevelID "
			"=seriesLevel.seriesLevelID WHERE seriesInstanceUID='%s' AND SOPInstanceUID='%s'",
			iSeriesUID, iSopInstanceUID);

	return SQLGetInt(strSQL, iStatus);
}

int	CPxDB::SetInstanceStatus( const char* iSeriesUID, const char* iSopInstanceUID, int iStatus)
{
	//must have the series UID, otherwise it will be very slow
	AqString dbStr;
	SQA sqa(getLocalDBType());
	int instanceID, status = 0;

	// detect which db the instance record is
	sqa.FormatCommandText("SELECT DBName, InstanceLevelID, InstanceView.status FROM InstanceView JOIN "
		"SeriesLevel ON InstanceView.seriesLevelID=seriesLevel.seriesLevelID "
		"WHERE seriesInstanceUID ='%s' AND SOPInstanceUID='%s'", iSeriesUID, iSopInstanceUID);

	dbStr.Empty();
	instanceID = 0;
	sqa.setOptions(0);
	
	int retcd = SQLExecuteBegin(sqa);
	if(retcd != kOK)  return retcd;

	if(sqa.GetRecordCount() <= 0)
		return kNoResult;

	retcd = sqa.MoveFirst(); if(retcd != kOK)  return retcd;
	dbStr = sqa.getDataS();
	SQL_GET_INT(instanceID, sqa);
	SQL_GET_INT(status, sqa);
		
	if(status == iStatus)
		return kOK;

	sqa.FormatCommandText("Update %s.dbo.InstanceLevel SET status=%d "
		"Where InstanceLevelID = %d", dbStr, iStatus, instanceID);
	sqa.setOptions(kDBLockExecute|kDBExecuteNoRecords);
	retcd = SQLExecuteBegin(sqa);
	if(retcd != kOK)  return retcd;

	SQLExecuteEnd(sqa);
	return kOK;


}

int	CPxDB::MakeInstance( const DICOMData& iData)
{
	//must have the series UID, otherwise it will be very slow
	AqString dbStr;
	SQA sqa(getLocalDBType());
	int instanceID, seriesID, status = 0;

	// detect which db the instance record is
	sqa.FormatCommandText("SELECT DBName, InstanceLevelID, InstanceView.status, InstanceView.SeriesLevelID "
		"FROM InstanceView JOIN SeriesLevel ON InstanceView.seriesLevelID=seriesLevel.seriesLevelID "
		"WHERE seriesInstanceUID ='%s' AND SOPInstanceUID='%s'", 
		iData.m_seriesInstanceUID, iData.m_SOPInstanceUID);

	dbStr.Empty();
	instanceID = 0;
	seriesID = 0;
	sqa.setOptions(0);
	int retcd = SQLExecuteBegin(sqa);
	if(retcd != kOK)  return retcd;

	if(sqa.GetRecordCount() > 0)
	{
		retcd = sqa.MoveFirst(); if(retcd != kOK)  return retcd;
		dbStr = sqa.getDataS();
		SQL_GET_INT(instanceID, sqa);
		SQL_GET_INT(status, sqa);
		SQL_GET_INT(seriesID, sqa);
		
		sqa.FormatCommandText("delete from %s.dbo.InstanceLevel where InstanceLevelID=%d", dbStr, instanceID);
		sqa.setOptions(kDBLockExecute|kDBExecuteNoRecords);
		retcd = SQLExecuteBegin(sqa);
		if(retcd != kOK)  return retcd;

		if(dbStr == "PxDcmDB") // the first db does not need to move
			dbStr.Empty();

		//SQLCommit(sqa); // commit to let insert works
		//SQLNewTrans(sqa);
	}

	retcd = SaveDICOMData(iData, status);
	if(retcd != kOK)  return retcd;

	SQLExecuteEnd(sqa);
	return kOK;
}

int CPxDB::UpdatePixelMinMax(const DICOMData& iData)
{
	//must have the series UID, otherwise it will be very slow
	AqString dbStr;
	SQA sqa(getLocalDBType());
	int instanceID;

	// detect which db the instance record is
	sqa.FormatCommandText("SELECT DBName, InstanceLevelID FROM InstanceView JOIN SeriesLevel "
		"ON InstanceView.seriesLevelID=seriesLevel.seriesLevelID WHERE seriesInstanceUID ='%s' "
		"AND SOPInstanceUID='%s'", iData.m_seriesInstanceUID, iData.m_SOPInstanceUID);

	sqa.setOptions(0);
	int retcd = SQLExecuteBegin(sqa);
	if(retcd != kOK)  return retcd;

	retcd = sqa.MoveFirst(); if(retcd != kOK)  return retcd;
	dbStr = sqa.getDataS();
	instanceID = sqa.getDataInt();
		
	sqa.FormatCommandText("Update %s.dbo.InstanceLevel set SmallestPixelValue=%d, "
		"LargestPixelValue=%d where InstanceLevelID=%d", dbStr, 
		iData.m_smallestPixelValue, iData.m_largestPixelValue, instanceID);

	sqa.setOptions(kDBLockExecute|kDBExecuteNoRecords);
	retcd = SQLExecuteBegin(sqa);
	if(retcd != kOK)  return retcd;

	SQLExecuteEnd(sqa);
	return kOK;

}

bool CPxDB::VerifySeriesUIDByInstances(const char* iSeriesUID, const vector<string>& iInstanceUIDs)
{
	map<string, int> dbInstanceUIDs;
	dbInstanceUIDs.clear(); 


	SQA sqa(getLocalDBType());
	sqa.FormatCommandText("SELECT SOPInstanceUID FROM InstanceView JOIN SeriesLevel ON InstanceView"
			".seriesLevelID=seriesLevel.seriesLevelID WHERE seriesInstanceUID='%s'", iSeriesUID);

	int retcd = SQLExecuteBegin(sqa);
	if(retcd != kOK) return false;
	
	// no records, it must come from QR source, no check need.
	int size = sqa.GetRecordCount(); if(size < 1) return true;

	retcd = sqa.MoveFirst(); if(retcd != kOK)  return false;
	while( retcd == kOK )
	{
		dbInstanceUIDs[sqa.getDataS()] = 1;
		retcd = sqa.MoveNext();
	}
	SQLExecuteEnd(sqa);

	map<string, int>::iterator iter;
	string seriesStr;
	for(int i=0; i<iInstanceUIDs.size(); i++)
	{
		iter = dbInstanceUIDs.find(iInstanceUIDs[i]);
		if(iter == dbInstanceUIDs.end())
		{
			GetAqLogger()->LogMessage("ERROR: verify fail, instance(%s) of series(%s) missed in databas\n",
				iInstanceUIDs[i].c_str(), iSeriesUID );

			return false;
		}
	}

	return true;
}


//------------------------------------------------------------------------------------------------
// update series information
//
int	CPxDB::AddNewSeries(const char*  iSeriesUID)
{
	if(!iSeriesUID) 
		return kParameterError;

	// this will update the pre-computing fields on series and study levels and mark the series is completed
	AqString strSQL;
	strSQL.Format("EXEC OnSeriesCompleted '%s'", iSeriesUID);
	return SQLExecute(strSQL);
}


//------------------------------------------------------------------------------------------------
// Mark series is a incompleted series that will be deleted
//
int	CPxDB::MarkBadSeries(const char*  iSeriesUID)
{
	if(!iSeriesUID) 
		return kParameterError;

	// this will mark the series is incompleted and should be deleted at series pushing ending time
	AqString strSQL;
	strSQL.Format("UPDATE dbo.SeriesLevel SET Status = -1	WHERE SeriesInstanceUID='%s'", iSeriesUID);

	return SQLExecute(strSQL);
}


//------------------------------------------------------------------------------------------------
// Mark series is a incompleted series that will be deleted
//
CPxDB::eSeriesStatus CPxDB::GetSeriesStatus(const char*  iSeriesUID)
{
	if(!iSeriesUID) 
		return eNotExists;

	AqString strSQL;
	strSQL.Format("Select Status from dbo.SeriesLevel WHERE SeriesInstanceUID='%s'", iSeriesUID);

	int oVal;

	if(SQLGetInt(strSQL, oVal) != kOK)
		return eUnknown;

	eSeriesStatus stat;
	switch(oVal)
	{
		case -1:
			stat = eBadInCompleted;
			break;

		case 0:
			stat = eCompLeted;
			break;

		case 1:
			stat = eInprogress;
			break;
		default:
			stat = eUnknown;
			break;
	}

	return stat;
}

//------------------------------------------------------------------------------------------------
// Mark series is a incompleted series that will be deleted
//
int CPxDB::SetSeriesStatus(const char*  iSeriesUID, CPxDB::eSeriesStatus iStat)
{
	if(!iSeriesUID) 
		return kParameterError;


	int stat = 0;
	switch(iStat)
	{
		case eBadInCompleted:
			stat = -1;
			break;

		case eCompLeted:
			stat = 0;
			break;

		case eInprogress:
			stat = 1;
			break;

		default:
			stat = eUnknown;
			break;
	}


	// this will mark the series is incompleted and should be deleted at series pushing ending time
	AqString strSQL;
	strSQL.Format("UPDATE dbo.SeriesLevel SET Status = %d	WHERE SeriesInstanceUID='%s'", stat, iSeriesUID);

	return SQLExecute(strSQL);

}



//----------------------------------------------------------------------
// User group functions
int	CPxDB::DeleteSeries(const char* seriesUID) 
{
	GetAqLogger()->LogMessage(kDebug, "DEBUG: -CPxDB::DeleteSeries start\n");
	
	AqString	strSQL;

	strSQL.Format("DELETE SeriesLevel WHERE SeriesInstanceUID='%s'", seriesUID);

	int ret = SQLExecute(strSQL);
	
	GetAqLogger()->LogMessage(kDebug, "DEBUG: -CPxDB::DeleteSeries end\n");
	return ret;
}

//-------------------------------------------------------------------------
int	CPxDB::GetMapStrings(const char* iQueryStr, map<string, vector<string> >& oVal)
{
	GetAqLogger()->LogMessage(kDebug, "DEBUG: -CPxDB::GetMapStrings start\n");

	SQA sqa(getLocalDBType());
	GetAqLogger()->LogMessage(kDebug, "DEBUG: -CPxDB::GetMapStrings with: %s\n", iQueryStr);	

	sqa.SetCommandText(iQueryStr);
	int retcd = SQLExecuteBegin(sqa);
	if(retcd != kOK) 
	{
		oVal.clear();
		return retcd;
	}

	map<string, vector<string> >::iterator iter;
	string str1, str2;
	
	retcd = sqa.MoveFirst(); if(retcd != kOK)  return retcd;
	while( retcd == kOK)
	{
		str1 = sqa.getDataS();
		str2 = sqa.getDataS();

		iter = oVal.find(str1);
		if(iter == oVal.end())
		{
			vector<string> v;
			v.push_back(str2);
			oVal[str1] = v;
		}
		else
		{
			iter->second.push_back(str2);
		}
		
		retcd = sqa.MoveNext();
	}
	SQLExecuteEnd(sqa);	 

	GetAqLogger()->LogMessage(kDebug, "DEBUG: -CPxDB::GetMapStrings end\n");
	return kOK;
}
 

//-------------------------------------------------------------------------
int CPxDB::AssignGroupSeries(vector<int>& iGroupIDs, vector<string>& seriesUIDs)
{
	GetAqLogger()->LogMessage(kDebug, "DEBUG: -CPxDB::AssignGroupSeries() start\n");
 
	SQA sqa(getLocalDBType());
	AqString selectStr = "SELECT GroupID FROM GroupSeries INNER JOIN seriesLevel ON "
				   "GroupSeries.SeriesLevelID=seriesLevel.SeriesLevelID ";
 

	AqString whereStr;
	int i,j, retcd;
	for (i=0; i<iGroupIDs.size(); i++)
	{
		for(j=0;j<seriesUIDs.size();j++)
		{
			whereStr.Format("WHERE GroupSeries.GroupID=%d AND SeriesLevel.SeriesInstanceUID='%s' ",
				iGroupIDs[i], seriesUIDs[j].c_str());
			
			sqa.FormatCommandText("IF NOT EXISTS (%s %s) INSERT GroupSeries (GroupID, SeriesLevelID) SELECT %d, s.SeriesLevelID FROM SeriesLevel s WHERE s.SeriesInstanceUID='%s'", selectStr, whereStr, iGroupIDs[i],seriesUIDs[j].c_str() );
			retcd = SQLExecuteBegin(sqa);
			if(retcd != kOK) 
				return retcd;

		}
	}

	SQLExecuteEnd(sqa);	
 	GetAqLogger()->LogMessage(kDebug, "DEBUG: -CPxDB::AssignGroupSeries()  end\n");
	return retcd;
}

//-------------------------------------------------------------------------
int CPxDB::AssignGroupSeries(vector<int>& iGroupIDs, vector<string>& seriesUIDs,int iRetries, int iTimemSec)
{
	GetAqLogger()->LogMessage(kDebug, "DEBUG: -CPxDB::AssignGroupSeries() start\n");
 
	SQA sqa(getLocalDBType());
	AqString selectStr = "SELECT GroupID FROM GroupSeries INNER JOIN seriesLevel ON "
				   "GroupSeries.SeriesLevelID=seriesLevel.SeriesLevelID ";
 

	AqString whereStr;
	int i,j, retcd;
	for(j=0;j<seriesUIDs.size();j++)
	{

		int SeriesCnt=0, nRetries = iRetries;
		sqa.FormatCommandText("SELECT COUNT(*) from SeriesLevel WHERE SeriesLevel.SeriesInstanceUID='%s'",seriesUIDs[j].c_str());	
		// try to get study ID first
		sqa.setOptions(kDBNoTransaction);
		for (int k =0; k < nRetries && SeriesCnt == 0 ; k++)
		{
			retcd = SQLExecuteBegin(sqa);
			if(retcd == kOK && sqa.MoveFirst() == kOK) 
			{
				SeriesCnt = sqa.getDataInt();
			}
			SQLExecuteEnd(sqa);
			if (SeriesCnt < 1)
				::Sleep(iTimemSec);
			
		}

		for (i=0; i<iGroupIDs.size(); i++)
		{
			whereStr.Format("WHERE GroupSeries.GroupID=%d AND SeriesLevel.SeriesInstanceUID='%s' ",
				iGroupIDs[i], seriesUIDs[j].c_str());
			
			sqa.FormatCommandText("IF NOT EXISTS (%s %s) INSERT GroupSeries (GroupID, SeriesLevelID) SELECT %d, s.SeriesLevelID FROM SeriesLevel s WHERE s.SeriesInstanceUID='%s'", selectStr, whereStr, iGroupIDs[i],seriesUIDs[j].c_str() );
			retcd = SQLExecuteBegin(sqa);
			if(retcd != kOK) 
				return retcd;

		}
	}

	SQLExecuteEnd(sqa);	
 	GetAqLogger()->LogMessage(kDebug, "DEBUG: -CPxDB::AssignGroupSeries()  end\n");
	return retcd;
}


int CPxDB::UnassignGroupSeries(int iGroupID, vector<string>& seriesUIDs)
{
	GetAqLogger()->LogMessage(kDebug, "DEBUG: -CPxDB::UnassignGroupSeries() start\n");
 
	SQA sqa(getLocalDBType());
 
	int i, retcd;
	for(i=0;i<seriesUIDs.size();i++)
	{
		sqa.FormatCommandText("DELETE FROM groupSeries WHERE  GroupID = %d  AND SeriesLevelID = "
			"(SELECT  SeriesLevel.SeriesLevelID FROM SeriesLevel WHERE SeriesLevel.SeriesInstanceUID = '%s')", iGroupID, seriesUIDs[i].c_str());
		retcd = SQLExecuteBegin(sqa);
		if(retcd != kOK) 
			return retcd; 
	}
 
	SQLExecuteEnd(sqa);	
 	GetAqLogger()->LogMessage(kDebug, "DEBUG: -CPxDB::UnassignGroupSeries()  end\n");
	return retcd;
}

int	CPxDB::QueryPrinter(const char* iSQLStr, vector<PrinterInfo> &oVal)
{
	GetAqLogger()->LogMessage(kDebug, "DEBUG: -CPxDB::QueryPrinter start with SQL: %s\n", iSQLStr);

	SQA sqa(getLocalDBType());
	sqa.SetCommandText(iSQLStr);
	int retcd = SQLExecuteBegin(sqa);
	oVal.clear(); if(retcd != kOK) return retcd;
	int size = sqa.GetRecordCount(); if(size < 1) return kNoResult;
	oVal.resize(size);

	PrinterInfo* pInfo;
	int index = 0;
	retcd = sqa.MoveFirst(); if(retcd != kOK)  return retcd;
	while( retcd == kOK && index < size )
	{
		pInfo = &(oVal[index++]);
		SQL_GET_INT(pInfo->m_ID, sqa);
		SQL_GET_STR(pInfo->m_name, sqa);
		SQL_GET_STR(pInfo->m_AETitle, sqa);
		SQL_GET_STR(pInfo->m_IPAddress, sqa);
		SQL_GET_INT(pInfo->m_port, sqa);
		SQL_GET_INT(pInfo->m_color, sqa);
		SQL_GET_STR(pInfo->m_mediaSize, sqa);
		SQL_GET_STR(pInfo->m_destType, sqa);
		SQL_GET_STR(pInfo->m_orientation, sqa);
		SQL_GET_STR(pInfo->m_magType, sqa);
		SQL_GET_STR(pInfo->m_mediaType, sqa);
		SQL_GET_STR(pInfo->m_manufacturer, sqa);
		SQL_GET_STR(pInfo->m_model, sqa);
		SQL_GET_STR(pInfo->m_location, sqa);

		retcd = sqa.MoveNext();
	}
	SQLExecuteEnd(sqa);
	GetAqLogger()->LogMessage(kDebug, "DEBUG: -CPxDB::QueryPrinter ended\n");	
	return kOK;

}

int	CPxDB::QueryPrinter(vector<PrinterInfo>& oVal, int iGroupID, const char* iWhereFilter)
{
	GetAqLogger()->LogMessage(kDebug, "DEBUG: -CPxDB::QueryPrinter(vector<PrinterInfo>& oVal, int iGroupID, const char* iWhereFilter) start\n");
	
	AqString	strSQL = "SELECT DISTINCT Printer.* FROM Printer JOIN PrinterGroupAssignment "
					 "ON Printer.ID=PrinterGroupAssignment.PrinterID ";

	if (iWhereFilter)
	{
		strSQL = strSQL + iWhereFilter;
	}

	AqString clause;

	//	Not scan group - need to check if allowed
	if(iGroupID != c_admGroupID)
	{
		if (iWhereFilter)
		{
			clause = " AND WHERE ";
		}
		else
		{
			clause = " WHERE ";
		}
	
		AqString whereFilter;
		if (iGroupID)
		{
			whereFilter.Format("%s PrinterGroupAssignment.GroupID = %d ", clause, iGroupID);
			strSQL = strSQL + whereFilter;
		}
	}

	strSQL += " ORDER BY Printer.Name ";

	//TRACE( "SQLStr : %s\n", strSQL );
	return QueryPrinter(strSQL,  oVal);
}

int	CPxDB::GetPrinterInfo(const char* iName, PrinterInfo& oVal)
{
	GetAqLogger()->LogMessage(kDebug, "DEBUG: -CPxDB::GetPrinter(const char* iName, PrinterInfo& oVal) start\n");
	
	AqString	strSQL;
	strSQL.Format("SELECT DISTINCT Printer.* FROM Printer WHERE Name = '%s' ", iName);
	vector<PrinterInfo> oTemp;
	int retcd = QueryPrinter(strSQL,  oTemp);
	if(retcd != kOK ) return retcd;
	oVal = oTemp[0];
	return retcd;
}

int	CPxDB::GetPrinterNames(vector <string>&oVal)
{
	return SQLStringValues("SELECT Name FROM Printer", oVal);
}

int	CPxDB::GetPrinterAssignedGroupIDs(const char* iName, vector<int>& oGroupIDs)
{
	GetAqLogger()->LogMessage(kDebug, "DEBUG: -CPxDB::GetPrinterAssignedGroupIDs() starts\n");
	
	SQA sqa(getLocalDBType());
	sqa.FormatCommandText("SELECT pg.GroupID FROM PrinterGroupAssignment pg JOIN Printer p ON p.ID = pg.PrinterID WHERE p.Name = '%s'", iName);
 	
	int retcd = SQLExecuteBegin(sqa);
	oGroupIDs.clear(); if(retcd != kOK) return retcd;
	int size = sqa.GetRecordCount(); if(size < 1) return kNoResult;
	oGroupIDs.resize(size);

	int index = 0;
	retcd = sqa.MoveFirst(); if(retcd != kOK)  return retcd;
	while( retcd == kOK && index < size )
	{
		SQL_GET_INT(oGroupIDs[index++], sqa);
		retcd = sqa.MoveNext();
	}
	SQLExecuteEnd(sqa);
 
	return kOK;

}

int	CPxDB::InsertToPrinterGroupAssignment(SQA& iSQA, int iPrinterID, const vector<int>& iGroupIDs)
{
	int size = iGroupIDs.size();
	if (size <=0) return kParameterError;
	int retcd = -1;
	for(int i=0; i<size; i++)
	{
		/* While there are tokens in "string" */
		iSQA.FormatCommandText( "IF NOT EXISTS (SELECT PrinterID FROM PrinterGroupAssignment WHERE GroupID= %d AND PrinterID = %d) "\
					"INSERT PrinterGroupAssignment (PrinterID, GroupID) VALUES (%d, %d)", iGroupIDs[i], iPrinterID, iPrinterID, iGroupIDs[i]);
		retcd = SQLExecuteBegin(iSQA);
		if(retcd != kOK) 
		{
			return retcd;
		}
	}
	return kOK;
}

int	CPxDB::ModifyPrinter(const char* iUpdatePrinterSQL,int iPrinterUID, const vector<int> &iGroupIDs)
{

	GetAqLogger()->LogMessage(kDebug, "DEBUG: -CPxDB::ModifyPrinter start\n");
	SQA sqa(getLocalDBType());
	GetAqLogger()->LogMessage(kDebug, "DEBUG: -CPxDB::ModifyPrinter with: Update Printer SQL: %s\n", iUpdatePrinterSQL);	
	sqa.SetCommandText(iUpdatePrinterSQL);
	int retcd = SQLExecuteBegin(sqa);
	if(retcd != kOK) 
	{
		SQLExecuteEnd(sqa, false); // Nothing updated
		return retcd;
	}

	 // remove all entrie from printerGroupAssignment
	sqa.FormatCommandText("DELETE FROM PrinterGroupAssignment WHERE printerID = %d ",iPrinterUID);
	retcd = SQLExecuteBegin(sqa);
	if(retcd != kOK) 
	{
		SQLExecuteEnd(sqa, false); // Nothing updated
		return retcd;
	}
	// Insert all entries to printerGroupAssignment based on updated groups
	if(iGroupIDs.size() > 0){
		retcd = InsertToPrinterGroupAssignment(sqa,  iPrinterUID, iGroupIDs);
		if(retcd != kOK) 
		{
			SQLExecuteEnd(sqa, false); // Nothing updated
			return retcd;
		}
	}
   	SQLExecuteEnd(sqa);	
	GetAqLogger()->LogMessage(kDebug, "DEBUG: -CPxDB::ModifyPrinter end\n");
	return kOK;
}

int	CPxDB::AddPrinterDefinition(const char* iAddPrinterSQL, const char* iPrinterName, const vector<int>& iGroupIDs)
{
	GetAqLogger()->LogMessage(kDebug, "DEBUG: -CPxDB::AddPrinter start\n");
	if(!iAddPrinterSQL ) 
		return kParameterError;
 
	// Insert Printer into table first
	SQA sqa(getLocalDBType());
	int printerID = 0;
	int retcd =	InsertToPrinter(sqa, iAddPrinterSQL,iPrinterName, printerID);
	if(retcd != kOK) {
		SQLExecuteEnd(sqa, false); // Nothing updated
		return retcd;
	}
 
 	// Insert all entries to printerGroupAssignment based on updated groups
	if(iGroupIDs.size() > 0){
		retcd = InsertToPrinterGroupAssignment(sqa, printerID, iGroupIDs);
		if(retcd != kOK) 
		{
			SQLExecuteEnd(sqa, false); // Nothing updated
			return retcd;
		}
	}
	SQLExecuteEnd(sqa);	
	GetAqLogger()->LogMessage(kDebug, "DEBUG: -CPxDB::AddPrinter  end\n");
	return retcd;
}

int	CPxDB::InsertToPrinter(SQA& iSQA, const char* iAddPrinterSQL, const char* iPrinterName, int &oPrinterID)
{
 
	GetAqLogger()->LogMessage(kDebug, "DEBUG: -CPxDB::InsertToPrinter start with SQL: %s\n", iAddPrinterSQL);
	if(!iAddPrinterSQL ) 
		return kParameterError;
 
	// Insert Printer into table first
	iSQA.SetCommandText(iAddPrinterSQL);
	int retcd = SQLExecuteBegin(iSQA);
	if(retcd != kOK) 
	{
		SQLExecuteEnd(iSQA, false); // Nothing updated
		return retcd;
	}

	// Get printerUID for the entry just inserted
	int printerID = 0;
	iSQA.FormatCommandText("SELECT ID FROM Printer WHERE Name = '%s' ", iPrinterName); 

	retcd = SQLExecuteBegin(iSQA);
	if(retcd != kOK) 
	{
		SQLExecuteEnd(iSQA, false); // Nothing updated
		return retcd;
	}
	SQL_GET_INT(oPrinterID, iSQA);
	return kOK;
}

int CPxDB::GetRemoteAESpecification (int iRemoteAEID, RemoteAESpecification& oVal)
{
	GetAqLogger()->LogMessage(kDebug, "DEBUG: -CPxDB::GetRemoteAESpecification() starts\n");
	AqString strSQL;
	strSQL.Format("SELECT AETitleID FROM StoreTargetAE WHERE AETitleID = %d ",iRemoteAEID);
	int tmp = 0;
	int retcd = SQLGetInt(strSQL,tmp);
	if (retcd != kOK && retcd != kNoResult)  return retcd;
	if(tmp == iRemoteAEID) oVal.m_isStoreTargetAE = true;

	strSQL.Format("SELECT AETitleID FROM QRSourceAE WHERE AETitleID = %d ",iRemoteAEID);
	retcd = SQLGetInt(strSQL,tmp);
	if (retcd != kOK && retcd != kNoResult)  return retcd; 
	if(tmp == iRemoteAEID) oVal.m_isQRSourceAE = true;

	strSQL.Format("SELECT AETitleID FROM QRAllowedAE WHERE AETitleID = %d ",iRemoteAEID);
	retcd = SQLGetInt(strSQL,tmp);
	if (retcd != kOK && retcd != kNoResult)  return retcd; 
	if(tmp == iRemoteAEID) oVal.m_isQRAllowedAE = true;

	return retcd = retcd == kNoResult? kOK:retcd;

}


int CPxDB::AddRemoteAE(ApplicationEntity& iAE, const vector<int>& iGroupIDs,
						 int iQRAllowed, int iQRSource, int iStoreAE)
{
	GetAqLogger()->LogMessage(kDebug, "DEBUG: -CPxDB::InsertRemoteAE start\n");
 
	SQA sqa(getLocalDBType());
	sqa.FormatCommandText("EXEC MakeRemoreAE '%s', '%s', '%s', '%s', %d, %d, %d, '%s', %d, %d, %d", 
				iAE.m_AEName, iAE.m_AETitle, iAE.m_hostName, iAE.m_IPAddress, iAE.m_port, 
				iAE.m_level, iAE.m_priority, iAE.m_description, iStoreAE, iQRAllowed, iQRSource); 
	

	int retcd = SQLExecuteBegin(sqa);
 	if(retcd != kOK) 
		return retcd;

	sqa.FormatCommandText("SELECT ID FROM RemoteAE WHERE AEName='%s'",iAE.m_AEName);
	retcd = SQLExecuteBegin(sqa);
 	if(retcd != kOK) 
		return retcd;
	retcd = sqa.MoveFirst(); if(retcd != kOK)  return retcd;
	SQL_GET_INT(iAE.m_AEID, sqa);
	

	if(iQRSource)
	{
		retcd = AssignQRSourceAEGroup(sqa, iAE.m_AEID, iGroupIDs);
		if(retcd != kOK) return retcd;
	}
	SQLExecuteEnd(sqa);
 	GetAqLogger()->LogMessage(kDebug, "DEBUG: -CPxDB::AddRemoteAE  end\n");
	return retcd;
}
 

//--------------------------------------------------------------------------------
// AEName is not allowed to change. 
// If it is storeTarget AE, AE Title has to be unique, 
// otherwise, the combination of AE title, hostname, IP and port should be unique 
// This function may be combinated with other later on
// need remove entry from prefetchPattern if it is no longer as QRSource AE
// auto routing entry will be removed automatically by cascade if storeTarget AE is removed.

int CPxDB::ModifyRemoteAE(ApplicationEntity& iAE, const vector<int> &iGroupIDs,
						 int iQRAllowed, int iQRSource, int iStoreAE)
{

	GetAqLogger()->LogMessage(kDebug, "DEBUG: -CPxDB::ModifyRemoteAE() start\n");
	int AEID = iAE.m_AEID;
 // 1. remoteAE table
	AqString getSQL;
	if(iStoreAE){
		getSQL.Format(" SELECT ID FROM RemoteAE INNER JOIN StoreTargetAE ON RemoteAE.ID = StoreTargetAE.AETitleID WHERE AETitle = '%s' AND RemoteAE.ID <> %d",iAE.m_AETitle, AEID);
	}
	else {
		getSQL.Format(" SELECT ID FROM RemoteAE WHERE AETitle = '%s' AND HostName = '%s' AND IPAddress = '%s' AND Port = %d AND ID<> %d "\
			" ", iAE.m_AETitle, iAE.m_hostName, iAE.m_IPAddress, iAE.m_port, AEID);
	}
	
	SQA sqa(getLocalDBType());

	int id = -1;
	int retcd = SQLGetInt(getSQL, id);
	if(retcd == kOK) return kFailedUnknown; // need return here and borrow this kFailedUnknown since uniqueness does not meet.

	sqa.FormatCommandText(" UPDATE RemoteAE SET AETitle = '%s', HostName = '%s', IPAddress = '%s', Port = %d, Level = %d, Priority = %d, Description = '%s' WHERE AEName = '%s' "\
		" ", iAE.m_AETitle, iAE.m_hostName, iAE.m_IPAddress, iAE.m_port, iAE.m_level, iAE.m_priority, iAE.m_description, iAE.m_AEName); 

	 retcd = SQLExecuteBegin(sqa);
	if(retcd != kOK) {
		SQLExecuteEnd(sqa, false);
		return retcd;
	}

	// check remote AE specification
	
	if(iQRAllowed){
		sqa.FormatCommandText("IF NOT EXISTS (SELECT AETitleID FROM QRAllowedAE WHERE AETitleID = %d ) INSERT QRAllowedAE (AETitleID) VALUES (%d) ", AEID, AEID);
	}
	else {
		sqa.FormatCommandText("DELETE FROM QRAllowedAE WHERE  AETitleID  = %d", AEID);
	}
	retcd = SQLExecuteBegin(sqa);
	if(retcd != kOK) {
		SQLExecuteEnd(sqa, false);
		return retcd;
	}

	if(iStoreAE){
		sqa.FormatCommandText("IF NOT EXISTS (SELECT AETitleID FROM  StoreTargetAE WHERE AETitleID = %d ) INSERT StoreTargetAE (AETitleID) VALUES (%d) ", AEID, AEID);
	}
	else {
		sqa.FormatCommandText("DELETE FROM  StoreTargetAE WHERE  AETitleID  = %d", AEID);
	}
	retcd = SQLExecuteBegin(sqa);
	if(retcd != kOK) {
		SQLExecuteEnd(sqa, false);
		return retcd;
	}

	// always delete entries in QRSourceAEGroupAssignment
	sqa.FormatCommandText("DELETE FROM QRSourceAEGroupAssignment WHERE QRSourceAEID = %d ", AEID);
	retcd = SQLExecuteBegin(sqa);
	if(retcd != kOK) {
		SQLExecuteEnd(sqa, false);
		return retcd;
	}

	if(iQRSource){
		sqa.FormatCommandText("IF NOT EXISTS (SELECT AETitleID FROM QRSourceAE WHERE AETitleID = %d ) INSERT QRSourceAE (AETitleID) VALUES (%d) ", AEID, AEID);
		retcd = SQLExecuteBegin(sqa);
		if(retcd != kOK) {
			SQLExecuteEnd(sqa, false);
			return retcd;
		}
		// Insert new entries to QRSourceAEGroupAssignment if any
		retcd = AssignQRSourceAEGroup(sqa, AEID, iGroupIDs);

	}
	else {
		sqa.FormatCommandText("Delete from prefetchPattern WHERE ID = (SELECT PrefetchPatternID FROM PrefetchPatternAE p INNER JOIN "\
		" RemoteAE r ON p.QRSourceAEID = r.ID WHERE r.ID = %d) DELETE FROM QRSourceAE WHERE  AETitleID  = %d", AEID, AEID);
		retcd = SQLExecuteBegin(sqa);
	}
	
	if(retcd != kOK) {
		SQLExecuteEnd(sqa, false);
		return retcd;
	}
 
	SQLExecuteEnd(sqa);
	return retcd;

}


//------------------------------------------------------------------------------------------------
// Need delete entry form prefetchpattern table if it is QRSourceAE
//
int	CPxDB::DeleteRemoteAE(const char* iAEName)
{
	GetAqLogger()->LogMessage(kDebug, "DEBUG: -CPxDB::DeleteRemoteAE start\n");
 
	AqString strSQL;
	strSQL.Format("Delete from prefetchPattern WHERE ID = (SELECT PrefetchPatternID FROM PrefetchPatternAE p INNER JOIN "\
		" RemoteAE r ON p.QRSourceAEID = r.ID WHERE r.AEName = '%s') Delete From remoteAE Where AEName='%s'", iAEName,iAEName);
		 
	GetAqLogger()->LogMessage(kDebug, "DEBUG: -CPxDB::DeleteRemoteAE with SQL: %s\n", strSQL);	
	int retcd = SQLExecute(strSQL);
 	GetAqLogger()->LogMessage(kDebug, "DEBUG: -CPxDB::DeleteRemoteAE  end\n");
	return retcd;
}



//------------------------------------------------------------------------------------------------
// Will be called by multiple functions
//
int	CPxDB::AssignQRSourceAEGroup(int iAEID, const vector<int>& iGroupUIDs)
{
	int retcd = kOK;
	int size = iGroupUIDs.size();
	int groupID;

	SQA sqa(getLocalDBType());
	for(int i=0; i<size; i++)
	{
		groupID = iGroupUIDs[i];

		//While there are tokens in "string"  
		sqa.FormatCommandText( "IF NOT EXISTS (SELECT GroupID FROM QRSourceAEGroupAssignment "
			"WHERE GroupID= %d AND QRSourceAEID = %d) INSERT QRSourceAEGroupAssignment (QRSourceAEID, GroupID)"
			"VALUES (%d, %d)", groupID, iAEID, iAEID, groupID);
		retcd = SQLExecuteBegin(sqa);
		if(retcd != kOK) 
			return retcd;
	}
	SQLExecuteEnd(sqa);
	return retcd;
}

int	CPxDB::AssignQRSourceAEGroup(SQA& iSQA, int iAEID, const std::vector<int>& iGroupIDs)
{
	int retcd = kOK;
	int size = iGroupIDs.size();
	
	for(int i=0; i<size; i++)
	{ 
		iSQA.FormatCommandText( "IF NOT EXISTS (SELECT GroupID FROM QRSourceAEGroupAssignment "
			"WHERE GroupID= %d AND QRSourceAEID = %d) INSERT QRSourceAEGroupAssignment (QRSourceAEID, GroupID)"
			"VALUES (%d, %d)", iGroupIDs[i], iAEID, iAEID, iGroupIDs[i]);
		retcd = SQLExecuteBegin(iSQA);
		if(retcd != kOK) 
			return retcd;
	}
 
	return retcd;
}

//------------------------------------------------------------------------------------------------
//
int	CPxDB::InitDefaultLocalAE(ApplicationEntity& iAE)
{
	GetAqLogger()->LogMessage(kDebug, "DEBUG: -CPxDB::InitHostnameLocalAE() starts\n");
 
	//	If there are more than one default localAE with this AETitle, 
	//	do not try to update, just delete them all, and insert a new one
	AqString tmpSQL;
	AqString whereSQL;
	
//	whereSQL.Format(" WHERE AETitle = '%s' AND HostName = '%s'", iAE.m_AETitle, iAE.m_hostName);
	whereSQL.Format(" WHERE AETitle = '%s' ", iAE.m_AETitle); //hostname変更された場合

	tmpSQL.Format("IF (SELECT COUNT(ID) FROM LocalAE %s) > 1 DELETE LocalAE %s", whereSQL, whereSQL);
	SQLExecute(tmpSQL);

	//	If there aren't any, insert one; otherwise, update the one that's there so it's definitely correct
	AqString strSQL;
	AqString updateSQLStr;
	// Risk: one machine can only have one IP address
	updateSQLStr.Format(" UPDATE LocalAE SET IPAddress='%s', HostName='%s', Port=%d %s", iAE.m_IPAddress, iAE.m_hostName, iAE.m_port, whereSQL);

	strSQL.Format("IF NOT EXISTS (SELECT ID FROM LocalAE %s) INSERT LocalAE "
		"(AEName, AETitle, HostName, IPAddress, Port, Level, Priority, Description) "
		" VALUES('%s','%s','%s','%s',%d, %d, %d,'%s') ELSE  %s  ", whereSQL,
		iAE.m_AEName, iAE.m_AETitle, iAE.m_hostName, iAE.m_IPAddress, iAE.m_port, iAE.m_level, 
		iAE.m_priority, iAE.m_description, updateSQLStr); 

	GetAqLogger()->LogMessage(kDebug, "DEBUG: -CPxDB::InitHostnameLocalAE with SQL: %s\n", strSQL);	

	int retcd = SQLExecute(strSQL);
 
	GetAqLogger()->LogMessage(kDebug, "DEBUG: -CPxDB::InitHostnameLocalAE()  ends\n");
	return retcd;
}


//------------------------------------------------------------------------------------------------
//
int	CPxDB::ActiveLocalAE(const char* iHostname, bool iOnline)
{
	GetAqLogger()->LogMessage(kDebug, "DEBUG: -CPxDB::ActiveLocalAE() starts\n");
 
	vector <ApplicationEntity> aeList;

	//	If there aren't any, insert one; otherwise, update the one that's there so it's definitely correct
	AqString strSQL;

	strSQL.Format(" Where HostName = '%s'", iHostname) ;

	int	retcd = QueryApplicationEntity(CPxDB::kLocalAE, aeList,  strSQL);
	if(retcd != kOK) return retcd;
	
	int level;
	for(int i=0; i<aeList.size(); i++)
	{
		ApplicationEntity& ae = aeList[i];
		level = ae.m_level;
		if( (iOnline && level >= 0) || (!iOnline && level < 0) )
			continue;

		// make level as negtive number to mark it offline, or positive as online
		if(level == 0)
			level = -999;
		else if (level == -999)
			level = 0;
		else
			level = -level;


		strSQL.Format("Update LocalAE set level=%d Where ID=%d", level, ae.m_AEID);
		SQLExecute(strSQL);

	}
	
	GetAqLogger()->LogMessage(kDebug, "DEBUG: -CPxDB::ActiveLocalAE()  ends\n");
	return retcd;
}

//------------------------------------------------------------------------------------------------
//
int	CPxDB::DeleteLocalAE(const char* iAEName)
{
	GetAqLogger()->LogMessage(kDebug, "DEBUG: -CPxDB::DeleteLocalAE start\n");
 
	AqString strSQL;
	strSQL.Format("DELETE FROM TagRule WHERE value = (SELECT AETitle FROM localAE WHERE AEName = '%s') Delete From LocalAE Where AEName='%s'", iAEName,iAEName);
		 
	GetAqLogger()->LogMessage(kDebug, "DEBUG: -CPxDB::DeleteLocalAE with SQL: %s\n", strSQL);	
	int retcd = SQLExecute(strSQL);
 	GetAqLogger()->LogMessage(kDebug, "DEBUG: -CPxDB::DeleteLocalAE  end\n");
	return retcd;
}

//-----------------------------------------------------------------------------------------
// Default login group name will be used when user login and no geoup name is selected
// 
int	CPxDB::GetDefaultLoginGroupID(int iAccountID, int& oGroupID)
{
	AqString sql;
	sql.Format("SELECT GroupID FROM UserDefaultGroup WHERE accountID = %d ", iAccountID);
	return SQLGetInt(sql, oGroupID);
}

//-----------------------------------------------------------------------------------------
// 1. Move current one from userDefaultGroup to userOtherGroup
// 2. Move new one from userOtherGroup to userDefaultGroup.
int CPxDB::UpdateDefaultLoginGroup(int iUserAccountUID, int iDefaultGroupID)
{
 
	GetAqLogger()->LogMessage(kDebug, "DEBUG: -CPxDB::UpdateDefaultLoginGroup() starts\n");
	
	int retcd, oldDefaultGroupID = 0;
	SQA sqa(getLocalDBType());

	AqString getSQL;
	getSQL.Format("SELECT GroupID FROM UserDefaultGroup WHERE AccountID = %d", iUserAccountUID );
	SQLGetInt(getSQL, oldDefaultGroupID);
	if(oldDefaultGroupID <= 0 )return  oldDefaultGroupID;

	// remove current one from userDefaultGroup to uerOtherGroup
	sqa.FormatCommandText("IF EXISTS (SELECT GroupID FROM UserDefaultGroup WHERE AccountID = %d AND GroupID = %d) DELETE FROM UserDefaultGroup WHERE AccountID = %d AND GroupID = %d ", iUserAccountUID,oldDefaultGroupID, iUserAccountUID,oldDefaultGroupID);
	
	retcd = SQLExecuteBegin(sqa);
	if(retcd != kOK) return retcd;

	sqa.FormatCommandText("IF NOT EXISTS (SELECT GroupID FROM UserOtherGroup WHERE GroupID = %d AND AccountID = %d ) INSERT UserOtherGroup (AccountID, GroupID) VALUES (%d, %d)", oldDefaultGroupID,iUserAccountUID, iUserAccountUID, oldDefaultGroupID);

	retcd = SQLExecuteBegin(sqa);
	if(retcd != kOK) return retcd;

	// Deal with new one
	sqa.FormatCommandText("IF NOT EXISTS (SELECT GroupID FROM UserDefaultGroup WHERE GroupID = %d AND AccountID = %d ) INSERT UserDefaultGroup (AccountID, GroupID) VALUES (%d, %d)", iDefaultGroupID,iUserAccountUID, iUserAccountUID, iDefaultGroupID);

	retcd = SQLExecuteBegin(sqa);
	if(retcd != kOK) return retcd;

	sqa.FormatCommandText("IF EXISTS (SELECT GroupID FROM UserOtherGroup WHERE GroupID = %d AND AccountID = %d ) DELETE FROM UserOtherGroup  WHERE AccountID = %d AND GroupID = %d ", iDefaultGroupID, iUserAccountUID,iUserAccountUID,iDefaultGroupID);

	retcd = SQLExecuteBegin(sqa);
	if(retcd != kOK) return retcd;
	SQLExecuteEnd(sqa);

	return retcd;
 
}

int	CPxDB::QueryUserGroup(vector<UserGroupInfo> &oVal, const char* iWhereFilter)
{
 
	AqString	strSQL = "SELECT UserGroup.UserGroupID, UserGroup.DomainId, UserGroup.Name, UserGroup.Privilege, "\
		" UserGroup.Description, DomainT.DomainID,DomainT.Name, DomainT.OrganizationID, "\
		" DomainT.Description, DomainT.type, Organization.OrganizationID,Organization.Name, Organization.Address, "\
		" Organization.Phone, Organization.Fax, Organization.Description "\
		" FROM UserGroup JOIN  DomainT ON UserGroup.DomainId = DomainT.DomainID "\
		" JOIN Organization ON DomainT.OrganizationId = Organization.OrganizationID ";
	
	if(iWhereFilter) strSQL += iWhereFilter;
	
	SQA sqa(getLocalDBType());
	GetAqLogger()->LogMessage(kDebug, "DEBUG: -CPxDB::QueryUserGroup with: %s\n", strSQL);
	sqa.SetCommandText(strSQL);
	int retcd = SQLExecuteBegin(sqa);
	oVal.clear(); if(retcd != kOK) return retcd;
	int size = sqa.GetRecordCount(); if(size < 1) return kNoResult;
	oVal.resize(size);

	UserGroupInfo* pUserGroupInfo;
	DomainInfo* pDomainInfo;
	int index = 0;
	retcd = sqa.MoveFirst(); if(retcd != kOK)  return retcd;
	while( retcd == kOK && index < size )
	{
		pUserGroupInfo = &(oVal[index++]);
		pDomainInfo = &pUserGroupInfo->m_domainInfo ;
	 
		SQL_GET_INT(pUserGroupInfo->m_userGroup.m_groupUID,sqa);
		SQL_GET_INT(pUserGroupInfo->m_userGroup.m_domainID,sqa);
		SQL_GET_STR(pUserGroupInfo->m_userGroup.m_name, sqa);
		SQL_GET_INT(pUserGroupInfo->m_userGroup.m_privilege,sqa);
		SQL_GET_STR(pUserGroupInfo->m_userGroup.m_description, sqa);

		SQL_GET_INT(pDomainInfo->m_domain.m_domainID,sqa);
		SQL_GET_STR(pDomainInfo->m_domain.m_name, sqa);
		SQL_GET_INT(pDomainInfo->m_domain.m_organizationID, sqa);
		SQL_GET_STR(pDomainInfo->m_domain.m_description, sqa);
		SQL_GET_INT(pDomainInfo->m_domain.m_type, sqa);

		SQL_GET_INT(pDomainInfo->m_organization.m_organizationUID, sqa);
		SQL_GET_STR(pDomainInfo->m_organization.m_name, sqa);
		SQL_GET_STR(pDomainInfo->m_organization.m_address, sqa);
		SQL_GET_STR(pDomainInfo->m_organization.m_phone, sqa);
		SQL_GET_STR(pDomainInfo->m_organization.m_fax, sqa);
		SQL_GET_STR(pDomainInfo->m_organization.m_description, sqa);
 	
		retcd = sqa.MoveNext();
	}

	SQLExecuteEnd(sqa);
	 
	return kOK;
}

int	CPxDB::GetUserGroupInfo(int iUserGroupID, UserGroupInfo &oVal)
{
	AqString	iWhereFilter;
	iWhereFilter.Format(" WHERE UserGroup.UserGroupID = %d", iUserGroupID);
	vector<UserGroupInfo> oTempData;
	int retcd = QueryUserGroup(oTempData, iWhereFilter);
	if(retcd != kOK || oTempData.size() <1) return kNoResult;
	oVal = oTempData[0];
	return kOK;

}


//-----------------------------------------------------------------------------------------
// 
int CPxDB::GetTagFilterRules(std::vector<TagFilterRule>& oVal, const char* iWhereFilter)
{
	AqString	strSQL = "SELECT F.TagFilterID, D.Tag, C.ID, R.Value FROM TagRule R "\
	" JOIN DicomTag D ON R.DicomTagID = D.ID "\
	" JOIN Comparator C ON R.ComparatorID = C.ID "\
	" JOIN TagFilterRules F ON F.TagRuleID = R.ID "\
//	" ORDER BY F.TagFilterID "; //2012/05/09 K.Ko for iWhereFilter
	" ";
	if(iWhereFilter) strSQL += iWhereFilter;

	strSQL += " ORDER BY F.TagFilterID "; //2012/05/09 K.Ko for iWhereFilter
	
	SQA sqa(getLocalDBType());
	GetAqLogger()->LogMessage(kDebug, "DEBUG: -CPxDB::GetTagFilterRules with: %s\n", strSQL);
	sqa.SetCommandText(strSQL);
	int retcd = SQLExecuteBegin(sqa);
	oVal.clear(); if(retcd != kOK) return retcd;
	int size = sqa.GetRecordCount(); if(size < 1) return kNoResult;
	oVal.resize(size);

	TagFilterRule* pRule;
	int index = 0;
	retcd = sqa.MoveFirst(); if(retcd != kOK)  return retcd;
	while( retcd == kOK && index < size )
	{
		pRule = &(oVal[index++]);

		SQL_GET_INT(pRule->m_filterID,sqa);
		SQL_GET_INT(pRule->m_tag,sqa);
		SQL_GET_INT(pRule->m_comparatorID,sqa);
		SQL_GET_STR(pRule->m_value,sqa);
			
		retcd = sqa.MoveNext();
	}

	SQLExecuteEnd(sqa);
	 
	return kOK;
}

int CPxDB::GetPrefetchPatternEntry(std::vector<PrefetchPatternEntry>& oVal)
{
	
	SQA sqa(getLocalDBType());
	sqa.SetCommandText("SELECT PrefetchPattern.ID, TagFilterID, TagFilter.Name, Modality, StudyNotOlderThan, UnitType, MaxNumberResults FROM PrefetchPattern INNER JOIN TagFilter ON TagFilter.ID = TagFilterID ");
	int retcd = SQLExecuteBegin(sqa);
	oVal.clear(); if(retcd != kOK) return retcd;
	int size = sqa.GetRecordCount(); if(size < 1) return kNoResult;
	oVal.resize(size);

	PrefetchPatternEntry* pRule;
	int index = 0;
	retcd = sqa.MoveFirst(); if(retcd != kOK)  return retcd;
	while( retcd == kOK && index < size )
	{
		pRule = &(oVal[index++]);
		SQL_GET_INT(pRule->m_ID,sqa);
		SQL_GET_INT(pRule->m_tagFilterID,sqa);
		SQL_GET_STR(pRule->m_tagFilterName,sqa);
		SQL_GET_STR(pRule->m_modality,sqa);
		SQL_GET_INT(pRule->m_studyNotOlderThan,sqa);
		SQL_GET_INT(pRule->m_dayUnitType,sqa);
		SQL_GET_INT(pRule->m_maxNumberResults,sqa);
		retcd = sqa.MoveNext();
	}

	SQLExecuteEnd(sqa);
	 
	return kOK;


}
//-----------------------------------------------------------------------------------------
// 
//	TODO: 
//		(1) Get SeriesLevelID once (not in loop)
//		(2) Get Group list in first query, then assign to groups
//		(3) Move whole thing into stored procedure
//
int	CPxDB::InsertIntoGroupSeries(const char* iSeriesInstanceUID, std::map<int, int>& iTagFilterIDs, std::vector<int>& oAssignedGroups)
{
	SQA sqa(getLocalDBType());
	int retcd = kOK;
	int size;
	int groupID = 0;
	AqString querySQL, insertSQL;
	std::map<int, int>::iterator iter;
	int tagFilterID;
	oAssignedGroups.clear();
	for(iter = iTagFilterIDs.begin(); iter != iTagFilterIDs.end(); iter++)
	{
		tagFilterID = iter->first;

		//	Obtain the set difference between what *should* be in GroupSeries and what *is* in GroupSeries
		//		This will be used first to populate oAssignedGroups for AuditTrail.  Then, it will be used
		//		to insert later
		querySQL.Format("SELECT F.GroupID, E.SeriesLevelID "\
			" FROM TagFilterGroupAssignment F, SeriesLevel E "\
			" WHERE F.TagFilterID = %d AND E.SeriesInstanceUID = '%s' "\
			" AND NOT EXISTS "\
			" (SELECT G.GroupID, G.SeriesLevelID FROM GroupSeries G  "\
			" JOIN TagFilterGroupAssignment T ON G.GroupID = T.GroupID "\
			" JOIN SeriesLevel S ON G.SeriesLevelID = S.SeriesLevelID "\
			" WHERE T.TagFilterID = %d AND S.SeriesInstanceUID = '%s' "\
			" AND F.GroupID = T.GroupID AND E.SeriesLevelID = G.SeriesLevelID) ", tagFilterID, iSeriesInstanceUID, tagFilterID, iSeriesInstanceUID);

/*
 *	NOTE: the code below was wrong because it didn't include constraints between the two sets
 *			to produce set difference.
 *
		strSQL.Format("IF NOT EXISTS (SELECT G.GroupID, G.SeriesLevelID	FROM GroupSeries G "\
			" JOIN TagFilterGroupAssignment T ON G.GroupID = T.GroupID "\
			" JOIN SeriesLevel S ON G.SeriesLevelID = S.SeriesLevelID "\
			" WHERE TagFilterID = %d AND SeriesInstanceUID = '%s') "\
			" INSERT GroupSeries SELECT GroupID, SeriesLevelID "\
			" FROM TagFilterGroupAssignment, SeriesLevel "\
			" WHERE TagFilterID = %d AND SeriesInstanceUID = '%s' ", tagFilterID, iSeriesInstanceUID, tagFilterID, iSeriesInstanceUID);
*/

		GetAqLogger()->LogMessage(kDebug, "DEBUG: -AQNetDB::InsertIntoGroupSeries with: %s\n", querySQL);	
		sqa.SetCommandText(querySQL);
		retcd = SQLExecuteBegin(sqa);
		if(retcd != kOK) 
		{
			GetAqLogger()->LogMessage("ERROR: -AQNetDB::InsertIntoGroupSeries with: %s\n", querySQL);	
			continue;
		}

		size = sqa.GetRecordCount(); 
		if (size < 1) 
			continue;

		int  groupID = 0, seriesLevelID = 0;
		int  index = 0;
		retcd = sqa.MoveFirst(); 
		if(retcd != kOK)  
			continue;

		std::vector<int> tmpV;
		tmpV.clear();
		while( retcd == kOK && index < size )
		{
			SQL_GET_INT(groupID, sqa);
			SQL_GET_INT(seriesLevelID, sqa);
		
			tmpV.push_back(groupID);
			retcd = sqa.MoveNext();
			index++;
		}
		SQLExecuteEnd(sqa);	

		//	Do the insert
		insertSQL.Format("INSERT GroupSeries %s", querySQL);

		GetAqLogger()->LogMessage(kDebug, "DEBUG: -AQNetDB::InsertIntoGroupSeries with: %s\n", insertSQL);	
		retcd = SQLExecute(insertSQL);
		if(retcd != kOK) 
		{
			GetAqLogger()->LogMessage("ERROR: -AQNetDB::InsertIntoGroupSeries with: %s\n", insertSQL);	
			continue;
		}

		//	The insert succeeded, so add the groupIDs to the output vector so we can log them in the Audit Trail.
		for(int i = 0; i < tmpV.size(); i++)
		{
			oAssignedGroups.push_back(tmpV[i]);
		}
	}

	return retcd;
}



/*
	@TypeName 	VARCHAR(64),
	@ViewName	VARCHAR(64),
	@Description VARCHAR(256)
*/
int	CPxDB::MakeAqObjectType(AqObjectType& ioObject)
{
	ioObject.m_ID = gMapAqObjectType.Get(ioObject.m_TypeName);
	if(ioObject.m_ID > 0)
		return kOK;

	AqString strSQL;
	strSQL.Format("EXEC PxDcmHistDB.dbo.MakeAqObjectType '%s', '%s', '%s'", 
		ioObject.m_TypeName, ioObject.m_ViewName, ioObject.m_Description);

	int rcode = SQLMakeGetID(strSQL, ioObject.m_ID);
	if(rcode == kOK && ioObject.m_ID > 0)
	{
		gMapAqObjectType.Add(ioObject.m_TypeName, ioObject.m_ID);	
	}

	return rcode;
	
}

int	CPxDB::GetAqObjectType(int iID, AqObjectType& oObject, bool nameOnly/*=false*/)
{
	oObject.Clear();

	if(nameOnly)
	{
		const std::string* s = gMapAqObjectType.ItemKey(iID);
		if(s)
		{
			oObject.m_ID = iID;
			ASTRNCPY(oObject.m_TypeName, s->c_str());
			return kOK;
		}
	}

	SQA sqa(getLocalDBType());
	sqa.FormatCommandText("SELECT TypeName, ViewName, Description FROM PxDcmHistDB.dbo.AqObjectType"
		" Where ID=%d", iID);

	int retcd = SQLExecuteBegin(sqa);
	if(retcd != kOK) return retcd;

	retcd = sqa.MoveFirst(); if(retcd != kOK)  return retcd;
		
	oObject.m_ID = iID;
	SQL_GET_STR(oObject.m_TypeName, sqa);
	if(!nameOnly)	
	{
		SQL_GET_STR(oObject.m_ViewName, sqa);
		SQL_GET_STR(oObject.m_Description, sqa);
	}
			
	SQLExecuteEnd(sqa);
		 
	return kOK;

}

int	CPxDB::GetAllAqObjectTypeName(std::vector<std::string>& oVal)
{
	oVal.clear();
	return SQLStringValues("SELECT TypeName FROM PxDcmHistDB.dbo.AqObjectType", oVal);
}

int	CPxDB::GetAqObjectTypeID(const char* iName)
{
	if(!iName)
		return 0;

	int id;
	id = gMapAqObjectType.Get(iName);
	
	if(!id)
	{
		AqString strSQL;
		strSQL.Format("Select id from PxDcmHistDB.dbo.AqObjectType Where TypeName = '%s'", iName);
		CPxDB db;
		int rcode = db.SQLGetInt(strSQL, id);
		if(rcode == kOK && id > 0)
		{
			gMapAqObjectType.Add(iName, id);	
		}
	}

	return id;
}

static const char* AqObjectTypeMap[100] = {
"_Null_",			
"ImageServer",	
"DICOMServer",	
"DatabaseServer",	
"StorageServer",	
"WebServer",		
"ProcessServer",	
"Client",			
"LocalAE",		
"RemoteAE",		
"Printer",		
"Domain",		
"Organization",	
"UserGroup",		
"UserAccount",
"end"
};

int	CPxDB::GetAqObjectTypeID(eAqObjectType iKey)
{
	const char* keyName = 0;
	if( iKey == kTypeNull)
		return 0;
	return GetAqObjectTypeID(AqObjectTypeMap[iKey]);
}

/*
  Type 		INT not null REFERENCES AqObjectType(ID) ON DELETE NO ACTION,
  EntityName VARCHAR(64) not null, -- active directory user name size is 20
  FullName 	VARCHAR(128) not null,
  Hostname	VARCHAR(64)	default '',
  Address	VARCHAR(64)	default '',
  Port 		INT default 0,
  DomainName 		VARCHAR(128) default '',	-- security ID from single sign on
  Description 	VARCHAR(256) default '',
*/

int	CPxDB::MakeAqObject(AqObjectInterface& ioObject)
{
	AqString strSQL;

	int objectType = ioObject.GetType();
	if(objectType == CPxDB::GetAqObjectTypeID(kTypeUserAccount))
	{
		strSQL.Format("EXEC PxDcmHistDB.dbo.MakeUserObject '%s', '%s', '%s', '%s', '%s', '%s'", 
			ioObject.GetEntityName(), ioObject.GetFullName(), ioObject.GetHostname(), 
			ioObject.GetAddress(), ioObject.GetDomainName(), ioObject.GetDescription());
	}
	else
	{
		strSQL.Format("EXEC PxDcmHistDB.dbo.MakeAqObject %d, '%s', '%s', '%s', '%s', %d, '%s', '%s'", 
			ioObject.GetType(), ioObject.GetEntityName(), ioObject.GetFullName(), ioObject.GetHostname(), 
			ioObject.GetAddress(), ioObject.GetPort(), ioObject.GetDomainName(), ioObject.GetDescription());		
	}
	return SQLMakeGetID(strSQL, ioObject.GetID());

}

int	CPxDB::GetAqObject(int iID, AqObjectInterface& oObject)
{
	oObject.Clear();

	
	SQA sqa(getLocalDBType());
	sqa.FormatCommandText("SELECT Type, EntityName, FullName, Hostname, Address, Port, DomainName, Description "
		" FROM PxDcmHistDB.dbo.AqObject Where ID=%d", iID);

	int retcd = SQLExecuteBegin(sqa);
	if(retcd != kOK) return retcd;

	retcd = sqa.MoveFirst(); if(retcd != kOK)  return retcd;
		
	oObject.GetID() = iID;
	oObject.SetEntityName(sqa.getDataS());
	oObject.SetFullName(sqa.getDataS());
	oObject.SetHostname(sqa.getDataS());
	oObject.SetAddress(sqa.getDataS());
	oObject.SetPort(sqa.getDataInt());
	oObject.SetDomainName(sqa.getDataS());
	oObject.SetDescription(sqa.getDataS());
	
			
	SQLExecuteEnd(sqa);
		 
	return kOK;

}

#if 1
int UserGroup::GetType() const 
{
	return CPxDB::GetAqObjectTypeID(kTypeUserGroup);
};

int ApplicationEntity::GetType() const 
{
	return CPxDB::GetAqObjectTypeID((m_IsLocalAE)?kTypeLocalAE:kTypeRemoteAE);
};

void ApplicationEntity::SetType(int iValue)
{
	m_IsLocalAE = (iValue == CPxDB::GetAqObjectTypeID(kTypeLocalAE));
}

#endif

/*
MakeAEObjectFromSystem( 
	@ID 		INT,
	@AEName		VARCHAR(128),
	@IsLocal	INT = 0
*/

// can not cache AE object careted from main database, because the AE ca can change in main DB
int	CPxDB::MakeAEObject(const char* iAEName, int& objectID, int IsLocalAE)
{
	AqString strSQL;

	if(!iAEName)
		return kParameterError;

	strSQL.Format("EXEC dbo.MakeAEObjectFromSystem 0, '%s', %d", iAEName, IsLocalAE);

	return SQLMakeGetID(strSQL, objectID);

}

int	CPxDB::MakeAEObject(int AE_ID, int& objectID, int IsLocalAE)
{
	AqString strSQL;

	if(AE_ID < 1)
		return kParameterError;

	strSQL.Format("EXEC dbo.MakeAEObjectFromSystem %d, '', %d", AE_ID, IsLocalAE);

	return SQLMakeGetID(strSQL, objectID);

}

int UserAccount::GetType() const 
{
	return CPxDB::GetAqObjectTypeID(kTypeUserAccount);
};

/*
CREATE PROCEDURE MakeUserObject( 
	@Username 	VARCHAR(20),
	@LastName 	VARCHAR(32),
	@FirstName 	VARCHAR(32),
	@Email 		VARCHAR(64),
	@DomainName		VARCHAR(128) = '',
	@Description 	VARCHAR(128) = ''
	)
*/

/*
CREATE PROCEDURE MakeUserObjectFromUserAccount( 
	@UserID INT, 
	@Username VARCHAR(20) = '',
	@DomainID	INT = 0

*/

int	CPxDB::MakeUserObjectFromUserAccount(int iUserID, int& objectID)
{
	AqString strSQL;

	strSQL.Format("EXEC dbo.MakeUserObjectFromUserAccount %d", iUserID);

	return SQLMakeGetID(strSQL, objectID);

}

/*
CREATE PROCEDURE MakeSeriesAttribute(
	@Name      VARCHAR(64),
	@Type 		int,
	@SubType 	VARCHAR(64),
	@ProcessName VARCHAR(64),
	@ProcessType VARCHAR(64),
	@Description 	VARCHAR(256) = ''
)
*/

int CPxDB::MakeSeriesAttribute(const AuxDataInfo& iAuxInfo, int& oSeriesAttributeID)
{
	AqString strSQL;

	strSQL.Format("EXEC PxDcmHistDB.dbo.MakeSeriesAttribute '%s', %d, '%s', '%s', '%s'", 
			iAuxInfo.m_name, iAuxInfo.m_type, iAuxInfo.m_subtype, 
			iAuxInfo.m_processName, iAuxInfo.m_processType);

	return SQLMakeGetID(strSQL, oSeriesAttributeID);

}


/*
CREATE PROCEDURE MakePatient  (
	@StudyInstanceUID VARCHAR(64), 
	@PatientsName VARCHAR(332), 
	@PatientID VARCHAR(64), 
	@PatientsBirthDate VARCHAR(10), 
	@PatientsSex	VARCHAR(16), 
	@StudyDate VARCHAR(10), 
	@StudyTime VARCHAR(16), 
	@AccessionNumber VARCHAR(16), 
	@StudyID VARCHAR(16), 
	@ReferringPhysiciansName VARCHAR(332), 
	@SeriesInstanceUID VARCHAR(64), 
	@SeriesNumber INT, 
	@Modality VARCHAR(16),
	@SeriesInstances INT,
	@TransferSyntax INT,
	@QRFlag tinyint,
	@SeriesDescription VARCHAR(64), 
	@Attribute INT = 0,
	@SourceAE INT = 0 -- '_NULL_' object
	)
*/

int	CPxDB::MakePatientObject(DICOMData& ioPatient, int iTransferSyntax, int iSourceAE_ID, int iSeriesAttributeID)
{
	AqString strSQL;

	strSQL.Format("EXEC PxDcmHistDB.dbo.MakePatient '%s', '%s', '%s', '%s', '%s', '%s', '%s', '%s', '%s',"
		"'%s', '%s', %d, '%s', %d, %d, %d, '%s', %d, %d", 
		ioPatient.m_studyInstanceUID,
		ToSQLString(ioPatient.m_patientsName),
		ToSQLString(ioPatient.m_patientID),
		ioPatient.m_patientsBirthDate,
		ioPatient.m_patientsSex,
		ioPatient.m_studyDate,
		ioPatient.m_studyTime,
		ioPatient.m_accessionNumber,
		ioPatient.m_studyID,
		ToSQLString(ioPatient.m_referringPhysiciansName),
		ioPatient.m_seriesInstanceUID,
		ioPatient.m_seriesNumber,
		ioPatient.m_modality,
		ioPatient.m_numberOfSeriesRelatedInstances,
		iTransferSyntax,
		ioPatient.m_IsQRData,
		ioPatient.m_seriesDescription,
		iSeriesAttributeID,
		iSourceAE_ID
		);

	return SQLMakeGet2ID(strSQL, ioPatient.m_studyIndex, ioPatient.m_seriesIndex );

}



/*
CREATE PROCEDURE MakePatientFromSeries  (
	@SeriesLevelUID VARCHAR(64), 
	@SeriesLevelID INT,
	@SourceAE INT = 0 -- '_NULL_' object
	)
*/


int	CPxDB::MakePatientObject(const char* iSeriesLevelUID, int& oStudyIndex, int& oSeriesIndex, int iSourceAE_ID)
{
	AqString strSQL;

	if(!iSeriesLevelUID)
		return kParameterError;

	strSQL.Format("EXEC dbo.MakePatientFromSeries '%s', 0, %d", iSeriesLevelUID, iSourceAE_ID);
	
	return SQLMakeGet2ID(strSQL, oStudyIndex, oSeriesIndex);

}

int	CPxDB::MakePatientObject(int iSeriesLevelID, int& oStudyIndex, int& oSeriesIndex, int iSourceAE_ID)
{
	AqString strSQL;

	if(iSeriesLevelID < 1)
		return kParameterError;

	strSQL.Format("EXEC dbo.MakePatientFromSeries '', %d, %d", iSeriesLevelID, iSourceAE_ID);
	
	return SQLMakeGet2ID(strSQL, oStudyIndex, oSeriesIndex);

}



int	CPxDB::MakeActionWord(Actions& ioAction)
{
	ioAction.m_ID = gMapActions.Get(ioAction.m_ActionName);
	if(ioAction.m_ID > 0)
		return kOK;
	
	AqString strSQL;

	strSQL.Format("EXEC PxDcmHistDB.dbo.MakeActions '%s', '%s'", ioAction.m_ActionName, ioAction.m_Description);

	int rcode = SQLMakeGetID(strSQL, ioAction.m_ID);
	if(rcode == kOK && ioAction.m_ID > 0)
	{
		gMapActions.Add(ioAction.m_ActionName, ioAction.m_ID);	
	}

	return rcode;


}

int	CPxDB::GetActionID(const char* iName)
{
	if(!iName)
		return 0;

	int id;
	id = gMapActions.Get(iName);
	
	if(!id)
	{
		AqString strSQL;
		strSQL.Format("Select id from PxDcmHistDB.dbo.Actions Where ActionName = '%s'", iName);
		int rcode = SQLGetInt(strSQL, id);
		if(rcode == kOK && id > 0)
		{
			gMapActions.Add(iName, id);	
		}
	}

	return id;
}

static const char* AqActionsMap[100] = {
"",			
"Login",
"Logout",
"Create",
"Delete",
"Reject",

"Receive",
"Send",
"Read",
"Write",

"Join",
"Leave",

"Start",
"Pause",
"Stop",
"Cancel",
"Resume",

"Change",
"Anonymize",
"Export",
"Import",
"Assign",
"Unassign",
"Film",
"Print",

"Retrieve",

"__end__"
};

int	CPxDB::GetActionID(eLogAction iKey)
{
	return GetActionID(AqActionsMap[iKey]);
}


/*
  ID	 		INT IDENTITY(1,1) PRIMARY KEY,
  Actor			INT not null REFERENCES AqObject(ID) ON DELETE NO ACTION,
  Activity		INT not null REFERENCES Actions(ID) ON DELETE NO ACTION,
  ActOn			INT not null REFERENCES PatientStudy(StudyIndex) ON DELETE NO ACTION,
  Requestor		INT not null REFERENCES AqObject(ID) ON DELETE NO ACTION,
  ActionFrom	INT not null REFERENCES AqObject(ID) ON DELETE NO ACTION,
  ActionAt		INT not null REFERENCES AqObject(ID) ON DELETE NO ACTION,
  TimeOfAction	datetime DEFAULT GETDATE(),
  Description 	VARCHAR(256),
  Status		INT -- may use to indicate action success or fail

*/

int	CPxDB::LogEvent(EventLog& iEvent)
{
	AqString strSQL;
	const char* eventTable;
		
	if (iEvent.m_eventType == kSeriesEventLog)
		eventTable = "SeriesEventLog";
	else if (iEvent.m_eventType == kStudyEventLog)
		eventTable = "StudyEventLog";
	else // kSystemEventLog
		eventTable = "sysEventLog";

	if(iEvent.m_TimeOfAction < 1) // use database default time
	{
		strSQL.Format("INSERT PxDcmHistDB.dbo.%s (Actor, Activity, ActOn, Requestor, ActionFrom, ActionAt,"
			"TransferTo, Description, Status) VALUES ( %d, %d, %d, %d, %d, %d, %d,'%s', %d)", eventTable,
			 iEvent.m_Actor, iEvent.m_Activity, iEvent.m_ActOn, iEvent.m_Requestor, iEvent.m_ActionFrom, 
			iEvent.m_ActionAt, iEvent.m_TransferTo, iEvent.m_Description, iEvent.m_Status);
	}
	else
	{

		strSQL.Format("INSERT PxDcmHistDB.dbo.%s (Actor, Activity, ActOn, Requestor, ActionFrom, ActionAt, "
			"TimeOfAction, TransferTo, Description, Status) VALUES ( %d, %d, %d, %d, %d, %d, %f,%d,'%s', %d)",
			eventTable, iEvent.m_Actor, iEvent.m_Activity, iEvent.m_ActOn, iEvent.m_Requestor, iEvent.m_ActionFrom, 
			iEvent.m_ActionAt, iEvent.m_TimeOfAction, iEvent.m_TransferTo, iEvent.m_Description, iEvent.m_Status);
	}

	return SQLExecute(strSQL);
	
}


int CPxDB::MakeEvent(EventLog& oEvent, 
					AqObjectInterface& ioActor, Actions& ioAct,
					AqObjectInterface& ioActOn,
					AqObjectInterface& ioRequestor,
					AqObjectInterface& ioActionFrom,
					AqObjectInterface& ioActionAt,
					AqObjectInterface& ioTransferTo)
{
	int rcode = UpdateAqObjectID(ioActor);
	if(rcode != kOK)
		return rcode;
	
	if(ioAct.m_ID < 1)
	{
		rcode = MakeActionWord(ioAct);
		if(rcode != kOK)
			return rcode;
	}

	rcode = UpdateAqObjectID(ioActOn);
	if(rcode != kOK)
		return rcode;

	rcode = UpdateAqObjectID(ioRequestor);
	if(rcode != kOK)
		return rcode;

	rcode = UpdateAqObjectID(ioActionFrom);
	if(rcode != kOK)
		return rcode;
	
	rcode = UpdateAqObjectID(ioActionAt);
	if(rcode != kOK)
		return rcode;
	
	rcode = UpdateAqObjectID(ioTransferTo);
	if(rcode != kOK)
		return rcode;

	oEvent.m_Actor = ioActor.GetID();
	oEvent.m_Activity = ioAct.m_ID;
	oEvent.m_ActOn = ioActOn.GetID();
	oEvent.m_Requestor = ioRequestor.GetID();
	oEvent.m_ActionFrom = ioActionFrom.GetID();
	oEvent.m_ActionAt = ioActionAt.GetID();
	oEvent.m_TransferTo = ioTransferTo.GetID();
	oEvent.m_eventType = kSystemEventLog;

	return kOK;

}

int	CPxDB::LogEvent(AqObjectInterface& ioActor, Actions& ioAct,
				 AqObjectInterface&	ioActOn,
				 AqObjectInterface& ioRequestor,
				 AqObjectInterface& ioActionFrom,
				 AqObjectInterface& ioActionAt,
				 AqObjectInterface& ioTransferTo,
				 const char* iDescription,
				 int iStatus,
				 double	iTimeOfAction/*=0.0*/)
{
	EventLog event;
	
	int rcode = MakeEvent(event, ioActor, ioAct, ioActOn, ioRequestor, ioActionFrom, ioActionAt, ioTransferTo);
	if(rcode != kOK)
		return rcode;

	if(iDescription)
		ASTRNCPY(event.m_Description, iDescription);
	event.m_Status = iStatus;
	event.m_TimeOfAction = iTimeOfAction;

	return LogEvent(event);
}


int	CPxDB::LogEvent(AqObjectInterface& ioActor, Actions& ioAct,
				 AqObjectInterface& ioRequestor,
				 AqObjectInterface& ioActionFrom,
				 AqObjectInterface& ioActionAt,
				 const char* iDescription,
				 int iStatus,
				 int iDICOMIndex, bool isStudy/*=false*/,
				 int iTransferToIndex /*= 0*/,
				 double	iTimeOfAction/*=0.0*/)
{
	EventLog event;
	AqObject fakeActOn, fakeTransferTo;

	int rcode = MakeEvent(event, ioActor, ioAct, fakeActOn, ioRequestor, ioActionFrom, ioActionAt, fakeTransferTo);
	if(rcode != kOK)
		return rcode;
	
	event.m_ActOn = iDICOMIndex;
	event.m_TransferTo = iTransferToIndex;
	event.m_eventType = (isStudy)?kStudyEventLog:kSeriesEventLog;

	if(iDescription)
		ASTRNCPY(event.m_Description, iDescription);
	event.m_Status = iStatus;
	event.m_TimeOfAction = iTimeOfAction;

	return LogEvent(event);
}


int CPxDB::GetAqNETOption(const char* iTag, AqString& oVal)
{
	SQA sqa(getLocalDBType());
	oVal.Empty();
	
	sqa.FormatCommandText("SELECT valueStr FROM AqNETOption WHERE keyStr='%s'", iTag);
	int retcd = SQLExecuteBegin(sqa);
	if(retcd != kOK)  return retcd;

	retcd = sqa.MoveFirst(); if(retcd != kOK)  return retcd;

	oVal = sqa.getDataS();
	SQLExecuteEnd(sqa);

	return kOK;

}

int CPxDB::GetAqNETOption(const char* iTag, long& oVal)
{
	AqString strVal;
	oVal = 0;
	int ret;
	ret = GetAqNETOption(iTag, strVal);
	if(ret != kOK)
		return ret;
	oVal = atol(strVal.GetString());
	return kOK;

}

//-----------------------------------------------------------------------
// It is assumed that APS_hostname will be the  value of keyStr
// 0 (off) or 1 (on) will be the value of the valueStr
// hostname can be the value of dispaly
// Return true if keyStr starts with APS_ and 1 as the value of valueStr
bool CPxDB::IsAPSEnableded(){
	SQA sqa(getLocalDBType());
	AqString strSQL = "SELECT top 1 KeyStr FROM AqNETOption WHERE keyStr like 'APS_%' and valueStr = '1'";
	std::string value = "";
	int retcd = SQLGetString(strSQL, value);
	if(retcd != kOK)  return false;
	 
	return (!value.empty());

}
//-----------------------------------------------------------------------
//
bool CPxDB::IsAuditTrailEnabled()
{
	long value = 0;
	GetAqNETOption("EnableAuditTrail", value);
	return (value != 0);
}

//-----------------------------------------------------------------------
//
bool CPxDB::IsSSOEnable()
{
	long value = 0;
	GetAqNETOption("EnableSSO", value);
	return (value != 0);
}

int CPxDB::GetDBSizeInfo(DBSizeInfo& ioData)
{


	// get databse size information
	SQA sqa(getLocalDBType());
	sqa.FormatCommandText("EXEC SP_DBSizeInfo '%s'", ioData.m_DBName);
	int retcd = SQLExecuteBegin(sqa);
	if(retcd != kOK) return retcd;
	int size = sqa.GetRecordCount(); if(size < 1) return kNoResult;
	
	ioData.m_database_size_KB = sqa.getDataD();
	ioData.m_unallocated_space_KB = sqa.getDataD();
	ioData.m_reserved_KB = sqa.getDataD();
	ioData.m_data_size_KB = sqa.getDataD();
	ioData.m_index_size_KB = sqa.getDataD();
	ioData.m_unused_KB = sqa.getDataD();
	ioData.m_Instance_Rows = sqa.getDataInt();
	ioData.m_Instance_data_KB = sqa.getDataD();
	ioData.m_Instance_index_size_KB = sqa.getDataD();

	SQLExecuteEnd(sqa);

	// get top warning instance size
	ioData.m_instanceFragmentWarningSize = 350;
	long value = 0;
	GetAqNETOption("OneInstanceTopDBSize", value);
	if(value > 0)
		ioData.m_instanceFragmentWarningSize = value;
	
	long rows = 1;
	if(ioData.m_Instance_Rows > 1)
		rows = ioData.m_Instance_Rows;

	ioData.m_instanceActualSize = (long)(1000*(ioData.m_Instance_data_KB+ioData.m_Instance_index_size_KB)/rows);
	

	return kOK;
}

//-----------------------------------------------------------------------
// -- 2005.08.04
// Needs this for poor-man's PIR
// Do this only if study already exists
int CPxDB::UpdateStudyInfo(DICOMData &iData)
{
	AqString strSQL;

	// now update the columns
	strSQL.Format("UPDATE StudyLevel SET %s='%s',%s='%s',%s='%s',%s='%s',%s='%s',%s='%s',%s='%s',"
		"%s='%s',%s='%s',%s='%s',%s='%s', %s=%d WHERE StudyInstanceUID='%s'",
		"PatientsName",			ToSQLString(iData.m_patientsName),
		"PatientID",		    ToSQLString(iData.m_patientID),
		"PatientsBirthDate",	iData.m_patientsBirthDate,
		"PatientsSex",			iData.m_patientsSex,
		"StudyDate",			iData.m_studyDate,
		"StudyTime",			iData.m_studyTime,
		"AccessionNumber",		iData.m_accessionNumber,
		"StudyID",				iData.m_studyID,
		"ReadingPhysiciansName",ToSQLString(iData.m_radiologistName),
		"ReferringPhysiciansName",ToSQLString(iData.m_referringPhysiciansName),
		"StudyDescription",		ToSQLString(iData.m_studyDescription),
		"PatientsAge",			iData.m_patientsAge,
		iData.m_studyInstanceUID);

#ifdef _DEBUG
	fprintf(stderr,"OverwriteSQL=%s\n",strSQL);
#endif

	return SQLExecute(strSQL);
}

//----------------------------------------------------------------------
// Faked series object 
int CPxDB::MakeSeriesReadUnreadObject(bool iIsRead, int& oID)
{
	int status;
	DICOMData d;
	char* text = iIsRead? "Read":"Unread";
 
	ASTRNCPY(d.m_patientsName, text);
	ASTRNCPY(d.m_patientID, text);
	ASTRNCPY(d.m_seriesInstanceUID, text);
	ASTRNCPY(d.m_seriesDescription, text);

	status = MakePatientObject(d, 0, 0, 0);
	if (status != kOK)
	{
		return status;
	}

	oID = d.m_seriesIndex;
	return kOK;
}

int CPxDB::GetAutoFilmingPattern(int iFilterID, std::vector<FilmingPatternEntry>&oVal)
{
	SQA sqa(getLocalDBType());
	sqa.FormatCommandText("SELECT printer.name, filmingPattern.skipN, filmingPattern.displayMode "\
		" FROM Printer INNER JOIN FilmingPattern ON Printer.ID = filmingPattern.PrinterID "\
		" WHERE filmingPattern.TagfilterID = %d ", iFilterID);

	
	
	int retcd = SQLExecuteBegin(sqa);
	oVal.clear(); if(retcd != kOK) return retcd;
	int size = sqa.GetRecordCount(); if(size < 1) return kNoResult;
	oVal.resize(size);

	FilmingPatternEntry* p ;
	int index = 0;
	retcd = sqa.MoveFirst(); if(retcd != kOK)  return retcd;
	while( retcd == kOK && index < size )
	{
		p = &(oVal[index++]);

		SQL_GET_STR(p->m_printerName,sqa);
		SQL_GET_INT(p->m_skipN,sqa);
		SQL_GET_STR(p->m_displayMode,sqa);
			
		retcd = sqa.MoveNext();
	}

	SQLExecuteEnd(sqa);
	 
	return kOK;

}

//--------------------------------------------------------------------------------------------------------------------------------------
//
int 
CPxDB::GetJobByFilters(const std::vector<int>& iFilters,  std::vector<DataProcessJob>& oJobs, bool iZeroID)
{

	AqString sqlStr = "SELECT DISTINCT Job FROM DataProcessPattern WHERE TagfilterID in (";

	int i;
	int n = iFilters.size();
	if (n < 1)
		return kParameterError;

	char ibuf[12];

	sqlStr += itoa(iFilters[0], ibuf, 10); 

	for(i=1; i<n; i++)
	{
		sqlStr = sqlStr + ","+itoa(iFilters[i], ibuf, 10); 
	}
	sqlStr += ")";

	SQA sqa(getLocalDBType());
	sqa.FormatCommandText("Select JobID, JobName, JobDescription, ProcessorID,	ProcessName, "
		"Handler, ProcessorDescription  From DataProcessJobView Where JobID in (%s) "
		" order by JobID, ProcessOrder", sqlStr);

	int retcd = SQLExecuteBegin(sqa);
	oJobs.clear(); if(retcd != kOK) return retcd;
	int size = sqa.GetRecordCount(); if(size < 1) return kNoResult;

	DataProcessJob* pJob = 0;
	DataProcessor*	pProcessor = 0;
	int processorIndex, jobID, jobIndex = -1, index = 0, curJobID=-1;
	retcd = sqa.MoveFirst(); if(retcd != kOK)  return retcd;
	while( retcd == kOK && index < size )
	{	
		SQL_GET_INT(jobID, sqa);
		if(jobID != curJobID)
		{
			curJobID = jobID;
			oJobs.push_back(DataProcessJob());
			
			jobIndex++;
			pJob = &(oJobs[jobIndex]);
			
			pJob->m_processors.push_back(DataProcessor());
			processorIndex = 0;
			pProcessor = &(pJob->m_processors[processorIndex]);

			if(!iZeroID)
				pJob->m_id = curJobID;
			SQL_GET_STR(pJob->m_jobName,sqa);
			SQL_GET_STR(pJob->m_description,sqa);

			SQL_GET_INT(pProcessor->m_id,sqa);
			SQL_GET_STR(pProcessor->m_processName,sqa);
			SQL_GET_STR(pProcessor->m_handler,sqa);
			SQL_GET_STR(pProcessor->m_description,sqa);
			
		}
		else
		{
			pJob->m_processors.push_back(DataProcessor());
			++processorIndex;
			pProcessor = &(pJob->m_processors[processorIndex]);

		//	sqa.SkipData(); //JobID
			sqa.SkipData();	//JobName
			sqa.SkipData();	//JobDescription

			if(iZeroID)
				sqa.SkipData();
			else
				SQL_GET_INT(pProcessor->m_id,sqa);

			SQL_GET_STR(pProcessor->m_processName,sqa);
			SQL_GET_STR(pProcessor->m_handler,sqa);
			SQL_GET_STR(pProcessor->m_description,sqa);
		}
		
		
		retcd = sqa.MoveNext();
	}

	SQLExecuteEnd(sqa);	 
	return kOK;

}


//--------------------------------------------------------------------------------------------------------------------------------------
//
static void MakeAuxFilter(const AuxDataInfo& iFilter, AqString& cond, bool iDoRef=false)
{

	cond = "";
	AqString tmpStr = "";
	if( iFilter.m_type ) 
	{
			tmpStr.Format(" Type=%d", iFilter.m_type);
			if(cond == "")
				cond = tmpStr;
			else
				cond += " AND" + tmpStr;

	}

	if( iFilter.m_name[0]) 
	{
			tmpStr.Format(" Name='%s'", iFilter.m_name);
			if(cond == "")
				cond = tmpStr;
			else
				cond += " AND" + tmpStr;

	}

	if( iFilter.m_subtype[0]) 
	{
			tmpStr.Format(" Subtype='%s'", iFilter.m_subtype);
			if(cond == "")
				cond = tmpStr;
			else
				cond += " AND" + tmpStr;

	}

	if( iFilter.m_processName[0]) 
	{
			tmpStr.Format(" ProcessName='%s'", iFilter.m_processName);
			if(cond == "")
				cond = tmpStr;
			else
				cond += " AND" + tmpStr;

	}
	
	if( iFilter.m_processType[0]) 
	{
			tmpStr.Format(" ProcessType='%s'", iFilter.m_processType);
			if(cond == "")
				cond = tmpStr;
			else
				cond += " AND" + tmpStr;

	}

	if( iFilter.m_volumesHash[0]) 
	{
			tmpStr.Format(" VolumesHash='%s'", iFilter.m_volumesHash);
			if(cond == "")
				cond = tmpStr;
			else
				cond += " AND" + tmpStr;

	}

	if( iFilter.m_parameterHash[0]) 
	{
			tmpStr.Format(" ParameterHash='%s'", iFilter.m_parameterHash);
			if(cond == "")
				cond = tmpStr;
			else
				cond += " AND" + tmpStr;

	}


	// this filter seems unless, because it is same as m_volumesHash
	// after we want to put the first referenced voluem ID in m_volumesHash
	// may be we can use it to point the second referenced volume
	if( iDoRef && iFilter.m_volumeID[0]) 
	{
			tmpStr.Format(" VolumeID='%s'", iFilter.m_volumeID);
			if(cond == "")
				cond = tmpStr;
			else
				cond += " AND" + tmpStr;

	}

	//char m_parameterHash[kVR_UI];
	//char m_volumeID[kVR_UI];					// volume ID from PrivateDataReference

}



/*
select privateData.* from  privateData join

(select privateDataID FROM PrivateDataReference p
group by privateDataID having 

(exists (select* from PrivateDataReference where p.privateDataID=privateDataID and 
auxRefSeriesUID = '1.2.124.113532.1.1.192.168.115.100.20030423083942.352.692')
and
exists (select* from PrivateDataReference where p.privateDataID=privateDataID and 
auxRefSeriesUID = '1.2.124.113532.1.1.192.168.115.100.20030423084009.591.736'))) t1

on privateDataID = ID

order by date desc

test code
	{
		vector<const char*> iRefSeriesUIDs;
		vector<AuxDataInfo> oVal;
		AuxDataInfo iFilter;

		iRefSeriesUIDs.push_back("1.2.124.113532.1.1.192.168.115.100.20030423083942.352.692");
		iRefSeriesUIDs.push_back("1.2.124.113532.1.1.192.168.115.100.20030423084009.591.736");

		strcpy(iFilter.m_name,"ParaEnhancing");

		retcd = db.GetAuxDataInfos(iRefSeriesUIDs, oVal, iFilter, 1);

		strcpy(iFilter.m_name,"");
		retcd = db.GetAuxDataInfos(iRefSeriesUIDs, oVal, iFilter);
		

	}
*/

int	CPxDB::GetAuxDataInfos(const vector<const char*> iRefSeriesUIDs, vector<AuxDataInfo>&oVal, const AuxDataInfo& iFilter, int iTopN)
{
	int refSize = iRefSeriesUIDs.size();
	if( refSize < 1)
		return kParameterError;
	
	AqString strSQL, idQuery ;
	AqString whereStr;


	MakeAuxFilter(iFilter, whereStr);
	if(whereStr != "")
		whereStr = " Where " + whereStr;


	int i;
	
	idQuery.Format(" JOIN (SELECT privateDataID FROM PrivateDataReference p GROUP BY privateDataID HAVING "
					"(EXISTS (SELECT * FROM PrivateDataReference WHERE p.privateDataID=privateDataID "
					"AND auxRefSeriesUID='%s') ", iRefSeriesUIDs[0]);

	AqString tmpStr;
	for(i=1; i<refSize; i++)
	{
		tmpStr.Format(" AND EXISTS (SELECT * FROM PrivateDataReference WHERE p.privateDataID=privateDataID "
			"AND auxRefSeriesUID='%s')", iRefSeriesUIDs[i] );
		idQuery += tmpStr;
	}
	idQuery += ")) t1 ON privateDataID=ID ";


	tmpStr = "";
	if(iTopN > 0)
		tmpStr.Format(" Top %i", iTopN);

	strSQL.Format("SELECT %s ID, AuxStudyUID, AuxSeriesUID, AuxSOPUID, Type, Name, Date, Subtype, ProcessName, "
			"ProcessType, VolumesHash FROM  PrivateData %s %s ORDER BY DATE DESC", tmpStr, idQuery, whereStr);

		
	SQA sqa(getLocalDBType());
	sqa.SetCommandText(strSQL);
	int retcd = SQLExecuteBegin(sqa);
	oVal.clear(); if(retcd != kOK) return retcd;
	int size = sqa.GetRecordCount(); if(size < 1) return kNoResult;
	oVal.resize(size);

	AuxDataInfo* pAuxDataInfo;
	int index = 0;
	double tf;
	SYSTEMTIME st;

	retcd = sqa.MoveFirst(); if(retcd != kOK)  return retcd;
	while( retcd == kOK && index < size )
	{
		pAuxDataInfo = &(oVal[index++]);

		SQL_GET_INT(pAuxDataInfo->m_key, sqa);
		SQL_GET_STR(pAuxDataInfo->m_auxStudyInstanceUID, sqa);
		SQL_GET_STR(pAuxDataInfo->m_auxSeriesInstanceUID, sqa);
		SQL_GET_STR(pAuxDataInfo->m_auxSOPInstanceUID, sqa);
		SQL_GET_INT(pAuxDataInfo->m_type,sqa);
		SQL_GET_STR(pAuxDataInfo->m_name,sqa);
		tf	= sqa.getDataDate();
		VariantTimeToSystemTime(tf,&st);
		sprintf(pAuxDataInfo->m_auxSeriesDate,"%d%02d%02d",st.wYear,st.wMonth,st.wDay);
		sprintf(pAuxDataInfo->m_auxSeriesTime,"%02d%02d%02d",st.wHour,st.wMinute,st.wSecond);
		SQL_GET_STR(pAuxDataInfo->m_subtype, sqa);
		SQL_GET_STR(pAuxDataInfo->m_processName,sqa);
		SQL_GET_STR(pAuxDataInfo->m_processType,sqa);
		SQL_GET_STR(pAuxDataInfo->m_volumesHash,sqa);

		retcd = sqa.MoveNext();
	}
	SQLExecuteEnd(sqa);	 

	return kOK;


}



int	CPxDB::GetPatientAuxData(const DICOMData& iPatientFilter, const AuxDataInfo& iAuxDataFilter, vector<PatientAuxDataInfo>& oVal )
{
	AqString strSQL;
	AqString tmpStr, filterStr;

	MakeAuxFilter(iAuxDataFilter, filterStr, true);
	CPxDB::MakeUserStudiesFilter(iPatientFilter, tmpStr);


	if(filterStr.IsEmpty())
	{
		 if(!tmpStr.IsEmpty())
			filterStr = "Where " + tmpStr;
	}
	else
	{
		if(!tmpStr.IsEmpty())
			filterStr = "Where " + filterStr + " AND " + tmpStr;
		
	}

	
	// get aux data sort as newest data in front bigger ID means newer get by
	// 'order by PrivatedataID DESC '
	// and in each record the series is in reference order control as 'order byReferenceSortKey ASC'
	strSQL.Format("SELECT PatientID, PatientsName, StudyInstanceUID, AccessionNumber, StudyDescription, "
		"SeriesInstanceUID, SeriesDescription, Modality, ModifyTime, SeriesDate, SeriesTime, "
		"PrivateDataID, AuxStudyUID, AuxSeriesUID, AuxSOPUID, Type, Name, Date, Subtype, ProcessName, ProcessType, "
		"VolumesHash, ParameterHash, VolumeID From PatientAuxDataView %s "
		" ORDER BY PrivatedataID DESC, ReferenceSortKey ASC", filterStr);

	SQA sqa(getLocalDBType());
	sqa.SetCommandText(strSQL);
	int retcd = SQLExecuteBegin(sqa);
	oVal.clear(); if(retcd != kOK) return retcd;
	int size = sqa.GetRecordCount(); if(size < 1) return kNoResult;
	oVal.resize(size);

	PatientAuxDataInfo* pInfo;
	int index = 0;

	double tf;
	SYSTEMTIME st;
	retcd = sqa.MoveFirst(); if(retcd != kOK)  return retcd;
	while( retcd == kOK && index < size )
	{
		pInfo = &(oVal[index++]);


		SQL_GET_STR(pInfo->m_study.m_patientID, sqa);
		SQL_GET_STR(pInfo->m_study.m_patientsName, sqa);
		SQL_GET_STR(pInfo->m_study.m_studyInstanceUID, sqa);
		SQL_GET_STR(pInfo->m_study.m_accessionNumber, sqa);
		SQL_GET_STR(pInfo->m_study.m_studyDescription, sqa);

		SQL_GET_STR(pInfo->m_series.m_seriesInstanceUID, sqa);
		SQL_GET_STR(pInfo->m_series.m_seriesDescription, sqa);
		SQL_GET_STR(pInfo->m_series.m_modality, sqa);
		pInfo->m_series.m_seriesModifyTime = VariantTimeToTime_t(sqa.getDataDate());
		SQL_GET_STR(pInfo->m_series.m_seriesDate, sqa);
		SQL_GET_STR(pInfo->m_series.m_seriesTime, sqa);
		
		SQL_GET_INT(pInfo->m_auxdata.m_key, sqa);
		SQL_GET_STR(pInfo->m_auxdata.m_auxStudyInstanceUID, sqa);
		SQL_GET_STR(pInfo->m_auxdata.m_auxSeriesInstanceUID, sqa);
		SQL_GET_STR(pInfo->m_auxdata.m_auxSOPInstanceUID, sqa);
		SQL_GET_INT(pInfo->m_auxdata.m_type,sqa);
		SQL_GET_STR(pInfo->m_auxdata.m_name,sqa);
		tf	= sqa.getDataDate();
		VariantTimeToSystemTime(tf,&st);
		sprintf(pInfo->m_auxdata.m_auxSeriesDate,"%d%02d%02d",st.wYear,st.wMonth,st.wDay);
		sprintf(pInfo->m_auxdata.m_auxSeriesTime,"%02d%02d%02d",st.wHour,st.wMinute,st.wSecond);
		SQL_GET_STR(pInfo->m_auxdata.m_subtype, sqa);
		SQL_GET_STR(pInfo->m_auxdata.m_processName,sqa);
		SQL_GET_STR(pInfo->m_auxdata.m_processType,sqa);
		SQL_GET_STR(pInfo->m_auxdata.m_volumesHash,sqa);
		SQL_GET_STR(pInfo->m_auxdata.m_parameterHash,sqa);
		SQL_GET_STR(pInfo->m_auxdata.m_volumeID,sqa);
		

		retcd = sqa.MoveNext();
	}
	SQLExecuteEnd(sqa);	 

	return kOK;


}


int	CPxDB::GetPatientAuxData(const vector<AuxReference>& iReferenceFilter, AuxDataInfo& ioAuxData )
{
	AqString strSQL;
	AqString tmpStr, filterStr;

	MakeAuxFilter(ioAuxData, filterStr);
	if(!filterStr.IsEmpty())
	{
		filterStr = "Where " + filterStr + " AND ";
	}
	else
	{
		filterStr = "Where ";
	}

	int i, rSize = iReferenceFilter.size();
	if(rSize < 0)
		return kNoResult;

	filterStr = filterStr + "VolumeID in ( '" + iReferenceFilter[0].m_volumeID + "'";
	for(i=1; i< rSize; i++)
	{
		filterStr = filterStr + ",'" + iReferenceFilter[i].m_volumeID +"'";
	}
	filterStr += ")";

	
	// get aux data sort as newest data in front bigger ID means newer get by
	// 'order by PrivatedataID DESC '
	// and in each record the series is in reference order control as 'order byReferenceSortKey ASC'
	strSQL.Format("SELECT PrivateDataID, AuxStudyUID, AuxSeriesUID, AuxSOPUID, Type, Name, Date, Subtype, "
		"ProcessName, ProcessType, VolumesHash, ParameterHash, StudyInstanceUID, SeriesInstanceUID, VolumeID "
		"From PatientAuxDataView %s  ORDER BY PrivatedataID DESC, ReferenceSortKey ASC", filterStr);

	SQA sqa(getLocalDBType());
	sqa.SetCommandText(strSQL);
	int retcd = SQLExecuteBegin(sqa);
	if(retcd != kOK) return retcd;
	int size = sqa.GetRecordCount(); if(size < 1) return kNoResult;

	map<int, vector<AuxReference> > mapRefData;
	map<int, AuxDataInfo > mapAuxData;

	vector<AuxReference> dumvRef;
	AuxDataInfo auxdata;
	AuxReference refData;
	int auxID, index = 0;

	double tf;
	SYSTEMTIME st;
	retcd = sqa.MoveFirst(); if(retcd != kOK)  return retcd;
	while( retcd == kOK && index < size )
	{
		SQL_GET_INT(auxID, sqa);
		if(mapAuxData.find(auxID) == mapAuxData.end())
		{
			auxdata.Clear();
			auxdata.m_key = auxID;
			SQL_GET_STR(auxdata.m_auxStudyInstanceUID, sqa);
			SQL_GET_STR(auxdata.m_auxSeriesInstanceUID, sqa);
			SQL_GET_STR(auxdata.m_auxSOPInstanceUID, sqa);
			SQL_GET_INT(auxdata.m_type,sqa);
			SQL_GET_STR(auxdata.m_name,sqa);
			tf	= sqa.getDataDate();
			VariantTimeToSystemTime(tf,&st);
			sprintf(auxdata.m_auxSeriesDate,"%d%02d%02d",st.wYear,st.wMonth,st.wDay);
			sprintf(auxdata.m_auxSeriesTime,"%02d%02d%02d",st.wHour,st.wMinute,st.wSecond);
			SQL_GET_STR(auxdata.m_subtype, sqa);
			SQL_GET_STR(auxdata.m_processName,sqa);
			SQL_GET_STR(auxdata.m_processType,sqa);
			SQL_GET_STR(auxdata.m_volumesHash,sqa);
			SQL_GET_STR(auxdata.m_parameterHash,sqa);
			mapAuxData[auxID] =  auxdata;

			mapRefData[auxID] = dumvRef;
		}
		else
		{
			// skip duplicate aux data information
			sqa.SetIndex(12);
		}


		SQL_GET_STR(refData.m_referencedStudyInstanceUID, sqa);
		SQL_GET_STR(refData.m_referencedSeriesInstanceUID, sqa);
		SQL_GET_STR(refData.m_volumeID,sqa);

		mapRefData[auxID].push_back(refData);
		
		retcd = sqa.MoveNext();
	}
	SQLExecuteEnd(sqa);	
	
	map<int, vector<AuxReference> >::iterator iter;
	for(iter=mapRefData.begin(); iter != mapRefData.end(); iter++)
	{
		if(iter->second.size() != rSize)
			continue;

		if(iter->second == iReferenceFilter)
		{
			ioAuxData = mapAuxData[iter->first];
			return kOK;
		}

	}


	return kNoResult;


}


int	CPxDB::CreateAppLock(SQA& iSqa, const char* iName, LOCK_TYPE iType, int iTimeOut/*=2000*/ )
{
	if(iTimeOut <= 0)
		iTimeOut = 0;

	iSqa.FormatCommandText("declare @ret int EXEC @ret=sp_getapplock '#%d-%s', 'Exclusive', 'Session', %d select @ret", iType, iName, iTimeOut);
	int retcd = SQLExecuteBegin(iSqa);
	if(retcd != kOK) 
	{
		SQLCommit(iSqa, false);
		return retcd;
	}

	retcd = iSqa.MoveFirst(); 
	if(retcd != kOK)	
	{
		SQLCommit(iSqa, false);
		return retcd;
	}

	int pcode = iSqa.getDataInt();
	
	if(pcode != 0)
	{
		SQLCommit(iSqa, false);
		return kDBTimeout;
	}

		
	// skip SQLExecuteEnd call to keep the connection to hold the lock
	// just do commit;
	SQLCommit(iSqa, true);
	return retcd;
}

int	CPxDB::ReleaseAppLock(SQA& iSqa, const char* iName, LOCK_TYPE iType)
{
	iSqa.FormatCommandText("declare @ret int EXEC @ret=sp_releaseapplock '#%d-%s', 'Session' select @ret", iType, iName);
	int retcd = SQLExecuteBegin(iSqa);
	if(retcd != kOK) 
	{
		SQLCommit(iSqa,false);
		return retcd;
	}

	retcd = iSqa.MoveFirst(); 
	if(retcd != kOK)	
	{
		SQLCommit(iSqa,false);
		return retcd;
	}

	int pcode = iSqa.getDataInt();
	
	if(pcode != 0)
	{
		SQLCommit(iSqa,false);
		return kParameterError;
	}
		

	// skip SQLExecuteEnd call to keep the connection to hold ptential other locks on this connection
	// user msut hold the SQA object to avoid lost connection

	// just do commit;
	SQLCommit(iSqa,true);
	return retcd;
}


// #15 add locale ConectionInfo 2012/04/24
void CPxDB::SetMyDBName(const wchar_t *DBName)
{
	
	AqUString strSQL;
	 
#if 0
	strSQL.Format(L"Provider=SQLOLEDB;Data Source=%S;Integrated Security=SSPI;Initial Catalog=%s;APPLICATION NAME=%S", 
			c_dbServerName, 
			DBName,
			GetCurrentProcessName());
		 
	 SetMyDBConectionInfo(strSQL);
#else //2012/05/14
	 ///////////
	 switch(getLocalDBType()){ // add SQLite 2011/09/08 K.Ko
		case kDBType_MSSQL:
		{
			if(c_dbUsername[0] == 0)
			{
				strSQL.Format(L"Provider=SQLOLEDB;Data Source=%S;Integrated Security=SSPI;Initial Catalog=%s;APPLICATION NAME=%S", 
					c_dbServerName, 
					DBName,
					GetCurrentProcessName());
			}
			else
			{
				if(c_dbUserPassword[0])
				{
					strSQL.Format(L"Provider=SQLOLEDB;Data Source=%S;User ID=%S;Password=%S;Initial Catalog=%s;APPLICATION NAME=%S", 
						c_dbServerName, 
						DBName,
						c_dbUsername, c_dbUserPassword, GetCurrentProcessName());
				}
				else
				{
					strSQL.Format(L"Provider=SQLOLEDB;Data Source=%S;User ID=%S;Initial Catalog=%s;APPLICATION NAME=%S", 
						c_dbServerName, DBName,
						c_dbUsername, GetCurrentProcessName());

				}
			}
		}
		break;
		case kDBType_SQLite:
		{
		 	strSQL.Format(L"Provider=SQLITE;Data Source=%s",DBName);
		//	strSQL.Format(L"%S",c_dbServerName);
		}
		break;

		}

	 ////////
	 SetMyDBConectionInfo(strSQL);
#endif	
}


bool CPxDB::SaveMediaPoints(const std::vector<MediaPoint>& mediaPoint)//#46
{
	 
	AqString drv;
		
	int listNum = mediaPoint.size();

	int index = 0;
	for( index=0; index<listNum; index++ )
	{
       
		AqString regPath;
		regPath.Format("%s\\DriveList\\%02X", kDefaultAQCommonLocation, index);

        
		if(!SetRegistryKey("Type",regPath,mediaPoint[index].m_mediaType)){
			return false;
		}

		if(!SetRegistryKey("Label",regPath,mediaPoint[index].m_mediaLabel)){
			return false;
		}

		if(!SetRegistryKey("Path",regPath,mediaPoint[index].m_mediaPoint)){
			return false;
		}

		if(!SetRegistryKey("Drive",regPath,"")){
			return false;
		}
		//
		if(!SetRegistryKey("DiskHighWaterMark",regPath,mediaPoint[index].m_highWaterMark)){
			return false;
		}
		if(!SetRegistryKey("DiskLowWaterMark",regPath,mediaPoint[index].m_lowWaterMark)){
			return false;
		}
 

	}
	//Other
	for( ; index<cDriveListMax; index++ )
	{
        AqString regPath;
		regPath.Format("%s\\DriveList\\%02X", kDefaultAQCommonLocation, index);

        
		SetRegistryKey( "Label",regPath, "" );
		SetRegistryKey( "Type", regPath,"" );
		SetRegistryKey( "Path", regPath,"" );
		SetRegistryKey( "Drive", regPath,"" );
	}

 
	return true;
}

const char *CPxDB::getAppCommonRegistry() //#46
{
	return kDefaultAQCommonLocation;
}

const char *CPxDB::getAppDefaultRegistry() //#46
{
	return kDefaultLocation;
}

//#140_search_Japanese_JIS_UTF8
void _getSQLStr_jis_utf8_fromSJIS(const std::string &inputSJIS, AqUString &outJis, AqUString &outUTF8)
{
	if (inputSJIS.size() < 1) return;

	int iCodePage = _Def_CodePage_SJIS_932;

	bool bPreAsterisk = false;
	std::string jisTemp = inputSJIS;
	if (g_JisCharLib != nullptr){
		//convert from sjis to jis(DB search only,no ESC keys of JIS)
		jisTemp = g_JisCharLib->convertSJToJCodeOnlyForSQL(inputSJIS);
		if ((jisTemp != inputSJIS) && (inputSJIS[0] != '*')){
			bPreAsterisk = true;
		}
	}
	outJis.Convert(jisTemp.c_str(), iCodePage);
	outUTF8.Convert(inputSJIS.c_str(), iCodePage);
	if (bPreAsterisk){
		outJis.Format(L"%s%s", L"*", outJis);
		outUTF8.Format(L"%s%s", L"*", outUTF8);
	}
}
void _getSQLStr_jis_utf8_fromJIS(const std::string &inputJIS, AqUString &outJis, AqUString &outUTF8)
{
	if (inputJIS.size() < 1) return;
	if (g_JisCharLib == nullptr) return;
	std::string sjis_str = g_JisCharLib->convertJIStoSJIS(inputJIS);
	_getSQLStr_jis_utf8_fromSJIS(sjis_str, outJis, outUTF8);
}
void _getSQLStr_jis_fromUTF8(const std::string &inputStr, AqUString &outJis, AqUString &outUTF8)
{
	
	if (inputStr.size() < 1) return;

	std::string sjis_str = inputStr;
	{//convert UTF8 to SJIS
		std::string strUTF8;
		int WCharStrBuffSize = inputStr.size()*4 ;
		WCHAR *WCharStrBuff = new WCHAR[WCharStrBuffSize];

		int iCodePage = CP_UTF8;
		int WnewLen = MultiByteToWideChar(
			CP_UTF8, 0,
			inputStr.c_str(), inputStr.size(),
			WCharStrBuff, WCharStrBuffSize);
		if (WnewLen > 0){
			WCharStrBuff[WnewLen] = 0;
		}
		{
			int CharStrBuffSize = 2*WnewLen+1;
			char *CharStrBuff = new char[CharStrBuffSize];
			int newLen = WideCharToMultiByte(
				_Def_CodePage_SJIS_932, 0,
				WCharStrBuff, WnewLen,
				CharStrBuff, CharStrBuffSize,
				NULL,
				NULL);
			CharStrBuff[newLen] = 0;
			sjis_str = CharStrBuff;
			delete[] CharStrBuff;
		}
		delete[] WCharStrBuff;
	}
	_getSQLStr_jis_utf8_fromSJIS(sjis_str, outJis, outUTF8);

}
//#136 2021/01/12 N.Furutsuki unicode version
static int MakeUserStudiesFilter_UicodeField(DICOMData& iKey, AqUString& cond, int iUserID, unsigned int iCodePage)
{
	cond.Empty();
	AqUString tmpStr;
	AqString range;

	std::map<const char*, const char*> mapConLike;
	std::map<const char*, const char*>::iterator iter;

	// setup key map
	mapConLike["PatientsName"] = iKey.m_patientsName;
	mapConLike["ReadingPhysiciansName"] = iKey.m_radiologistName;
	mapConLike["ReferringPhysiciansName"] = iKey.m_referringPhysiciansName;
	mapConLike["StudyDescription"] = iKey.m_studyDescription;
	mapConLike["SeriesDescription"] = iKey.m_seriesDescription;

	// walk throiugh map to generate query string
	for (iter = mapConLike.begin(); iter != mapConLike.end(); iter++)
	{
		if (iter->second[0] == 0)
			continue;

		AqUString UKeyTemp;
		AqUString UValTemp;
		UKeyTemp.ConvertUTF8(iter->first);
		//	UValTemp.ConvertUTF8(iter->second);
#if 1 //#2240_for_Japanese_DB_Search
		std::string orgValStr = iter->second;
		AqUString UValTempUTF8;
		if (iCodePage == _Def_MyCodePage_JIS){
			//for Japanese SJI (with ESC key)
			_getSQLStr_jis_utf8_fromJIS(orgValStr, UValTemp, UValTempUTF8);
		}
		else
		if (iCodePage == _Def_CodePage_SJIS_932){
			//for Japanese SJIS only
			_getSQLStr_jis_utf8_fromSJIS(orgValStr, UValTemp, UValTempUTF8);
		}
		else
		if (iCodePage == CP_UTF8){
			//try Japanese UTF8
			_getSQLStr_jis_fromUTF8(orgValStr, UValTemp, UValTempUTF8);
			
		}else{

			//for Japanese
			//this case means, JIS converted  already
			//and MultiLang other
			UValTemp.Convert(iter->second, iCodePage);
		}
		AqUString tmpStr2;
		if (!HasMatchChar(orgValStr.c_str()))
		{
			tmpStr.Format(L"%s = N'%s'", UKeyTemp, ToSQLUString(UValTemp));
			if (!UValTempUTF8.IsEmpty()){//append Japanese UTF8
				tmpStr2.Format(L"%s = N'%s'", UKeyTemp, ToSQLUString(UValTempUTF8));
			}
			tmpStr.Format(L" %s OR %s", tmpStr, tmpStr2);
		}
		else if (strcmp(iter->second, "*") != 0) // skip match all condition
		{
			tmpStr.Format(L"%s LIKE N'%s'", UKeyTemp, ToSQLUString(UValTemp, true));
			//	Converting DICOM Wildcards to SQL Wildcards, GL
			if (!UValTempUTF8.IsEmpty()){//append Japanese UTF8
				tmpStr2.Format(L"%s LIKE N'%s'", UKeyTemp, ToSQLUString(UValTempUTF8, true));
				tmpStr.Format(L"( %s OR %s)", tmpStr, tmpStr2);
			}

			//#578  2013/09/04 K.Ko
			sqlUCharReplace(tmpStr);

		}
		if (!cond.IsEmpty()) cond += L" AND "; cond += tmpStr;
#else
		UValTemp.Convert(iter->second, iCodePage);
		if (!HasMatchChar(iter->second))
		{
			tmpStr.Format(L"%s = N'%s'", UKeyTemp, ToSQLUString(UValTemp));
			if (!cond.IsEmpty()) cond += L" AND "; cond += tmpStr;
		}
		else if (strcmp(iter->second, "*") != 0) // skip match all condition
		{
			tmpStr.Format(L"%s LIKE N'%s'", UKeyTemp, ToSQLUString(UValTemp, true));
			//	Converting DICOM Wildcards to SQL Wildcards, GL

			//#578  2013/09/04 K.Ko
			sqlUCharReplace(tmpStr);
			if (!cond.IsEmpty()) cond += L" AND "; cond += tmpStr;
		}
#endif
	}
	return kOK;
}
int CPxDB::MakeUserStudiesFilterU(const DICOMData& iKey, AqUString& cond, int iUserID)
{
	AqUString UFieldFilter;
	DICOMData iFilterLocal = iKey;
	{//pick up filter for unicode field

		DICOMData  unicode_iKey;
		if (strlen(iFilterLocal.m_patientsName) > 0){
			strcpy(unicode_iKey.m_patientsName, iFilterLocal.m_patientsName);
			iFilterLocal.m_patientsName[0] = 0;
		}
		if (strlen(iFilterLocal.m_referringPhysiciansName) > 0){
			ASTRNCPY(unicode_iKey.m_referringPhysiciansName, iFilterLocal.m_referringPhysiciansName);
			iFilterLocal.m_referringPhysiciansName[0] = 0;
		}
		if (strlen(iFilterLocal.m_studyDescription) > 0){
			ASTRNCPY(unicode_iKey.m_studyDescription, iFilterLocal.m_studyDescription);
			iFilterLocal.m_studyDescription[0] = 0;
		}
		if (strlen(iFilterLocal.m_seriesDescription) > 0){
			ASTRNCPY(unicode_iKey.m_seriesDescription, iFilterLocal.m_seriesDescription);
			iFilterLocal.m_seriesDescription[0] = 0;
		}
		unsigned int iCodePage = CPxDB::getCodePageFromCharatorSet(iKey.m_characterSet);
		MakeUserStudiesFilter_UicodeField(unicode_iKey, UFieldFilter, iUserID, iCodePage);
	}
	// for no-unicode filed
	AqString filterTemp = "";
	CPxDB::MakeUserStudiesFilter(iFilterLocal, filterTemp, iUserID);

	//make unicode SQL
	AqUString UstrSQL;

	if (UFieldFilter.GetLength() > 0){
		UstrSQL.Format(L" %s", UFieldFilter);
	}
	if (filterTemp.GetLength() > 0){
		AqUString UfilterTemp;
		UfilterTemp.ConvertUTF8(filterTemp);
		if (UstrSQL.GetLength() > 0){
			UstrSQL.Format(L"%s AND %s", UstrSQL, UfilterTemp);
		}
		else{
			UstrSQL.Format(L" %s", UfilterTemp);
		}
	}
	cond = UstrSQL;
	return kOK;
}
inline std::string _normalizeCharsetStr(const char *charSet)
{
	std::string retStr;
	for (int i = 0; i < strlen(charSet); i++){
		char c = charSet[i];
		if (c == '_'){
			c = ' ';
		}
		retStr.push_back(c);
	}
	return retStr;
}
unsigned int CPxDB::getCodePageFromCharatorSet(const char *charSetIn)
{
	std::string normalize_charSet = _normalizeCharsetStr(charSetIn);
	const char *charSet = normalize_charSet.c_str();
	unsigned int iCodePage = 1252;//Latin1  <-changed default  //CP_UTF8;
	AqString SpecCharSet = charSet;
	SpecCharSet.ToUpper();
	//#2103
	if (SpecCharSet.GetLength() > 0){
		if (AqString::StrStr(SpecCharSet, "GB18030") != NULL){
			//for Chinese charator
			iCodePage = 54936;//GB18030
		}
		else //#2108 2021/01/06
		if (AqString::StrStr(SpecCharSet, "ISO IR 144") != NULL){
			//for Russian charator
			iCodePage = 1251;//Russian ISO-8859-5
		}
		else
		if (AqString::StrStr(SpecCharSet, "ISO IR 192") != NULL){
			// #139_Viwer(#2215)_unicode_UTF8
			//for Taiwan Chinese charator
			iCodePage = CP_UTF8;
		}
		else //#139_ChineseTW_modify_Multi-LANG
		if (AqString::StrStr(SpecCharSet, "BIG5") != NULL){
			// #139_Viwer(#2215)_input_from_gui_big5
			//for Taiwan Chinese charator
			iCodePage = 950;
		}
		else
		if (AqString::StrStr(SpecCharSet, "SJIS") != NULL){
			// added 2022/11/08
			// Japanese
			//note:
			//when search string
			//Already converted to JIS (ANSI), ignored this codepage
			iCodePage = _Def_CodePage_SJIS_932;
		}
		else
		if (AqString::StrStr(SpecCharSet, "ISO 2022 IR 87") != NULL){
			//JIS of my define
			iCodePage = _Def_MyCodePage_JIS;

		}
		else //2021/01/26
		if (AqString::StrStr(SpecCharSet, "IR 100") != NULL){
			//for Latin1
			iCodePage = 1252;//Latin1

		}
		
	}
	return iCodePage;
}