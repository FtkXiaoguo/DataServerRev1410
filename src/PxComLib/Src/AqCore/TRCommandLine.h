/***********************************************************************
 * TRCommandLine.h
 *---------------------------------------------------------------------
 *		Copyright, PreXion 2012, All rights reserved.
 *
 *	PURPOSE:
 *		Process Command Line Arguments
 *
 *	
 *
 *-------------------------------------------------------------------*/

#ifndef _TRCOMMANDLINE_H
#define _TRCOMMANDLINE_H

#include <map>
#include <vector>
#include <string>
#include "AqCore.h"



typedef std::map<std::string, std::string> MapStrStr;


class TRCommandLine
{
public:
	TRCommandLine() {}

	~TRCommandLine() {} // no intend for inherinting


	void ParseCommandLine(int argc, char** argv);
	void LogArguments(int iLogLevel = kInfo)  const;

	const char* GetOption(const char* iKey)  const;
	const char* GetParameter(const char* iKey)  const;

	std::string m_callpath;
	MapStrStr m_options;				// option foramt (key : value) : -o v
	MapStrStr m_parameters;				// parameter foramt (key : value) : --p v
	std::vector<std::string> m_args;	//  any argments is not option or parameter

};

#endif _TRCOMMANDLINE_H

