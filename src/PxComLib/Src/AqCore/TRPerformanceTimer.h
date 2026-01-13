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
// Filename:	TRPerformanceTimer.h
// Author:		David Guigonis
// Created:		Thursday, April 06, 2006 at 1:52:13 PM
//

#ifndef TRPERFORMANCETIMER_H
#define TRPERFORMANCETIMER_H
///////////////////////////////////////////////////////////////////////////////
#ifndef WIN32
#  error "Please implement this class for different OS"
#else

#include <assert.h>
#include <windows.h>

class TRPerformanceTimer
{
public:
	TRPerformanceTimer();

	void start();
	void end();

	double elapse_s() const; // elapse time in seconds
	double elapse_ms() const; // elapse time in milliseconds
	double elapse_us() const; // elapse time in microseconds
	double elapse_ns() const; // elapse time in nanoseconds

protected:
	__int64 start_;
	__int64 end_;
	__int64 freq_;
};

///////////////////////////////////////////////////////////////////////////////

inline
TRPerformanceTimer::TRPerformanceTimer()
{
	QueryPerformanceFrequency((LARGE_INTEGER*)&freq_);
}

inline void
TRPerformanceTimer::start()
{
	QueryPerformanceCounter((LARGE_INTEGER*)&start_);
	end_ = start_ - 1;
}

inline void
TRPerformanceTimer::end()
{
	QueryPerformanceCounter((LARGE_INTEGER*)&end_);
}

inline double
TRPerformanceTimer::elapse_s() const
{
	assert( start_ <= end_ );

	return ( double(end_ - start_) / double(freq_) );
}

inline double
TRPerformanceTimer::elapse_ms() const
{
	return ( 1000.0 * elapse_s() );
}

inline double
TRPerformanceTimer::elapse_us() const
{
	return ( 1000000.0 * elapse_s() );
}

inline double
TRPerformanceTimer::elapse_ns() const
{
	return ( 1000000000.0 * elapse_s() );
}


#endif // WIN32

///////////////////////////////////////////////////////////////////////////////
//EOF
#endif	// TRPERFORMANCETIMER_H
