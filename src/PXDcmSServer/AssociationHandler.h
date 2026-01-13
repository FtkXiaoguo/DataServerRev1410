/***********************************************************************
 * AssociationHandler.h
 *---------------------------------------------------------------------
 *		Copyright, PreXion 2012, All rights reserved.
 *
 *	PURPOSE:
 *		Accepts images over an open DICOM association.  If the incoming
 *		images belong to a new series, a SeriesDirMonitor thread is 
 *		kicked off.
 *
 *	
 *
 *-------------------------------------------------------------------
 */
#ifndef ASSOCIATION_HANDLER_H
#define ASSOCIATION_HANDLER_H

#include <time.h>
//#include "AutoClean.h"
#include "RTVAssociationHandler.h"

class AssociationHandler : public RTVAssociationHandler
{
public:
	AssociationHandler(DiCOMConnectionInfo& connectInfo);
	~AssociationHandler();// {};

	int PreProcess(void);
	int CheckLicenseStatus(void);

	int setupThreadPriority();;/// 2010/03/15 K.Ko
protected:
	virtual void Close();
	bool ProcessMessages();
	int SendLicenseWillExpireEmail(const char* iSubj, const char* iMsg);
	int m_ThreadPriority; /// 2010/03/15 K.Ko
	//
	std::string  m_seriesUID;  //#21 2012/05/29 K.Ko
};

#endif // ASSOCIATION_HANDLER_H


