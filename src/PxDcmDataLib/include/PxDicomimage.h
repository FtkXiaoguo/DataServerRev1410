/***********************************************************************
 *  PxDicomImage.h
 *   
 *-------------------------------------------------------------------
 */

#ifndef FX_DICOM_IMAGE_H
#define FX_DICOM_IMAGE_H

#pragma warning (disable: 4786)
#pragma warning (disable: 4616)

#include <string>
#include <vector>

#include "PxDicomDict.h"
#include "PxDicomStatus.h"
#include "PxDicomMessage.h"
#include "TICache.h"


#include "PxDBData.h"

/////////////////////////////////////////////////////////////////////
 

class TRCriticalSection;
class TRSemaphore;
class PxDicomPalette;

class  CPxDicomImage : public CPxDicomMessage	
{
public:
	
	// Constructor for building an empty PxDicomImage object
	CPxDicomImage(void);										// Default Constructor

	// Constructor for building a PxDicomImageObject from a message
	explicit CPxDicomImage(int iID,int MsgLogout=1);//#91 2017/01/12 N.Furutsuki

	bool operator <  (const CPxDicomImage&);
	bool operator == (const CPxDicomImage&);

	virtual ~CPxDicomImage(void);

	virtual void Reset(void);

	//GL PopulateFromMessage did not reset tags before populate
	//only work for fresh created object
	void PopulateFromMessage(int iID);


	unsigned char*	GetImagePixels (void);  

	unsigned char*	GetFrame (int iFrameNumber);
	int				AddFrame (unsigned char* inPixels);
	int				RleaseFrame(unsigned char* inPixels);
	int				RleaseFrame(int index);

	unsigned short  GetNumberOfFrameBuffers();
	long int		GetFrameSizeInBytes(); 

    unsigned short GetBitsAllocated (void) {return m_bitsAllocated;}
    void           SetBitsAllocated (unsigned short in) {m_bitsAllocated = in;}

    unsigned short GetBitsStored (void) {return m_bitsStored;}
    void           SetBitsStored (unsigned short in) {m_bitsStored = in;}

    unsigned short GetHighBit (void) {return m_highBit;}
    void           SetHighBit (unsigned short in) {m_highBit = in;}

    unsigned short GetSamplesPerPixel (void) {return m_samplesPerPixel;}
    void           SetSamplesPerPixel (unsigned short in) {m_samplesPerPixel = in;}

    unsigned short GetBytesPerPixel (void) {return m_bytesPerPixel;}
    void           SetBytesPerPixel (unsigned short in) {m_bytesPerPixel = in;}

	unsigned short GetPhotometricInterpretation (void) {return m_photometricInterpretation;}
    void           SetPhotometricInterpretation (unsigned short in) {m_photometricInterpretation = in;}

	unsigned short GetPlanarConfiguration (void) {return m_planarConfiguration;}
    void           SetPlanarConfiguration (unsigned short in) {m_planarConfiguration = in;}
	
	long int  GetNumberOfBytesOfPixelData (void)  const {return m_numberOfBytesOfPixelData;}
	void SetNumberOfBytesOfPixelData (long int inBytes) {m_numberOfBytesOfPixelData = inBytes;}

	void GetPixelSpacing (double out[2]);
	double*	GetPixelSpacing(void) { return this->m_pixelSpacing;}
	void SetPixelSpacing (double in[2]);

 	eDicomScanType GetScanType (void) {return m_scanType;}
	void		   SetScanType (eDicomScanType inType);

	std::string    GetModalityStr (void) {return m_modalityStr;}
	void           SetModalityStr (std::string iM) {m_modalityStr = iM;};

	eDicomModality GetModality (void) {return m_modality;}
	void		   SetModality (eDicomModality inModality);
 
	void GetImagePosition (double out[3]);
	double*	GetImagePosition(void) { return m_imagePosition;}
	void SetImagePosition (double in[3]);

	eDicomPatientPosition GetPatientPosition (void) {return m_patientPosition;}
	void				  SetPatientPosition (eDicomPatientPosition inPosition);

	void GetAxisOfAcquisition (double out[3]);
 	void SetAxisOfAcquisition (double out[3]);

