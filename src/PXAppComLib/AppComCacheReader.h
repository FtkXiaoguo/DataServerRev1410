/***********************************************************************
 * AppComCacheReader.h
 *---------------------------------------------------------------------
 *		Copyright, PreXion 2012, All rights reserved.
 *
 *	PURPOSE:
 *		This class is used to read the Terarecon Cache.
 *		DICOM Series.
 *
 *	
 *   
 *-------------------------------------------------------------------
 */
#if !defined(APPCOM_CACHEREADER_H)
#define APPCOM_CACHEREADER_H

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#define USE_HASH
#define USE_STDIO


#ifndef TICACHE_H
#include "TICache.h"
#endif

const int kNumberOfIRTVSliceBuffers = 4;
const int kLengthOfEachLineInBytes = 768;
const int kLengthOfkeywordValueInBytes = 128;



#if !defined(RTVSLICEINFORMATION_H)
 #include "RTVSliceInformation.h"
#endif

#if !defined(RTVDICOMGROUP_H)
#include "RTVDICOMGroup.h"
#endif




#include <string>
//#include <fstream>
#include <vector>
//#include <iostream.h>

#include <string>

#if !defined (USE_STDIO)

#if _MSC_VER >= 1300 // for vc7
	#include <fstream>
	#include <iostream>
	using namespace std;
#else
	#include <fstream.h>
	#include <strstrea.h>
	#include <iostream>     // Required to avoid LNK2001 error
#endif

#else

#include <stdio.h>

#endif
 
//-----------------------------------------------------------------------------

//-----------------------------------------------------------------------------

#include <map>

class AppComCacheReader
{
	public:

		AppComCacheReader();
		virtual ~AppComCacheReader();

		// Open the Cache and read the "Cache.Description" file
		// If iProcessSlices==false teh slices are not processed and m_allSlices
		// is in the order the slice are written in the Cache File
		//
		int ReadCacheDescription (const char* iCacheLocation, 
			                      bool iProcessSlices=true,
								  bool ForceReadCompressed=false);// -- 07/18/2003

		// use this to fix some old cache problems (zhao 2004.01.16)
		virtual void	OnOpenFile(const char* iFile) {}
		virtual int		DescriptionDone() {return kSuccess;}

		static void		SetSortMRByScanTypeFirst(int iYesNo); // -- 2005.11.15
		static void		SetPresortCTByImagePositionAfterImageNumber(int iYesNo); // Vikram 07/17/2006

		static void     SetSortCTByScanOptionsFirst(int iYesNo);	// Murali 2007.01.03	
		static int		GetSortCTByScanOptionsFirst() { return m_sortCTByScanOptionsFirst; }


		std::vector <iRTVDICOMGroup>& GetGroups() {return m_groups;}
		int GetNumberOfGroups() {return m_groups.size();}

		std::string& GetModality() {return m_modality;}
		std::string& GetSOPClassUID() {return m_sopClassUID;}

		// -- 2004.04.05
		// fix scenes from Beacon Sciences and subseries handling in general
		void	PruneSOPs(std::map<std::string,int>& inSOPs) ;


		// -- 2006.07.28
		int		IsPruned(void) const { return m_isPruned;}

        // Vikram 07/09/02 - If iBegin or iEnd == -1 than all the slices
        // are loaded
		int  OpenDataFile (int iGroupID, int iBegin = -1, int iNumSlices = -1);
		
        int  CloseDataFile (int iGroupID);
        
		bool ShouldSortByScanOption(const char* iSOPClassUID, const char* iManufactureer, const char* iScanOption);
        // Vikram 07/09/02
        // This loads the next best slice 
        // When you use this function the slices are loaded in the
        // order they are written in the cache file to speed up loading
        iRTVSliceBuffer* LoadNextSlice (int GroupID);

		// Vikram 09/16/02 - Added for loading compressed images
		iRTVSliceBuffer* LoadNextCompressedSlice (int GroupID, float iCompressionRatio);

		// -- 2005.10.11
		// Read really raw slice, except for CT - where the data is in HU
		iRTVSliceBuffer* ReadRawSlice(int iGroupID, int iSliceNumber);

		// Vikram 09/23/03 Subtract Volumes
		// This function just reads the slice from the file without doing any post process
		iRTVSliceBuffer* ReadSlice (int iGroupID, int iSliceNumber);

		iRTVSliceBuffer* LoadSlice (int iGroupID, int iSliceNumber);
		iRTVSliceBuffer* LoadSlice (iRTVSOPLocation& iSliceNumber, int iGroupID = 0);

