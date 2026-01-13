/***********************************************************************
 * RTVSliceInformation.h
 *---------------------------------------------------------------------
 *		Copyright, PreXion 2012, All rights reserved.
 *
 *	PURPOSE:
 *		This class is used to create the Terarecon Cache for incoming 
 *		DICOM Series.
 *
 *	
 *   
 *-------------------------------------------------------------------
 */

//-----------------------------------------------------------------------------
#if !defined(RTVSLICEINFORMATION_H)
#define RTVSLICEINFORMATION_H

#include <string>
#include <vector>
//-----------------------------------------------------------------------------

// -- 2006.06.10 For PET, we may need many things. Use a structure for these
// Eventually we may need one of these for each modality
struct tPETAttrib
{
	tPETAttrib(void) 
	{
		Reset();
	}


	void Reset(void)
	{
		m_totalDose   = 0;
		m_decayFactor = 0.0;
		m_halfLife = 0;
		m_units = "";
		m_startTime = "";
		m_acquisitionTime = "";
		m_patientWeight = 0;
		m_patientSize = 0;
	}

	double CalculateDecayCorrection(void);

	int			m_totalDose;
	double		m_decayFactor;
	int			m_halfLife;
	double		m_patientWeight;
	double		m_patientSize;
	std::string	m_units;
	std::string	m_startTime;
	std::string	m_acquisitionTime;
};
// END -- 2006.06.11
	//-----------------------------------------------------------------------------
// This is a class that has in it the SOPInstanceUID and ImagePosition
// of the slice loaded on to VolumePro. This is used in a Map which maps
// the  sliceNumber (as loaded on VolumePro) to this class so we can pick out and
// send the right patient position when syncing linked cursors with GE.
// Vikram 04/30/03



//-----------------------------------------------------------------------------
// This class is only used internally.
//
class iRTVSliceInformation
{
	public:
		iRTVSliceInformation (){};
		~iRTVSliceInformation () {};

		void Initialize ();

		// Just define the Copy constructor and assignment operatio
		//
//		iRTVSliceInformation (const iRTVSliceInformation&);
//		iRTVSliceInformation& operator = (const iRTVSliceInformation&);


		friend class	AQNetCacheFromMemory;
		friend class	AQNetSortUsingDB;
		friend class	AppComCacheReader;
		friend class	iRTVPtrToSliceInformation;
		friend class	iRTVDICOMGroup;
		friend class	iRTVSLoadAndConvert;
		friend class	LoadedSliceInformation;

		int				GetImageNumber(void)	const { return m_imageNumber;}
		int				GetSliceNumber(void)	const { return m_sliceNumber;}

		int				GetPositonInAllSlicesVector(void)	const { return m_positonInAllSlicesVector;}

		const double*	GetImagePosition(void)  const { return m_imagePosition;}
		const double*	GetImageOrientation(void) const { return m_imageOrientation;}
		const double*	GetPixelSpacing(void) const { return m_pixelSpacing;}
		const char*		GetSOPInstanceUID(void) const { return m_sopInstanceUID.c_str();}
 		const char*		GetSOPClassUID(void) const { return m_sopClassUID.c_str();}
        
        int             GetScanType (void) const {return m_scanType;}
        
		int				GetSliceMinVoxelValue ()const { return m_sliceMinVoxelValue;}
		int				GetSliceMaxVoxelValue () const{ return m_sliceMaxVoxelValue;}

		int				GetGlobalMinVoxelValue ()const { return m_globalMinVoxelValue;}
		int				GetGlobalMaxVoxelValue ()const { return m_globalMaxVoxelValue;}

		int  GetSizeX ()  const { return m_sizeX;}
		int	 GetSizeY ()  const { return m_sizeY;}
		int	 GetBitsAllocated () const { return m_bitsAllocated;}
		double GetSlopeFactor () const  { return m_rescaleSlope;}
		double GetInterceptValue () const  { return m_rescaleIntercept;}

		__int64  GetStartOfData ()const {return m_startOfDataInDataFile;}
		__int64  GetSizeOfData  () const { return m_sizeOfData;}
 