	// Dicom tag (0028,1050)
	double GetWindowCenter (void) {return m_windowCenter;}
    void  SetWindowCenter (double iWindowCenter) {m_windowCenter = iWindowCenter;}
    
	// Dicom tag (0028,1051)
	double GetWindowWidth (void) {return m_windowWidth;}
    void  SetWindowWidth (double iWindowWidth) {m_windowWidth = iWindowWidth;}
    
	// Dicom tag (0028,1052)
	double GetRescaleIntercept (void) {return m_rescaleIntercept;}
    void   SetRescaleIntercept (double inIntercept) {m_rescaleIntercept = inIntercept;}
    
	// Dicom tag (0028,1053)
	double GetRescaleSlope (void) {return m_rescaleSlope;}
    void   SetRescaleSlope (double inSlope) {m_rescaleSlope = inSlope;}

	unsigned short GetNumberOfRows (void) {return m_numberOfRows;}
    void		   SetNumberOfRows (unsigned short inRows) {m_numberOfRows = inRows;}
    
	unsigned short GetNumberOfColumns (void) {return m_numberOfColumns;}
    void		   SetNumberOfColumns (unsigned short inColumns) {m_numberOfColumns = inColumns;}

    unsigned short GetNumberOfFrames (void) {return m_numberOfFrames;}
    void		   SetNumberOfFrames (unsigned short inFrames) {m_numberOfFrames = inFrames;}

	unsigned short GetPixelRepresentation (void) {return m_pixelRepresentation;}
    void		   SetPixelRepresentation (unsigned short in) {m_pixelRepresentation = in;}
	double*		   GetImageOrientation(void) { return m_imageOrientation;}


	int			   GetLargestPixelValueInSeries(void) ;
	int			   GetSmallestPixelValueInSeries(void) ;
	int			   GetLargestPixelValue(void) ;
	int			   GetSmallestPixelValue(void) ;

	const char* GetStudyInstanceUID() { return m_studyInstanceUID;};
	const char* GetSeriesInstanceUID() { return m_seriesInstanceUID;};

	const char* GetSOPInstanceUID (void) {return m_SOPInstanceUID;}
	void        SetSOPInstanceUID (const char* iSOPUID) {ASTRNCPY(m_SOPInstanceUID, iSOPUID);}

	const char* GetSOPClassUID (void) {return m_SOPClassUID;}
	const char* GetReferencedSOPInstanceUID() { return m_referencedSOPInstanceUID;};
	


	std::vector <std::string>& GetImageTypeTokens (void) {return m_imageTypeTokens;}

	int GetImageStorageType(void) {return m_imageStorageType;}

	
	bool IsSecondaryCapture() { return m_isSecondaryCapture; }
//	bool IsDerived() { return m_isDerived; }
	bool IsCompressed() { return m_isCompressed; }
	bool IsLittleEndian() { return m_isLittleEndian; }			
	bool IsBiPlane() { return m_isBiPlane; }					

	//GL comment out, because it is in VLIDicomMessage 
	//int GetTransferSyntax() const { return m_transferSyntax; }

	int GetInstanceNumber() const { return m_instanceNumber; }


	void ReleaseImagePixels();

	void SetMinMaxVoxelValues (int iMin, int iMax)
	{
		m_sliceMinVoxelValue = iMin;
		m_sliceMaxVoxelValue = iMax;
	}

	int GetSliceMinVoxelValue () {return m_sliceMinVoxelValue;}
	int GetSliceMaxVoxelValue () {return m_sliceMaxVoxelValue;}

	void SetIsLittleEndian (bool iF) {m_isLittleEndian = iF;}

	int		GetScaledWidth(void) const		{ return m_scaledW;}
	int		GetScaledHeight(void) const		{ return m_scaledH;}
	int		IsAlreadyConverted(void) const	{ return m_converted;}
	void	SetScaledSize(int w, int h)		{ m_scaledW = w; m_scaledH = h;}
	void	SetIsConverted(int iYN)			{ m_converted = iYN;}


	void SetAutoConvert (int iYN)			{ m_autoConvert = iYN;}

