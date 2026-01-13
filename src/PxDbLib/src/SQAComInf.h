/***********************************************************************
 *-------------------------------------------------------------------
 */
 
#ifndef	__SQAComInf_h__
#define	__SQAComInf_h__

#include "AqCore/AqString.h"



class _SQAData;

class SQAParam;
class  SQAComInf
{
 
public:
 
	virtual void Reset() =0 ;
	virtual int	GetRecordCount() =0 ;
	virtual int SetIndex(int iInd) =0 ;
	virtual int GetIndex() =0 ;
	virtual int GetFieldCount() =0 ;

	virtual int GetRecordPostion() =0 ;
	
	virtual int	MoveFirst() =0 ;
	virtual int	MoveLast() =0 ;
	virtual int	MovePrevious() =0 ;
	virtual int	MoveNext() =0 ;
	virtual int	Move(long iNumber) =0 ;
    
	virtual int NextRecordset() =0 ;

	virtual void	SkipData() =0 ;
//	virtual const char* getDataS(int index=-1) =0 ;
	virtual const wchar_t* getDataUS(int index=-1) =0 ;
//	virtual void	setDataS(char* iTarget, int iSize, int index=-1) =0 ;
//	virtual void	setDataS(wchar_t* iTarget, int iSize, int index=-1) =0 ;
	
	virtual int     getDataInt(int index=-1) =0 ;
	virtual void	getDataInt(char* oNum, int size, int index=-1) =0 ;
	virtual double	getDataD(int index=-1) =0 ;
	virtual double  getDataDate(int index=-1) =0 ;
	virtual int		getDataST(AqUString& oName, AqUString& oValue, int index=-1) =0 ;
	virtual __int64 getDataInt64(int index=-1) =0 ;
	

	virtual int SetCommandText(const wchar_t* iCmd) =0 ;
	virtual int SetCommandText(const char* iCmd) =0 ;
//	virtual int FormatCommandText(const wchar_t* fmt, ...) =0 ;
//	virtual int FormatCommandText(const char* fmt, ...) =0 ;
	virtual const wchar_t* GetCommandText() =0 ;
	virtual const wchar_t* GetParamtersString() =0 ;

	//2012/04/17 K.Ko
	virtual int GetParamtersCount()  =0 ; 
	virtual AqUString GetParamtersString(int index) =0 ;
	/////

	virtual int AddParameter(const char* iValue, const char* iName=0, bool output=false) =0 ;
	virtual int AddParameter(const wchar_t* iValue, const char* iName=0, bool output=false) =0 ;

	virtual int AddParameter(long iValue, const char* iName=0, bool output=false) =0 ;
	virtual int AddParameter(int iValue, const char* iName=0, bool output=false)  =0 ;//{return AddParameter((long)iValue, iName, output) =0 ;}
	virtual int AddParameter(char iValue, const char* iName=0, bool output=false) =0 ;


	virtual int AddParameter(short iValue, const char* iName=0, bool output=false) =0 ;
	virtual int AddParameter(double iValue, const char* iName=0, bool output=false ) =0 ;
	virtual int AddParameter(float iValue, const char* iName=0, bool output=false ) =0 ;
	virtual int AddParameter(__int64 iValue, const char* iName=0, bool output=false) =0 ;


	virtual int SetParameter(const wchar_t* iValue, long iIdx, const char* iName=0) =0 ;
	virtual int SetParameter(const char* iValue, long iIdx, const char* iName=0) =0 ;

	virtual int SetParameter(long iValue, long iIdx, const char* iName=0) =0 ;
	virtual int SetParameter(int iValue, long iIdx, const char* iName=0) =0 ;//{return SetParameter((long)iValue, iIdx, iName) =0 ;}
	virtual int SetParameter(char iValue, long iIdx, const char* iName=0) =0 ;
	virtual int SetParameter(short iValue, long iIdx, const char* iName=0) =0 ;
	virtual int SetParameter(double iValue, long iIdx, const char* iName=0) =0 ;
	virtual int SetParameter(float iValue, long iIdx, const char* iName=0) =0 ;
	virtual int SetParameter(__int64 iValue, long iIdx, const char* iName=0) =0 ;

	///
	virtual int ExecuteBegin(SQAParam * param) =0 ;
	virtual int ExecuteProcBegin(SQAParam * param) = 0; //2012/04/17 K.Ko
	///
	_SQAData *getSQAData(){ return _pData;} ;
	int	getOptions(){ return m_options;};
	void setOptions(int opt){ m_options=opt;};
	int getCommandTimeout(){ return m_commandTimeout;} ;
	void setCommandTimeout(int timeout){ m_commandTimeout=timeout;};
protected:
	int	m_options;
	int m_commandTimeout;

	AqUString	m_cUStr;
	AqString	m_cStr;
 	_SQAData* _pData;
} ;

#endif //__SQAComInf_h__