		int GetXSize (int iGroupID);
		int GetYSize (int iGroupID);
		int GetZSize (int iGroupID);

		// Vikram 12/27/01
		// This should give us the number of images in a series
		int GetNumberOfUniqueSOPInstanceUIDS (void) {return m_uniqueSOPInstanceUIDS.size();}

		int GetBitsUsedPerPixel (int iGroupID);
		int GetBytesPerPixel (int iGroupID);
		int GetBitsAllocated(int iGroupID);

        // For SC images - Vikram 04/16/02
		int GetSamplesPerPixel (int iGroupID);
		int GetPhotometricInterpretation (int iGroupID);
		int GetPlanarConfiguration (int iGroupID);
		

		bool IsUniformlySpaced (int iGroupID);

		std::string& GetPlaneType(int iGroupID);
		std::string& GetSOPInstanceUID(int iGroupID);
		std::string& GetReferencedSOPInstanceUID(int iGroupID);
		int GetReferencedGroupID(int iGroupID);

		// Added 11/13/01 --
		// We sort the SOP on demand so we can pull any slice based on SOP
		iRTVSOPLocation Find(const char* iSOPUID, int iFrameNumber);
		void SortSOP(void);

        // This function returns the best group to load if there are multiple groups
        //
        int GetIndexOfBestGroupToLoad ();

		// temp hack 2003/02/27  -- so the cacheReader can keep its state
		void SetIgnoreCompressedCache(int YN) { m_ignoreCompressedCache = YN;}

		void Clear ();
        int AddSlice (iRTVPtrToSliceInformation& iPtr);

		//-- 2004.11.10
		// Find sliceInfo for a particular SOP
		const iRTVPtrToSliceInformation* GetSliceInfo(const char* iSOPUID, int iFrameNum=0);

        // Returns the current sort type
        eSortType GetSortType () {return m_currentSortType;}

		// Returns the current number of phases
        int GetNumberOfPhases () {return m_currentPhases;}

		int	SetSortMethod(int iSort, int iNphase=1);

		int ProcessByImagePosition ();
        int ProcessByImageNumber (int iPhases);
        int ProcessByListOfSOPInstanceUIDs (std::vector<std::string>& iSOPInstanceUIDs);
		int ProcessAs2DSlices (void);

		
		// -- 2005.11.17
		void	GenerateSortedIndex(void);
		void	SortByImageNumber(void);
		void	SortByScanType(void);
		// End of 2005.11.17

		// -- JULY-14-2003 added SliceNum for preview 
		int GetGroupIDForSOPLocation (iRTVSOPLocation& iSOPLocation, int& oSliceNum);
		int GetGroupIDForSOPUID(const char* iSOPUID);
		
		std::vector <iRTVPtrToSliceInformation> & GetAllSlices(void) { return m_allSlices;}

		int	GetNumberOfSlices(void) { return m_allSlices.size();}

        // This is only one deep. Not recursive
        // This is used to push  and pop the sort type stack
        int PushSortStack ();
        int PopSortStack();

		// Have you read the L0Compressed Cache
		bool IsL0CompressedCacheRead () { return m_L0CompressedCacheRead;}
		bool IsL0CompressedCachePresent () { return m_L0CompressedCachePresent;}

		/* get the sorted list of slices information in the VP1000 order */
		int		GetSortedSliceInformation(int iGroup, tLoadedSlice& oSliceInfo);

		double GetSlope(int iG); // t.c. zhao 2003-03-18

        friend class AQNetCacheFromMemory;
		friend class AQNetSortUsingDB;

		bool	IsValid(void) const; // -- 2004.04.13

		bool	DescriptionIsValidated(void) const ;
		
		std::string	m_PETUnits;

protected:
#if !defined (USE_STDIO)
		ifstream		m_srcFileStream;
#else
		FILE*			m_srcFile;
#endif
		std::string	m_cacheDirectory;
		std::string	m_cacheDataFilename;
		std::string	m_compressedL0CacheDataFilename;
		std::string	m_modality;
   		std::string	m_sopClassUID;

		static int	m_sortMRByScanTypeFirst;

		// Vikram 07/17/2006 Fix for data with no corelation with ImageNumber and ImagePosition
	    static int  m_presortCTByImagePositionAfterImageNumber;

		// Murali 2007.01.03 Sorting CT with scanOptions specified.
	    static int  m_sortCTByScanOptionsFirst;

		// Does L0COmpressed Cache Exist for this Series
		bool m_L0CompressedCacheRead;

		// Does L0COmpressed Cache Exist for this Series
		bool m_L0CompressedCachePresent;

