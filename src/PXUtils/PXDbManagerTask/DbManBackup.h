// DbManBackup.h: CDbManBackup クラスのインターフェイス
//
//////////////////////////////////////////////////////////////////////

#if !defined(AFX_DBMANBACKUP_H__ED8D4ED3_CFB3_4117_A506_125E1B75E30A__INCLUDED_)
#define AFX_DBMANBACKUP_H__ED8D4ED3_CFB3_4117_A506_125E1B75E30A__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#include "DbManBase.h"
#include <vector>
class CDbManBackup : public CDbManBase  
{
#define BACKUP_GENERATIONS (8)
public:
	CDbManBackup();
	virtual ~CDbManBackup();

	void addDbName(std::string DbName);
	void setBackupDestFolder(std::string folder) { m_backupDestFolder = folder;};
	virtual bool doDbManTask();

	void setBackupEmptyStudy(bool backup) { m_backupEmptyStudy = backup;};
virtual int doMain(int argc, char** argv) ;
virtual int initMain(int argc, char** argv) ;
protected:

	bool backDB(const std::string dbName,const std::string backName);
	virtual bool loadConfiguration(const char *fileName);
	void ParseCommandLine(int argc, char** argv);

	std::string getBackupDbName(const std::string dbName);

	bool backupOneDb(const std::string dbName);
	bool renameFileName(const std::string srcFileName, const std::string destFileName);
	bool shiftGenerationFile(const std::string srcFileName,int genNum=0);

	bool needBackupGeneration(const std::string backupfile,int ageDays);
	std::vector<std::string> m_DbNameList;
	std::string m_backupDestFolder;

	int m_BackupGenerationNumber;
	int m_BackupGenerationAgeDays[BACKUP_GENERATIONS];

	bool m_backupEmptyStudy; //初期にDBもバックアップするように 2011/04/13 K.Ko
};

#endif // !defined(AFX_DBMANBACKUP_H__ED8D4ED3_CFB3_4117_A506_125E1B75E30A__INCLUDED_)
