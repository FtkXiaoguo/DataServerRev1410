// DbManagerTaskBase.h: DbMangerTaskBase クラスのインターフェイス
//
//////////////////////////////////////////////////////////////////////

#if !defined(AFX_DBMANGERTASKBASE_H__9EF02614_45FA_49F2_A601_CAD2BDF241C2__INCLUDED_)
#define AFX_DBMANGERTASKBASE_H__9EF02614_45FA_49F2_A601_CAD2BDF241C2__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#include <string>

#pragma warning (disable: 4786)
#pragma warning (disable: 4503)

class TRLogger;
class DbManagerTaskBase  
{
public:
	enum DbServerType {
		MSSQL_2000,
		MSSQL_2005,
		MSSQL_2008
	};
	DbManagerTaskBase();
	virtual ~DbManagerTaskBase();
	virtual int doMain(int argc, char** argv) = 0;
		
	virtual bool skipRun();

	void setDbServerType(DbServerType type) { m_DbServType = type;};

		
	static void setLogFileName(const std::string fileName);

	static TRLogger *getLogger();
protected:
	virtual bool loadConfiguration(const char *fileName)=0;
	virtual void ParseCommandLine(int argc, char** argv);
	std::string m_configName;
	DbServerType m_DbServType;
	//
	std::string m_taskName;
	//
	int m_repeatRunTask;
	//
	std::string m_logFileName;
 
};

#endif // !defined(AFX_DBMANGERTASKBASE_H__9EF02614_45FA_49F2_A601_CAD2BDF241C2__INCLUDED_)
