// DbManBackup.h: CDbManShrink クラスのインターフェイス
//
//////////////////////////////////////////////////////////////////////

#if !defined(AFX_DBSHRINK_H__ED8D4ED3_CFB3_4117_A506_125E1B75E30A__INCLUDED_)
#define AFX_DBSHRINK_H__ED8D4ED3_CFB3_4117_A506_125E1B75E30A__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#include "DbManBase.h"
#include <vector>
class CDbManShrink : public CDbManBase  
{
#define BACKUP_GENERATIONS (8)
public:
	CDbManShrink();
	virtual ~CDbManShrink();

	void addDbName(std::string DbName);
 
	virtual bool doDbManTask();

 
virtual int doMain(int argc, char** argv) ;
virtual int initMain(int argc, char** argv) ;
protected:

	bool shrinkDB(const std::string dbName);
	virtual bool loadConfiguration(const char *fileName);
	void ParseCommandLine(int argc, char** argv);

	std::string getBackupDbName(const std::string dbName);

	bool shrinkOneDb(const std::string dbName);
	 
	std::vector<std::string> m_DbNameList;
 
};

#endif // !defined(AFX_DBSHRINK_H__ED8D4ED3_CFB3_4117_A506_125E1B75E30A__INCLUDED_)
