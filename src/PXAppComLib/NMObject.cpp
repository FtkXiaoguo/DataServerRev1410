/***********************************************************************
 * NMObject.cpp
 *---------------------------------------------------------------------
 *		Copyright, TeraRecon 2004, All rights reserved.
 *
 *	PURPOSE:
 *		Abstract representation of DICOM Nuclear Medicine Object
 *
 *	AUTHOR(S):  Rob Lewis, July, 2004
 *
 *-------------------------------------------------------------------
 */
#pragma warning (disable: 4786)
#pragma warning (disable: 4503)

#include "PxDicomutil.h"
#define RTVEXPORT

#include "nvrmath.h"
#include "NMObject.h"
#define kMinDist 0.000001f

#if 1
#include "IDcmLib.h"
#include "IDcmLibApi.h"
using namespace XTDcmLib;
 
#else
#include "rtvMergeToolKit.h"
#endif

static inline OKFor2CTags(int status)
{
	return status == MC_NORMAL_COMPLETION || status == MC_NULL_VALUE || status == MC_NO_MORE_VALUES;
}

#if 0
//-----------------------------------------------------------------------------
// 
VLIVector3D CrossProduct(const VLIVector3D& v1, const VLIVector3D& v2)
{
	return VLIVector3D(v1[1] * v2[2] - v1[2] * v2[1],
					   v1[2] * v2[0] - v1[0] * v2[2],
					   v1[0] * v2[1] - v1[1] * v2[0]);
}
#endif
//-----------------------------------------------------------------------------
//
void CNMRotation::Init()
{
	m_startAngle = 0;
	m_angularStep = 0;
	m_rotationDirection = 0;
	m_scanArc = 0;
	m_frameDuration = 0;
	m_numberOfFramesInRotation = 0;

	m_status = MC_NORMAL_COMPLETION;
}

//-----------------------------------------------------------------------------
//	
int CNMRotation::Populate()
{
	//	All are type 1C - if a SQ item is present, they all must present too
	MC_STATUS status = MC_NORMAL_COMPLETION;
	status = MC_Get_Value_To_Double(m_sqID, MC_ATT_START_ANGLE, &m_startAngle);
	if (status != MC_NORMAL_COMPLETION)
	{
		return status;
	}

	status = MC_Get_Value_To_Double(m_sqID, MC_ATT_ANGULAR_STEP, &m_angularStep);
	if (status != MC_NORMAL_COMPLETION)
	{
		return status;
	}

	status = MC_Get_Value_To_Int(m_sqID, MC_ATT_SCAN_ARC, &m_scanArc);
	if (status != MC_NORMAL_COMPLETION)
	{
		return status;
	}

	status = MC_Get_Value_To_Int(m_sqID, MC_ATT_ACTUAL_FRAME_DURATION, &m_frameDuration);
	if (status != MC_NORMAL_COMPLETION)
	{
		return status;
	}

	status = MC_Get_Value_To_Int(m_sqID, MC_ATT_NUMBER_OF_FRAMES_IN_ROTATION, &m_numberOfFramesInRotation);
	if (status != MC_NORMAL_COMPLETION)
	{
		return status;
	}

	char direction[4];
	status = MC_Get_Value_To_String(m_sqID, MC_ATT_ROTATION_DIRECTION, sizeof(direction), direction);
	direction[sizeof(direction)-1] = 0;
	if (status != MC_NORMAL_COMPLETION)
	{
		return status;
	}

	if (!stricmp(direction, "CC"))
		m_rotationDirection = 1;
	else if (!stricmp(direction, "CW"))
		m_rotationDirection = -1;

	return status;
}

//-----------------------------------------------------------------------------
//
void CNMDetector::Init()
{
	m_imagePosition[0] = 0.;
	m_imagePosition[1] = 0.;
	m_imagePosition[2] = 0.;
	m_imageOrientation[0] = 1.;
	m_imageOrientation[1] = 0.;
	m_imageOrientation[2] = 0.;
	m_imageOrientation[3] = 0.;
	m_imageOrientation[4] = 1.;
	m_imageOrientation[5] = 0.;

	m_collimatorType[0] = 0;
	m_focalDistance = 0;
	m_distSourceToDetector = 0;

	m_hasValidImagePosition = true;
	m_hasValidImageOrientation = true;

	m_status = MC_NORMAL_COMPLETION;
}