		double GetCompressionFactor ()const {return m_compressionFactor;}
		int    GetInputL0CompressionSizeX () const{return m_inputL0CompressionSizeX;}
		int    GetInputL0CompressionSizeY () const{return m_inputL0CompressionSizeY;}

        void DetermineScanType();
		
		double GetVOIWindowWidth ()  const {return m_voiWindowWidth;}
		double GetVOIWindowCenter () const {return m_voiWindowCenter;}

		double GetAxialPosition(void) const ;  // -- 2003-07-07

		int	   GetPixelRepresentation(void) const // -- 2005.10.11
		{
			return m_pixelRepresentation;
		}


		// Vikram 07/22/04 - NM Support
		int GetDetectorIndex		(void) {return m_detector;}
		int GetRotationIndex		(void) {return m_rotation;}
		int GetEneryWindowIndex		(void) {return m_energyWindow;}
		int GetPhaseIndex			(void) {return m_phase;}
		int GetRRIntervalIndex		(void) {return m_rrInterval;}
		int GetTimeSlotIndex		(void) {return m_timeSlot;}
		int GetSliceIndex			(void) {return m_sliceIndex;}
		int GetAngularViewIndex		(void) {return m_angularView;}
		int GetTimeSliceIndex		(void) {return m_timeSlice;}
		int GetHasValidOrientation	(void) {return m_hasValidOrientation;}
		int GetIsRotational			(void) {return m_isRotational;}

		friend class iRTVSubSeriesSorter;
		friend class iRTVSCacheReader;
		friend class iRTVSWorkProc;
		friend class iRTVSImage;
		friend class iRTVSRenderer;
		friend class iRTVPtrToSliceInformation;

		// Vikram 08/30/04 Making tolerance for sorting configurable
		static void   SetImagePostionTolerance (double iT) {m_imagePositionTolerance = iT;}
		static double GetImagePostionTolerance (void)      {return m_imagePositionTolerance;}

		// -- 2006.05.06
		const char*		GetImageDate(void) const	{ return m_imageDate.c_str();}
		const char*		GetImageTime(void) const	{ return m_imageTime.c_str();}

		//Murali 2007.01.03
		const char*		GetScanOptions(void) const	{ return m_scanOptions.c_str();}
		const char*	    GetManufacturer(void)const	{ return m_manufacturer.c_str();}


		tPETAttrib		m_PETAttrib;

	protected:

		int	   m_scanType;
		double m_imagePosition[3];
		double m_imageOrientation[6];
		double m_pixelSpacing[2];
		int	   m_imageNumber; // Only used for Tri-phase CT


		double	m_rescaleIntercept;
		double	m_rescaleSlope;
		int		m_used1024;

		int	   m_sizeX; 
		int    m_sizeY;

		int m_bitsAllocated;
		int m_bitsStored;
		int m_highBit;

		// Vikram 09/05/02 - Needed to handle signed data
		int m_pixelRepresentation;
		int m_sliceMinVoxelValue;
		int m_sliceMaxVoxelValue;

		int m_globalMinVoxelValue;
		int m_globalMaxVoxelValue;


		__int64  m_startOfDataInDataFile;
		__int64  m_sizeOfData;

        int m_startOfL0CompressedDataInDataFile;
		int m_sizeOfL0CompressedData;

		double m_compressionFactor;
		int	   m_inputL0CompressionSizeX;
		int	   m_inputL0CompressionSizeY;

       // This is for loading purposes
        int m_sliceNumber;

       // This is the position in All Slicec
        int m_positonInAllSlicesVector;

		std::string   m_modality;
        std::string	  m_sopClassUID;
		std::string   m_sopInstanceUID;
		std::string   m_referencedSOPInstanceUID;
		std::string	  m_version;

        // SC Image Related
        int m_samplesPerPixel;
        int m_photometricInterpretation;
        int m_planarConfiguration;

		std::vector <std::string> m_imageTypeTokens;
        std::string m_imageTypeTokensAsOneString;