	int WaitForConversionToComplete();

	void SetConversionComplete();

	//	Process callback data for extracting pixels - internal use only
	int HandoverPixelData(int dataSize,void* dataBufferPtr,int isFirst,int isLast);
	int ProcessPIXEL(unsigned long tag, int CBtype, unsigned long* dataSizePtr,void** dataBufferPtr,int isFirst,int* isLastPtr);

	//	
	// ---- CPxDicomMessage

	//GL load did not reset tags before load only work for fresh created object,
	//and it is not multi-thread safe, beacause MC_Register_Callback_Function for globle call back used
	PxDicomStatus Load(const char *iFilePath, int iHeaderOnly = 0);

	PxDicomStatus Save(const char* iSavePath, const char* iLocalAE = 0);

	PxDicomStatus RemoveZeroPixelPadding();

	PxDicomStatus ExtractTransferSyntax (int& oTransferSyntax, bool& oIsCompressed);

	unsigned long GetPixelDataSize(void) const { return m_OBOWlength; }

	PxDicomPalette* GetPalette(void) const { return m_palette;}

	// currently, haven't added m_pixelAspectRatio
	// as a member in PxDicomImage
	// Just use this function to extract information to build new cache.
	PxDicomStatus ExtractPixelAspectRatio     (double & oAspectRatio) const;

	bool HasValidImageOrientation(void) const { return m_hasValidImageOrientation; }
	bool HasValidImagePosition(void) const { return m_hasValidImagePosition; }

	PxDicomStatus  ConvertToFile(const char* iFilePath, const char* iLocalAE = 0);
	PxDicomStatus  FillSortInfo(DICOMData& oData, bool bFillPixelInfo=true);
	PxDicomStatus  GetSortInfo(const char* iFilePath, DICOMData& oData);
	
	double GetSliceThickness(void) const { return m_sliceThickness; }
	void   SetSliceThickness(double iThickness) { m_sliceThickness = iThickness; }

	const char*	GetImageDate(void) const	{ return m_imageDate;}
	const char* GetImageTime(void) const	{ return m_imageTime;}

	static void PopulateModalityMap(void);
	static int ConvertModalityFromStringToEnum(const char* iModality);
	static std::string ConvertModalityFromEnumToString(int iModality);
	static void PopulateSOPClassMap(void);

	//  although introduced for PET, this one might be of general interest
	const char* GetAcquisitionTime(void) const { return m_acquisitionTime;}

	// - PET SUV related
	const char* GetPETUnits(void)		const { return m_PETUnits;}
	double		GetPETDecayFactor(void)	const { return m_PETDecayFactor;}
	int         GetRadionuclideTotalDose(void)   const { return m_radionuclideTotalDose;}

	int			GetRadionuclideHalfLife(void) const { return m_radionuclideHalfLife;}
	const char* GetRadiopharmaceuticalStartTime(void) const { return m_radioPharmaStartTime;}
	double		GetPatientWeight(void) const	{ return m_patientWeight;}

	// 
	const char*		GetScanOptions(void) const { return m_scanOptions; }
	const char*     GetManufacturer(void) const{return m_manufacturer; }


protected:
	enum eAbortStatus
	{
		// This is local to here
		kContinue      = 10000,
		kDoNotContinue = 10001
	};

	int m_imageStorageType;

	PxDicomStatus DetermineImageStorageType ();
	PxDicomStatus ExtractImageTypeTokens  (std::vector <std::string>& oTokens, int& oNumber);
	PxDicomStatus ExtractImageDimensions(unsigned short& oNumberOfRows, unsigned short& oNumberOfColumns, 
										  unsigned short& oNumberOfFrames);
	PxDicomStatus ExtractPixelCellInfo	   (unsigned short& oBitsAllocated, unsigned short& oBitsStored, 
											unsigned short& oHighBit, unsigned short& oPixelRepresentation,
											bool ibLogging);
	PxDicomStatus ExtractPixelSpacing     (double oSpacing[2]);


	PxDicomStatus ExtractImageOrientation (double oOrientation[6]);
	PxDicomStatus ExtractImagePosition    (double oPosition[3]);

