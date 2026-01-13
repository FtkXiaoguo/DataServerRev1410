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
// Filename:	TRThread.h
// Author:		David Guigonis
// Created:		Friday, March 24, 2006 at 11:29:36 AM
//

#ifndef _TRTHREAD_H_
#define _TRTHREAD_H_
///////////////////////////////////////////////////////////////////////////////

typedef unsigned (__stdcall *PTHREAD_BEGIN_ROUTINE)(void* args);

class TRJobBase;

//////////////////////////////////////////////////////////////////////////

class TRThread
{
public:

    TRThread(PTHREAD_BEGIN_ROUTINE thread_routine,  void* routine_parameter, bool detached );

	TRThread(TRJobBase* job);

    ~TRThread();
    
    // start the thread.
    bool run(); 

    // block the calling thread until this thread ends
    void join();

	// stop the thread
    void stop(); 

	// is the thread stopped?
    bool isStopped() const;

	// yield
    static void yield();

    // get the thread parameter 
    void* getParameter();

	// is the thread dead?
    bool isDead() const;

	// thread priority
	enum Priority {
		AboveNormal,
		BelowNormal,
		Highest,
		Idle,
		Lowest,
		Normal,
		TimeCritical,
		Unknown
	};

	// set thread priority on a running thread
	bool setPriority(Priority p);

	// set current thread priority
	static bool setCurrentThreadPriority(Priority p);

	// get thread priority
	Priority getPriority() const;

	// get current thread priority
	static Priority getCurrentThreadPriority();

	enum Affinity {
		NoPreferred = 0,
		Cpu0  = 1,      Cpu1  = 1 << 1, Cpu2  = 1 << 2, Cpu3  = 1 << 3,
		Cpu4  = 1 << 4, Cpu5  = 1 << 5, Cpu6  = 1 << 6, Cpu7  = 1 << 7,
		Cpu8  = 1 << 8, Cpu9  = 1 << 8, Cpu10 = 1 << 8, Cpu11 = 1 << 8,
		Cpu12 = 1 << 8, Cpu13 = 1 << 8, Cpu14 = 1 << 8, Cpu15 = 1 << 8
	};

	// set thread affinity (on which processors will it run) on a running thread
	// examples: setAffinity( Cpu0 | Cpu1 ); thread runs on processor 0 and 1 only.
	//           setAffinity( NoPreferred ); thread has no preferred processor.
	bool setAffinity(unsigned long affinityMask, unsigned long* previousMask = 0);

	// set current thread affinity
	static bool setCurrentThreadAffinity(unsigned long affinityMask, unsigned long* previousMask = 0);

private:
	typedef struct _Thread_Handle_ 
	{
		void*     thrhd;
		unsigned  thrid;
	} ThreadHandle;

    TRThread();

    PTHREAD_BEGIN_ROUTINE   thread_routine_;        // Start routine for the thread
    void *                  thread_routine_param_;  // Parameters for the thread routine
    ThreadHandle            handle_;
    bool                    stopped_;
    unsigned                exit_code_;

	static bool     setPriority_(const ThreadHandle& handle, Priority p);
	static Priority getPriority_(const ThreadHandle& handle);

	static bool     setAffinity_(const ThreadHandle& handle, unsigned long affinityMask, unsigned long* previousMask = 0);
};

//////////////////////////////////////////////////////////////////////////
// is the thread stopped?
inline bool
TRThread::isStopped() const
{
	return stopped_;
}

inline void*
TRThread::getParameter()
{
	return thread_routine_param_;
}

///////////////////////////////////////////////////////////////////////////////
// EOF
#endif	// _TRTHREAD_H_