		// Histogram information
		// For right now we only store the first 12 bits

		// -- 2006.07.18
		// For large datasets, histogram takes too much memory. Disable it.
		// Besides, we don't really use it anyway, and if necessary, we can
		// always regenerate using the voxels on volumePro
		int    m_numberOfEntriesInHistogram;
		unsigned int m_histogram[1];

        // This is used when we build Cache Description in memory
        int m_uniqueIndex;
        bool m_sortInAscendingOrder;


		// VOI LUT Support
		// Vikram 11/06/02

		// tcz 2005.12.05 changed to doubles
		double m_voiWindowWidth;
		double m_voiWindowCenter;


		// -- 2003-03-18
		int	m_useModalityLUT;
        void MakeImageTypeTokensIntoOneString ();

		// -- 2003-07-07
		double	m_axialPosition; // position along the axis of acquisition;

		// -- 2003-07-09 Need to track the position in vector as imageNumber
		// is not reliable
		int		m_sequence;

		// -- 2003-22-2003 scaled version for compressed cache
		double	m_sVOIWindowWidth;
		double  m_sVOIWindowCenter;

		int	m_hasPixelSpacing; // mostly for CR/DR
		int	m_hasWindowLevel;


		// Vikram 07/22/04 - NM Support
		int m_detector;
		int m_rotation;
		int m_energyWindow;
		int m_phase;
		int m_rrInterval;
		int m_timeSlot;
		int m_sliceIndex;
		int m_angularView;
		int m_timeSlice;
		int m_hasValidOrientation;
		int m_isRotational;

		// -- 2004.10.22
		float	m_sliceThickness;
		int		m_hasSliceThickness;
		static double m_imagePositionTolerance;    // Vikram 08/30/04 The amount by which image position can vary by

		float	m_sliceLocation; // -- 2004.10.22

		// -- 2006.05.06
		std::string		m_imageDate;
		std::string		m_imageTime;
		int				m_hasImageDateTime;

		//Murali 2007.01.03
		std::string	    m_scanOptions;
		std::string	    m_manufacturer;  

};

//-----------------------------------------------------------------------------
// This is place holder class that is inserted into a list so 
// everything n the slice infomation is not copied twice
// This class does not have a copy constructor or assignment operator
// as we want only the address of m_ptr to be copied
// Vikram 
//
class iRTVPtrToSliceInformation
{
	public:
		explicit iRTVPtrToSliceInformation (iRTVSliceInformation* iPtr)
		{
			m_ptr = iPtr;
		}

		iRTVPtrToSliceInformation ()
		{
			m_ptr = 0;
		}

		~iRTVPtrToSliceInformation ()
		{

			// The pointer is not deleted in the desctuctor intentionally
			// This is done to over come the deletion that occurs when you 
			// insert into a vector - Vikram
			//
			//if (m_ptr) delete m_ptr;
		}

		// This is called explicity outside
		//
		void DeleteData()
		{
			if (m_ptr) delete m_ptr, m_ptr=0;
		}

	    // To access the pointer directly
	    //
	    friend class AQNetCacheFromMemory;
		friend class AQNetSortUsingDB;
        friend class AppComCacheReader;
	    friend class iRTVDICOMGroup;
		friend class iRTVSCacheReader;
		friend class iRTVSubSeriesSorter;
		friend class iRTVSWorkProc;
		friend class iRTVSRenderer;

        static bool SortByImageNumber (const iRTVPtrToSliceInformation& iPSi0, 
                                       const iRTVPtrToSliceInformation& iPSi1);

        static bool SortByImageNumberSOP (const iRTVPtrToSliceInformation& iPSi0, 
                                       const iRTVPtrToSliceInformation& iPSi1);

        static bool SortByPositionInCache (const iRTVPtrToSliceInformation& iPSi0, 
                                           const iRTVPtrToSliceInformation& iPSi1);

        static bool SortByImagePosition (const iRTVPtrToSliceInformation& iPSi0, 
                                         const iRTVPtrToSliceInformation& iPSi1);

