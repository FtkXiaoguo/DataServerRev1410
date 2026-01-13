/***********************************************************************
 * RTVConfigFile.h
 *---------------------------------------------------------------------
 *		Copyright, TeraRecon 2001, All rights reserved.
 *
 *	PURPOSE:
 *		Read KVP based configuration files.  Based on code from 
 *		nvrserver.  Should eventually replace this with rtvloadoption 
 *
 *	AUTHOR(S):  T.C. Zhao, Rob Lewis, June 2001
 *   
 *-------------------------------------------------------------------
 */
#ifndef RTV_CONFIG_FILE_H
#define RTV_CONFIG_FILE_H

#pragma warning (disable: 4786)

#include "RTVDICOMDef.h"
//#define	kMaxItemValueLength	60
#define kMaxFilenameLength	512
#define kMaxHostnameLength  512
#define kMaxPathnameLength	512
//#define kMaxMediaLabelLength 16
//#define kMaxAELen			64
//#define kMaxUIDLen			64

#include "AqCore/TRLogger.h"
#include <map>


// basically a wrapper around char * so we can use it in std::map
class MemberName
{
public:
	MemberName(const char *iName) { m_name = iName;}
	bool	operator < (const MemberName& B) const	{ return stricmp(this->m_name, B.m_name) < 0; }
	bool	operator == (const MemberName& B) const	{ return stricmp(this->m_name, B.m_name) == 0;}
	operator const char*(void) const { return m_name;}
private:
	const char*	m_name;
};

class NameValue
{
public:
	enum {UNKNOWN, SHORT, INT, FLOAT, STRING };
	
	
	NameValue(short* iV=0)	{ m_value = (void *)iV; m_type = SHORT;}
//	NameValue(uint16* iV)	{ m_value = (void *)iV; m_type = SHORT;}
	NameValue(int* iV)		{ m_value = (void *)iV; m_type = INT;  }
//	NameValue(uint32* iV)   { m_value = (void *)iV; m_type = INT;  }
	NameValue(float* iV)	{ m_value = (void *)iV; m_type = FLOAT;}
	NameValue(char* iV, int iLen=0)  { m_value = (void *)iV; m_type = STRING; m_length = iLen;}
	
	
	void	Output(FILE *fp, const char* iName)
	{
		fprintf(fp,"%15s = ", iName);
		if (m_type == FLOAT)
			fprintf(fp,"%g\n" ,*(float*)m_value);
		else if (m_type == SHORT)		
			fprintf(fp,"%d\n" ,*(short*)m_value);
		else if (m_type == INT)
			fprintf(fp,"%d\n" ,*(int*)m_value);
		else if (m_type == STRING)
			fprintf(fp,"%s\n", m_value);
		else
		{
			fprintf(stderr,"Bad Type\n");
		}
	}
	
	
	void	Convert(const char *iValue)
	{
		if (m_type == FLOAT)
			*(float*)m_value = float(atof(iValue));
		else if (m_type == SHORT)
			*(short*)m_value = short(atoi(iValue));
		else if (m_type == INT)
			*(int*) m_value = int(atoi(iValue));
		else if (m_type == STRING)
		{
			if (m_length)
			{
				strncpy((char *)m_value, iValue, m_length-1);
				*((char *)m_value + m_length-1)= '\0';
			}
			else
				strcpy((char *)m_value, iValue);
		}
		else
			;
	}
	
private:
	void*			m_value;
	int				m_type;
	int				m_length;
};

typedef std::map<MemberName, NameValue>	KVPMap;

inline KVPMap::iterator operator + (KVPMap::iterator p, int n)
{
	for  (int i = 0; i < n; i++)
		p++;
	return p;
}

class RTVConfigFile : public TRLogger
{

public:
	RTVConfigFile();
	virtual ~RTVConfigFile() {}
	
	void SetConfigFilename(const char* iConfigFilename);
	virtual void InitKVPMap() = 0;
	void ReadConfiguration();
	virtual void ValidateAndFixConfiguration() {}

	void PrintConfiguration();

	char m_configFilename[kMaxFilenameLength];
	FILE* m_fp;
	KVPMap m_pair;
};

#endif // RTV_CONFIG_FILE_H