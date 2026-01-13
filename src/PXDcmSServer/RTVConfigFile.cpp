/***********************************************************************
 * $Id: RTVConfigFile.cpp 35 2008-08-06 02:57:21Z atsushi $
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
#include "RTVConfigFile.h"

RTVConfigFile::RTVConfigFile()
{
}

void RTVConfigFile::SetConfigFilename(const char* iFilename)
{
	if (iFilename)
	{
		strncpy(m_configFilename, iFilename, kMaxFilenameLength);
	}
}

void RTVConfigFile::ReadConfiguration(void) 
{
	InitKVPMap();

	if (!m_configFilename)
		return;
	if (!(m_fp = fopen(m_configFilename,"r")))
	{
		LogMessage("Can't open config file %s. Using defaults\n", m_configFilename);
		return;
	}

	KVPMap::iterator p;
	char buf[1024], name[512], value[512];
	
	for (buf[0] = '\0' ; fgets(buf,sizeof(buf), m_fp) && !ferror(m_fp);)
	{
		if (buf[0] == '#') 
			continue; // ignore comments
		
		if ( sscanf(buf, " %s = %s ", name, value) == 2 )
		{
			if ((p = m_pair.find(name)) != m_pair.end())
				p->second.Convert(value);
			else
			{
#ifdef _DEBUG
				fprintf(stderr,"Unknown token: %s = %s\n", name, value);
#endif
				;	// ignore stuff we don't know
			}
		}
#ifdef _DEBUG
		else
		{
		//	fprintf(stderr,"Ignoring line %s", buf);
		}
#endif
	}
	
	ValidateAndFixConfiguration();
		
	fclose(m_fp);
	
	// respond to the configuration
}

void RTVConfigFile::PrintConfiguration()
{
	KVPMap::iterator p;

	fprintf(stdout,"\n\n--------- Configuration Values ---------\n\n");
	for ( p = m_pair.begin(); p != m_pair.end(); p++)
	{
		if (strcmp(p->first, "[s@gt$29") != 0)
			p->second.Output(stdout, p->first);
	}
	fprintf(stdout,"----------------------------------------\n\n");
}

