/*********************************************************************/
/*                                                                   */
/*                       msec Time Class Library                     */
/*                                                                   */
/*********************************************************************/

#ifndef __libmtime_h__
#define __libmtime_h__

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#ifdef __CYGWIN32__
#include <unistd.h>
#include <sys/time.h>
#endif

#ifdef __sun4os4__
#include <unistd.h>
#include <sys/time.h>
extern "C" int gettimeofday( struct timeval* ... );
#endif

#ifdef WIN32

#include <winsock.h>
//#include <largeint.h>
#endif

class MTickCount
{
//#define HighPart_Scale ((double)(0xFFFFFFFFFFFFFFFF))
#define HighPart_Scale (4294967296.0)
public:
	MTickCount(){ 
		QueryPerformanceFrequency( &unit );
//		d_unit = (unit.LowPart + unit.HighPart*HighPart_Scale)/1000.0; //msec
		d_unit = (unit.LowPart + unit.HighPart*HighPart_Scale)/1000000.0; //usec
 	 
		start_time = 0.0;
		start_time = getCurTickCount();
	};
	~MTickCount(){};
	inline void resetTickCount(){
		start_time = 0.0;
		start_time = getCurTickCount();
	}
	inline double getCurTickCount()
	{
		QueryPerformanceCounter( &pc );
		return((double)( (pc.LowPart + pc.HighPart*HighPart_Scale)/d_unit - start_time));
	};
protected:
	LARGE_INTEGER unit;
	LARGE_INTEGER pc;
	double d_unit;
	double start_time;
};
class MTIME

	{
	/************************************/
	/*          member variable         */
	/************************************/
	protected:
#ifdef __CYGWIN32__
		LARGE_INTEGER unit;
#endif

	public:
#ifdef WIN32
		LARGE_INTEGER unit;
		LARGE_INTEGER pc;
	
#endif

		struct timeval tm;


	/************************************/
	/*          member function         */
	/************************************/
	protected:

	public:
		MTIME( void );
		MTIME( double s );
		MTIME( const MTIME& s );

	   ~MTIME( void );

		void update( void );
		int	 	msec( void );
		double	sec( void );

		MTIME& operator  = ( const MTIME& s );
		MTIME& operator += ( const MTIME& s );
		MTIME& operator -= ( const MTIME& s );
	};


/********************************************************************/
/*                             Constructor                          */
/********************************************************************/

inline MTIME::MTIME() 
	{
#ifdef __CYGWIN32__
	QueryPerformanceFrequency( &unit );
#endif

#ifdef WIN32
	QueryPerformanceFrequency( &unit );

#endif
	}

inline MTIME::MTIME( double s ) 
	{
	this->tm.tv_sec  = int( s );
	this->tm.tv_usec = int( ( s - double( int( s ) ) )*1000000.0 + 0.5 );
	}

inline MTIME::MTIME( const MTIME& s )
	{
	this->tm = s.tm;
	}


/********************************************************************/
/*                              Destructor                          */
/********************************************************************/

inline MTIME::~MTIME() 
	{
	}


/********************************************************************/
/*                        Operator override                         */
/********************************************************************/

inline MTIME& MTIME::operator  = ( const MTIME& s )
	{
	tm = s.tm;
#ifdef WIN32
	pc = s.pc;
#endif
#ifdef __CYGWIN32__
	pc = s.pc;
#endif
	return *this;
	}

inline MTIME& MTIME::operator += ( const MTIME& s )
	{
	tm.tv_usec += s.tm.tv_usec;
	if ( tm.tv_usec>1000000 ) 
		{
		tm.tv_sec++;
		tm.tv_usec -= 1000000;
		}

	tm.tv_sec += s.tm.tv_sec;

	return *this;
	}

inline MTIME& MTIME::operator -= ( const MTIME& s )
	{
	tm.tv_usec -= s.tm.tv_usec;
	if ( tm.tv_usec<0 )
		{
		tm.tv_sec--;
		tm.tv_usec += 1000000;
		}

	tm.tv_sec -= s.tm.tv_sec;

	return *this;
	}


/********************************************************************/
/*                            member func                           */
/********************************************************************/

