/***********************************************************************
 * SQAData.h
 *---------------------------------------------------------------------
 *
 *-------------------------------------------------------------------
 */
 
#ifndef	__SQAData_h__
#define	__SQAData_h__

class _SQAData
{
public:
	_SQAData():m_transBegin(0)
	{
	};
	
	virtual ~_SQAData(){};

	virtual int InitConnectionPtr() = 0;
	virtual int InitRecordsetPtr() = 0;
	virtual int InitCommandPtr() = 0;

	virtual int Connect(const wchar_t* iConStr, bool logErr=true) = 0;
	virtual int Commit( bool iCommit) = 0;
	virtual int NewTrans() = 0;
	virtual int Disconnect(bool iCommit) = 0;

	///
	int				m_transBegin;
};

#endif //__SQAData_h__