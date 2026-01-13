/***********************************************************************
 * DBCore.cpp
 *---------------------------------------------------------------------
 *
 *-------------------------------------------------------------------
 */
 
#ifndef	__SQADataADO_h__
#define	__SQADataADO_h__

#include "SQAData.h"

class _SQADataADO : public _SQAData
{
public:
	_SQADataADO() : m_currentIndex(-1),  m_recordPostion(-1)
	{
		// To save unnecessary com object creation cost and do error hadling for it,
		// we are not create instances here, InitConnectionPtr, InitRecordsetPtr, 
		// and PreProcCall will do it.
		// mark all com pointer as unintitialized
		m_pConnection = 0;
		m_bConnection = false;
		
		m_pRecordset = 0;
		m_bRecordset = false;

		m_pCommand = 0;
		m_bCommand = false;
		
	};
	
	virtual ~_SQADataADO();

	virtual int InitConnectionPtr();
	virtual int InitRecordsetPtr();
	virtual int InitCommandPtr();

	virtual int Connect(const wchar_t* iConStr, bool logErr=true);
	virtual int Commit( bool iCommit);
	virtual int NewTrans();
	virtual int Disconnect(bool iCommit);

	 
 
///
#if 1
	long			m_currentIndex;
//	int				m_transBegin;

	int				m_recordPostion;

	_ConnectionPtr	m_pConnection;
	bool			m_bConnection;

	_RecordsetPtr	m_pRecordset;
	bool			m_bRecordset;

	_CommandPtr		m_pCommand;
	bool			m_bCommand;
#endif

};

#endif //__SQADataADO_h__