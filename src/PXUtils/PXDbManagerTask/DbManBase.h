// DbManBase.h: CDbManBase クラスのインターフェイス
//
//////////////////////////////////////////////////////////////////////

#if !defined(AFX_DBMANBASE_H__A3100A78_9462_471F_ACD5_672C43674BB8__INCLUDED_)
#define AFX_DBMANBASE_H__A3100A78_9462_471F_ACD5_672C43674BB8__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#include "DbManagerTaskBase.h"

#include <string>

#pragma warning (disable: 4786)
#pragma warning (disable: 4503)


class TRLogger;
class CDbManBase  : public DbManagerTaskBase
{
public:

	CDbManBase();
	virtual ~CDbManBase();
		
	bool tryOpenDB();
	bool findValidData(bool ignoreStudyCheck=false);


	virtual bool doDbManTask()=0;

	virtual int doMain(int argc, char** argv) ;
	virtual int initMain(int argc, char** argv) ;
protected:

	virtual void ParseCommandLine(int argc, char** argv);


	std::string getSQLCmd(bool cmd_option=true) const;

	bool runCmd(const std::string cmd);


	std::string m_DBServerName;
};

#endif // !defined(AFX_DBMANBASE_H__A3100A78_9462_471F_ACD5_672C43674BB8__INCLUDED_)
