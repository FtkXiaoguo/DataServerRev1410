/***********************************************************************
 *CommandLine.h
 *---------------------------------------------------------------------
 *		Copyright, Terarecon 2002, All rights reserved.
 *
 *	PURPOSE:
 *		Process Command Line Arguments
 *
 *	AUTHOR(S):  Rob Lewis - July, 2002
 *
 *-------------------------------------------------------------------*/

#ifndef _COMMAND_LINE_H
#define _COMMAND_LINE_H

#include <map>
#include <string>
#include "AqCore/AqCore.h"
#include "JobInfo.h"

#define CL(a) m_cmdLine.GetValue(a)

typedef std::map<std::string, std::string> KVP_MAP;

class CommandLine
{
public:
	CommandLine() : m_programName("") {m_pLogger = GetAqLogger();}
	CommandLine(const char* iProgramName) :
	  m_programName(iProgramName) {m_pLogger = GetAqLogger();}
	virtual ~CommandLine() {}

	void InitKVP();

	const char* GetValue(const char* iKey);
	int ProcessCmdLineArgs(int argc, char** argv);
	virtual void Print(int iLogLevel = kInfo, int iOmitOptional = 0);
	virtual void Print(FILE* iFP);
	void LogArguments(int iLogLevel = kInfo, int iOmitOptional = 0);

	JobInfoPublisher* GetJobInfoPublisher(void);

	std::string m_programName;
	KVP_MAP m_kvp;

protected:
	void PrintError(const char* iMsg, const char* iArg);
	void LoadJobFile(std::string iFilename);
	virtual int Usage();
	
	AqLoggerInterface* m_pLogger;

};

#endif _COMMAND_LINE_H

/*
	static int GenerateJobID(bool iReset);
	static JobInfoPublisher* Create(int iJobID = 0);
*/