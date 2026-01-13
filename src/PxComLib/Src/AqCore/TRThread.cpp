// -*- C++ -*-
// Copyright 2006 PreXion 
// ALL RIGHTS RESERVED
//
// UNPUBLISHED -- Rights reserved under the copyright laws of the United
// States.   Use of a copyright notice is precautionary only and does not
// imply publication or disclosure.
//
// THE CONTENT OF THIS WORK CONTAINS CONFIDENTIAL AND PROPRIETARY
// INFORMATION OF TERARECON, INC. ANY DUPLICATION, MODIFICATION,
// DISTRIBUTION, OR DISCLOSURE IN ANY FORM, IN WHOLE, OR IN PART,
// IS STRICTLY PROHIBITED WITHOUT THE PRIOR EXPRESS WRITTEN
// PERMISSION OF TERARECON, INC.
//
// Filename:	TRThread.cpp
// Author:		David Guigonis
// Created:		Friday, March 24, 2006 at 11:40:42 AM
//

#include "TRThread.h"
#include "TRThreadJob.h"
#include <process.h>
#include <errno.h>

#ifdef WIN32
#  ifndef _WIN32_WINNT
#    define _WIN32_WINNT 0x0400
#  endif
#  include <windows.h>
#else
#  error Thread: Only Win32 targets supported!
#endif


TRThread::TRThread(PTHREAD_BEGIN_ROUTINE thread_routine,
			   void* routine_parameter,
			   bool detached )
:
    thread_routine_( thread_routine ),
    thread_routine_param_(routine_parameter),
    stopped_(false),
    exit_code_(0)
{
      handle_.thrhd = NULL;
      handle_.thrid = 0;
}

unsigned __stdcall thread_routine(void* param)
{
	TRJobBase * worker = static_cast<TRJobBase*>(param);

	worker->execute();

	return 0;
}

TRThread::TRThread(TRJobBase * job)
:
    thread_routine_(thread_routine),
	thread_routine_param_(job),
	stopped_(false),
	exit_code_(0)
{
	handle_.thrhd = NULL;
	handle_.thrid = 0;
}

TRThread::~TRThread()
{
    try 
    {
        join();
    }
    catch(...)
    {
    }

}

// start the thread.
bool
TRThread::run()
{    
	handle_.thrhd = (HANDLE)_beginthreadex(NULL, 0, thread_routine_, thread_routine_param_, 0, &handle_.thrid);
    if( handle_.thrhd == 0 )
        return false;

    return true;
}

// block the calling thread until this thread ends
void
TRThread::join()
{
    if( handle_.thrhd == 0 )
		return;

	if( !stopped_ ) {                
		::WaitForSingleObject( handle_.thrhd, INFINITE );
	}
	else {
		if( WaitForSingleObject( handle_.thrhd, 10000 ) == WAIT_TIMEOUT )
		{
			::TerminateThread( handle_.thrhd, 0 );
		}
	}
	
	DWORD exit_code = 0;
	::GetExitCodeThread( handle_.thrhd, &exit_code );
	exit_code_ = unsigned(exit_code);
	
	::CloseHandle( handle_.thrhd );
	handle_.thrhd = NULL;
	handle_.thrid = 0;
}

// yield
void
TRThread::yield()
{
	::Sleep(0);
}

// is the thread dead?
bool
TRThread::isDead() const
{
    bool dead = true;
    if( handle_.thrhd == NULL )
		return dead;
	
	DWORD exitCode;
	if( ::GetExitCodeThread(handle_.thrhd, &exitCode) )
	{
		if( exitCode == STILL_ACTIVE )
		{
			dead = false;
		}
	}
    return dead;
}

// stop the thread
void
TRThread::stop()
{
    stopped_ = true;
}

// set thread priority
bool
TRThread::setPriority_(const ThreadHandle& handle, Priority p)
{
	if( handle.thrhd == NULL )
		return false;
 
	int priority = 0;
	switch( p ) {
		case AboveNormal:  priority = THREAD_PRIORITY_ABOVE_NORMAL; break;
		case BelowNormal:  priority = THREAD_PRIORITY_BELOW_NORMAL; break;
		case Highest:      priority = THREAD_PRIORITY_HIGHEST; break;
		case Idle:         priority = THREAD_PRIORITY_IDLE; break;
		case Lowest:       priority = THREAD_PRIORITY_LOWEST; break;
		case TimeCritical: priority = THREAD_PRIORITY_TIME_CRITICAL; break;
		case Normal:
		case Unknown:
		default:           priority = THREAD_PRIORITY_NORMAL; break;
	}

	BOOL rv = ::SetThreadPriority(handle.thrhd, priority);
	if( rv == 0 ) return false;

	return true;
}

bool
TRThread::setPriority(Priority p)
{
	return setPriority_(handle_, p);
}

// get thread priority
TRThread::Priority
TRThread::getPriority_(const ThreadHandle& handle)
{
	if( handle.thrhd == NULL )
		return Unknown;

	int rv = GetThreadPriority(handle.thrhd);

	switch( rv ) {
		case THREAD_PRIORITY_ABOVE_NORMAL:  return AboveNormal;
		case THREAD_PRIORITY_BELOW_NORMAL:  return BelowNormal;
		case THREAD_PRIORITY_HIGHEST:       return Highest;
		case THREAD_PRIORITY_IDLE:          return Idle;
		case THREAD_PRIORITY_LOWEST:        return Lowest;
		case THREAD_PRIORITY_NORMAL:        return Normal;
		case THREAD_PRIORITY_TIME_CRITICAL: return TimeCritical;
		case THREAD_PRIORITY_ERROR_RETURN:
		default:                            return Unknown;
	}
}

TRThread::Priority
TRThread::getPriority() const
{
	return getPriority_(handle_);
}

// set thread affinity (on which CPUs will it run)
// examples: setAffinity( Cpu0 | Cpu1 ); thread runs on processor 0 and 1 only.
//           setAffinity( NoPreferred ); thread has no preferred processor.
bool
TRThread::setAffinity_(const ThreadHandle& handle, unsigned long affinityMask, unsigned long* previousMask /* = 0 */)
{
	if( handle.thrhd == NULL )
		return false;

	unsigned long prev = 0;
	if( affinityMask == 0 ) {
		prev = ::SetThreadIdealProcessor(handle.thrhd, MAXIMUM_PROCESSORS);
		if( prev == (DWORD)-1 )
			return false;
	}
	else {
		prev = ::SetThreadAffinityMask(handle.thrhd, affinityMask);
		if( prev == 0 )
			return false;
	}

	if( previousMask )
		*previousMask = prev;

	return true;
}

bool
TRThread::setAffinity(unsigned long affinityMask, unsigned long* previousMask /* = 0 */)
{
	return setAffinity_(handle_, affinityMask, previousMask);
}

// set current thread affinity
bool
TRThread::setCurrentThreadAffinity(unsigned long affinityMask, unsigned long* previousMask /* = 0 */)
{
	ThreadHandle handle;
	handle.thrhd = ::GetCurrentThread();

	return setAffinity_(handle, affinityMask, previousMask);
}