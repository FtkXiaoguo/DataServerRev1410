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
// Filename:	TRProgressAPI.h
// Author:		David Guigonis
// Created:		Monday, May 08, 2006 at 12:58:41 PM
//

#ifndef _TRPROGRESSAPI_H_
#define _TRPROGRESSAPI_H_
//////////////////////////////////////////////////////////////////////////
#include <cassert>

//
// Based on Sha CProgressCounter in WS
//
// Sha 2006.4.28 Display progress bar for prolonged operations (ID:5593) 
//

//------------------------------------------------------------------------------
// Progress counter class that is used by engines
// See example at "AqMfcControls\TestMfcCtrls\SlowEngine.cpp"
//
class AqProgressCounter
{
public:
	// Creates a main progress
	AqProgressCounter()
	{
		Reset();
	}

	// Creates a sub-progress
	AqProgressCounter(AqProgressCounter& Parent, int iEndPercentInParent)
	{
		Reset();
		SetParent(Parent, iEndPercentInParent);
	}

	virtual ~AqProgressCounter() {};

	// Make this a sub-progress
	void SetParent(AqProgressCounter& Parent, int iEndPercentInParent)
	{
		m_pParent = &Parent;
		m_iBeginInParent = Parent.GetProgress();
		m_iEndInParent = Parent.GetEndValue() * iEndPercentInParent / 100;
	}

	// Default end value is 100 but you can change
	void SetEndValue(int iEnd) { assert(iEnd); m_iEnd = iEnd; }

	// Update the progress
	virtual int SetProgress(int iValue)
	{
		m_iCurrent = iValue;
		
		// Update the parent:
		if (m_pParent)
		{
			int iUpdate = (m_iEndInParent - m_iBeginInParent) * m_iCurrent / m_iEnd + m_iBeginInParent;
			if (iUpdate != m_pParent->GetProgress())
			{
				m_pParent->SetProgress(iUpdate);
			}
		}

		return m_iCurrent;
	}

	// Increase by 1
	int StepIt(int iIncrement = 1)
	{
		if (GetProgress() >= GetEndValue())
		{
			return GetProgress();
		}
		return SetProgress(GetProgress() + iIncrement);
	}

	// Interruption:
	virtual bool WantToCancel()  // Returns true if "Cancel" button is pressed.
	{
		if (m_pParent)
		{
			return m_pParent->WantToCancel();
		}
		return false;
	}

	// To read the progress:
	int GetProgress() const { return m_iCurrent; };
	int GetEndValue() const { return m_iEnd; };

protected:
	int m_iCurrent;  // Begins from 0.
	int m_iEnd;
	AqProgressCounter* m_pParent;  // Owned by outside.
	int m_iBeginInParent, m_iEndInParent;

	void Reset()
	{
		m_iCurrent = 0;  m_iEnd = 100;  
		m_pParent = 0;
		m_iBeginInParent = 0;  m_iEndInParent = 100;
	}
};

//------------------------------------------------------------------------------
// Base class for engines
// See example at "AqMfcControls\TestMfcCtrls\SlowEngine.h"
//
class AqProgressEngine
{
public:
	AqProgressEngine()
	{
		m_pProgressCounter = 0;
	};

	virtual ~AqProgressEngine() {}

	// Set the pointer from outside
	void SetProgressCounter(AqProgressCounter* pCounter)
	{
		m_pProgressCounter = pCounter;
	}

	// Update the progress
	void SetProgress(int iValue)
	{
		if (m_pProgressCounter) m_pProgressCounter->SetProgress(iValue);
	}

	// Increase by 1
	void StepIt(int iIncrement = 1)
	{
		if (m_pProgressCounter) m_pProgressCounter->StepIt(iIncrement);
	}

	// Attach a sub-progress
	void AttachSubProgress(AqProgressCounter& SubProgress, int iEndPercent)
	{
		if (m_pProgressCounter) SubProgress.SetParent(*m_pProgressCounter, iEndPercent);
	}

	// Interruption. Returns true if "Cancel" button is pressed.
	bool WantToCancel()
	{
		if (m_pProgressCounter) return m_pProgressCounter->WantToCancel();
		return false;
	}

protected:
	AqProgressCounter* m_pProgressCounter;
};


//////////////////////////////////////////////////////////////////////////
// EOF
#endif	// _TRPROGRESSAPI_H_
