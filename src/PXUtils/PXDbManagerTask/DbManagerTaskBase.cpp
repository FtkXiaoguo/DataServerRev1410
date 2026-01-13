// DbManagerTaskBase.cpp: DbManagerTaskBase クラスのインプリメンテーション
//
//////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "DbManagerTaskBase.h"
#include "AqCore/TRLogger.h"

static TRLogger _gLogger;

//////////////////////////////////////////////////////////////////////
// 構築/消滅
//////////////////////////////////////////////////////////////////////

TRLogger *DbManagerTaskBase::getLogger()
{
	return &_gLogger;
}

DbManagerTaskBase::DbManagerTaskBase()
{
	m_DbServType = MSSQL_2005;
	m_repeatRunTask = 1;
	m_taskName = "";
	m_logFileName = "";
}

DbManagerTaskBase::~DbManagerTaskBase()
{

}
void DbManagerTaskBase::setLogFileName(const std::string fileName)
{
	getLogger()->SetLogFile(fileName.c_str());
}
void DbManagerTaskBase::ParseCommandLine(int argc, char** argv)
{
	if(argc>2){
		for(int i=2;i<argc;i++){
			if (argv[i][0] == '-'){
				switch(argv[i][1])
				{
				case 'f':
				case 'F':
					//configuration file name
					m_configName = argv[i+1];
					i++; //jump to next
					break;
				case 't':
				case 'T':
					//task Name
					m_taskName = argv[i+1];
					i++; //jump to next
					break;
				case 'r':
				case 'R':
					//repeat run task
					m_repeatRunTask = atoi(argv[i+1]);
					i++; //jump to next
					break;
				case 'd':
				case 'D':
					//database server type
					{
						int dataServType = atoi(argv[i+1]);
						switch(dataServType){
						case 0:
							m_DbServType = MSSQL_2000;
							break;
						case 1:
							m_DbServType = MSSQL_2005;
							break;
						case 2:
							m_DbServType = MSSQL_2008;
							break;
						}
					}
					i++; //jump to next
					break;
				}
			}
		}
	}

}
 
bool DbManagerTaskBase::skipRun()
{
	if(m_repeatRunTask == 1) return false;
	if(m_taskName.size()<1) return false;
	//not supported yet

	 return false;
}