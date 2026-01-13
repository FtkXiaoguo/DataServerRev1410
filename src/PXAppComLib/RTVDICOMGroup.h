/***********************************************************************
 * $Id: RTVDICOMGroup.h 35 2008-08-06 02:57:21Z atsushi $
 *---------------------------------------------------------------------
 *		Copyright, PreXion 2012, All rights reserved.
 *
 *	PURPOSE:
 *		This class is used by the TerareconCache Reader
 *
 *	
 *   
 *-------------------------------------------------------------------
 */
#if !defined(RTVDICOMGROUP_H)
#define RTVDICOMGROUP_H

#include "RTVSliceInformation.h"
#include "TICache.h"
#include <string>
#include <vector>
#include <map>
#include <windows.h> // for HANDLE		m_srcFile; 
#include "AqCore/AqCore.h"

#define AWARE_ONLY

//#define PAGE_SIZE_MASK (4096-1)
//#define HDISK_SECTOR_SIZE 512


//-----------------------------------------------------------------------------

class iRTVSOPLocation
{
    public:
	    iRTVSOPLocation(int iIndexInVectorOfSlices = -1, __int64  iStartOfData = 0, __int64  iSizeOfData = 0) 
        { 
            m_indexInVectorOfSlices = iIndexInVectorOfSlices; 
            m_sliceNumber           = 0;
            m_startOfDataInDataFile = iStartOfData; 
            m_sizeOfData            = iSizeOfData;
        }

	    bool Bad(void) { return m_indexInVectorOfSlices < 0;}
	    
        int	m_indexInVectorOfSlices;
	    int	m_sliceNumber;
        __int64  m_startOfDataInDataFile;
		__int64  m_sizeOfData;

};


//-----------------------------------------------------------------------------
class iRTVSliceBuffer
{
	public :
		iRTVSliceBuffer ()
		{
			m_bufferPageAlignedPtr = 0;
			m_bufferUnaligned = 0;
			m_bytesAllocated = 0;
			m_bufferUsed = false;
			m_numberOfBytesStored = 0;
			m_pixelRepresentation = 0;
						
            m_positonInAllSlicesVector = m_sliceNumber = -1;
			m_workingSliceNumber = -1;	//Murali 2005.11.15
		}

		//=====================================================================
		~iRTVSliceBuffer ()
		{
			FreeAllMemory ();
		}

		//=====================================================================

		void Release ()
		{
			m_bufferUsed = false;
		}

		//=====================================================================
        void  SetSliceNumber(int iNumber) {m_sliceNumber = iNumber;}
        int   GetSliceNumber() {return m_sliceNumber;}

		// -- 2004.4.5
		void  SetWorkingSliceNumber(int iNumber) {m_workingSliceNumber = iNumber;}
        int   GetWorkingSliceNumber() {return m_workingSliceNumber;}
        
		void  SetPositonInAllSlicesVector(int iNumber) {m_positonInAllSlicesVector = iNumber;}
        int   GetPositonInAllSlicesVector() {return m_positonInAllSlicesVector;}
		
		char* GetData () {return m_bufferPageAlignedPtr;}

		bool  IsUsed () {return m_bufferUsed;}
		void  SetIsUsed () {m_bufferUsed = true;}
		int   GetNumberOfBytesStored () {return m_numberOfBytesStored;}
		void  SetNumberOfBytesStored (int iBytes) {m_numberOfBytesStored = iBytes;}

		// -- 2005.10.11
		bool  IsSigned(void) const
		{
			return m_pixelRepresentation == 1;
		}

		//=====================================================================

		void FreeAllMemory ()
		{
			if (m_bufferUnaligned)
				delete [] m_bufferUnaligned, m_bufferUnaligned = 0;
			
			m_bufferPageAlignedPtr = 0;
			m_bytesAllocated = 0;
			m_bufferUsed = false;
		}
	
		//=====================================================================
		void Allocate (int iBytesToAllocate)
		{
			if (m_bytesAllocated < iBytesToAllocate)
			{
				FreeAllMemory ();
				
				// prepare memory block can be page aligned and can be used in sector aligned
				// reading e.g. two more sectors.
				// it can read one extra sectore in front and one in back,

				m_bufferUnaligned = new  char[iBytesToAllocate+PAGE_SIZE_MASK+2*HDISK_SECTOR_SIZE];
				
				//	Allocate and align the ptr to the next page boundary
				m_bufferPageAlignedPtr = (char *)((((unsigned int)m_bufferUnaligned) + HDISK_SECTOR_SIZE + PAGE_SIZE_MASK) & ~PAGE_SIZE_MASK);
				m_bytesAllocated = iBytesToAllocate;
			}

			m_bufferUsed = false;
		}
		

//		friend class AppComCacheReader;
		friend class iRTVDICOMGroup;

