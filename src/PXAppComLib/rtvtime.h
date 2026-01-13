/***********************************************************************
 * rtvtime.h
 *---------------------------------------------------------------------
 *
 *   
 *-------------------------------------------------------------------
 */

#ifndef RTVTIME_H_
#define RTVTIME_H_

#if defined(_WIN32)
#include <windows.h>
#else
#include <sys/time.h>
#endif

class RTVTime 
{
public:
	RTVTime(int iOffsets=0);
	int			GetTime(void)  const ; // in sec
	inline int	GetmTime(void) const { return GetSysmTime();} // in msec
	inline int	Elapsedm(void) const { return (GetmTime() - m_start);} // in msec
	void		Reset(void);
	int			Elapsed(void) const  { return GetTime() - GetStartTime();}
	int			GetStartTime(void) const;	// in seconds
	void		SetElapsed(int iSeconds) ; // changes the start time
private:
	int			m_start;		// milli-seconds	
	int			m_offset;		// in seconds
	int			GetSysmTime(void) const; 
};

//-----------------------------------------------------------------------------
//
//	Subtracts two dates and returns the result in days
//
class RTVSystemTime
{	
public:
	RTVSystemTime(SYSTEMTIME iTime) : m_time(iTime) {}
	~RTVSystemTime() {}

	const SYSTEMTIME* GetTime() const { return &m_time; }

	// return the difference in days
	int operator -(const RTVSystemTime& iTime)
	{
		FILETIME fta, ftb;
		SystemTimeToFileTime(&m_time, &fta);
		SystemTimeToFileTime(iTime.GetTime(), &ftb);

		//	Reinterpret cast for easy subtraction
		__int64 *a = (__int64 *)&fta;
		__int64 *b = (__int64 *)&ftb;

		//	difference in 100 nanosecond ticks
		__int64 diff = *a - *b; 

		//	convert to days
		__int64 seconds = diff / (__int64) 10000000;
		return (int) ((int)seconds / (int)86400);
	}

private:
	const SYSTEMTIME m_time;
};

#endif