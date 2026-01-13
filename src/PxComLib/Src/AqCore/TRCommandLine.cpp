/***********************************************************************
 * TRCommandLine.cpp
 *---------------------------------------------------------------------
 *		Copyright, PreXion 2012, All rights reserved.
 *
 *	PURPOSE:
 *		Process Command Line Arguments
 *
 *	
 *
 *-------------------------------------------------------------------*/

#pragma warning (disable: 4786)
#include "TRCommandLine.h"

#ifndef _WIN32_WINNT
#define _WIN32_WINNT 0x0500
#endif

#define _WIN32_DCOM
#include "Windows.h"


//-----------------------------------------------------------------------------------------
//
const char* TRCommandLine::GetOption(const char* iKey) const
{
	TRCommandLine* me = (TRCommandLine*)(this);
	MapStrStr::iterator iter = me->m_options.find(iKey);
	if (iter != me->m_options.end())
		return iter->second.c_str(); 

	return 0;
}


const char* TRCommandLine::GetParameter(const char* iKey)  const
{
	TRCommandLine* me = (TRCommandLine*)(this);
	const MapStrStr::iterator iter = me->m_parameters.find(iKey);
	if (iter != me->m_parameters.end())
		return iter->second.c_str(); 

	return 0;
}


//-----------------------------------------------------------------------------------------
//
void TRCommandLine::ParseCommandLine(int argc, char** argv)
{
	if(m_callpath.size() != 0)
	{
		m_callpath = "";
		m_options.clear();
		m_parameters.clear();
		m_args.clear();

	}
	if(argc < 1)
		return;

	m_callpath = argv[0];
	char *key, *value, *curArg;
	//	Process the cmdLine args
	for (int i = 1; i < argc;)
	{
		curArg = argv[i];
		i++;
		if (curArg[0] == '-')
		{
			value = "";
			if(i < argc && argv[i][0] != '-')
			{
				value = argv[i];
				i++;
			}

			if (curArg[1] != '-')
			{
				key = (curArg+1);
				m_options[key] = value;

			}
			else
			{
				key = (curArg+2);
				m_parameters[key] = value;
			}
		}
		else 
		{
			m_args.push_back(curArg);
		}

	}

}

void TRCommandLine::LogArguments(int iLogLevel)  const
{
	GetAqLogger()->LogMessage(iLogLevel, "call path: %s\n", m_callpath);

	TRCommandLine* me = (TRCommandLine*)(this);
	std::string options, parameters, argments;
	MapStrStr::iterator iter;
	for (iter = me->m_options.begin(); iter != me->m_options.end(); iter++)
	{
		options += iter->first + ":" + iter->second + " ";;
	}

	for (iter = me->m_parameters.begin(); iter != me->m_parameters.end(); iter++)
	{
		parameters += iter->first + ":" + iter->second + " ";;
	}

	for (int i = 0; i < m_args.size(); i++)
	{
		argments += m_args[i] + " ";
	}

	GetAqLogger()->LogMessage(iLogLevel, "call path: %s; options: %s; paramters: %s; argments: %s\n", 
		m_callpath.c_str(), options.c_str(), parameters.c_str(), argments.c_str());
}
