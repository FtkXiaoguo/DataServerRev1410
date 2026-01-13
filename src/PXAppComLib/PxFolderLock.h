/***********************************************************************
 * CStore.h
 *---------------------------------------------------------------------
 *	
 *
 *-------------------------------------------------------------------
 */
#ifndef C_PX_FOLDER_LOCK_H
#define C_PX_FOLDER_LOCK_H

#include "string"
#include "vector"
#include "map" 
 
class CPxFolderLocker
{
public:
	CPxFolderLocker(void);
	CPxFolderLocker(const std::string &folderName, const std::string &lock_file = ".px_lock.me");
	~CPxFolderLocker();
	void setup(const std::string &folderName, const std::string &lock_file = ".px_lock.me");
	bool lock(int try_nn = 50 /*x 200Sec*/);
	void unlock(void);
	void setupCancelFlag(bool *cancelFlag){
		m_cancelFlag = cancelFlag;
		}
		;
protected:
	bool m_lockFlag;
	bool lock_in(float time_span /*mSec*/);
	std::string m_lockFileName;

	const int m_waitInterval;//mSec
	bool *m_cancelFlag;
};
//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
#endif // C_PX_FOLDER_LOCK_H