	PxDicomStatus ExtractSlopeIntercept   (double& oSlope, double& oIntercept);
	PxDicomStatus ExtractVOILut		   (double& oWindowCenter, double& oWindowWidth);
	PxDicomStatus ExtractSamplesPerPixel  (unsigned short& oSamplesPerPixel, unsigned short& oPlanarConfiguration);
	PxDicomStatus ExtractPhotometricInterpretation (int& oPhotometricInterpretation);

	PxDicomStatus ExtractReferencedSOPInstanceUID(bool& oIsBiPlane);	// -- - 03/26/02
	PxDicomStatus ExtractPixelData(std::vector<unsigned char*>& oImagePixels);
	PxDicomStatus ExtractPalette(PxDicomPalette& ioPalette);
	PxDicomStatus ExtractSliceThickness(double& oThickness);

	std::vector<unsigned char*> m_imagePixels;

	eDicomScanType m_scanType;

	eDicomModality m_modality;
	std::string m_modalityStr;

	void ConvertPatientPosition(char* iPosition);
	eDicomPatientPosition m_patientPosition;

    // Axis of Acquisition - should also contain the gantry tilt info
    double m_axisOfAcquisition [3];
    
    // Image position XYZ coordinate of the upper left pixel of the image
	double m_imagePosition[3];

	// Image orientation specified by unit vectors for column and row
	double m_imageOrientation[6];

    // In-plane pixel spacing
 	double m_pixelSpacing[2];
	long int m_numberOfBytesOfPixelData;

	//	VOI LUT 
	// changed to double (was shorts)
	double m_windowCenter;				//	(0028, 1050)
	double m_windowWidth;				//	(0028, 1051)

	//	MODALITY LUT
    double m_rescaleIntercept;			//	(0028, 1052)
	double m_rescaleSlope;				//	(0082, 1053)

	//	Image dimensions
    unsigned short m_numberOfColumns;
	unsigned short m_numberOfRows;
	unsigned short m_numberOfFrames;

    // DICOM TAG (0028, 0103)
    // 0000H = unsigned integer
    // 0001H = 2's compliment number
    //
	unsigned short m_pixelRepresentation;

	unsigned short m_bitsAllocated;
	unsigned short m_bitsStored;
	unsigned short m_highBit;

	unsigned short m_bytesPerPixel;
	unsigned short m_samplesPerPixel;
	
	//	MONOCHROME1		= 1 sample per pixel  - grayscale from white to black
	//	MONOCHROME2		= 1 sample per pixel  - grayscale from black to white
	//	PALETTE COLOR	= 1 sample per pixel  - 1 image plane with 3 LUTs
	//	RGB				= 3 samples per pixel - 3 separate image planes
	int m_photometricInterpretation;

	//	Only set if samplesPerPixel > 1
	//
	//	000	= R1 G1 B1, R2 G2 B2, R3 G3 B3
	//	001	= R1 R2 R3, G1 G2 G3, B1 B2 B3
	unsigned short m_planarConfiguration;

	// Added by
	// This is to enable the creation of Cache in Memory
	std::vector <std::string> m_imageTypeTokens;

	// 
	// Added to enable proper handling of secondary capture images
	char m_studyInstanceUID[ kVR_UI ];
	char m_seriesInstanceUID[ kVR_UI ];
	char m_SOPInstanceUID[ kVR_UI ];				
	char m_SOPClassUID[ kVR_UI ];				
	char m_referencedSOPInstanceUID[ kVR_UI ];

	bool m_isSecondaryCapture;
//	bool m_isDerived;
	bool m_isCompressed;
	bool m_isBiPlane;

	// 
//	unsigned short m_instanceNumber;
	//  We need the instanceNumber to be int, per DICOM standard
	int			m_instanceNumber;

	// 
	//GL comment out, because it is in CPxDicomMessage 
	//int m_transferSyntax;

	// 
	bool m_isLittleEndian;
	

	// 
	int m_sliceMinVoxelValue;
	int m_sliceMaxVoxelValue;

	TRCriticalSection *m_cs;