inline void MTIME::update( void )
	{
#ifdef __CYGWIN32__

	LARGE_INTEGER pc;
	unsigned long long l1;

	QueryPerformanceCounter( &pc );

	l1 = pc.HighPart;
	l1<<=32;
	l1 |= pc.LowPart;

	tm.tv_sec  = (int)(l1/unit.LowPart);
	tm.tv_usec = (int)(((l1%unit.LowPart)*1000000)/unit.LowPart);
#endif

#ifdef WIN32
	QueryPerformanceCounter( &pc );
#endif

#ifdef __sun4os4__
	gettimeofday( &tm, NULL );
#endif
	}

inline int MTIME::msec( void )
{

	return( tm.tv_sec*1000 + tm.tv_usec/1000 );

}

inline double MTIME::sec( void )
	{
	return double( tm.tv_sec*1000 + tm.tv_usec/1000 )/1000.0;
	}


/********************************************************************/
/*                        Operator override                         */
/********************************************************************/

inline MTIME operator + ( const MTIME& x, const MTIME& y )
	{
	MTIME t = x;

	t.tm.tv_usec += y.tm.tv_usec;
	if ( t.tm.tv_usec>1000000 ) 
		{
		t.tm.tv_sec++;
		t.tm.tv_usec -= 1000000;
		}

	t.tm.tv_sec += y.tm.tv_sec;

	return t;
	}

inline MTIME operator - ( const MTIME& x, const MTIME& y )
	{
	MTIME t = x;

	t.tm.tv_usec -= y.tm.tv_usec;
	if ( t.tm.tv_usec<0 ) 
		{
		t.tm.tv_sec--;
		t.tm.tv_usec += 1000000;
		}

	t.tm.tv_sec -= y.tm.tv_sec;

	return t;
	}

inline int operator == ( const MTIME& x, const MTIME& y )
	{
	return ( x.tm.tv_sec==y.tm.tv_sec && x.tm.tv_usec==y.tm.tv_usec );
	}

inline int operator > ( const MTIME& x, const MTIME& y )
	{
	if ( x.tm.tv_sec>y.tm.tv_sec )
		return 1;

	if ( x.tm.tv_sec<y.tm.tv_sec )
		return 0;

	return ( x.tm.tv_usec>y.tm.tv_usec );
	}

inline int operator < ( const MTIME& x, const MTIME& y )
	{
	if ( x.tm.tv_sec<y.tm.tv_sec )
		return 1;

	if ( x.tm.tv_sec>y.tm.tv_sec )
		return 0;

	return ( x.tm.tv_usec<y.tm.tv_usec );
	}

inline int operator <= ( const MTIME& x, const MTIME& y )
	{
	if ( x<y ) return 1;
	if ( x==y ) return 1;
	return 0;
	}

inline int operator >= ( const MTIME& x, const MTIME& y )
	{
	if ( x>y ) return 1;
	if ( x==y ) return 1;
	return 0;
	}

inline int operator != ( const MTIME& x, const MTIME& y )
	{
	return ( x.tm.tv_sec!=y.tm.tv_sec || x.tm.tv_usec!=y.tm.tv_usec );
	}


/********************************************************************/
/*                            utility func                          */
/********************************************************************/

inline int elapse( MTIME& s, MTIME& e )

	{
#ifndef WIN32
	MTIME t;
	t = e - s;
	return t.msec();
#else

#if 0
	LARGE_INTEGER tmp0;
	LARGE_INTEGER tmp1;

	tmp0 = LargeIntegerSubtract( e.pc, s.pc );
	tmp1 = ExtendedIntegerMultiply( tmp0, 1000 );
	tmp0 = LargeIntegerDivide( tmp1, e.unit, NULL );

	return tmp0.LowPart;
#endif

#if 1
//	unsigned int h, l;

	double fs  = float( s.pc.LowPart );
	       fs += float( s.pc.HighPart * 4294967296.0 );

	double fe  = float( e.pc.LowPart );
	       fe += float( e.pc.HighPart * 4294967296.0 );

	double fu  = float( s.unit.LowPart );
	       fu += float( s.unit.HighPart * 4294967296.0 );

	double tmp = ( fe - fs )/ fu;

	return int( tmp*1000.0 );
#endif

#endif
	}

inline int elapseus( MTIME& s, MTIME& e )

	{
	double fs  = float( s.pc.LowPart );
	       fs += float( s.pc.HighPart * 4294967296.0 );

	double fe  = float( e.pc.LowPart );
	       fe += float( e.pc.HighPart * 4294967296.0 );

	double fu  = float( s.unit.LowPart );
	       fu += float( s.unit.HighPart * 4294967296.0 );

	double tmp = ( fe - fs )/ fu;

	return int( tmp*1000000.0 );
	}


#endif