        static bool SortBySOPInstanceUID (const iRTVPtrToSliceInformation& iPSi0, 
                                          const iRTVPtrToSliceInformation& iPSi1);
		
		// Murali 2007.01.03
		static bool SortByScanOptions (const iRTVPtrToSliceInformation& iPSi0, 
										const iRTVPtrToSliceInformation& iPSi1);

		// Vikram 11/16/2005 Cox Health Fix
        static bool SortByScanType (const iRTVPtrToSliceInformation& iPSi0, 
                                    const iRTVPtrToSliceInformation& iPSi1);

		bool operator < (const iRTVPtrToSliceInformation&) const;
	    bool operator == (const iRTVPtrToSliceInformation&) const;

		/* -- 07/10/2002 */
		int				GetImageNumber(void)	const { return m_ptr ? m_ptr->GetImageNumber():-1;}
		int				GetSliceNumber(void)	const { return m_ptr ? m_ptr->GetSliceNumber():-1;}
		int				GetPositonInAllSlicesVector(void)	const { return m_ptr ? m_ptr->GetPositonInAllSlicesVector():-1;}
		
		const double*	GetImagePosition(void)  const { return m_ptr ? m_ptr->GetImagePosition():0;}
		const double*	GetImageOrientation(void) const { return m_ptr ? m_ptr->GetImageOrientation():0;}	
		const double*	GetPixelSpacing(void) const { return m_ptr ? m_ptr->GetPixelSpacing() :0;}
		const char*		GetSOPInstanceUID(void) const { return m_ptr ? m_ptr->GetSOPInstanceUID():"";}
		const char*		GetSOPClassUID(void)    const { return m_ptr ? m_ptr->GetSOPClassUID():"";}
		
		int				GetScanType(void)		const { return m_ptr ? m_ptr->GetScanType():0;}
		
		int				GetSliceMinVoxelValue(void)		const { return m_ptr ? m_ptr->GetSliceMinVoxelValue():0;}
		int				GetSliceMaxVoxelValue(void)		const { return m_ptr ? m_ptr->GetSliceMaxVoxelValue():0;}

		int				GetGlobalMinVoxelValue(void)		const { return m_ptr ? m_ptr->GetGlobalMinVoxelValue():0;}
		int				GetGlobalMaxVoxelValue(void)		const { return m_ptr ? m_ptr->GetGlobalMaxVoxelValue():0;}

		
		double GetCompressionFactor ()     const {return m_ptr ? m_ptr->GetCompressionFactor():1.0;}
		int    GetInputL0CompressionSizeX () const {return m_ptr ? m_ptr->GetInputL0CompressionSizeX():0;}
		int    GetInputL0CompressionSizeY () const {return m_ptr ? m_ptr->GetInputL0CompressionSizeY():0;}

		/* END -- modification*/


		int  GetSizeX ()      const     { return m_ptr ? m_ptr->GetSizeX():0;}
		int	 GetSizeY ()       const    { return m_ptr ? m_ptr->GetSizeY():0;}
		int	 GetBitsAllocated () const  { return m_ptr ? m_ptr->GetBitsAllocated():0;}
		double GetSlopeFactor () const  { return m_ptr ? m_ptr->GetSlopeFactor():0;}
		double GetInterceptValue () const  { return m_ptr ? m_ptr->GetInterceptValue():0;}

		__int64  GetStartOfData () { return m_ptr ? m_ptr->GetStartOfData():0;}
		__int64  GetSizeOfData  () { return m_ptr ? m_ptr->GetSizeOfData():0;}
 

		double GetVOIWindowWidth ()  const {return m_ptr ? m_ptr->GetVOIWindowWidth():0;}
		double GetVOIWindowCenter () const {return m_ptr ? m_ptr->GetVOIWindowCenter():0;}

		// -- JULY-2003
		double GetAxialPosition(void)  const { return m_ptr ? m_ptr->GetAxialPosition():0;}
		static bool SortBySequenceNumber (const iRTVPtrToSliceInformation& iPSi0, 
                                            const iRTVPtrToSliceInformation& iPSi1);
		int		GetSequenceNumber(void) const { return m_ptr->m_sequence;}