        // Vikram 07/05/02
        eSortType	m_currentSortType;
 		int			m_currentPhases;

        eSortType	m_sortTypeOnStack;
  		int			m_phasesOnStack;

		int			m_ignoreCompressedCache;
       
        bool	m_isCacheDescriptionRead;
		bool	m_isMultiPhasic; // Is the current data multi-phasic ?
		int     m_firstMultiPhasicGroupID;
		int	    m_numberOfMultiPhasicGroups;
		int		m_firstMultiPhasicImageNumber;

        iRTVSliceBuffer m_sliceBuffers[kNumberOfIRTVSliceBuffers];
        iRTVSliceBuffer m_compressedSliceBuffers[kNumberOfIRTVSliceBuffers];


        // This is a vector of all the slice in this series
        // In the case of multi-frame XA this is all the frames
        // Vikram 07/05/02 - Sub Series support
        std::vector <iRTVPtrToSliceInformation> m_allSlices;

 
		// This maps the SOPInstanceUID to the index in the vector
        // m_allSlices
		// Added by TC and Vikram
       std::map<std::string, iRTVSOPLocation> m_sortedSOPToSliceLocation;

        

        
        // This the number of groups based on the current sort type        
        std::vector <iRTVDICOMGroup> m_groups;



		// Vikram 12/27/01
		// This was added to get the number of unique SOP instance UIDs'
		// This is used by nvrServer to retieve the number of images at the
		// Series level from the cache. The int here does nto mean anything
		//
		std::map<std::string, int> m_uniqueSOPInstanceUIDS;

#if !defined (USE_HASH)
		// Vikram 12/27/01
		// Added to speed up the parsing of Cache Description
		// This returns an int which is used in a switch statement
		// We need to have the ints start from 1 not zero. See P.206
		// The  C++ Standard Library: A tutorial and reference
		// by Nicolai M. Josuttis
		//
		static std::map<std::string, int> m_cacheDataTokens;
#else
		// This is a map of Hash to Idx 
		static std::map <int, int> m_tokenToIdx;

#endif

#if !defined (USE_STDIO)
		bool GetNextValidLine (std::string& oLine, ifstream& iSrcFile);
#else
		bool GetNextValidLine (std::string& oLine, FILE* iSrcFile);
		bool GetNextValidLine (char* oLine, FILE* iSrcFile);
#endif

		int CreateNewGroup (iRTVPtrToSliceInformation& iPSi);
		int ProcessGroups ();
		bool IsScanOptionsUsedForSorting(void);

		/* -- 4-30-2002
		 * need the following to make sure file did not change between two calls
		 */
		unsigned int m_descriptionSize;
		unsigned int m_descriptionTime;


        int ProcessAllSlices ();

		// -- 2004.04.13 - need this to invalidate description cache
		// and thus force a re-read of the cache description
		void SetForceReadDescription(void);
     
        // By Axis of Aquisition
        int AddCTToGroupByImagePosition (iRTVPtrToSliceInformation& iPSi);
        int AddMRToGroupByImagePosition (iRTVPtrToSliceInformation& iPSi);

        // By Instance Number
        int AddCTMRToGroupByImageNumber (iRTVPtrToSliceInformation& iPSi);


        int AddXAToGroup (iRTVPtrToSliceInformation& iPSi);
        int AddSCToGroup (iRTVPtrToSliceInformation& iPSi);
        int AddUSToGroup (iRTVPtrToSliceInformation& iPSi);
        int AddNMToGroup (iRTVPtrToSliceInformation& iPSi);
		int AddRestOfTheModalitiesToGroup (iRTVPtrToSliceInformation& iPSi);

		// Vikram 06/13/03 Pure 2D Rendering
		int AddCTMRToGroupAs2DSlices (iRTVPtrToSliceInformation& iPSi);

		int ReadCompressedCacheDescription (void);

		char m_nextLine [kLengthOfEachLineInBytes];
		char m_token [8][kLengthOfEachLineInBytes];
 
		// -- 2007-07-09 
		bool    HasImageNumber(void) const;

		int		m_isPruned;					// -- 2004.04.13
		int		m_descriptionIsValidated;	// -- 2004.05.13

	private:
		// Vikram 06/08/2005 06/08/2005 1212_3434_5656 Fix
		// 

		//Murali 2007/03/20 Use %R-R for sorting siemens CT images with scanOptions set.
		void    ValidateAndFixSortedGroups(void);

		int m_currentNumberOfMultiPhasicGroups;
		int m_currentFirstGroup;
};

//-----------------------------------------------------------------------------

#endif // APPCOM_CACHEREADER_H