	private:
		// To Optimize load on the VP1000 we need to allocate  a chunk of memory
		// and have a pagealigned ptr with in that
		char* m_bufferUnaligned; 
		char* m_bufferPageAlignedPtr;
		bool  m_bufferUsed;
		int	  m_bytesAllocated;
        int   m_sliceNumber;
		int	  m_positonInAllSlicesVector;
		int   m_workingSliceNumber;
		int	  m_pixelRepresentation; // -- 2005.10.11

		int   m_numberOfBytesStored;
		
		iRTVSliceBuffer (iRTVSliceBuffer& );
		iRTVSliceBuffer& operator = (iRTVSliceBuffer& );

};

//-----------------------------------------------------------------------------
class iRTVDICOMGroup
{
	public:	
		iRTVDICOMGroup ();
		~iRTVDICOMGroup ();

		// Get the current Modality
		std::string& GetModality () {return m_modality;}

		// Get the current SOPClassUID
		std::string& GetSOPClassUID () {return m_sopClassUID;}
		
		// Is the data set uniformly spaced for CT/MR
		bool IsUniformlySpaced () {return m_isUniformlySpaced;}
		
		// Get the Model Matrix
		void GetModelMatrix (double oMatrix[16]) 
		{
			for (int i=0; i < 16; i++)
			{
				oMatrix[i] = m_modelMatrix[i];
			}
		}

		// Get the Scan Matrix - This is the matrix without scales
		void GetScanMatrix (double oMatrix[16]) 
		{
			for (int i=0; i < 16; i++)
			{
				oMatrix[i] = m_scanMatrix[i];
			}
		}

		// Vikram & TC 07/20/04
		// Always use the scan matrix to calculate the scan type
		int	GetScanType();

		int GetXSize ()		{return m_sizeX;}
		int GetYSize ()		{return m_sizeY;}
		int GetZSize ()		{return m_ptrToAllSlices.size();}

		float GetXScale(void) const { return m_xScale;}
		float GetYScale(void) const { return m_yScale;}
		float GetZScale(void) const { return m_zScale;}
		
		int GetGroupID () {return m_groupID;}

		// For XA only
		int GetReferencedGroupID () {return m_referencedGroupID;}
		
		std::string& GetSOPInstanceUID() {return m_sopInstanceUID;}
		std::string& GetReferencedSOPInstanceUID() {return m_referencedSOPInstanceUID;}

		int OpenDataFile (std::string& iLoadFile, int iBegin = -1, int iNumSlices = -1);
		int CloseDataFile ();

		int OpenCompressedDataFile (std::string& iCompressedFile);
		int CloseCompressedDataFile ();


		// -- 2006.06.11
		static void SetPETCorrect(int iYN)
		{
			m_correctPET = iYN;
		}

		
        int LoadNextSlice (iRTVSliceBuffer& iBuffer);
		int LoadNextCompressedSlice (iRTVSliceBuffer& iBuffer, float iCompressionRatio);

		// -- 2005.10.11
		// really reads the raw data - except for CT where data is in HU
		int ReadRawSlice (iRTVSliceBuffer& iBuffer, int iSliceNumber);

		// Vikram 09/23/03 Subtract Volumes
		// This function just reads the slice from the file without doing any post process
		int ReadSlice (iRTVSliceBuffer& iBuffer, int iSliceNumber);

		int LoadSlice (iRTVSliceBuffer& iBuffer, int iSliceNumber);
		int LoadSlice (iRTVSliceBuffer& iBuffer, iRTVSOPLocation& iSOPLocation);


		int GetBitsUsedPerPixel () {return m_bitsUsedPerPixel;}
		int GetBytesPerPixel () {return m_bytesPerPixel;}
		int GetBytesPerSlice () {return m_bytesPerSlice;}
		int	GetBitsAllocated () {return m_bitsAllocated;}

		// added by shiying
		int GetBitsStored () { return m_bitsStored; }
		int GetHighBit()	 { return m_highBit;  }
		float GetRescaleSlope() { return m_rescaleSlope; }
		float GetRescaleIntercept() { return m_rescaleIntercept; }