	//	For callback data to extract pixels
	unsigned char*	m_OBOWbuffer;
	unsigned long	m_OBOWoffset;
	unsigned long	m_OBOWlength;

	// 
	int				m_scaledW;
	int				m_scaledH;
	int				m_converted;

	//	
	bool		 	m_isConverting;
TRSemaphore		*m_conversionComplete;
	int				m_autoConvert;

	
	// 
	PxDicomPalette*	m_palette;

	bool m_hasValidImageOrientation;
	bool m_hasValidImagePosition;

	std::string m_filePath;

	char		m_imageDate[32];
	char		m_imageTime[32];

	//	
	double			m_sliceThickness;

	// 
	char			m_PETUnits[32];
	double			m_PETDecayFactor;
	int				m_radionuclideTotalDose;
	char			m_acquisitionTime[32];
	char			m_radioPharmaStartTime[32];
	int				m_radionuclideHalfLife;
	double			m_patientWeight;

	// 
	char		m_scanOptions[20];
	char		m_manufacturer[66];

	int m_msgLogoutInvalidUIDFlag;//#91 2017/01/12 N.Furutsuki
private:
	bool   createMyResource();
	//GL	operator= has memory leak, hide it now
	CPxDicomImage& operator= (const CPxDicomImage& iImage);		// = operator
	//GL not completed
	CPxDicomImage(const CPxDicomImage& iImage);					// Copy Constructor

};

//------------------------------------------------------------------
// 
// We need to handle Palette_color images
class  PxDicomPalette
{
public:
	enum { kMaxComp = 3}; // 0,1,2 means Red, Green, Blue
	PxDicomPalette(void)
	{
		Init();
	}

	PxDicomPalette(const PxDicomPalette& iP)
	{
		Init();

		for ( int i = 0; i < kMaxComp; ++i)
		{
			SetProperty(i, iP.m_entryCount[i],iP.m_mapmin[i],iP.m_precision[i]);
			if (OK())
			{
				if (iP.m_entryCount[i] && iP.m_entry[i])
					memcpy(m_entry[i], iP.m_entry[i],GetDataLength(i));
			}
		}
	}

	virtual ~PxDicomPalette(void)
	{
		for ( int i = 0; i < kMaxComp; ++i)
		{	
			if (m_entry[i] && m_entryCount[i])
				delete m_entry[i], m_entry[i] = 0;
			m_entryCount[i] = 0;
			m_mapmin[i] = 0;
			m_precision[i] = 0;
		}
	}

	bool OK(void) const { return m_status == 0;}

	void	SetProperty(int iIndex, int iCount,  int iMin,  int iPrec);
	void	GetProperty(int iIndex, int& oCount, int& oMin, int& oPrec);

	int		ConvertPrecision(int toBits);
	void	SetData(int iIndex, unsigned char*  iData,	int iStart, int iNum);
	void	SetData(int iIndex, unsigned short* iData,	int iStart, int iNum);

	int		Lookup(int iPixels, unsigned char& oRed, unsigned char& oGreen, unsigned char &oBlue);
	int		Lookup(int iPixels, unsigned short& oRed, unsigned short& oGreen, unsigned short &oBlue);

	
	int		GetDataLength(int iIndex)
	{
		return (iIndex<0 || iIndex>kMaxComp) ? 0:(m_entryCount[iIndex] * sizeof *m_entry[iIndex]);
	}

	unsigned short* GetData(int iIndex) // not very safe. user needs to take care
	{
		return (iIndex >=0 && iIndex < kMaxComp) ? m_entry[iIndex]:0;
	}

protected:

	void Init(void)
	{
		for ( int i = 0; i < kMaxComp; ++i)
		{
			m_entryCount[i] = 0;
			m_mapmin[i] = 0;
			m_precision[i] = 0;
			m_entry[i] = 0;
		}

		m_uid[0] = '\0';
		m_status = 0;
	}

	int				m_status;
	char			m_uid[68];			// not currently used
	int				m_entryCount[3];
	int				m_mapmin[3];		// if value less than min, map to entry at position 0
	int				m_precision[3];		// number of bits in color specification
	unsigned short*	m_entry[3];
};


#endif // FX_DICOM_IMAGE_H