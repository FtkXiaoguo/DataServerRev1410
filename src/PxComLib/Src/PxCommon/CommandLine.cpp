/***********************************************************************
 * $Id: CommandLine.cpp 35 2008-08-06 02:57:21Z atsushi $
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
#include "rtvloadoption.h"
#include "Job.h"
#include "CommandLine.h"

//-----------------------------------------------------------------------------------------
//
void KVPCopyKey(KVP_MAP& oMap, KVP_MAP& iMap, const char* iKey)
{
	KVP_MAP::iterator iter = iMap.find(iKey);

	if (iter != iMap.end())
	{
		if (iter->second.size() > 0)
			oMap[iKey] = iter->second;
	}
}

//-----------------------------------------------------------------------------------------
//
JobInfoPublisher* CommandLine::GetJobInfoPublisher(void)
{
	int jobID = 0;
	std::string procName = "";
	KVP_MAP progressKVP;
	JobInfoPublisher* pProgress;

	//!!! GL 5-2-2006 It is a design flaw that JobInfoPublisher need job ID
	// for progress handling. Process itself does not need to know anything about job.
	// The process should be able to run outside queue mamnger
	// Any job information is job manager knowledge, and process should not keep/manage any those information

	//	Get the jobID and create a JobInfoPublisher for it
	KVP_MAP::iterator iter = m_kvp.find("id");
	jobID = (iter != m_kvp.end()) ? jobID = atoi(iter->second.c_str()) : 0;	//CJob::GenerateID();
	pProgress = new JobInfoPublisher(jobID);
	
	
	
	//!!! GL 5-2-2006 again, job information is job manager knowledge, process should not keep/manage any those information

	//	Populate the JobInfo keys from the job
	KVPCopyKey(progressKVP, m_kvp, kJOBKEYdisplayName);
	KVPCopyKey(progressKVP, m_kvp, kJOBKEYtargetProcessName);
	KVPCopyKey(progressKVP, m_kvp, kJOBKEYrequestingUserID);
	KVPCopyKey(progressKVP, m_kvp, kJOBKEYsubmitTime);
	KVPCopyKey(progressKVP, m_kvp, kJOBKEYsourceAE);
	KVPCopyKey(progressKVP, m_kvp, kJOBKEYdestAE);
	KVPCopyKey(progressKVP, m_kvp, kJOBKEYpatientID);
	KVPCopyKey(progressKVP, m_kvp, kJOBKEYpatientName);
	KVPCopyKey(progressKVP, m_kvp, kJOBKEYseriesNumber);
	KVPCopyKey(progressKVP, m_kvp, kJOBKEYaccessionNumber);
	KVPCopyKey(progressKVP, m_kvp, kJOBKEYpriority);

	pProgress->SetInfo(progressKVP);

	return pProgress;
}

//-----------------------------------------------------------------------------------------
//
int CommandLine::ProcessCmdLineArgs(int argc, char** argv)
{
	std::string key, value;
	bool printUsage = false;
	std::string jobFile = "";

	m_pLogger = GetAqLogger();

	//	Looking for key value pairs only
	if (!(argc % 2))
	{
		PrintError("ERROR: Number of arguments must be even!\n", "");
		return Usage();
	}

	//	Mark empty values as REQUIRED args.  That way, we can distinguish
	//		between these and empty values from a jobFile.
	KVP_MAP::iterator iter;
	for(iter = m_kvp.begin(); iter != m_kvp.end(); iter++)
	{
		if (iter->second.compare("") == 0)
		{
			m_kvp[iter->first] = "REQUIRED";
		}
	}

	m_kvp["argv0"] = argv[0];

	//	Process the cmdLine args
	for (int i = 1; i < argc;)
	{
		if (!strcmp(argv[i], "-jobFile"))
		{
			jobFile = argv[i+1];
//			i+=2;
//			continue;
		}
		else if (argv[i][0] != '-' || (m_kvp.find(argv[i]+1) == m_kvp.end()))
		{
			PrintError("ERROR: Invalid argument: ", argv[i]);
			printUsage = true;
			i += 2;
			continue;
		}

		key.assign(argv[i++]+1);
		value.assign(argv[i++]);
		m_kvp[key] = value;
	}

	//	If there are keys in this file, they override what was passed on the cmdLine
	LoadJobFile(jobFile);

	//	Check for missing required arguments
	for(iter = m_kvp.begin(); iter != m_kvp.end(); iter++)
	{
		if (iter->second.compare("REQUIRED") == 0)
		{
			printUsage = true;
			PrintError("ERROR: Missing Required argument: ", iter->first.c_str());
		}
	}

	if (printUsage)
	{
		fprintf(stderr,"\n");
		return Usage();
	}

	return 0;
}

//-----------------------------------------------------------------------------------------
//
void CommandLine::LoadJobFile(std::string iFilename)
{
	iRTVOptions opt;
	std::string key, value, existingValue;
	int optCount = 0, i;
	bool genKeys = true;
	KVP_MAP::iterator iter;

	if (iFilename.size() < 1)
		return;

	//	Load the file into "opt"
	opt.LoadWithLock(iFilename.c_str(), genKeys);
	optCount = opt.GetCount();

	for (i = 0; i < optCount; i++)
	{
		key = opt.GetOptionName(i);
		value = opt.GetOptionValue(key.c_str());

		//	If the value is empty, check original map to see if it's
		//		a required argument.  If it is, don't overwrite it.
		//		That way, we can report it later as a missing required arg.
		if (value.size() == 0)
		{
			iter = m_kvp.find(key);
			if (iter != m_kvp.end())
			{
				existingValue = iter->second;
				if (existingValue.compare("REQUIRED") == 0)
					continue;
			}
		}

		m_kvp[key] = value;
	}
}

//-----------------------------------------------------------------------------------------
//
const char* CommandLine::GetValue(const char* iKey)
{ 
	KVP_MAP::iterator iter = m_kvp.find(iKey);
	if (iter != m_kvp.end())
		return iter->second.c_str(); 

	return "";
}

//-----------------------------------------------------------------------------------------
//
int CommandLine::Usage()
{
	std::string msg = "USAGE: " +  m_programName + " <arglist>";
	PrintError(msg.c_str(),"");
	PrintError("Possible arguments","");

	KVP_MAP::iterator iter;
	std::string argName, argValue, both;
	for(iter=m_kvp.begin(); iter != m_kvp.end(); iter++)
	{
		argName  = iter->first;
		argValue = iter->second;
		argValue = (argValue.empty()) ? "[REQUIRED]" : "[DEFAULT VALUE = " + argValue + "]";
		both = argName + std::string(" ") + argValue;
		PrintError("\t-", both.c_str());
	}

	return -1;
}

//-----------------------------------------------------------------------------------------
//
void CommandLine::Print(int iLogLevel, int iOmitOptional)
{
	KVP_MAP::iterator iter;
	for(iter=m_kvp.begin(); iter != m_kvp.end(); iter++)
	{
		if (iOmitOptional && (iter->second.compare("OPTIONAL") == 0 || iter->second.compare("CONDITIONAL") == 0))
			continue;

		m_pLogger->LogMessage(iLogLevel, "%s = %s\n", iter->first.c_str(), iter->second.c_str());
	}
}

//-----------------------------------------------------------------------------------------
//
void CommandLine::Print(FILE* iFP)
{
	if (!iFP)
		return;

	KVP_MAP::iterator iter;
	for(iter=m_kvp.begin(); iter != m_kvp.end(); iter++)
	{
		fprintf(iFP, "%s = %s\n", iter->first.c_str(), iter->second.c_str());
	}
}

//-----------------------------------------------------------------------------------------
//
void CommandLine::LogArguments(int iLogLevel, int iOmitOptional)
{
	m_pLogger->LogMessage(iLogLevel,"\n%s was called with:\n\n", m_programName.c_str());
	m_pLogger->LogMessage(iLogLevel,"------------------\n");
	Print(iLogLevel, iOmitOptional);
	m_pLogger->LogMessage(iLogLevel,"------------------\n");
}

//-----------------------------------------------------------------------------------------
//
void CommandLine::PrintError(const char* iMsg, const char* iArg)
{ 
	m_pLogger->LogMessage("%s%s\n", iMsg, iArg);
#ifndef _DEBUG
	fprintf(stderr, "%s%s\n", iMsg, iArg);
#endif
}

