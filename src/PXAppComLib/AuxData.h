/***********************************************************************
 *---------------------------------------------------------------------
 *	
 *
 *-------------------------------------------------------------------*/

#ifndef _AUX_DATA_H
#define _AUX_DATA_H

#include "PxDBData.h"

#define TR_GROUP										0x0077
#define TR_CREATOR										"TERARECON AQUARIUS"

#define TR_ATT_ORIGINAL_SERIES_AND_STUDY_INSTANCE_UID	0x10
#define TR_ATT_ORIGINAL_SOP_INSTANCE_UID				0x12
#define TR_ATT_REFERENCED_VOLUME_ID						0x14
#define TR_ATT_BINARY_DATA_NAME							0x20
#define TR_ATT_NUMBER_OF_SOP_INSTANCES					0x22
#define TR_ATT_NUMBER_OF_SERIES							0x24
#define TR_ATT_NUMBER_OF_BINARY_DATA					0x26
#define TR_ATT_BINARY_DATA_TYPE							0x28
#define TR_ATT_BINARY_DATA_SIZE							0x30
#define TR_ATT_ADDITIONAL_INFORMATION					0x40
#define TR_ATT_FIRST_BINARY_DATA						0x50
#define TR_ATT_FIRST_THUMBNAIL							0x60
#define TR_ATT_ALGORITHM_ID								0x70
#define TR_ATT_VOLUME_ID								0x80

#define TR_ATT_CA_SCORE									0x00090100
#define TR_ATT_CA_REPORT								0x00090200

#define kDeltaImagingRpt "0DELTA_REPORT0"

//-----------------------------------------------------------------------------
//
class AuxData
{
public:
	AuxData(): m_messageID(-1), m_isOldCaScore(false),m_hasAuxData(false),m_binaryDataSize(0), m_cofVertion(0), m_cofRevision(0) {};
	virtual ~AuxData() {};
	
	static bool CheckHasAuxData(int iMessageID);
	static bool CheckIsOldCaScore(int iMessageID);
	static bool IsDeltaImagingReport(int iMessageID);

	int Init(int iMessageID, const char* iStudyUID, const char* iSeriesUID, const char* iSOPUID);
	bool IsOldCaScore() {return m_isOldCaScore;}
	bool HasAuxData() {return m_hasAuxData;}


	// T.C. Zhao 2006.05.30 unify auxdata file names
	static std::string ComposeAuxDataFileName(int dbKey, const char* iAuxData, const char* iSuffix);

	AuxDataInfo m_auxData;
	std::vector<AuxReference> m_auxReferencs;

private:

	int ExtractFirstOne();
	int ExtractReferences();
	int	ExtractAdditionalInformation();
	static int TypeStringToEnum(const char* iTypeString);

	int m_messageID;
	bool m_isOldCaScore;
	bool m_hasAuxData;
	long m_binaryDataSize;
	
	int m_cofVertion;
	int m_cofRevision;
};

#endif // _AUX_DATA_H