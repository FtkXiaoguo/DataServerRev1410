/***********************************************************************
 * rtvbase.h
 *---------------------------------------------------------------------
 *		Copyright, Mitsubishi Electric Information Technology Center 
 *					America, Inc., 2000, All rights reserved.
 *
 *	PURPOSE:
 *		Basic message/logging facility and some misc. utilities
 *      used throughout the AQNet project
 *
 *	AUTHOR:  T.C. Zhao, Sept. 2000.
 * 
 */

#ifndef RTVBASE_H_
#define RTVBASE_H_

#include "AqCore/TRCriticalsection.h"

//------------------------------------------------------

class iRTVBase
{
public:

	iRTVBase(void);
	virtual ~iRTVBase(void);

	/* set default verbosity level: iRTVSBase constructor initializes
	 * the verbosity level using sVerbose
	 */
	static void		SetDefaultVerbose(int iV) 
	{
		sVerbose = iV;
	}

	/* modify the verbosity level of this instance
	 */
	void			SetVerbose(int iFlag)
	{ 
		m_verbose = iFlag;
	}

	
	/* obtain the verbosity level of this instance
	 */
	int				GetVerbose() { return m_verbose; }

	/* output message if verbosity setting > 0
	 */
	void			Message(const char *fmt, ...);

	/* output message if verbosity setting >= 0
	 */
	void			Message0(const char *, ...);

    /* output message if verbosity setting > 1
	 */
	void			Message2(const char *, ...);

	/* output message if verbosity setting >= 0.
	 * Error message in addition will append date/time in front of the message
	 */
	void			ErrMessage(const char *, ...);

	/* always output the message regardless of setting of verbosity level
	 */
	virtual void	Echo(const char *, ...);

protected:
	int				m_verbose;
	static int		sVerbose;
	//GL 2005-9-14  too many TRCriticalSection will be wasted
	// make childer class hard to use assign operator
	static TRCriticalSection	m_stderrcs;
};

#endif
