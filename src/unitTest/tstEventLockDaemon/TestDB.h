// TestDB.h: CTestDB クラスのインターフェイス
//
//////////////////////////////////////////////////////////////////////

#if !defined(AFX_TESTDB_H__E4E73C47_2805_43EF_8982_FBD6EDF0494C__INCLUDED_)
#define AFX_TESTDB_H__E4E73C47_2805_43EF_8982_FBD6EDF0494C__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

class CTestDB  
{
public:
	CTestDB();
	virtual ~CTestDB();

	static void initTestDB(bool global);

	bool waitQueueEvent();
	void addQueueEvent();
	void lockDB();
	void unlockDB();
 
};

#endif // !defined(AFX_TESTDB_H__E4E73C47_2805_43EF_8982_FBD6EDF0494C__INCLUDED_)
