/***********************************************************************
 * DiCOMStore.h
 *---------------------------------------------------------------------
 *		Copyright, PreXion 2012, All rights reserved.
 *
 *	PURPOSE:
 *		Processes DiCOMStore Requests
 *
 *	
 *
 *-------------------------------------------------------------------
 */
#ifndef DiCOMStore_H
#define DiCOMStore_H

#include "RTVDiCOMStore.h"

#include "rtvPoolAccess.h"



//-----------------------------------------------------------------------------
class DiCOMStore : public RTVDiCOMStore
{
public:
	typedef RTVMapAccess<int, DiCOMStore*> MSG_TO_DiCOMStore_MAP;	
	static MSG_TO_DiCOMStore_MAP map_mssageToDiCOMStore;

	DiCOMStore (DiCOMConnectionInfo& connectInfo, int iMessageID);
	~DiCOMStore();
	
	int ProcessPIXEL(unsigned long tag, int CBtype, unsigned long* dataSizePtr,
					void** dataBufferPtr,int isFirst,int* isLastPtr);
	int Process();
	void LogProcessStatus(void);
	const char* GetSeriesInstanceUID() const ;
	const char* GetStudyInstanceUID() const ;

private:
	int theProcess();
	int CoerceSOPInstanceUID();
	int HandleTerareconSpecific ();
	int WriteDICOMFileInCache();
	int SaveDicomFile(const char* iSavePath, const char* iFileName, bool head_only=false);
	int HandleXAImage (unsigned char* pixel, unsigned long pixelLen);
	int HandleNMImage(unsigned char* pixel, unsigned long pixelLen);
	int UpdateProgress(void);

	unsigned char*	m_PIXELbuffer;
	unsigned long	m_PIXELoffset;
	unsigned long	m_PIXELlength;
	bool			m_head_only;
	char			m_seriesInstanceUID[65];
	char			m_studyInstanceUID[65];
};

//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
#endif // DiCOMStore_H
