/***********************************************************************
 * PxNetDB.h 
 *
 *-------------------------------------------------------------------
 */

#ifndef	__PX_FXDB_SQLITE_h__
#define	__PX_FXDB_SQLITE_h__

#pragma warning (disable: 4786)
#pragma warning (disable: 4503)

#include "PxNetDB.h"
 
/////////////////////////////////////////////////////////////////////

class FxDefDllCls_FxDbLib CPxDBSQLite : public CPxDcmDB
{

public:
	 
	CPxDBSQLite();
	virtual ~CPxDBSQLite();
	int	SaveDICOMData(const DICOMData& iData, int iInstanceStatus = kImageFormatInDCM) override;
};

#endif	/* __PX_FXDB_SQLITE_h__ */