        // SC
		int GetSamplesPerPixel () {return m_samplesPerPixel;}
		int GetPhotometricInterpretation () {return m_photometricInterpretation;}
		int GetPlanarConfiguration () {return m_planarConfiguration;}

		// Only For XA
		std::string& GetPlaneType() {return m_planeType;}

		void GetImagePosition(int iSlice, double oIP[3]);
		double* GetImageOrientation(int iSlice);
		double*	GetPixelSpacing(int iSlice);
		
		// -- 2004.07.22
		bool HasValidOrientation(int iSlice=0) const;

		bool IsRotational(void); 
		bool BadOrientation(void);
		//-------------------------------------------------------------------------
		// Internal helper funcions
		void Clear ();
		void AddFirstToGroup(iRTVPtrToSliceInformation& iPSi, int iGroupID, eSortType iCurrentSortType);
		bool AddToGroup(iRTVPtrToSliceInformation& iPSi, bool iIgnoreGroupSpacing=false);

		// -- 2003-07-07 
		// Need a way to remove a slice for consolidation
		void AddToEraseList(int iSlice); 
		void EraseGoner(void);

		void SortAndProcess ();
		
		const char* GetImageTypeTokenAsOneString(void)
		{
			return 	m_imageTypeTokensAsOneString.c_str();
		}

		// -- support SUV measurement
		void		ProcessPETData(void);

		// -- 2005.03.16 PET Q/R
		void	PETConvert(unsigned short*, int);
		void	OldPETConvert(unsigned short*, int);

		friend class AppComCacheReader;
		friend class iRTVSLoadAndConvert;

		// -- 2003-07-09
		bool AddToGroupAlmost2D(iRTVPtrToSliceInformation& iPSi);

//		iRTVDICOMGroup& operator = (const iRTVDICOMGroup& iIn);
//		iRTVDICOMGroup (const iRTVDICOMGroup& iIn);
        const std::vector <iRTVPtrToSliceInformation>& GetPtrToAllSlices () const {return m_ptrToAllSlices;}
		const std::vector <iRTVPtrToSliceInformation>& GetPtrToWorkingSlices () const {return m_ptrToWorkingSlices;}
     
		int GetNumberOfWorkingSlices (){return m_ptrToWorkingSlices.size();}

		// TCZ 2004.03.31 need this to do valiation
		const std::vector<iRTVPtrToSliceInformation>& GetAllSlices(void) const
		{ 
			return m_ptrToWorkingSlices;
		}

		int BuildModelMatrix (int iStart, int iEnd);

		// Vikram 05/04/04 Fixing Linked Multi-Data load without cache
		int BuildModelMatrixForSort2D (int iStart, int iEnd);

		int GetCurrentMinVoxelValue () {return m_currentMinVoxelValue;}
		int GetCurrentMaxVoxelValue () {return m_currentMaxVoxelValue;}

		double GetSlope(void) ;

		double GetVOIWindowWidth ()  {return m_voiWindowWidth;}
		double GetVOIWindowCenter () {return m_voiWindowCenter;}

		int	GetSortedSliceInformation(std::map<int,LoadedSliceInformation>& oOut);

		/* -- JULY-14-2003 fix the VOI LUT mapping */
		float	GetMappingSlope(void)  const { return m_slope;}
		float	GetMappingOffset(void) const { return m_offset;}
		// END of -- JULY-14-2003

		// -- 2004.05.11 for FMS project
		bool	IsConsolidatedGroup(void) const { return m_isConsolidated;}
		void	SetIsConsolidated(bool iYN)		{ m_isConsolidated = iYN;}

		friend class iRTVSubSeriesSorter;

		// Zhao 2004.12.29 PET SUV support
		int			m_PETProcessed;
		float		m_clampedMin;
		float		m_clampedMax;
		float		m_averageSlope;
		float		m_averageIntercept;

		// using the clamped to convert back to [0,4095]
		static float	m_maxGML;
		static float	m_minGML;
		static float	m_maxBQML;
		static float	m_minBQML;
		float			m_remapScale;
		float			m_remapIntercept;
		float			m_averageVOICenter;
		float			m_averageVOIWidth;
		bool			UseNewPETDataConversion(void) const;
		std::string		m_PETUnits;
		int				m_useGMLConversion;
		// -- 2005.10.13
		// detect shear for axial ROI stuff. 
		bool HasShearOrRotation(void);