		double		GetScaledVOIWindowCenter(void) const { return m_ptr ?m_ptr->m_sVOIWindowCenter:0;}
		double		GetScaledVOIWindowWidth(void) const  { return m_ptr ?m_ptr->m_sVOIWindowWidth:0;}
//		friend class	iRTVSSubSeriesSorter;

		const iRTVSliceInformation* GetSliceInfo(void) const { return m_ptr;}

		// -- 2005.10.11
		int	  GetPixelRepresentation(void) const
		{
			return m_ptr ? m_ptr->GetPixelRepresentation():0;
		}

		// -- 2004.11.15
		// quickly turn iRTVSliceInformation into SOPUID
		operator std::string& () { return m_ptr->m_sopInstanceUID;}
	protected:
		iRTVSliceInformation*	m_ptr;
         
};

//--------------------------------------------------------------
// each volume created will have a vector of these so we can
// find out exactly what kind slice in the volume. MAY,2003 --
class LoadedSliceInformation
{
	public:
		LoadedSliceInformation ()
		{
			memset(m_orientation,	0, sizeof m_orientation);
			memset(m_imagePosition, 0, sizeof m_imagePosition);
			m_pixelSpacing[0] = 1;
			m_pixelSpacing[1] = 1;
			m_slope = 1.0;
		}

		// -- 06/16/2003 Encapsulate fields from slice to loaded slice. 
		LoadedSliceInformation(const iRTVSliceInformation& iS)
		{
			*this = iS;
		}

		~LoadedSliceInformation ()
		{
		}



		template <class T> int Round(T f)
		{
			return int( f > 0.0f ? (f+0.5f):(f-0.5f) );
		}


		LoadedSliceInformation& operator=(const iRTVSliceInformation& iS)
		{
			const double *d;
			m_sopInstanceUID = iS.GetSOPInstanceUID();
			
			if ((d = iS.GetImageOrientation()))
			{
				m_orientation[0] = *d++;
				m_orientation[1] = *d++;
				m_orientation[2] = *d++;
				m_orientation[3] = *d++;
				m_orientation[4] = *d++;
				m_orientation[5] = *d++;
			}
			else
			{
				memset(m_orientation, 0, sizeof m_orientation);
				m_orientation[0] = m_orientation[4] = 1.0;
			}
			
			if ((d = iS.GetImagePosition()))
			{
				m_imagePosition[0] = *d++;
				m_imagePosition[1] = *d++;
				m_imagePosition[2] = *d++;
			}
			
			if ((d = iS.GetPixelSpacing()))
			{
				m_pixelSpacing[0] = d[0];
				m_pixelSpacing[1] = d[1];
			}
			
			m_VOILevel = Round(iS.GetVOIWindowCenter());
			m_VOIWidth = Round(iS.GetVOIWindowWidth());
			
			m_instanceNumber = iS.GetImageNumber();	

			m_sliceThickness = iS.m_sliceThickness;

			m_imageDate = iS.GetImageDate();
			m_imageTime	= iS.GetImageTime();

			m_slope = iS.GetSlopeFactor();

			m_scanOptions	= iS.GetScanOptions(); 
			m_manufacturer	= iS.GetManufacturer();
		
			return *this;
			
		}

		/* -- 06/16/2003 : more fields for 2D support */
		double			m_imagePosition[3];
		double			m_orientation[6];
		int				m_instanceNumber;
		double			m_pixelSpacing[2];
		int				m_VOILevel;
		int				m_VOIWidth;
		float			m_sliceThickness;
		std::string		m_sopInstanceUID;

		// -- 2006.05.05
		std::string		m_imageDate;
		std::string		m_imageTime;

		// -- 2006.06.27
		double			m_slope;

		// Murali 2007.01.03
		std::string		m_scanOptions;
		std::string		m_manufacturer;
};

#include <map>
typedef std::map<int,LoadedSliceInformation> tLoadedSlice;

//-----------------------------------------------------------------------------
#endif //RTVSLICEINFORMATION_H