//-----------------------------------------------------------------------------
//
void CNMDetector::Populate()
{
	double orientation[6] = {1., 0., 0., 0., 1., 0.};
	double position[3] = {0., 0., 0.};

	MC_STATUS status = MC_NORMAL_COMPLETION;
	status = MC_Get_Value_To_Double		(m_sqID, MC_ATT_IMAGE_POSITION_PATIENT,    &position[0]);
	if (status != MC_NORMAL_COMPLETION)
	{
		m_hasValidImagePosition = false;
	}

	status = MC_Get_Next_Value_To_Double(m_sqID, MC_ATT_IMAGE_POSITION_PATIENT,    &position[1]);
	if (status != MC_NORMAL_COMPLETION)
	{
		m_hasValidImagePosition = false;
	}

	status = MC_Get_Next_Value_To_Double(m_sqID, MC_ATT_IMAGE_POSITION_PATIENT,    &position[2]);
	if (status != MC_NORMAL_COMPLETION)
	{
		m_hasValidImagePosition = false;
	}

	status = MC_Get_Value_To_Double     (m_sqID, MC_ATT_IMAGE_ORIENTATION_PATIENT, &orientation[0]);
	if (status != MC_NORMAL_COMPLETION)
	{
		m_hasValidImageOrientation = false;
	}

	status = MC_Get_Next_Value_To_Double(m_sqID, MC_ATT_IMAGE_ORIENTATION_PATIENT, &orientation[1]);
	if (status != MC_NORMAL_COMPLETION)
	{
		m_hasValidImageOrientation = false;
	}

	status = MC_Get_Next_Value_To_Double(m_sqID, MC_ATT_IMAGE_ORIENTATION_PATIENT, &orientation[2]);
	if (status != MC_NORMAL_COMPLETION)
	{
		m_hasValidImageOrientation = false;
	}

	status = MC_Get_Next_Value_To_Double(m_sqID, MC_ATT_IMAGE_ORIENTATION_PATIENT, &orientation[3]);
	if (status != MC_NORMAL_COMPLETION)
	{
		m_hasValidImageOrientation = false;
	}

	status = MC_Get_Next_Value_To_Double(m_sqID, MC_ATT_IMAGE_ORIENTATION_PATIENT, &orientation[4]);
	if (status != MC_NORMAL_COMPLETION)
	{
		m_hasValidImageOrientation = false;
	}

	status = MC_Get_Next_Value_To_Double(m_sqID, MC_ATT_IMAGE_ORIENTATION_PATIENT, &orientation[5]);
	if (status != MC_NORMAL_COMPLETION)
	{
		m_hasValidImageOrientation = false;
	}

	if (m_hasValidImagePosition)
	{
		m_imagePosition[0] = position[0];
		m_imagePosition[1] = position[1];
		m_imagePosition[2] = position[2];
	}

	if (m_hasValidImageOrientation)
	{
		m_imageOrientation[0] = orientation[0];
		m_imageOrientation[1] = orientation[1];
		m_imageOrientation[2] = orientation[2];
		m_imageOrientation[3] = orientation[3];
		m_imageOrientation[4] = orientation[4];
		m_imageOrientation[5] = orientation[5];
	}

	MC_Get_Value_To_String(m_sqID, MC_ATT_COLLIMATOR_TYPE, sizeof(m_collimatorType), m_collimatorType);
	MC_Get_Value_To_Int(m_sqID, MC_ATT_FOCAL_DISTANCE, &m_focalDistance);
	MC_Get_Value_To_Int(m_sqID, MC_ATT_DISTANCE_SOURCE_TO_DETECTOR, &m_distSourceToDetector);
}


//-----------------------------------------------------------------------------
// 
void CNMDetector::GetImagePosition(double out[3]) 
{ 
	out[0] = m_imagePosition[0]; 
	out[1] = m_imagePosition[1]; 
	out[2] = m_imagePosition[2]; 
}

//-----------------------------------------------------------------------------
// 
void CNMDetector::GetImageOrientation(double out[6]) 
{ 
	out[0] = m_imageOrientation[0]; 
	out[1] = m_imageOrientation[1]; 
	out[2] = m_imageOrientation[2]; 
	out[3] = m_imageOrientation[3]; 
	out[4] = m_imageOrientation[4]; 
	out[5] = m_imageOrientation[5]; 
}


