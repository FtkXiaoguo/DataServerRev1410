/***********************************************************************
 * $Id: rtvsfileGuard.h 35 2008-08-06 02:57:21Z atsushi $
 *---------------------------------------------------------------------
 *		Copyright, TeraRecon,Inc., 2000-2001, All rights reserved.
 *
 *	PURPOSE:
 *		Make sure there are no file leaks
 *
 *	AUTHOR(S):  T.C. Zhao, Sept 2002
 * 
 *-------------------------------------------------------------------
 */

#ifndef RTVSFILEGUARD_H_
#define RTVSFILEGUARD_H_

// use prgma to shut up microsoft template header warnings
#pragma warning (disable: 4786)
#pragma warning (disable: 4616)

#include <string>
#include <vector>

class iRTVSFileGuard
{
public:
	iRTVSFileGuard();
	~iRTVSFileGuard();
	void	AddFile(std::string& iFileName)		{ m_files.push_back(iFileName);}
	void	AddFile(const char* iFileName)		{ m_files.push_back(iFileName);}
	void	Remove(void);
	void	AddOpenFile(void *fp)				{ if (fp) m_fp.push_back(fp);}
	void	Close(void);
	void	AddDirectory(std::string& iDir)		{ m_dirs.push_back(iDir);}
	void	AddOpenPipe(void *fp)				{ if (fp) m_pipe.push_back(fp);}
	void	CloseOpenFiles(void);
	void	CloseOpenPipes(void);
	void	AddHandle(void *handle)				{ if (handle) m_handles.push_back(handle);}
	void	CloseOpenHandles(void);
private:
	std::vector<std::string>	m_files;
	std::vector<void*		>	m_fp;
	std::vector<std::string>	m_dirs;
	std::vector<void*		>	m_pipe;
	std::vector<void*		>	m_handles;
};


#endif