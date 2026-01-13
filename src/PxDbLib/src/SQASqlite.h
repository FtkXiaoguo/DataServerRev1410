/***********************************************************************
 *-------------------------------------------------------------------
 */

#ifndef	__SQASQLITE_h__
#define	__SQASQLITE_h__

#include "SQAComInf.h"

class AQDataSetSqlite;
class  SQASqlite : public SQAComInf
{
 
public:
 
	SQASqlite( ) ;  
	~SQASqlite() ;
	virtual void Reset() ;
	virtual int	GetRecordCount() ;
	virtual int SetIndex(int iInd) ;
	virtual int GetIndex() ;
	virtual int GetFieldCount() ;

	virtual int GetRecordPostion() ;
	
	virtual int	MoveFirst() ;
	virtual int	MoveLast() ;
	virtual int	MovePrevious() ;
	virtual int	MoveNext() ;
	virtual int	Move(long iNumber) ;
    
	virtual int NextRecordset() ;

	virtual void	SkipData() ;
//	virtual const char* getDataS(int index=-1) ;
	virtual const wchar_t* getDataUS(int index=-1) ;
//	virtual void	setDataS(char* iTarget, int iSize, int index=-1) ;
//	virtual void	setDataS(wchar_t* iTarget, int iSize, int index=-1) ;
	
	virtual int     getDataInt(int index=-1) ;
	virtual void	getDataInt(char* oNum, int size, int index=-1) ;
	virtual double	getDataD(int index=-1) ;
	virtual double  getDataDate(int index=-1) ;
	virtual int		getDataST(AqUString& oName, AqUString& oValue, int index=-1) ;
	virtual __int64 getDataInt64(int index=-1) ;
	

	virtual int SetCommandText(const wchar_t* iCmd) ;
	virtual int SetCommandText(const char* iCmd) ;
//	virtual int FormatCommandText(const wchar_t* fmt, ...) ;
//	virtual int FormatCommandText(const char* fmt, ...) ;
	virtual const wchar_t* GetCommandText() ;
	virtual const wchar_t* GetParamtersString() ;

	//2012/04/17 K.Ko
	virtual int GetParamtersCount() ; 
	virtual AqUString GetParamtersString(int index) ;

	virtual int AddParameter(const char* iValue, const char* iName=0, bool output=false) ;
	virtual int AddParameter(const wchar_t* iValue, const char* iName=0, bool output=false) ;

	virtual int AddParameter(long iValue, const char* iName=0, bool output=false) ;
	virtual int AddParameter(int iValue, const char* iName=0, bool output=false)  {return AddParameter((long)iValue, iName, output) ;}
	virtual int AddParameter(char iValue, const char* iName=0, bool output=false) ;


	virtual int AddParameter(short iValue, const char* iName=0, bool output=false) ;
	virtual int AddParameter(double iValue, const char* iName=0, bool output=false ) ;
	virtual int AddParameter(float iValue, const char* iName=0, bool output=false ) ;
	virtual int AddParameter(__int64 iValue, const char* iName=0, bool output=false) ;


	virtual int SetParameter(const wchar_t* iValue, long iIdx, const char* iName=0) ;
	virtual int SetParameter(const char* iValue, long iIdx, const char* iName=0) ;

	virtual int SetParameter(long iValue, long iIdx, const char* iName=0) ;
	virtual int SetParameter(int iValue, long iIdx, const char* iName=0) {return SetParameter((long)iValue, iIdx, iName) ;}
	virtual int SetParameter(char iValue, long iIdx, const char* iName=0) ;
	virtual int SetParameter(short iValue, long iIdx, const char* iName=0) ;
	virtual int SetParameter(double iValue, long iIdx, const char* iName=0) ;
	virtual int SetParameter(float iValue, long iIdx, const char* iName=0) ;
	virtual int SetParameter(__int64 iValue, long iIdx, const char* iName=0) ;
 
	virtual int ExecuteBegin(SQAParam * param) ;
	virtual int ExecuteProcBegin(SQAParam * param) ; //2012/04/17 K.Ko
protected:
	std::string getStringData(int index);
	int		m_cancelFlag;
	AQDataSetSqlite *m_dataset;
	//
	AqUString	m_cUStr;
	AqUString	m_Parameter;
} ;


#endif //__SQASQLITE_h__