//-----------------------------------------------------------------------------
// 
CNMObject::CNMObject(CPxDicomImage* iImage) : m_pImage(iImage)
{
	m_status = kNMSuccess;

	if (!iImage)
	{
		m_status = kNMBadInputImage;
		return;
	}

	Init();
	Populate();
}

//-----------------------------------------------------------------------------
//	
int CNMObject::GetVectorIndex(int iFrameNumber, std::vector<int>& iVector)
{
	int index;
	int vSize = iVector.size();
	if (iFrameNumber >= vSize)
		return -1;
	else
		index = iVector[iFrameNumber] - 1;

	if (index >= vSize)
		return -1;

	return index;
}

//-----------------------------------------------------------------------------
//	
bool CNMObject::GetImageOrientation(int iFrameNumber, double oOrientation[6])
{
	int detectorIndex = GetDetectorIndex(iFrameNumber);
	if (detectorIndex < 0)
		return false;

	m_detectors[detectorIndex].GetImageOrientation(oOrientation);
	return true;
}

//-----------------------------------------------------------------------------
//	
bool CNMObject::GetImagePosition(int iFrameNumber, double oPosition[3])
{
	int detectorIndex = GetDetectorIndex(iFrameNumber);
	if (detectorIndex < 0)
		return false;

	m_detectors[detectorIndex].GetImagePosition(oPosition);
	return true;
}

#include <math.h>
//-----------------------------------------------------------------------------
//	

int  CNMObject::m_sFlipSliceSpacing = 0;
int	 CNMObject::m_sAddInitialOrientation = 0;

bool CNMObject::CalculateParallel(int iFrameNumber, double ioImagePosition[3], double ioImageOrientation[6])
{
	if (!m_hasValidInitialImageOrientation)
		return false;

	ioImagePosition[0] = m_initialImagePosition[0];
	ioImagePosition[1] = m_initialImagePosition[1];
	ioImagePosition[2] = m_initialImagePosition[2];

	//	Calculate Image Position
	VLIVector3D ip, dcr, dcc, cross, op;

	dcr.Assign (m_initialImageOrientation[0], m_initialImageOrientation[1], m_initialImageOrientation[2]);
	dcc.Assign (m_initialImageOrientation[3], m_initialImageOrientation[4], m_initialImageOrientation[5]);

	cross = VLICross(dcr, dcc);

	if (cross.Length() < kMinDist)
		return false;

	cross.Normalize ();

	ip.Assign (m_initialImagePosition[0], m_initialImagePosition[1], m_initialImagePosition[2]);

	// T.C. Zhao 2005.02.10
	// The standard PS 3.3, C.8.4.15 tableC.5-18 is not quite clear, but it seems to say
	// we need to reverse the sign of m_spacingBetweenSlices in the position
	// generation. We don't have too many datasets test this.
	// For HITACHI PET machine, we do need the reverse to get the orientation right
 
	if (m_sFlipSliceSpacing)
		op = ip - (m_spacingBetweenSlices*iFrameNumber*cross);
	else
		op = ip + (m_spacingBetweenSlices*iFrameNumber*cross);


	op.CopyTo (ioImagePosition);

	//	Orientation remains constant in the parallel case
	ioImageOrientation[0] = m_initialImageOrientation[0];
	ioImageOrientation[1] = m_initialImageOrientation[1];
	ioImageOrientation[2] = m_initialImageOrientation[2];
	ioImageOrientation[3] = m_initialImageOrientation[3];
	ioImageOrientation[4] = m_initialImageOrientation[4];
	ioImageOrientation[5] = m_initialImageOrientation[5];

	return true;
}

