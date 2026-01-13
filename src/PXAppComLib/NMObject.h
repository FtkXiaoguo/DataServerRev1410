/***********************************************************************
 * NMObject.h
 *---------------------------------------------------------------------
 *		Copyright, PreXion 2004, All rights reserved.
 *
 *	PURPOSE:
 *		Processes CStore Requests
 *
 *	
 *
 *-------------------------------------------------------------------
 */
#ifndef C_NM_OBJECT_H
#define C_NM_OBJECT_H

#include "PxDicomImage.h"

//-----------------------------------------------------------------------------
//	DICOM PS3.3 - C.8.4.12
class CNMRotation
{
public:
	CNMRotation(int isqID) : m_sqID(isqID) { Init(); m_status = Populate(); }
	virtual ~CNMRotation() {}
	int GetStatus(void) const { return m_status; }

	void Init();
	int Populate();

	double GetStartAngle(void) const { return m_startAngle; }
	double GetAngularStep(void) const { return m_angularStep; }
	int    GetRotationDirection(void) const { return m_rotationDirection; }
	int	   GetScanArc(void) const { return m_scanArc; }
	int    GetFrameDuration(void) const { return m_frameDuration; };
	int    GetNumberOfFramesInRotation(void) const { return m_numberOfFramesInRotation; };

private:
	int m_sqID;
	int m_status;

	double m_startAngle;			// 1C
	double m_angularStep;			// 1C
	int m_scanArc;					// 1C
	int m_frameDuration;			// 1C
	int m_numberOfFramesInRotation;	// 1C
	int	m_rotationDirection;		// 1C
};


//-----------------------------------------------------------------------------
//	DICOM PS-3.3 C.8.4.11
class CNMDetector
{
public:
	CNMDetector(int isqID) : m_sqID(isqID) { Init(); Populate(); }
	virtual ~CNMDetector() {}
	int GetStatus(void) const { return m_status; }

	void Init();
	void Populate();

	void GetImagePosition(double out[3]);
	void GetImageOrientation(double out[6]);

	char* GetCollimatorType(void) { return m_collimatorType; } 
	int   GetFocalDistance(void) const { return m_focalDistance; }
	int   GetDistanceSourceToDetector(void) const { return m_distSourceToDetector; }

	bool  HasValidImagePosition(void) const { return m_hasValidImagePosition; }
	bool  HasValidImageOrientation(void) const 
	{ 
		return m_hasValidImageOrientation; 
	}

private:
	int m_sqID;
	int m_status;

	double m_imagePosition[3];		// 2C
	double m_imageOrientation[6];	// 2C
	char   m_collimatorType[8];		// 2C
	int    m_focalDistance;			// 2C
	int	   m_distSourceToDetector;	// 2C

	bool   m_hasValidImagePosition;
	bool   m_hasValidImageOrientation;
};

//-----------------------------------------------------------------------------
// 
class CNMObject
{
public:

	enum
	{
		kNMSuccess = 0,
		kNMBadInputImage = -1,
		kNMBadImageType = -2
	};

	enum
	{
		kNMInvalidNMType = -1,
		kNMStatic = 0,
		kNMDynamic,
		kNMGated,
		kNMWholeBody,
		kNMTomo,
		kNMGatedTomo,
		kNMReconTomo,
		kNMReconGatedTomo,
	};

	enum
	{
		kNMUnknown = -1,
		kNMEmission = 0,
		kNMTransmission = 1
	};

	enum 
	{
		kNMUnknownOrientation = -1,
		kNMHeadFirst = 0,
		kNMFeetFirst,
		kNMProne,
		kNMSupine,
		kNMDecubitusLeft,
		kNMDecubitusRight
	};

	CNMObject(CPxDicomImage* iImage);
	virtual ~CNMObject() {}
	int GetStatus(void) const { return m_status; }

	int GetDetectorIndex		(int iFrameNumber) { return GetVectorIndex(iFrameNumber, m_detectorVector);}
	int GetRotationIndex		(int iFrameNumber) { return GetVectorIndex(iFrameNumber, m_rotationVector);}
	int GetEnergyWindowIndex	(int iFrameNumber) { return GetVectorIndex(iFrameNumber, m_energyWindowVector);}
	int GetPhaseIndex			(int iFrameNumber) { return GetVectorIndex(iFrameNumber, m_phaseVector);}
	int GetRRIntervalIndex		(int iFrameNumber) { return GetVectorIndex(iFrameNumber, m_rrIntervalVector);}
	int GetTimeSlotIndex		(int iFrameNumber) { return GetVectorIndex(iFrameNumber, m_timeSlotVector);}
	int GetSliceIndex			(int iFrameNumber) { return GetVectorIndex(iFrameNumber, m_sliceVector);}
	int GetAngularViewIndex		(int iFrameNumber) { return GetVectorIndex(iFrameNumber, m_angularViewVector);}
	int GetTimeSliceIndex		(int iFrameNumber) { return GetVectorIndex(iFrameNumber, m_timeSliceVector);}

