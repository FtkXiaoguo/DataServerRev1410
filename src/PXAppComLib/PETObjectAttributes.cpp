/***********************************************************************
 * PETObjectAttributes.cpp
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

#include "PETObjectAttributes.h"
#include "PxDicomimage.h"
#include <math.h>
#include <stdlib.h>

//----------------------------------------------------------
void PETObjectAttributes::Populate(CPxDicomImage *image)
{
	m_decayFactor		= image->GetPETDecayFactor();
	m_totalDose			= image->GetRadionuclideTotalDose();
	m_halfLife			= image->GetRadionuclideHalfLife();
	m_acquisitionTime	= image->GetAcquisitionTime();
	m_startTime			= image->GetRadiopharmaceuticalStartTime();
	m_units				= image->GetPETUnits();		
	m_patientWeight		= image->GetPatientWeight();
};

//----------------------------------------------------------
void PETObjectAttributes::Reset(void)
{
	m_halfLife = 0.0;
	m_totalDose = 0;
	m_decayFactor = 0.0;
	m_patientWeight = 0.0;

	m_units = "";
	m_acquisitionTime = "";
	m_startTime = "";
}

//----------------------------------------------------------
static int ConvertHHMMSSToSeconds(char iHHMMSS[])
{
	int s = 0;

	s = atoi(iHHMMSS+4);
	iHHMMSS[4] = '\0';
	s += atoi(iHHMMSS+2)* 60;
	iHHMMSS[2] = '\0';
	s += atoi(iHHMMSS)*3600;

	return s;
}

// This is by no means fool proof. This has a lot of assumptions with no
// error checking at all. Need to beef this up after MDCT'06
//----------------------------------------------------------
double PETObjectAttributes::GetCorrectionFactor(std::string& iStart, 
												std::string& iAcq, int iHalfLife)
{
	static double factor = 0.69314718;
	char startstr[32], acqstr[32], *p;

	if (iStart.empty() || iAcq.empty() || iHalfLife <= 0)
		return -1.0;
	
	strncpy(startstr, iStart.c_str(), sizeof startstr);
	strncpy(acqstr, iAcq.c_str(), sizeof acqstr);
	startstr[sizeof startstr - 1] = '\0';
	acqstr[sizeof acqstr - 1] = '\0';

	if (( p = strrchr(startstr,'.')))
		*p = '\0';
	if (( p = strrchr(acqstr,'.')))
		*p = '\0';

	if (strlen(acqstr) != 6 || strlen(startstr) != 6)
	{
//		assert(strlen(acqstr) == 6);
//		assert(strlen(startstr) == 6);
		return -1.0;
	}

	int start = ConvertHHMMSSToSeconds(startstr);
	int acq   = ConvertHHMMSSToSeconds(acqstr);

	if (acq < start)
	{
//		assert(acq >= start);
		return -1.0;
	}

	return exp(factor*(acq-start)/double(iHalfLife));
}