//-----------------------------------------------------------------------------
//	
bool CNMObject::CalculateRotational(int iFrameNumber, double oImagePosition[3], double oImageOrientation[6])
{
	if (!m_hasValidInitialImageOrientation)
		return false;

	if (m_rotations.size() < 1)
		return false;

	int rotationIndex = GetRotationIndex(iFrameNumber);
	rotationIndex = (rotationIndex < 0) ? 0 : rotationIndex;

	//	Calculate Image Orientation
	double startAngle = m_rotations[rotationIndex].GetStartAngle();
	double angularStep = m_rotations[rotationIndex].GetAngularStep();
	int direction = m_rotations[rotationIndex].GetRotationDirection();

	VLIMatrix mat = VLIMatrix::Identity();
	VLIVector3D dcr, dcc;

	startAngle   = startAngle - iFrameNumber*angularStep*direction;

	mat = VLIMatrix::Rotate (startAngle, VLIVector3D (0, 0, 1));


	dcr.Assign (m_initialImageOrientation[0], m_initialImageOrientation[1], m_initialImageOrientation[2]);
	dcc.Assign (m_initialImageOrientation[3], m_initialImageOrientation[4], m_initialImageOrientation[5]);

	mat.TransformVector (dcr, dcr);
	mat.TransformVector (dcc, dcc);

	oImageOrientation[0] = dcr[0];
	oImageOrientation[1] = dcr[1];
	oImageOrientation[2] = dcr[2];

	oImageOrientation[3] = dcc[0];
	oImageOrientation[4] = dcc[1];
	oImageOrientation[5] = dcc[2];

	//	Calculate Image Position
	oImagePosition[0] = m_initialImagePosition[0];
	oImagePosition[1] = m_initialImagePosition[1];
	oImagePosition[2] = m_initialImagePosition[2];

	return true;
}

//-----------------------------------------------------------------------------
//	
void CNMObject::GetInitialPosition()
{
	m_hasValidInitialImagePosition = false;

	//	Get the detector for frame 0
	int detectorIndex = GetDetectorIndex(0);
	if (detectorIndex < 0) 
	{
		if (m_detectors.size() > 0)
		{
			//	No indexing vector present, but there is a detector
			//		so we assume it's the first one
			detectorIndex = 0;
		}
	}

	if (detectorIndex >= 0)
	{
		m_detectors[detectorIndex].GetImagePosition(m_initialImagePosition);
		m_hasValidInitialImagePosition = m_detectors[detectorIndex].HasValidImagePosition();
	}

	if (!m_hasValidInitialImagePosition)
	{
		if (m_pImage->HasValidImagePosition())
		{
			double* position = m_pImage->GetImagePosition();
			m_initialImagePosition[0] = position[0];
			m_initialImagePosition[1] = position[1];
			m_initialImagePosition[2] = position[2];

			m_hasValidInitialImagePosition = true;
		}
	}
}

//-----------------------------------------------------------------------------
//	
// Vikram 07/26/04
// Only using the Orientation in the Image Orientation TAG
//
//
//
void CNMObject::GetInitialOrientation()
{
	m_hasValidInitialImageOrientation = false;

	//	Get the detector for frame 0
	int detectorIndex = GetDetectorIndex(0);
	if (detectorIndex < 0) 
	{
		if (m_detectors.size() > 0)
		{
			//	No indexing vector present, but there is a detector
			//		so we assume it's the first one
			detectorIndex = 0;
		}
	}

//	First look in the detector SQ
	if (detectorIndex >= 0)
	{
		m_detectors[detectorIndex].GetImageOrientation(m_initialImageOrientation);
		m_hasValidInitialImageOrientation = m_detectors[detectorIndex].HasValidImageOrientation();
	}

	if (m_hasValidInitialImageOrientation)
		return;

	//	Detector didn't have it - is it in the main level ImageOrientation tag?
	if (m_pImage->HasValidImageOrientation())
	{
		double* orientation	= m_pImage->GetImageOrientation();
		m_initialImageOrientation[0] = orientation[0];
		m_initialImageOrientation[1] = orientation[1];
		m_initialImageOrientation[2] = orientation[2];
		m_initialImageOrientation[3] = orientation[3];
		m_initialImageOrientation[4] = orientation[4];
		m_initialImageOrientation[5] = orientation[5];

		m_hasValidInitialImageOrientation = true;
		return;
	}

	// tc zhao 2005.05.19
	// kumamato
	m_hasValidInitialImageOrientation = (m_sAddInitialOrientation > 0);

}