	double GetSpacingBetweenSlices(void) const { return m_spacingBetweenSlices; }
	double GetSliceThickness(void) const { return m_sliceThickness; }

	bool GetImageOrientation(int iFrameNumber, double oOrientation[6]);
	bool GetImagePosition(int iFrameNumber, double oPosition[3]);

	int GetPatientGantryRelationshipCode(void) const { return m_patientGantryRelationshipCode; }
	int GetPatientOrientationCode(void) const { return m_patientOrientationCode; }

	int GetNumberOfDetectors(void) const { return m_detectors.size(); }
	int GetNumberOfRotations(void) const { return m_rotations.size(); }
	int GetNumberOfEnergyWindows(void) const { return m_numberOfEnergyWindows; }
	int GetNumberOfPhases(void) const { return m_numberOfPhases;  }
	int GetNumberOfRRIntervals(void) const { return m_numberOfRRIntervals; }
	int GetNumberOfTimeSlots(void) const { return m_numberOfTimeSlots; }
	int GetNumberOfSlices(void) const { return m_numberOfSlices; }

	//-----------------------------------------------------------------------------
	//	
	bool CalculateImageOrientationAndPosition(int iFrameNumber, double oImagePosition[3], double oImageOrientation[6]);
	bool CalculateRotational(int iFrameNumber, double oImagePosition[3], double oImageOrientation[6]);
	bool CalculateParallel(int iFrameNumber, double oImagePosition[3], double oImageOrientation[6]);

	bool GetIsRotational(void) const { return m_isRotational; }

	std::vector<CNMDetector> GetDetectors(void) const { return m_detectors; }
	std::vector<CNMRotation> GetRotations(void) const { return m_rotations; }
private:

	void Init(void);
	void ExtractFrameIndexVector(unsigned long iTag, std::vector<int>& oVector);
	void ExtractFrameIndexVectors(void);
	void ExtractPatientOrientationCodeSequence(void);
	void Populate(void);
	void ExtractNMType(void);
	int  GetVectorIndex(int iFrameNumber, std::vector<int>& iVector) { //K.Ko Debug
	//	assert(0);
		return 0;
	};
	void GetInitialPosition(void);
	void GetInitialOrientation(void);

	int m_status;

	CPxDicomImage* m_pImage;

	int m_msgID;
	
	int m_type;
	int m_emissionOrTransmission;
	bool m_isRotational;

	std::vector<CNMDetector> m_detectors;
	std::vector<CNMRotation> m_rotations;

	int m_numberOfDetectors;
	int m_numberOfRotations;
	int m_numberOfEnergyWindows;
	int m_numberOfPhases;
	int m_numberOfRRIntervals;
	int m_numberOfTimeSlots;
	int m_numberOfSlices;
	int m_numberOfAngularViews;
	int m_numberOfTimeSlices;

	std::vector<int> m_detectorVector;
	std::vector<int> m_rotationVector;
	std::vector<int> m_energyWindowVector;
	std::vector<int> m_phaseVector;
	std::vector<int> m_rrIntervalVector;
	std::vector<int> m_timeSlotVector;
	std::vector<int> m_sliceVector;
	std::vector<int> m_angularViewVector;
	std::vector<int> m_timeSliceVector;
	
	double m_spacingBetweenSlices;	//	Type 2
	double m_sliceThickness;		//	Type 2

	int m_patientOrientationCode;
	int m_patientGantryRelationshipCode;

	double m_initialImagePosition[3];
	double m_initialImageOrientation[6];
	bool   m_hasValidInitialImagePosition;
	bool   m_hasValidInitialImageOrientation;

	// tc zhao 2005.02.10
	// fallback stuff for the Hitachi PET machine fix
public:
	static int	m_sFlipSliceSpacing;
	// tc zhao 2005.05.19
	static int	m_sAddInitialOrientation;
};

#endif // C_NM_OBJECT_H
