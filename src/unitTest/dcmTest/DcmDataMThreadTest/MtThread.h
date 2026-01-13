// AssociationHandler.h: CAssociationHandler クラスのインターフェイス
//
//////////////////////////////////////////////////////////////////////

#if !defined(AFX_ASSOCIATIONHANDLER_H__ECC75A3E_E494_4408_BDCA_0CB42BFD4DC4__INCLUDED_)
#define AFX_ASSOCIATIONHANDLER_H__ECC75A3E_E494_4408_BDCA_0CB42BFD4DC4__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000


#include "rtvthread.h"
class TRLogger;
class CTstVLIDicomImage;
class CMtHandler : public iRTVThreadProcess  
{
public:
 
	CMtHandler( int id);
	virtual ~CMtHandler();
 
	/* the PreProcess() is called in the main thread. If Preprocess()
	* returns a negative value, the thread will NOT run Process()
	*/
	virtual  int	PreProcess(void);
	
	/* The main entry for the threaded execution. When Process() ends, the
	* thread terminates.
	*	 The Process() function must check for termination request (TerminationRequested())
	*/
	virtual  int	Process(void);
	//
	 
protected:
 
	bool readDicomMessage();
	bool readDicomImage();
	int m_count ;
	int m_id;
	//
 
 
};

#endif // !defined(AFX_ASSOCIATIONHANDLER_H__ECC75A3E_E494_4408_BDCA_0CB42BFD4DC4__INCLUDED_)