//-----------------------------------------------------------------------------
//	
bool CNMObject::CalculateImageOrientationAndPosition(int iFrameNumber, double oImagePosition[3], double oImageOrientation[6])
{
	bool validCalculatedOrientation = false;

	//	Get initial image orientation and position
	if (!m_hasValidInitialImageOrientation)
		return false;

	if (m_isRotational)
	{
		validCalculatedOrientation = CalculateRotational(iFrameNumber, oImagePosition, oImageOrientation);
	}
	else
	{
		validCalculatedOrientation = CalculateParallel(iFrameNumber, oImagePosition, oImageOrientation);
	}

	return validCalculatedOrientation;
}

//-----------------------------------------------------------------------------
// 
void CNMObject::Init(void)
{
	m_msgID = m_pImage->GetID();

	m_type = kNMInvalidNMType;
	m_emissionOrTransmission = kNMUnknown;
	m_isRotational = false;

	m_spacingBetweenSlices = 0.;
	m_sliceThickness = 0.;

	m_numberOfDetectors = 0;
	m_numberOfRotations = 0;
	m_numberOfEnergyWindows = 0;
	m_numberOfPhases = 0;
	m_numberOfRRIntervals = 0;
	m_numberOfTimeSlots = 0;
	m_numberOfSlices = 0;
	m_numberOfAngularViews = 0;
	m_numberOfTimeSlices = 0;

	m_patientOrientationCode = kNMUnknownOrientation;
	m_patientGantryRelationshipCode = kNMUnknownOrientation;

	m_hasValidInitialImagePosition = false;
	m_hasValidInitialImageOrientation = false;

	m_initialImagePosition[0] = 0.;
	m_initialImagePosition[1] = 0.;
	m_initialImagePosition[2] = 0.;
	m_initialImageOrientation[0] = 1.;
	m_initialImageOrientation[1] = 0.;
	m_initialImageOrientation[2] = 0.;
	m_initialImageOrientation[3] = 0.;
	m_initialImageOrientation[4] = 1.;
	m_initialImageOrientation[5] = 0.;
}


//-----------------------------------------------------------------------------
// 
void CNMObject::ExtractFrameIndexVector(unsigned long iTag, std::vector<int>& oVector)
{
	int index = -1;
	int count = 0;
	MC_STATUS status = MC_Get_Value_Count(m_msgID, iTag, &count);
	if (status != MC_NORMAL_COMPLETION || count < 1)
		return;

	status = MC_Get_Value_To_Int(m_msgID, iTag, &index);
	for(int i = 0; status == MC_NORMAL_COMPLETION && index > 0 && i < count; i++)
	{
		oVector.push_back(index);
		status = MC_Get_Next_Value_To_Int(m_msgID, iTag, &index);
	}
}

//-----------------------------------------------------------------------------
// 
void CNMObject::ExtractFrameIndexVectors(void)
{
	//	Find out how many of the remaining vectors there are
	MC_Get_Value_To_Int(m_msgID, MC_ATT_NUMBER_OF_DETECTORS, &m_numberOfDetectors);
	MC_Get_Value_To_Int(m_msgID, MC_ATT_NUMBER_OF_ROTATIONS, &m_numberOfRotations);
	MC_Get_Value_To_Int(m_msgID, MC_ATT_NUMBER_OF_ENERGY_WINDOWS, &m_numberOfEnergyWindows);
	MC_Get_Value_To_Int(m_msgID, MC_ATT_NUMBER_OF_PHASES, &m_numberOfPhases);
	MC_Get_Value_To_Int(m_msgID, MC_ATT_NUMBER_OF_R_R_INTERVALS, &m_numberOfRRIntervals);
	MC_Get_Value_To_Int(m_msgID, MC_ATT_NUMBER_OF_TIME_SLOTS, &m_numberOfTimeSlots);
	MC_Get_Value_To_Int(m_msgID, MC_ATT_NUMBER_OF_SLICES, &m_numberOfSlices);

	MC_Get_Value_Count(m_msgID, MC_ATT_ANGULAR_VIEW_VECTOR, &m_numberOfAngularViews);
	MC_Get_Value_Count(m_msgID, MC_ATT_TIME_SLICE_VECTOR, &m_numberOfTimeSlices);


	ExtractFrameIndexVector(MC_ATT_DETECTOR_VECTOR, m_detectorVector);
	ExtractFrameIndexVector(MC_ATT_ROTATION_VECTOR, m_rotationVector);
	ExtractFrameIndexVector(MC_ATT_ENERGY_WINDOW_VECTOR, m_energyWindowVector);
	ExtractFrameIndexVector(MC_ATT_PHASE_VECTOR, m_phaseVector);
	ExtractFrameIndexVector(MC_ATT_R_R_INTERVAL_VECTOR, m_rrIntervalVector);
	ExtractFrameIndexVector(MC_ATT_TIME_SLOT_VECTOR, m_timeSlotVector);
	ExtractFrameIndexVector(MC_ATT_SLICE_VECTOR, m_sliceVector);
	ExtractFrameIndexVector(MC_ATT_ANGULAR_VIEW_VECTOR, m_angularViewVector);
	ExtractFrameIndexVector(MC_ATT_TIME_SLICE_VECTOR, m_timeSliceVector);
}

