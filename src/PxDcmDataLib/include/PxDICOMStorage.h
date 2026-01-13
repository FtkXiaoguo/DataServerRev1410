/***********************************************************************
 *  AqDICOMStorage.h
 *---------------------------------------------------------------------
 *		Copyright, Terarecon 2001, All rights reserved.
 *
 *	PURPOSE:
 *		Provide AQNet DICOM data store and retriev API
 *
 *	AUTHOR(S):  Gang Li, June 2004
 *				Shiying Hu, August 2004	
 *			
 *-------------------------------------------------------------------
 */
#ifndef	__AQDICOMSTORAGE_H__
#define	__AQDICOMSTORAGE_H__

#include <assert.h>
#include <stdlib.h>
#include <string>
#include <vector>

#include "PxDB.h"

class CacheBuilderForOneSeries;

enum eAqErrorCode
{
	// use kOK for ok case because kAqSuccess is used by AqPE. (shiying reviewed with grey)
	kAqCancelled,
	kAqErrCannotAccessDirectory,
	kAqErrCannotOpenFile,
	kAqErrNotInitialized,
	kAq2ErrBadInputParameters, // (shiying reviewed with grey)
	kAqErrBadSkip,
	kAqErrSeekFail,
	kAqErrBadReadSize,
	kAqErrReadShort,
	kAqErrBufTooShort,
	kAqErrDataInvalidate,
	kAqErrNoDiskSpace,
	kAqErrFileStateIsNotGood,
	kAqErrFileIsNotOpen,
	kAqErrCannotCloseFile,
	kAqErrBadImage,
	kAqErrLoadDicomFileFailed, //added by shiying
	kAqErrSaveDicomFileFailed,//added by shiying
	kAqErrCannotGetTagValue, //added by shiying
	kAqErrCannotSetTagValue, //added by shiying
	kAqErrCannotStripPixel,//added by shiying
	kAqErrHeaderPixelKeyMismatch, //added by shiying
	kAqErrCannotSetPixelDataTag,//added by shiying
	kAqErrCacheFile,//added by shiying
	kAqErrFailDecompressData,//added by shiying
	kAqErrNoEnoughMemory,//added by shiying
	kAqErrBadWriteSize,//added by shiying
	kAqErrCannotRenameFile,//added by shiying
	kAqErrCannotRemoveFile,//added by shiying
	kAqErrCannotCopyFile,//added by shiying
	kAqErrNotBelongToSameSeries, //added by shiying
	kAqErrUpdateDatabase, //added by shiying
	kAqErrCannotGetPixelData, //added by shiying
	kAqErrNoDCMFiles, 
	kAqErrNoMatchingKey,
	kAqFileShareViolaiton,
	kAqErrUnknown
};


//-----------------------------------------------------------------------------
// This class is used for the following purposes
// 1) By any user to save DICOM data into the 
//    AqNET CACHE format
//
// 2) By any user to read a single slice out of the AqNET CACHE format
//
// 3) 

class AqDICOMTag 
{	
public:
	AqDICOMTag();
	~AqDICOMTag();

	void Reset();

	// single value operations
	void SetTag(unsigned long iTag) {m_tag = iTag;}
	unsigned long GetTag() const {return m_tag;}

	void SetDataBinary(bool iDataBinary=true) {m_binaryData = iDataBinary;}
	bool IsDataBinary() {return m_binaryData;}
	

	// call SetData will set this tag to no sequence data type
	void SetData(const char* iData, unsigned long iDataLength);
	unsigned long GetDataLength() const {return m_dataLength;}
	const char* GetData() const {return m_pData;}

	// sequence value operations
	int GetSquenceSize() {return m_squence.size();}
	AqDICOMTag* GetItem(unsigned int iIndex) 
	{return (m_squence.size() > iIndex)? m_squence[iIndex]:0;}
	
	void AddItem(AqDICOMTag* ipTag);// this functions will take over the owership of input object
	void DeleteItem(int iIndedx);
	void ClearAllItem();

	void SetPrivateCreator(const char * iPrivateCreator) { m_privateCreator = iPrivateCreator; }
	std::string GetPrivateCreator() { return m_privateCreator; }

protected:

	unsigned long	m_tag;
	char*			m_pData;
	unsigned long	m_dataLength;
	bool			m_binaryData;

	std::vector<AqDICOMTag*> m_squence;

	std::string		m_privateCreator;
};

enum eAqDICOMStorageOpenMode
{
	kAqDICOMStorageOpenRead = 0,
	kAqDICOMStorageOpenWrite
};

class CacheFileFragmentHandler;
class AqBuffer;

class AqDICOMStorage
{
public:
	 AqDICOMStorage();
	~AqDICOMStorage();

	// Open 
	virtual int Open(const char* iTopLevelPath,const bool iUpdateDatabase);
	virtual int Open(const char* iStudyUID, 
				 const char* iSeriesUID, const char* iTopLevelPath);

	virtual int Store (const char* iFileInMemory, int iSizeOfFile);

	//import DICOM file. if iFilename is not given import all DICOM files have extension of ".dcm"
	virtual int Store (const char* iPathToDICOMFile, const char* iFilename=0);

	virtual int Read (const char* iSOPUID, AqBuffer& oData);
	
	// if instance UID not given, do it for whole series
	// if filename not given, use instance UID as filename
	virtual int Read (const char* iDICOMFilePath, const char* iSOPUID=0, 
		const char* iFilename=0);
	
	// copy cache files associated with specified seriesUID and stutyUID
	// to iCopyPath
	virtual int Copy (const char* iCopyPath);
	
	// delete cache files associated with specified seriesUID and stutyUID
	virtual int Delete();

	virtual int GetDICOMTags (AqDICOMTag ioTags[], int iTagNumber, const char* iSOPUID);
	
	// if instance UID not given, do it for whole series
	virtual int SetDICOMTags (AqDICOMTag ioTags[], int iTagNumber, const char* iSOPUID=0); 

	virtual int Close ();
	
	// temporary implementation for testing purposes.
	// this should be called before open function.
	void	SetDatabasePointer(CPxDB * iDBPointer)
	{ m_pDB = iDBPointer;}


protected:
	// handler to convert DICOM file to cache files
	CacheBuilderForOneSeries * m_pCacheBuilder;
	CacheFileFragmentHandler * m_pPixelCache;
	CacheFileFragmentHandler * m_pDictionaryCache;

	// indicate if merge tookit is initialized successfully
	// also indicate if merge toolkit is initialized by this instance.
	bool m_mergeInitStatus;

	// user storage directory	
	std::string m_storageDir;

	std::string m_studyUID;
	std::string m_seriesUID;
	
	bool m_updateDatabase;

	bool m_openForStore;

	// pass db pointer from application which is calling
	// AqDICOMStorage APIs.
	// This is a temporary implementation for testing purposes.
	CPxDB * m_pDB; 

	void CleanUp();
	
};

#endif	/* __AQDICOMSTORAGE_H__ */