		// added by shiying hu, 2005-10-17
		// we could like to have model matrix, min, max information ready when
		// read cache description
		// if iBegin == -1 and iNumSlices == -1, then update information
		// for all slices
		int  UpdateGroupInfo (int iBegin = -1, int iNumSlices = -1);

		// Murali 2006.12.19  :Needed to pass on the sort type to client.
		eSortType GetCurrentSortType() const { return m_currentSortType; }

		// Murali 2007.01.03 Propagate the ScanOptions flag.
		void SetSortCTByScanOptionsFirst(int iYesNo) 
		{ 
			m_sortCTByScanOptionsFirst = iYesNo;
		}

		// Murali 2007.03.20 Use %R-R to sort siemens CT groups with ScanOptions set.
		static bool SortByPercentRR(const iRTVDICOMGroup& iGroup0, const iRTVDICOMGroup& iGroup1);

	private:

		HANDLE		m_srcFile;
		FILE*		m_compressedSrcFile;
		
		std::vector <iRTVPtrToSliceInformation> m_ptrToAllSlices;
		std::vector <iRTVPtrToSliceInformation> m_ptrToWorkingSlices;
		std::vector <std::string>               m_imageTypeTokens;

		typedef std::vector<iRTVPtrToSliceInformation> SSList;
		std::vector <int/*SSList::iterator*/> m_eraseList; // -- 2003-07-07

		float  m_modelMatrix[16];
		float  m_scanMatrix[16]; // This is the matrix without scales

		float  m_xScale, m_yScale, m_zScale; // -- (mar-01-02)

		bool   m_isUniformlySpaced;

		std::string m_modality;
		std::string m_sopClassUID;


		int m_sizeX;
		int m_sizeY;

		int m_groupID;

        eSortType m_currentSortType;

		int m_bitsAllocated;
		int m_bitsStored;
		int m_highBit;

		int m_bitsUsedPerPixel;
		int m_bytesPerSlice;
		int m_bytesPerPixel;

		int m_scanType;

		// FOR XA ONLY
		int m_referencedGroupID; 
		std::string m_sopInstanceUID;
		std::string m_referencedSOPInstanceUID;
		std::string m_planeType;
        
        // For SC
        int m_samplesPerPixel;
        int m_photometricInterpretation;
        int m_planarConfiguration;


		std::string m_imageTypeTokensAsOneString;
        bool m_sortInAscendingOrder;

        int m_currentSliceToLoad;
        int m_currentCompressedSliceToLoad;

		double m_spacing;
		double m_lastPosition;
		double m_planeNormal[3];
		double m_planeD;

		int m_currentMinVoxelValue;
		int m_currentMaxVoxelValue;

		// VOI Window/Level
		double m_voiWindowWidth;
		double m_voiWindowCenter;

		
		/* -- JULY-14-2003 fix the VOI LUT mapping */
		float	m_slope;
		float	m_offset;

		// Zhao 2004.4.5 ITEM fix 
		float	m_rescaleSlope;
		float	m_rescaleIntercept;

    
		bool	m_isConsolidated;

		// -- 2004.12.30 PET SUV
		void	OldPETConvert(iRTVSliceBuffer& iBuffer);
		void	PETConvert(iRTVSliceBuffer& iBuffer);


		bool m_imageIsDerived;
		void Initialize ();

		// Vikram 08/15/2006 - Fix for Gantry Tilted Data
		int  CalculateScalingAndUniformity (std::vector <iRTVPtrToSliceInformation>& iVectorOfSlices, bool iHasShear=false, float iShearCorrection=1.0);
		
		bool IsSubSeriesRotational (std::vector <iRTVPtrToSliceInformation>& iVectorOfSlices);

		// Vikram 08/15/2006 - Fix for Gantry Tilted Data
		bool HasShear (std::vector <iRTVPtrToSliceInformation>& iVectorOfSlices, float& oShearCorrection);

		int  RemoveShearAndNon90DegreeRotation ();
		void FixScaleOnLoadAxis (std::vector <iRTVPtrToSliceInformation>& iVectorOfSlices);
		int  ProcessLoadData(iRTVSliceBuffer& iBuffer, bool iPadOnly=false);

		// -- 2006.06.10
		static int m_correctPET;


		// Murali 2007.01.03
		std::string  m_scanOptions;
		int m_sortCTByScanOptionsFirst;

};

//-----------------------------------------------------------------------------
#endif