//-----------------------------------------------------------------------------
// 
void CNMObject::ExtractPatientOrientationCodeSequence(void)
{
	//	Patient Orientation Code
	int sqID  = -1;
	int sqID2 = -1;
	char codeValue[17];
	char codingSchemeDesignator[17];
	codeValue[0] = 0;
	codingSchemeDesignator[0] = 0;

	MC_STATUS status = MC_Get_Value_To_Int(m_msgID, MC_ATT_PATIENT_GANTRY_RELATIONSHIP_CODE_SEQUENCE, &sqID);
	if (status != MC_NORMAL_COMPLETION || sqID < 0)
	{
		m_patientGantryRelationshipCode = kNMUnknownOrientation;
	}
	else
	{
		MC_Get_Value_To_String(sqID, MC_ATT_CODE_VALUE, sizeof(codeValue), codeValue);
		MC_Get_Value_To_String(sqID, MC_ATT_CODING_SCHEME_DESIGNATOR, sizeof(codingSchemeDesignator), codingSchemeDesignator);

		if (!stricmp(codeValue, "G-5190") || !stricmp(codeValue, "F-10470"))
		{
			m_patientGantryRelationshipCode = kNMHeadFirst;
		}
		else if (!stricmp(codeValue, "G-5191") || !stricmp(codeValue, "F-10480"))
		{
			m_patientGantryRelationshipCode = kNMFeetFirst;
		}
		else
		{
			m_patientGantryRelationshipCode = kNMUnknownOrientation;
		}
	}

	status = MC_Get_Value_To_Int(m_msgID, MC_ATT_PATIENT_ORIENTATION_CODE_SEQUENCE, &sqID);
	if (status != MC_NORMAL_COMPLETION || sqID < 0)
	{
		m_patientOrientationCode = kNMUnknownOrientation;
	}
	else
	{
		status = MC_Get_Value_To_Int(sqID, MC_ATT_PATIENT_ORIENTATION_MODIFIER_CODE_SEQUENCE, &sqID2);
		if (status != MC_NORMAL_COMPLETION || sqID2 < 0)
		{
			m_patientOrientationCode = kNMUnknownOrientation;
		}
		else
		{

			MC_Get_Value_To_String(sqID2, MC_ATT_CODE_VALUE, sizeof(codeValue), codeValue);
			MC_Get_Value_To_String(sqID2, MC_ATT_CODING_SCHEME_DESIGNATOR, sizeof(codingSchemeDesignator), codingSchemeDesignator);

			if (!stricmp(codeValue, "F-10310"))
			{
				m_patientOrientationCode = kNMProne;
			}
			else if (!stricmp(codeValue, "F-10340"))
			{
				m_patientOrientationCode = kNMSupine;
			}
			else if (!stricmp(codeValue, "F-10317"))
			{
				m_patientOrientationCode = kNMDecubitusRight;
			}
			else if (!stricmp(codeValue, "F-10319"))
			{
				m_patientOrientationCode = kNMDecubitusLeft;
			}
			else
			{
				m_patientOrientationCode = kNMUnknownOrientation;
			}
		}
	}	
}

