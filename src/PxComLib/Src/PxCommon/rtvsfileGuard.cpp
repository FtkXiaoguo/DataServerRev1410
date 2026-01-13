/***********************************************************************
 * $Id: rtvsfileGuard.cpp 35 2008-08-06 02:57:21Z atsushi $
 *---------------------------------------------------------------------
 *		Copyright, PreXion,Inc., 2000-2001, All rights reserved.
 *
 *	PURPOSE:
 *		Make sure there are no file leaks
 *
 *	
 * 
 *-------------------------------------------------------------------
 */
#include "rtvsFileGuard.h"
#include "AqCore/TRPlatform.h"

//-------------------------------------------------------------------
iRTVSFileGuard::iRTVSFileGuard(void) 
{


}

//-------------------------------------------------------------------
iRTVSFileGuard::~iRTVSFileGuard(void)
{
	Close();
	Remove();
}

//-------------------------------------------------------------------
void iRTVSFileGuard::Remove(void)
{
	try
	{
		for (int n = m_files.size(); --n >= 0; )
		{
			TRPlatform::remove(m_files[n].c_str());
		}
	}
	catch(...) {}

	try
	{
		for (int n = m_dirs.size(); --n >= 0; )
		{
			TRPlatform::RemoveDirectory(m_dirs[n].c_str());
		}
	}
	catch(...) {}

	m_dirs.clear();
	m_files.clear();
}

//-------------------------------------------------------------------
void iRTVSFileGuard::CloseOpenFiles(void)
{
	
	int i, n;
	
	for ( n = m_fp.size(), i = 0; i < n; ++i)
	{
		try
		{
			if (m_fp[i] )
			{
				if (m_fp[i] != stdout && m_fp[i] != stderr && m_fp[i] != stdin)
					fclose((FILE *)m_fp[i]);
			}
		}	
		catch(...) {}
	}
	
	m_fp.clear();
	
}

//-------------------------------------------------------------------
void iRTVSFileGuard::CloseOpenHandles(void)
{
	
	int i, n;
	
	for ( n = m_handles.size(), i = 0; i < n; ++i)
	{
		try
		{
			if (m_handles[i] )
			{
				CloseHandle(m_handles[i]);
			}
		}	
		catch(...) {}
	}
	
	m_handles.clear();
	
}

//-------------------------------------------------------------------
void iRTVSFileGuard::CloseOpenPipes(void)
{
	int i, n;

	for ( n = m_pipe.size(), i = 0; i < n; ++i)
	{
		try
		{
			if (m_pipe[i])
				_pclose((FILE *)m_pipe[i]);
		}	
		catch(...) {}
	}
	
	m_pipe.clear();
}

//-------------------------------------------------------------------
void iRTVSFileGuard::Close(void)
{
	CloseOpenFiles();
	CloseOpenPipes();
	CloseOpenHandles();
}