/***********************************************************************
 * PETObjectAttributes.h
 *---------------------------------------------------------------------
 *		Copyright, PreXion 2012, All rights reserved.
 *
 *	PURPOSE:
 *		Structure that holds all PET SUV related information
 *
 *	
 *
 *-------------------------------------------------------------------
 */

#ifndef PETOBJECTATTRIBUTES_H_
#define PETOBJECTATTRIBUTES_H_

#pragma warning (disable: 4786)
#pragma warning (disable: 4503)

#include <string>

class CPxDicomImage;
class PETObjectAttributes
{
public:
	PETObjectAttributes()
	{
		Reset();
	}
	
	void Reset(void);

	static double GetCorrectionFactor(std::string& iStart, std::string& iAcq, int iHalfLife);
	
	void	Populate(CPxDicomImage*);
	
public:	
	double		m_decayFactor;
	double		m_patientWeight;
	int			m_halfLife;
	int			m_totalDose;
	std::string	m_units;
	std::string	m_acquisitionTime;
	std::string	m_startTime;
};

#endif