//-----------------------------------------------------------------------------
// 
void CNMObject::Populate(void)
{
	MC_STATUS status;

	ExtractNMType();		
	ExtractFrameIndexVectors();

	MC_Get_Value_To_Double(m_msgID, MC_ATT_SLICE_THICKNESS, &m_sliceThickness);

	//	Detector information sequence
	if (m_numberOfDetectors > 0)
	{
		int detectorItemID = -1;
		status = MC_Get_Value_To_Int(m_msgID, MC_ATT_DETECTOR_INFORMATION_SEQUENCE, &detectorItemID);
		for(int di = 0; status == MC_NORMAL_COMPLETION && di < m_numberOfDetectors; di++)
		{
			CNMDetector detector(detectorItemID);
			m_detectors.push_back(detector);

			status = MC_Get_Next_Value_To_Int(m_msgID, MC_ATT_DETECTOR_INFORMATION_SEQUENCE, &detectorItemID);
		}
	}

	//	Rotation sequence
	if (m_numberOfRotations > 0)
	{
		int rotationItemID = -1;
		status = MC_Get_Value_To_Int(m_msgID, MC_ATT_ROTATION_INFORMATION_SEQUENCE, &rotationItemID);
		for(int ri = 0; status == MC_NORMAL_COMPLETION && ri < m_numberOfRotations; ri++)
		{
			CNMRotation rotation(rotationItemID);
			m_rotations.push_back(rotation);
			status = MC_Get_Next_Value_To_Int(m_msgID, MC_ATT_ROTATION_INFORMATION_SEQUENCE, &rotationItemID);
		}
	}

//	if (m_type != kNMReconTomo && m_type != kNMReconGatedTomo && m_rotations.size() > 0)
//		m_isRotational = true;

	// VIkram 07/26/04 Changed the way we say if a scan is rotational or not.
	// Only TOMO and Gated TOMO scans are rotational all other are  parallel

	m_isRotational = false;

	// T.C. Zhao 2005.05.20
	// Have field data that seems to indicate we need && rather than ||
	// Need to read on the standard and more testing
	if ((m_type == kNMTomo || m_type == kNMGatedTomo) && m_rotations.size() > 0)
//	if ((m_type == kNMTomo || m_type == kNMGatedTomo) || m_rotations.size() > 0)
		m_isRotational = true;


	if (!m_isRotational)
	{
		status = MC_Get_Value_To_Double(m_msgID, MC_ATT_SPACING_BETWEEN_SLICES, &m_spacingBetweenSlices);
		if (status != MC_NORMAL_COMPLETION)
		{
			double* pixelSpacing = m_pImage->GetPixelSpacing();
			m_spacingBetweenSlices = pixelSpacing[0];
		}
	}

	ExtractPatientOrientationCodeSequence();

	GetInitialPosition();
	GetInitialOrientation();
}

//-----------------------------------------------------------------------------
// 
void CNMObject::ExtractNMType(void)
{
	std::vector<std::string> imageTypeTokens = m_pImage->GetImageTypeTokens();
	if (imageTypeTokens.size() < 3)
		m_status = kNMBadImageType;

	const char* nmType = imageTypeTokens[2].c_str();

	if      (!stricmp(nmType, "STATIC"))
		m_type = kNMStatic;
	else if (!stricmp(nmType, "DYNAMIC"))
		m_type = kNMDynamic;
	else if (!stricmp(nmType, "GATED"))
		m_type = kNMGated;
	else if (!stricmp(nmType, "WHOLE BODY"))
		m_type = kNMWholeBody;
	else if (!stricmp(nmType, "TOMO"))
		m_type = kNMTomo;
	else if (!stricmp(nmType, "GATED TOMO"))
		m_type = kNMGatedTomo;
	else if (!stricmp(nmType, "RECON TOMO"))
		m_type = kNMReconTomo;
	else if (!stricmp(nmType, "RECON GATED TOMO"))
		m_type = kNMReconGatedTomo;
	else
		m_type = kNMInvalidNMType;

	if (imageTypeTokens.size() < 4)
	{
		m_emissionOrTransmission = kNMUnknown;
		return;
	}

	const char* nmEmissionOrTransmission = imageTypeTokens[3].c_str();
	if      (!stricmp(nmEmissionOrTransmission, "EMISSION"))
		m_emissionOrTransmission = kNMEmission;
	else if (!stricmp(nmType, "TRANSMISSION"))
		m_emissionOrTransmission = kNMTransmission;
	else
		m_emissionOrTransmission = kNMUnknown;
}

