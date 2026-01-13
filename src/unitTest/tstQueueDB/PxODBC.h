#pragma once

class CPxODBC
{
public:
	CPxODBC(void);
	~CPxODBC(void);

	bool opendDB(const char* datasouce,const char*username=0,const char*pasword=0);
	bool closeDB();
	void command_select();

	void command_execProc();

	void doTest();
protected:
	void dumpError(void);
	void *m_odbcCnt;
};
