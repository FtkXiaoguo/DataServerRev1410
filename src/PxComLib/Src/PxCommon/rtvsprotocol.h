/***********************************************************************
 *  rtvsprotocol.h
 ***********************************************************************
 *
 *	Copyright (c), TeraRecon, Inc., 2001-2002. All rights reserved.
 *
 *	PURPOSE:
 *		NVR Protocol Definition. 
 *
 *      Care must be taken to ensure that all members in the structure
 *      are aligned with their natural size, i.e., an int must start
 *      on a 4bytes boundary from the begining of the structure and the
 *      size of the structure are multiple of 4bytes to ensure platform
 *      portability.
 *
 *	AUTHOR(S):  T.C. Zhao, June 2000 (VPNET), April 2001 (AQNET)
 *
 */

#ifndef RTVSPROTOCOL_H_
#define RTVSPROTOCOL_H_


/********************************************************************
 * Protocol Version: must update the version number if either
 * the major, minor or data structure changes.
 ********************************************************************/

////DataServerPX k.ko 2010/02/16
#if defined(PXADMIN_SERVER) || defined(PXADMIN_CLIENT) 
#define APP_Protocal_GrpID  100
#else
#define APP_Protocal_GrpID  0
#endif

#define kpRTVSProtocolVersion	(APP_Protocal_GrpID+1)
#define kpRTVSProtocolRevision	(APP_Protocal_GrpID+14)
#define	kpRTVSProtocolRelease	(APP_Protocal_GrpID+1)


// some limits
#define kpRTVSMaxNameLength			(256+64)		/* must be longer than 3*UID + patient Name */
#define kpRTVSMaxLUTEntries			4096
#define kpRTVSMaxVoxFields			4
#define kpRTVSMaxImageBuffers		32
#define kpRTVSMaxLights				1024 
#define	kpRTVSMaxUserName			(20+1)			/* 20 + null */
#define kpRTVSMaxUserNameStore		(4*((kpRTVSMaxUserName+3)/4))
#define kpRTVSMaxUserGroupStore		(64)
#define kpRTVSMaxUserDomainStore	68
#define kpRTVSMaxSectionNameLength	32

/* session ID: Sn-xxx.xxx.xxx.xxx-port-client where Sn is a number upto 5 digits */
#define kpRTVSMaxSessionIDLength 48		

#ifndef RTVSTYPES
#define RTVSTYPES
typedef unsigned int	RTVSuint32;
typedef unsigned short	RTVSuint16;
typedef short			RTVSint16;
typedef int				RTVSint32;
typedef float			RTVSfloat32;
typedef double			RTVSfloat64;
typedef	unsigned char	RTVSuint8;
#endif

typedef unsigned __int64	RTVSuint64;
#define uint8				RTVSuint8
#define uint16				RTVSuint16
#define int16				RTVSint16
#define uint32				RTVSuint32
#define int32				RTVSint32
#define float32				RTVSfloat32
#define float64				RTVSfloat64

typedef RTVSuint64          uint64;
#define kpRTVSMaxDateLength			12		// yyyy.mm.dd

/* for medical visualization */
#define	kpRTVSMaxDICOMPNameLength		(64+32)	// standard says 64*6
#define	kpRTVSMaxDICOMNameLength		64		 // other dicom names
#define kpRTVSMaxDICOMDateLength		12		// yyyymmdd
#define kpRTVSMaxDICOMTimeLength		(16+4)	// hhmmss.frac
#define	kpRTVSMaxDICOMDateRangeLength	20		// yyyymmdd-yyyymmdd
#define kpRTVSMaxDICOMUIDLength			(64+4)	// including the null
#define	kpRTVSMaxDICOMNumberLength		12		// 2^32-1
#define kpRTVSMaxDICOMSNumLength		 8		// 2^16-1
#define kpRTVSMaxIPAddress				16		// xxx.xxx.xxx.xxx
#define kpRTVSMaxAETitle				20		// standard says 16
#define kpRTVSMaxFilePath				256

#define	kpRTVSMaxDICOMTagResponse	kpRTVSMaxDICOMNameLength

typedef char	DICOMDate[kpRTVSMaxDICOMDateLength];
typedef char	DICOMTime[kpRTVSMaxDICOMTimeLength];
typedef char	DICOMPName[kpRTVSMaxDICOMPNameLength]; // patient name
typedef char	DICOMName[kpRTVSMaxDICOMNameLength];
typedef char	DICOMDateRange[kpRTVSMaxDICOMDateRangeLength];

typedef char	DICOMUID[kpRTVSMaxDICOMUIDLength];
typedef char	DICOMNUM[kpRTVSMaxDICOMNumberLength];
typedef char	DICOMSNUM[kpRTVSMaxDICOMSNumLength];

/* 
 *  The Major number of the protocols
 */
enum
{
	RTVS_INVALID	= -1,
	RTVS_HANDSHAKE	= 0,
	RTVS_QUERY		= 1,
	RTVS_ADD		= 2,
	RTVS_REMOVE		= 3,
	RTVS_UPDATE		= 4, 
	RTVS_STORE		= 5, 
	RTVS_FETCH		= 6,
	RTVS_RENDER		= 7,
	RTVS_EXECUTE	= 8,
	RTVS_RESET		= 9,
	RTVS_PUSH		= 10,			/* only server uses this */
	RTVS_SYNC		= 11,
	RTVS_CANCEL		= 12,
	RTVS_SYSTEM		= 13,			/* file open etc. */
	RTVS_PROCESS	= 14,			/* algorithm */
	RTVS_COPY		= 15,
	RTVS_DISCONNECT = 16,
	RTVS_PROXY		= 17,
//
#if defined(PXADMIN_SERVER) || defined(PXADMIN_CLIENT) 
	RTVS_ANONYMIZE	= 18, /* K.Ko 2010/03/03 for Anonymize */
#endif
	/* sentinel */
	RTVS_MAX_MAJOR
};

/*
 * The minor number of the protocols
 */
enum
{
	RTVS_HANDSHAKE_DATA	=	0,
	RTVS_SERVER_CONFIG	=	1,
	RTVS_SERVER_STATUS	=	2,
	RTVS_CLIENT_PREFERENCE=	3,
	RTVS_CONTEXT=			4,
	RTVS_CROP			=	5,
	RTVS_CURSOR			=	6,
	RTVS_CUTPLANE		=	7,
	RTVS_FEATURE_LIST	=	8,
	RTVS_GRADIENT_TABLE	=	9,
	RTVS_HISTOGRAM		=	10,
	RTVS_IMAGE			=	11,
	RTVS_LIGHT			=	12,
	RTVS_LOOKUPTABLE_NAME=	13,
	RTVS_LOOKUPTABLE_LIST=	14,
	RTVS_LOOKUPTABLE_DATA=	15,
	RTVS_MATERIAL		=	16,
	RTVS_MOVIE			=	17,
	RTVS_PRINT			=	18,
	RTVS_STATUS_UPDATE	=	19,
	RTVS_RENDER_OPTION	=	20,
	RTVS_RENDER_PROTOCOL=	21,
	RTVS_RENDER_SINGLE	=	22,
	RTVS_RENDER_MPR		=	23,
	RTVS_REPORT=			24,
	RTVS_SCAN_LIST		=	25,
	RTVS_SCAN_HEADER	=	26,
	RTVS_SCAN_DATA	=		27,
	RTVS_SCRIPT	=			28,
	RTVS_SETTING	=		29,
	RTVS_SETTING_LIST	=	30,
	RTVS_SETTING_DATA	=	31,
	RTVS_SUPERSAMPLE	=	32,
	RTVS_USER_INFO	=		33,
	RTVS_MODELMATRIX	=	34,
	RTVS_VIEWMATRIX	=		35,
	RTVS_PROJECTIONMATRIX=	36,
	RTVS_DEPTH_RANGE	=	37,
	RTVS_WINDOWLEVEL	=	38,
	RTVS_VOLUME_LIST	=	39,	/* volume exists for testing only */
	RTVS_VOLUME	=			40,
	RTVS_VOLUME_DATA	=	41,
	// sync
	RTVS_INPUT_QUEUE	=	42,
	RTVS_OUTPUT_QUEUE	=	43,
	// free node
	RTVS_BEST_NODE		=	44,
	RTVS_IMAGE_OPTION	=	45,
	RTVS_SUBVOLUME		=	46,
	RTVS_BYTE_DATA		=	47,
	RTVS_LIGHT_COUNT	=	48,
	RTVS_CUTPLANE_COUNT	=	49,
	RTVS_DICOM_TAG		=	50,
	RTVS_DICOM_IMAGEID	=	51,
	RTVS_REPORT_LIST	=	52,
	RTVS_FILE			=	53,
	RTVS_REPORT_SETTING	=	54,
	RTVS_SCAN_INFO		=	55,
	RTVS_SESSION_LIST	=	56,
	RTVS_SESSION		=	57,
	RTVS_SESSION_SECURITY = 58,
	RTVS_RENDER_SLICE	=   59,
	RTVS_RENDER_CMPR	=   60,
	RTVS_PREVIEW		=   61,
	RTVS_MEASUREMENT	=	62,
	RTVS_SLICE_OPTION	=	63,
	RTVS_CONNECTION		=	64, 
	RTVS_AUXDATA_LIST	=	65,
	RTVS_AUXDATA		=	66,
	RTVS_VOLUME_REF		=	67,

	/* processing related */
	RTVS_SEGMENTATION	=	68,
	
	/* conference	*/
	RTVS_ANNOTATION		=	69,		/* host's annotation			*/
	RTVS_POINTER		=	70,		/* conference host's pointer	*/
	RTVS_HOSTDATA		=	71,		/* conference host preference	*/

	RTVS_TEMPLATE_LIST	=	72,
	RTVS_TEMPLATE		=   73,
	RTVS_TEMPLATE_DATA	=	74,
	RTVS_TEMPLATE_ICON	=	75,

	/* filming related */
	RTVS_PRINTER_LIST	=	76,
	RTVS_PRINTER_QUEUE	=	77,
	RTVS_JOB_STATUS		=	78,

	RTVS_DEPTH_PICK		=	79,
	RTVS_SEGMENT_OP		=	80,  /* between two columes */
	RTVS_DICOMSERVER_LIST=	81,
	RTVS_SCANMATRIX		=	82,

	RTVS_CURSOR_POSITION=	83,

	RTVS_VOLUME_OP		=	84,

	RTVS_SERIES_ATTRIBUTE = 85,

	/* calibration	*/
	RTVS_CALIBRATION	  = 86,

	/* logs         */
	RTVS_CLIENT_LOG		  = 87,

	RTVS_USER_PREFERENCE  = 88,
	RTVS_USER_RIGHTS	  = 89,

	RTVS_CENTERLINE		  = 90,
	RTVS_STENOSIS		  = 91,

	RTVS_COMPRESSED_DICOM_IMAGEID	=	92,
	RTVS_NODE_LIST		=	93,
	RTVS_NODE_LOAD		=	94,

	RTVS_DICOM_FILE		=	95,
	RTVS_FILES			=	96,
	RTVS_LOCATION		=   97,
	RTVS_SERVER_LOAD	=	98,
	RTVS_ROI_STATISTICS =   99,
	RTVS_COMMAND		=	100,
	RTVS_ASYNC_TASK_STATUS	= 101,
	RTVS_FILE_OP			= 102,
	RTVS_TRANSFER_LIST  =   103,
	RTVS_MASK			=   104,
	RTVS_IMAGE_DEPTH	= 105,
	RTVS_MIPMAP_INFO	= 106,
	RTVS_MASKLUT_DATA	=   107,
	RTVS_SAMPLES_ALONG_RAY = 108,
	RTVS_DEPTH_OR_FRAME	 = 109,

#if defined( __DENTAL )
	RTVS_RENDER_IMPLANT_SLICE	  = 110,
	RTVS_IMPLANT      			  = 111,
	RTVS_MANDIBULARCANAL		  = 112,
	RTVS_IMPLANT_MODEL 			  = 113,
	RTVS_IMPLANT_DB 			  = 114,
	RTVS_IMPLANT_SIMULATOR_OPTION = 115,
#endif

	RTVS_PLUG_IN		= 120,
	RTVS_PING			= 121,

//K.Ko 2010/03/29
#if defined(PXADMIN_SERVER) || defined(PXADMIN_CLIENT) 
	RTVS_DATASERVER_STATUS	=	122,
#endif

	/* sentinel */
	RTVS_MAX_MINOR,

	/* aliases */
	RTVS_LUT_LIST = RTVS_LOOKUPTABLE_LIST,
	RTVS_LUT_NAME = RTVS_LOOKUPTABLE_NAME,
	RTVS_LUT_DATA = RTVS_LOOKUPTABLE_DATA,
	RTVS_WINDOW_LEVEL = RTVS_WINDOWLEVEL
};


/*
 * All NVR error codes are negative numbers
 */

enum
{
	kpRTVSOK				=  0,
	kpRTVSErrHandShake		= -1,
	kpRTVSErrVersion		= -3,		// protocol/renderer/client version mismatch
	kpRTVSErrAlloc			= -4,		// can't allocate resources, including VolumePro hardware
	kpRTVSErrNoVolume		= -5,		// invalid volume
	kpRTVSErrTooMany		= -6,		// two many connections
	kpRTVSErrDisk			= -7,		// out of disk space
	kpRTVSErrUser			= -8,		// wrong login name
	kpRTVSErrPassword		= -9,		// wrong password
	kpRTVSErrGroup			= -10,		// bad groupID
	kpRTVSErrParameter		= -11,		// illegal request/internal error
	kpRTVSErrLicense		= -12,		// no license
	kpRTVSErrSystem			= -13,		// unknown system error 
	kpRTVSErrLCD			= -14,		// LCD/serial port error
	kpRTVSErrConfig			= -15,		// wrong server configuration
	kpRTVSErrPermission		= -16,		// permission denied
	kpRTVSErrOverwrite		= -17,		// can't overwrite a file
	kpRTVSErrFile			= -18,		// non-existent file - usally GetSeriesInfo() returns this
	kpRTVSErrVoxelCount		= -19,	
	kpRTVSErrDataSize		= -20,		// datasize received is not the expected.
	kpRTVSErrAborted		= -21,		// per client request
	kpRTVSErrFileRead		= -22,		// can't read report/dicom cache/.vox file
	kpRTVSErrSession		= -23,		// requested session does not exit
	kpRTVSErrRequest		= -24,		// request not supported
	kpRTVSErrProcess		= -25,		// can't process the request
	kpRTVSErrSort			= -26,		// sort specification error or actual sort error
	kpRTVSErrCipher			= -27,		// encryptor/decryptor failed
	kpRTVSErrDatabase		= -28,		// can't access database
	kpRTVSErrUnimplemented	= -29,
	kpRTVSErrException		= -30,
	kpRTVSErrEmpty			= -31,		// not configured
	kpRTVSErrVolume			= -32,
	kpRTVSErrPrint			= -33,
	kpRTVSErrFileOpen		= -34,		// can't open file
	kpRTVSErrFileWrite		= -35,
	kpRTVSErrAccountDisabled= -36,		// disabled user account
	kpRTVSErrAccountExpired	= -37,
	kpRTVSErrNotFound		= -38,	
	kpRTVSErrOtherLicense	= -39,
	kpRTVSErrAlreadyRunning	= -40,
	kpRTVSErrPeer			= -41,		// can't find plugin host session

	/* codec/compression error -50 to -100 internal */
	kpRTVSErrCodec			= -50,		
	kpRTVSErrCodec2			= -100,

	kpRTVSErrConnection		= -998,		// client shuts down the connection
	kpRTVSErrInternal		= -999,

	/* renderer related */
	kpRTVSErrRenderer		= -1000,

	/* datasource related */
	kpRTVSErrDataSource1	= -3500,		
	kpRTVSErrDataSource		= -3900,		/* cache error or general dicom failure  */
	kpRTVSErrDataSourceUnsupported = -4499,
	kpRTVSErrDataSource2	= -4500,
};


enum 
{ 
	kpRTVSInProgress = 1, 
	kpRTVSTerminated = 2,
	kpRTVSError = kpRTVSErrProcess 
};

/* attributes of server stored info (luts, rendering protocol etc.) */
enum
{
	kpRTVSPrivate			= (1 << 0),   // user
	kpRTVSGroup				= (1 << 1),   // group
	kpRTVSPublic			= (1 << 2),   // public location
	kpRTVSHelp				= (1 << 3),   // help file system
	kpRTVSSystem			= (1 << 4),
	kpRTVSDICOMUID			= (1 << 5),
	kpRTVSTemp				= (1 << 6)
};

/* image buffer controls */
enum
{
	kpRTVSMaxRenderBuffers	= 10,
	kpRTVSMIPBuffer			= 12,
	kpRTVSAxialBuffer		= 13,
	kpRTVSSagittalBuffer	= 14,
	kpRTVSCoronalBuffer		= 15,

	kpRTVSDICOMBuffer1		= 16,
	kpRTVSDICOMBuffer2		= 17,
	kpRTVSDICOMBuffer3		= 18,
	kpRTVSDICOMBufferEnd	= kpRTVSDICOMBuffer3,

	kpRTVSSlicerBuffer1		= 19,
	kpRTVSSlicerBufferEnd	= 27,

	kpRTVSCMPRBuffer1		= 28,
	kpRTVSCMPRBufferEnd		= 31,


	kpRTVSZSlice = kpRTVSAxialBuffer,
	kpRTVSYSlice,
	kpRTVSXSlice,

	/* sentinel */
	kpRTVSFreeBuffer  = 0x7ff1,
	kpRTVSCacheBuffer = 0x7fff
};

enum
{
	kpRTVSXAxis		= 0,
	kpRTVSYAxis		= 1,
	kpRTVSZAxis		= 2,

	kpRTVSNegXAxis	= 3,
	kpRTVSNegYAxis	= 4,
	kpRTVSNegZAxis	= 5
};

enum 
{
	kpRTVSSagittal,
	kpRTVSCoronal,
	kpRTVSAxial,
	kpRTVSOblique,  // regular cutplane
	kpRTVSVR,
	kpRTVSOblique2, // for MIPs, RaySums,
	kpRTVSDefaultPlane,
	kpRTVSCPR,
	kpRTVSAllRenderTypes
};


enum
{
	kpRTVSAnterior,
	kpRTVSPosterior,
	kpRTVSSuperior,
	kpRTVSInferior,
	kpRTVSLeft,
	kpRTVSRight
};

/* we support the following connection types, and modifiers */
enum 
{ 
	kpRTVSTypeNormal	= (1 << 0),		/* regular, private sessions */
	kpRTVSTypeHost		= (1 << 1),		/* conference host		     */
	kpRTVSTypeListener	= (1 << 2),		/* conference participant    */
	kpRTVSTypeFilming	= (1 << 3),		/* filming only				 */
	kpRTVSTypeTransfer	= (1 << 4),		/* file transfer			 */
	kpRTVSTypeMultiNode	= (1 << 5),
	kpRTVSTypeStatus	= (1 << 6),		/* only for status query	 */
	kpRTVSTypePlugIn	= (1 << 7),

	kpRTVSAllType		= (1 << 11)-1,

	// masks
	kpRTVSTypeClone		= (1 << 11),	/* cloning a connection		 */
	kpRTVSTypeSecure	= (1 << 12),	/* encrypted session		 */
	kpRTVSModifyID		= (1 << 13),	/* add additional ID		 */
	kpRTVSTypeDataSource= (1 << 14),
	kpRTVSTypeTrueSSO	= (1 << 15)		/* security credential authentication */
};

/* interpolation order */

enum     
{
   kpRTVSInterpolateFirst,      // Interpolation then classification
   kpRTVSClassifyFirst          // Classification then interpolation
};

enum 
{
	kpRTVSNearestNeighbor,
	kpRTVSTrilinear
};


/*
 * Primary data structures
 */

#define kpRTVSProtocolHeaderSize	24
#define kpRTVSSignature				"AQNet24"
#define kpRTVSMaxRequestSerial		((1 << 13) -1)
#define kpRTVSPushSerial			(kpRTVSMaxRequestSerial+1)

struct pRTVSProtocolHeader
{
	char		m_signature[8];					/* always AQNet24		 */
	uint16		m_major;						/* protocol major		 */
	uint16		m_minor;						/* protocol minor		 */
	int16		m_status;						/* status of the request */
	uint16		m_serialNumber;					/* serial number		 */
	int32		m_dataSize;						/* size of data to follow*/
	int32		m_reserved;
};

/*  Must also modify nvr.h if changes are made here */

enum 
{
	kpRTVSRevABoard = 1,
    kpRTVSRevBBoard = 2,
	// workaround some IC/CI problems
	kpRTVSVirtualRevA = 4
};

//----------------------------------------------------------------------------
struct pRTVSServerConfig
{	
	uint32		m_serverIPAddress;				// serverIP address
	uint16		m_protocolMajorVersion;			/* protocol version			*/
	uint16		m_protocolMinorVersion;			/* protocol revision		*/
	uint16		m_serverMajorVersion;			/* server software version	*/
	uint16		m_serverMinorVersion;			/* server softwre revision  */
	uint16		m_rendererMajorVersion;			// rendering engine version
	uint16		m_rendererMinorVersion;			// rendering engine revision
	uint16		m_maxRenderableSize;			// max size(MB) of volume
	uint16		m_maxBitsPerVoxel;				// supported voxel size
	uint16		m_maxVoxelFields;				// supported voxel fields
	uint16		m_serverMemory;					// server main memory size(MB)
	uint16		m_idleTimeout;					// idle timeout (seconds)
	uint16		m_maxConnectionsPerClient;		// max connections allowed
	uint16		m_maxClients;					// max allowed client host
	uint16		m_gradientTableLength;
	short		m_preferredClientVersion[4];
	char		m_numberOfRenderers;				/* number of rendering engines */
	char		m_licensedNumberOfRenderers;
	char		m_CPUCount;						// number of CPU on server
	char		m_servletCount;					// number of servers in group
	char		m_isLittleEndian;
	char		m_maxCutplaneCount;				// number of support cutplanes
	char		m_dicomServerCount;
	char		m_dicomIsLicensed;
	char		m_rendererName[32];
	char		m_serverOSVersion[32];

#if defined (__DENTAL)
	uint16		m_EnableImplantSimulator;					// ImplantSimulator
	uint32		m_ImplantSimulatorImplantMaterial;			// ImplantSimulator
	uint32		m_ImplantSimulatorMandibularCanalMaterial;	// ImplantSimulator
#endif
};


//----------------------------------------------------------------------------
#define kpRTVSMaxPrinters 5

struct pRTVSDICOMServerConfig
{
	uint16		m_dicomServerCount;
	uint16		m_dicomPrinterCount;
	char		m_printerNames[kpRTVSMaxPrinters][32];
	char		m_printerIP	[kpRTVSMaxPrinters][32];
};

struct pRTVSOptionalFeatures
{
	int16		m_conferenceSupported;
	int16		m_multimaskSupported;
	int16		m_perspectiveSupported;
	int16		m_pad;
};

// feature access control
enum 
{
	kpRTVSFeaturePatientList	= (1 << 0),
	kpRTVSFeatureRender			= (1 << 1),
	kpRTVSFeatureOutput			= (1 << 2),
	kpRTVSFeatureProcessing		= (1 << 3),
	kpRTVSFeatureDataCtrl		= (1 << 4),
	kpRTVSFeatureMisc			= (1 << 5),

	kpRTVSFeatureAll			= (kpRTVSFeatureMisc << 1)-1,


	// ApplicationSpecific ones
	kpRTVSFeatureAppSpecific	= 4096
};

//----------------------------------------------------------------------------
// This structure is exposed to the client. Modify nvr.h if
// changes are made here.
struct pRTVSServerStatus
{
	uint32		m_firstErrorRequestSerialNumber;
	uint32		m_serverStartTime;				// server start time (sec) from reboot
	uint32		m_serverCurrentTime;			// server current time (sec)
	uint32		m_serverUpTime;					// server uptime (in minutes)
	uint16		m_numberOfConnections;			// number of current connections
	uint16		m_maxAllowedConnections;		// max. allowed connections
	uint16		m_numberOfClients;				// current client hosts
	uint16		m_maxAllowedClients;			// max allowed hosts
	uint16		m_status;
	uint16		m_loadedVolumeCount;			// number of loaded volumes
	uint16		m_loadedVolumeSize;				// total size (MB)
	uint16		m_firstErrorRequestMajor;
	uint16		m_firstErrorRequestMinor;
	uint16		m_firstErrorRequestStatus;
	char		m_userName[kpRTVSMaxUserNameStore];
	char		m_sessionID[kpRTVSMaxSessionIDLength];

};

//----------------------------------------------------------------------------
// Who Is On
struct pRTVSWhoIsOn
{
	char		m_userName[kpRTVSMaxUserNameStore];
	char		m_userDomain[kpRTVSMaxUserDomainStore]; // login domain
	char		m_from[32];								// in the form hostname(IPAddress)
	char		m_appName[24];
	uint32		m_onSince;								// in seconds from jan. 1, 1970
	uint32		m_volumeMemoryUsedInKB;					// volume ProMemory used
	uint16		m_volumeCount;							// how many volumes loaded
	uint16		m_pad;
};

//----------------------------------------------------------------------------
// Detailed load info
struct pRTVSServerLoad
{
	uint16		m_totalVolumeMemoryInMB;
	uint16		m_totalUsedVolumeMemoryInMB;
	uint16		m_totalClients;
	uint16		m_pad;
	// to be followed by m_totalClients of WhoIsOn
};
//----------------------------------------------------------------------------

struct pRTVSConnectionType
{
	uint16	m_type;
	uint16	m_length;   // encrypted pw length
	char	m_sessionID[kpRTVSMaxSessionIDLength];
	char	m_password[kpRTVSMaxUserNameStore+4]; // only host gives password
};

struct pRTVSConnectionID
{
	uint16	m_type;
	uint16	m_length;
	char	m_additionalID[32];
};


union pRTVSConnectionData
{
	uint16				 m_type;
	pRTVSConnectionType  m_connectionType;
	pRTVSConnectionID	 pRTVSConnectionID;
};

//----------------------------------------------------------------------------
struct pRTVSAuthorization
{
	char		m_userName[kpRTVSMaxUserNameStore];  // username
	char		m_userDomain[kpRTVSMaxUserDomainStore]; // login domain
	char		m_password[kpRTVSMaxUserNameStore+4];// password
	char		m_groupName [kpRTVSMaxUserGroupStore];	// group name
	uint16		m_passwordLength;
	uint16		m_encryptionMethod;
};


//----------------------------------------------------------------------------
struct pRTVSUserProfile
{
	int16		m_length; // profile length
	uint16		m_what;
	char		m_sectionName[kpRTVSMaxSectionNameLength];
	char		m_pTimeAndDate[24];   // yyyymmddhhmmss.mmm
	
	// to be followed by m_length bytes of data
};

//----------------------------------------------------------------------------
struct pRTVSHandshake
{
	uint32		m_byteOrder;					// 0x12345678 in native endian
	uint32		m_type;
	uint32		m_ID;
	uint16		m_daysToExpire;
	uint16		m_softwareBuildNumber;
	char		m_OSVersion[32];				// OS name
	char		m_agentName[24];				// Communication software
	char		m_applicationName[28];			// application name
	char		m_vendorName[20];				// who made the application
	char		m_licenseKey[32];				// not used yet
	char		m_userName[kpRTVSMaxUserNameStore];  // username
	char		m_userDomain[kpRTVSMaxUserDomainStore]; // login domain
	char		m_password[kpRTVSMaxUserNameStore+4];// password
	char		m_groupName [kpRTVSMaxUserGroupStore];	// group ID
	char		m_hostName[20];				    // don't need to be fully qualified
	char		m_loggedOnUser[kpRTVSMaxUserNameStore];				// who's making the request
	char		m_sessionID[kpRTVSMaxSessionIDLength];
	uint8		m_protocolMajorVersion;			// version of protocol understood
	uint8		m_protocolMinorVersion;			// revision of protocol
	uint8		m_protocolRelease;				// release
	uint8		m_softwareMajorVersion;			// software version
	uint8		m_softwareMinorVersion;			// software minor version
	uint8		m_softwareRelease;
	uint8		m_passwordLength;
	uint8		m_reserved;
};

//----------------------------------------------------------------------------
enum
{
	kpRTVSProfileWindowLevel = 1,
	kpRTVSProfileLayout		 = 2
};

//----------------------------------------------------------------------------
struct pRTVSProfileWindowLevel
{
	uint32		m_buttonID;
	char		m_buttonName[16];
	uint16		m_window;
	int16		m_level;
	int32		m_color[2]; /* high and low for slider */
	float		m_opacity;
	uint16		m_curveID;
	uint16		m_pad;
};

//----------------------------------------------------------------------------
struct pRTVSHostPreference
{
	int16	m_imageType;
	int16	m_pad;
};

//----------------------------------------------------------------------------
/* the reason the server shuts down a connection */
enum 
{ 
	kpRTVSIdleTimeout = -49, 
	kpRTVSServerStop,
	kpRTVSServerShutdown, 
	kpRTVSHostSession
};

//----------------------------------------------------------------------------
struct pRTVSConnectionShutdown
{
	uint16	m_reason;
	int16	m_status;
	uint32	m_reserved;
};

//----------------------------------------------------------------------------
struct pRTVSName
{
	char		m_name[kpRTVSMaxNameLength];
	uint16		m_length;
	uint16		m_attributes;
};

//----------------------------------------------------------------------------
// pRTVSSplitName must have the same size as pRTVSName

struct pRTVSSplitName
{
	char		m_name1[sizeof DICOMUID + sizeof DICOMPName];
	char		m_name2[kpRTVSMaxNameLength - sizeof DICOMUID - sizeof DICOMPName];
	uint16		m_length;
	uint16		m_attributes;
};

//----------------------------------------------------------------------------
// pRTVSSplitName must have the same size as pRTVSName
struct pRTVSDICOMSplitName
{
	pRTVSDICOMSplitName() 
	{ 
		m_seriesUID[0]		= '\0';
		m_patientName[0]	= '\0';
		m_studyUID[0]		= '\0';
		m_patientID[0]		= '\0';
		m_accessionNumber[0] = '\0';
		m_length = 0;
	}

	DICOMUID	m_seriesUID;
	DICOMPName	m_patientName;
	DICOMUID	m_studyUID;
	DICOMUID	m_patientID;
	char		m_accessionNumber[kpRTVSMaxNameLength - 3*sizeof DICOMUID -sizeof DICOMPName];
	uint16		m_length;
	uint16		m_attributes;
};


//----------------------------------------------------------------------------
struct pRTVSPoint2D
{
	float	m_x;
	float	m_y;
};

struct pRTVSPoint3D
{
	float	m_x;
	float	m_y;
	float	m_z;
};

//All info we need about a center point
struct pRTVSCenterPoint
{
	pRTVSPoint3D	m_center;
	float			m_dmsize;
};

//----------------------------------------------------------------------------
struct pRTVSSessionListRequest
{
	uint16	m_type;
	uint16	m_pad;
};

//----------------------------------------------------------------------------
/* session list information */
struct pRTVSSession
{
	char	m_sessionID[kpRTVSMaxSessionIDLength];
	char	m_userName[kpRTVSMaxUserName];
	char	m_realName[64];
	char	m_address[32];
	uint16	m_type;
	uint16	m_needPassword;
};


enum 
{ 
	kpRTVSOrientationUnknown = 0,
	kpRTVSOrientationHFP,		// head first prone
	kpRTVSOrientationHFS,		// head first supine
	kpRTVSOrientationHFDR,		// head first decubitus right
	kpRTVSOrientationHFDL,		// head first decubitus left
	kpRTVSOrientationFFDR = 16,	// feet first decubitus rught
	kpRTVSOrientationFFDL,		// feet first decubitus left
	kpRTVSOrientationFFP,		// feet first prone
	kpRTVSOrientationFFS		// feet first supine
};

//----------------------------------------------------------------------------
enum
{
	kpRTVSUnknownDICOMSource	= 0,
	kpRTVSQRSource				= (1<<0),
	kpRTVSStoreTarget			= (1<<1),
	kpRTVSGlobalServer			= (1<<2),
	kpRTVSAllServers			= (1<<3) - 1,

	// mask for server running on local machine
	kpRTVSLocalDICOMServer		= (1<<8)
};

/* basic identification of a dicom device - if make changes here, need to 
 * make changes in nvr.h as well
 */
struct pRTVSDICOMDevice
{
	uint16	m_type;
	uint16	m_port;
	char	m_AEName[20];
	char	m_AETitle[kpRTVSMaxAETitle];
	char	m_IPAddress[kpRTVSMaxIPAddress];
	char	m_hostname[16];		/* not need to be fully qualified */
};

//----------------------------------------------------------------------------
#define kpRTVSInvalidGroup  0xffff
#define kpRTVSAnyGroup		kpRTVSInvalidGroup

// volume status
enum 
{ 
    kpRTVSNonUniform		= (1<<0),			// fake volume
	kpRTVSPartial			= (1<<1),			// volume is made from a subset of the slices
	kpRTVS2D				= (1<<2),			// can only load into 2D
	kpRTVSUnknownScale		= (1<<3),			// can't do measurement. for CR/DR/DX etc
	kpRTVSConsolidated		= (1<<4),			// not true multiframe data
	kpRTVSUnknownOrientation= (1<<5),			// true multiframe data
	kpRTVSDerived			= (1<<6),			// as a result of subtraction for example
	kpRTVSHasShear			= (1<<7),			// volume has shear in it
	kpRTVSClampedVoxel		= (1<<8),			// if we used clamp in converting voxels to 12bits
	
	// alias
	kpRTVSNoScale			= kpRTVSUnknownScale
};

struct pRTVSLoadVolumeRequestOld
{
	pRTVSName	m_name;		// either the name of a .vox file or seriesUID/studyUID combo
	uint16		m_sort;		// how to sort the image
	uint16		m_start;
	uint16		m_end;
	int16		m_step;
	uint16		m_field;
	uint16		m_groupID;	// for duel plane XA or triphasic data
	uint16		m_encoder;
	uint16		m_quality;
	int16		m_nphase;
	uint16		m_pad;
	uint32		m_SOPSize;	// in Bytes. Each SOPUID is exactly 68 bytes 
	char*		m_SOPUIDs;  // pad
};

//----------------------------------------------------------------------------
struct pRTVSLoadVolumeRequest
{
	pRTVSName	m_name;		// either the name of a .vox file or seriesUID/studyUID combo
	uint16		m_sort;		// how to sort the image
	uint16		m_start;
	uint16		m_end;
	int16		m_step;
	uint16		m_field;
	uint16		m_groupID;	// for duel plane XA or triphasic data
	uint16		m_encoder;
	uint16		m_quality;
	int16		m_nphase;
	uint16		m_pad;
	uint32		m_SOPSize;	// in Bytes. Each SOPUID is exactly 68 bytes 
	char*		m_SOPUIDs;  // pad
	pRTVSDICOMDevice	m_dataSource;

};

//----------------------------------------------------------------------------
struct pRTVSVolume
{
	uint32		m_volumeID;						// unique server resource ID
	uint16		m_fieldCount;					// number of fields in volume
	uint16		m_xSize;						// volume size
	uint16		m_ySize;						// volume size
	uint16		m_zSize;						// volume size
	float32		m_xScale;
	float32		m_yScale;
	float32		m_zScale;
	float32		m_xi, m_yi, m_zi;
	float32		m_xf, m_yf, m_zf;
	float32		m_slope;
	float32		m_intercept;
	uint16		m_modality;
	uint16		m_orientation;					
	uint16		m_voxelSize[kpRTVSMaxVoxFields];// voxel field size in bits
	uint16		m_maxValue[kpRTVSMaxVoxFields]; 
	uint16		m_voxelFormat[kpRTVSMaxVoxFields];	
	uint16		m_voxelPosition[kpRTVSMaxVoxFields];
	pRTVSName	m_name;		// either the name of a .vox file or seriesUID/studyUID combo
	uint16		m_sort;		// how to sort the image
	uint16		m_start;
	uint16		m_end;
	uint16		m_step;
	uint16		m_field;
	uint16		m_groupID;	// for duel plane XA or triphasic data
	uint16		m_axis[4][3];		// indexed by  axial|coronal|sagittal, three x|y|z components
	// additional information about the volume
	uint16		m_groupCount;
	uint16		m_volumeStatus;
	uint16		m_scanType;		// kpRTVSAxial,kpRTVSCoronal,kpRTVSSagittal
	// volume can be generated using a list of SOPs
	uint16		m_isListSOPs;
	uint16		m_nphase;
	int16		m_averageVOICenter;
	int16		m_averageVOIWidth;
	char		m_volumeHash[68];
};

//----------------------------------------------------------------------------
struct pRTVSDupVolume
{
	uint32	m_volumeID;
	uint16	m_pad1;
	uint16	m_pad2;
};

//----------------------------------------------------------------------------
// for now only CR/DR scale information
enum { kpRTVSCalUnknown, kpRTVSCalScales };
struct pRTVSCalibration
{
	uint16		m_type;
	uint16		m_pad;
	uint32		m_volumeID;
	float32		m_floats[3];
	int32		m_ints[3];
};


//----------------------------------------------------------------------------
enum 
{ 
	kpRTVSFreehand		= 0,
	kpRTVSClear			= 1,
	kpRTVSReverse		= 2,
	kpRTVSRegionGrow	= 3,
	kpRTVSErode			= 4,
	kpRTVSDilate		= 5,

	/* opeation on two masks */
	kpRTVSCopy			= 6,
	kpRTVSXOR			= 7,
	kpRTVSAND			= 8,
	kpRTVSOR			= 9,
	kpRTVSMinus			= 10,
	kpRTVSNAND			= 11,
	kpRTVSMaxMasking	= 12

};

//----------------------------------------------------------------------------
struct pRTVSROI
{
	float			m_location;
	pRTVSPoint3D	m_direction;
	uint16			m_plane;				// axial, oblique etc
	uint16			m_pointCount;
	pRTVSPoint3D*	m_points;              // pad, makes coding easier
	// to be followed by m_pointCount of pRTVSPoint3D
};

struct pRTVSSegmentationRequestData
{
	uint32			m_volumeID;
	int16			m_segmentNumber;	// + mark, -1 unmark
	uint16			m_algorithm;		// free hand, clear, copy
	uint16			m_inside;
	int16			m_min;
	int16			m_max;
	int16			m_targetSegNumber;
	int16			m_numROIs;
	// to be followed by m_numROIs of pRTVSROI
};

#if 0
//----------------------------------------------------------------------------
struct pRTVSSegmentationRequest
{
	uint32			m_volumeID;
	int16			m_segmentNumber;	// + mark, -1 unmark
	uint16			m_algorithm;		// free hand, clear, copy
	uint16			m_pointCount;
	uint16			m_inside;
	pRTVSPoint3D	m_direction;		// direction for ROI or seed for RegionGrow
	float			m_location;
	int16			m_min;
	int16			m_max;
	int16			m_targetSegNumber;
	int16			m_plane;
	pRTVSPoint3D*	m_points;
};
#endif
//----------------------------------------------------------------------------
// between two volumes 
struct pRTVSSegmentOp
{
	uint32			m_srcVolumeID;
	uint32			m_destVolumeID;
	int16			m_segmentNumber;
	uint16			m_operation;
	uint32			m_reserved;
};

//----------------------------------------------------------------------------
struct pRTVSVolumeField
{
	uint32		m_volumeID;
	uint16		m_voxField;
	uint16		m_volumeFieldPad;
};

//----------------------------------------------------------------------------
struct pRTVSHistogram
{
	uint32		m_volumeID;				// unique server volumeID.
	uint16		m_xSize;
	uint16		m_ySize;
	uint16		m_zSize;
	uint16		m_fieldSize;
	uint16		m_format;
	uint16		m_maxValue;	
	uint16		m_actualMax;
	uint16		m_actualMin;
	uint16		m_voxelField;
	uint16		m_recommendedWindow;
	int16		m_recommendedLevel;
	uint16		m_pad;
	uint16		m_indexOfLocalMaximum[10];
	uint32		m_histogram[4096];
};

//----------------------------------------------------------------------------
struct	pRTVSClassifier
{
 	uint32		m_ALU[3];	
	uint16		m_luts[kpRTVSMaxVoxFields];
	uint16		m_interpolationMode[kpRTVSMaxVoxFields];
	uint16		m_source[kpRTVSMaxVoxFields];
};

//----------------------------------------------------------------------------
//status and monitors
enum 
{ 
	kpRTVSTaskUnknown		= (1 << 0),
	kpRTVSVolumeLoadStatus	= (1 << 1), 
	kpRTVSRetrieveStatus	= (1 << 2),
	kpRTVSQueryStatus		= (1 << 3),
	kpRTVSProcessingStatus	= (1 << 4),

	kpRTVSHostStatus		= (1 << 6),		/* number of conference hosts change */
	kpRTVSJoinedStatus		= (1 << 7),		/* number of joined session changes  */
	kpRTVSFileTransferStatus= (1 << 8),
	kpRTVSImageTransfer		= (1 << 9),
	kpRTVSFilmingStatus		= (1 << 10),
	kpRTVSCenterLine		= (1 << 11),
	kpRTVSDirTransferStatus = (1 << 12),
	kpRTVSIdleTimeoutStatus	= (1 << 13),
	kpRTVSPatientList		= (1 << 14),
	kpRTVSAsyncTaskStatus	= (1 << 15),

	kpRTVSAllStatus			= (1 << 20)-1,

	// number of monitorable events
	kpRTVSMonitorCount		= 16
};


//----------------------------------------------------------------------------
struct pRTVSServerOption
{
	uint32	m_monitor;					/* if non-zero, server will push status */
	uint16	m_frameRate;
	uint16	m_imageType;
	uint16	m_priority;					/* similiar to unix nice. not used. */
	uint16	m_mute;						/* suspend multi-cast				*/
	uint32	m_reserverd[2];
};

//----------------------------------------------------------------------------
// Similar to user preference but influences the server behavior
struct pRTVSServerPreferenceByClient
{
	uint16	m_disableVirtualRevA;
	uint16	m_usePreClampedWindowLevel;
	uint16	m_idleTimeoutInSeconds;
	uint16	m_padshort;
	int		m_padInt;
	float	m_padfloat;
};

//----------------------------------------------------------------------------
struct pRTVSStatusUpdate
{
	int32		m_completed;			// completed sofar
	int32		m_remaining;			// how much is remaining.
	uint32		m_what;					// what the update is about
};

//----------------------------------------------------------------------------
enum { kpRTVSProjectAutoOrtho = 1, kpRTVSProject};

struct pRTVSMatrix
{
	float32		m_matrix[4][4];
	uint32      m_id;
};

struct pRTVSMatrixD
{
	float64		m_matrix[4][4];
	uint32      m_id;
};

enum { kpRTVSLightDirectional = 0};
struct pRTVSLight
{
	uint16		m_type;
	uint16		m_id;
	float32		m_x;
	float32		m_y;
	float32		m_z;
	float32		m_intensity;
	float32		m_angle;
	float32		m_attenuation;
};

//----------------------------------------------------------------------------
#define	kpRTVSSpecularColorMax 255

struct pRTVSMaterial
{
	float32		m_diffuse;
	float32		m_specular;
	float32		m_emissive;
	float32		m_shininess;
	uint16		m_specularColor[3];	// 8bits color for now
	uint16		m_materialPad;
};

//----------------------------------------------------------------------------
struct pRTVSTemplateName
{
	uint16		m_id;
	uint16		m_access;	/* private/group/system */
	char		m_name[32];
	char		m_partName[16];
	char		m_date[kpRTVSMaxDateLength];	//yyyy.mm.dd
	char		m_time[16];	//hh:mm:ss
};

//----------------------------------------------------------------------------
/* curveID */
enum
{
	kpRTVSCurveRightOneThird,
	kpRTVSCurveLeftOneThird,
	kpRTVSCurveOneHalf,
	kpRTVSCurveLinearRampUp,
	kpRTVSCurveLinearRampDown,
	kpRTVSCurveRectangle,
	kpRTVSCurveFree
};

/*
#define CURVE_R_TRI	0
#define CURVE_L_TRI	1
#define CURVE_ISOSCELES	2
#define CURVE_R_LINEAR	3
#define CURVE_L_LINEAR	4
#define CURVE_RECTANGLE	5
#define CURVE_FREE	6 // Free
#define CURVE_R_CURVE	7
#define CURVE_L_CURVE	8
*/

struct pRTVSLUTObject
{
	uint16		m_id;
	uint16		m_active;
	uint16		m_curveID;
	uint16		m_window;
	int16		m_level;
	int16		m_opacity;			/* 0 to 4095  */
	uint8		m_highColor[4];		/* RGBA 0-255 */
	uint8		m_lowColor[4];		/* RGBA 0-255 */
};

struct pRTVSTemplateData
{
	float32			m_intensity;		/* light intensity */
	pRTVSMaterial	m_material;
	uint16			m_objectCount;
	/* followed by m_objectCount * sizeof pRTVSLUTObject */
};

//----------------------------------------------------------------------------
enum { kpRTVSProcessedNone, kpRTVSProcessedMask};

struct pRTVSLookupTableHeader
{
	uint16		m_id;
	uint16		m_serial;
	uint16		m_entryCount;
	uint16		m_maxColor;
	uint16		m_maxAlpha;
	uint16		m_unprocessed;
};

// lookup table order is implicitly exposed to the API.
// Change with care and make sure also change nvr.h
struct pRTVSLookupTable
{
	uint16		m_id;
	uint16		m_serial;
	uint16		m_entryCount;
	uint16		m_maxColor;
	uint16		m_maxAlpha;
	uint16		m_unprocessed;
	pRTVSName	m_name;
	uint16		m_red[kpRTVSMaxLUTEntries];
	uint16		m_green[kpRTVSMaxLUTEntries];
	uint16		m_blue[kpRTVSMaxLUTEntries];
	uint16		m_alpha[kpRTVSMaxLUTEntries];
	int			m_pad; // need this as API needs it
};

//----------------------------------------------------------------------------
struct pRTVSSmallLookupTable
{
	uint16		m_id;
	uint16		m_serial;
	uint16		m_entryCount;
	uint16		m_maxColor;
	uint16		m_maxAlpha;
	uint16		m_unprocessed;
	pRTVSName	m_name;
	uint8		m_red[kpRTVSMaxLUTEntries];
	uint8		m_green[kpRTVSMaxLUTEntries];
	uint8		m_blue[kpRTVSMaxLUTEntries];
	uint16		m_alpha[kpRTVSMaxLUTEntries];
	int			m_pad; // need this as API needs it
};

//--------------------------------------------------------------
// Change with care and make sure also change nvr.h
#define kMaxMaskLUTLength 16
struct pRTVSMaskLookupTable
{
	uint16		m_id;
	uint16		m_serial;
	uint16		m_entryCount;
	uint16		m_maxColor;
	uint16		m_maxAlpha;
	uint16		m_unprocessed;
	uint8		m_red[kMaxMaskLUTLength];
	uint8		m_green[kMaxMaskLUTLength];
	uint8		m_blue[kMaxMaskLUTLength];
	uint16		m_alpha[kMaxMaskLUTLength];
	uint32		m_finalBlend;
};

//----------------------------------------------------------------------------
struct pRTVSMaskSmallLookupTable
{
	int16					m_objectID;
	int16					m_mask;
	pRTVSSmallLookupTable	m_lut;
};

struct pRTVSLookupTable256
{
	uint16		m_id;
	uint16		m_serial;
	uint16		m_entryCount;
	uint16		m_maxColor;
	uint16		m_maxAlpha;
	uint16		m_unprocessed;
	pRTVSName	m_name;
	uint16		m_red[256];
	uint16		m_green[256];
	uint16		m_blue[256];
	uint16		m_alpha[256];
	int			m_pad; // need this as API needs it
};

//----------------------------------------------------------------------------
enum 
{ 
	kpRTVSFitAlphaToWindow		= 1,  
	kpRTVSFitRGBToWindow		= 2,
	kpRTVSWindowAlpha			= 4,
	kpRTVSWindowRGB				= 8,
	kpRTVSWindowRGBA			= (kpRTVSWindowAlpha|kpRTVSWindowRGB)
};

struct pRTVSWindowLevel
{
	uint16		m_window;
	int16		m_level;
	uint16		m_option;
	uint16		m_minOpaque;		// starting voxel value for MakeOpaque
	uint16		m_opaqueCount;		// ending   voxel vlaue for MakeOpaque
	uint16		m_pad;
	float32		m_contrast;
};

struct pRTVSCrop
{
	float32		m_minX, m_maxX;
	float32		m_minY, m_maxY;
	float32		m_minZ, m_maxZ;
	uint16		m_configuration;
	uint16		m_pad;
};

struct pRTVSCutPlane
{
	uint16		m_id;
	uint16		m_isOutside;
	float32		m_xNormal;
	float32		m_yNormal;
	float32		m_zNormal;
	float32		m_distance;
	float32		m_thickness;
	float32		m_falloff;
};

//----------------------------------------------------------------------------
struct pRTVSQueryID
{
	uint32		m_id;
};

struct pRTVSQueryID2
{
	uint32		m_id;
	char		m_name[16];
	char		m_part[16];
};

struct pRTVSCursor
{
	uint32		m_configuration;
	float32		m_position[3];
	float32		m_width;
	float32		m_length;
	uint16		m_RGBA[3][4];
	uint32		m_pad;
};

// exposed to nvr.h
enum {kpRTVSSuperSampleGrow, kpRTVSSuperSampleBlend};
struct pRTVSSuperSample
{
	uint16		m_xSuperSample;
	uint16		m_ySuperSample;
	uint16		m_zSuperSample;
	uint16		m_superSampleSpace;
	uint16		m_accumulation;
	uint16		m_supersamplePad;
};

//----------------------------------------------------------------------------
struct pRTVSDepthRange
{
	float32		m_minz;
	float32		m_maxz;
};

enum { kpXSlice = 1, kpYSlice = 2, kpZSlice = 4, kpVolume = 8};
enum { kpRTVSVOICenter = -10000, kpRTVSVOIWidth = -10000 };

enum { kpRTVSHFlip = 1, kpRTVSVFlip = 2};
// specifies a slice of the volume
struct pRTVSSlicer
{
	float		m_slice;		// slice number - in voxel coordinate
	float		m_thickness;	// in voxel space
	float		m_normal[3];	// orientation of the slice
	float		m_zoomX;
	float		m_zoomY;
	float		m_panX;
	float		m_panY;
	pRTVSMatrix	m_viewMatrix;
	float		m_xmin;
	float		m_xmax;
	float		m_ymin;
	float		m_ymax;
	float		m_zmin;
	float		m_zmax;
	uint16      m_flags;
	uint16		m_plane;		// kAxial, kSagittal, kCoronal, kOblique
	uint16		m_original;		// overrides the slice options
	// copied from slicer option for measurement
	uint16		m_width;
	uint16		m_height;
	uint16		m_pad;
 	pRTVSMatrix	m_inplaneMatrix; // we don't actually need this
};

//----------------------------------------------------------------------------
// we should consolidate depthPick and depthPick2 when
// prototol version changes the next time
enum { kpRTVSPickDepth = 0 };
struct pRTVSDepthPickRequest
{
	uint32			m_volumeID;
	uint16			m_pick;			/* alpha, gradient etc. Ignored for now */
	uint16			m_isSlicer;
	float			m_min;
	float			m_max;
	pRTVSPoint3D	m_location;		/* in image space */
};

struct pRTVSDepthPickRequest2
{
	uint32			m_volumeID;
	uint16			m_pick;			/* alpha, gradient etc. Ignored for now */
	uint16			m_isSlicer;
	float			m_min;
	float			m_max;
	pRTVSPoint3D	m_location;		/* in image space */
	pRTVSSlicer		m_slicer;
};

/*	
 *	This structure is used to obtain depth buffer from the server.
 */ 
struct pRTVSDepthBuffer
{
	uint32  m_ID;						// ID used to identify the depth buffer
	uint32  m_length;					// Buffer length transmitted
    uint16	m_originx;					// X origin
	uint16	m_originy;					// Y origin
	uint16	m_sizex;					// X size
	uint16	m_sizey;					// Y size
	void*   m_buffer;					// Not used for transmission
};


struct pRTVSSamplesAlongRayRequest 
{
	uint32				m_volumeID;
	pRTVSPoint3D		m_loc;		/* in world space */
	pRTVSPoint3D		m_dir;		/* in world space*/
	unsigned short*		m_samples;	/* voxel samples returned from server */
	int					m_count;	/* sample count */

};

/*
 * patient location/cursor. replies with pRTVSLocationAndVoxel
 */

struct pRTVSCursorPositionRequest
{
	uint32			m_volumeID;
	uint16			m_IsIntersection;
	uint16			m_pad;
	float			m_axial;
	float			m_coronal;
	float			m_sagittal;
	pRTVSPoint3D	m_location;
};

struct pRTVSLocationAndVoxel
{
	uint32			m_volumeID;
	pRTVSPoint3D	m_location;		/* in voxel space or world space*/
	uint16			m_voxelValue;
	int16			m_pad;
};

//----------------------------------------------------------------------------
struct	pRTVSSegment
{
	float	m_p1[3];
	float	m_p2[3];
//Keep for PDLB compatibility - YZ 01/09/05
//	float	m_up[3];
};

enum { kpRTVS3DTo2D, kpRTVS2DTo3D };
//----------------------------------------------------------------------------
struct pRTVSTransferRequest
{
	uint16			m_pointCount;
	uint16			m_dirFlag;
	pRTVSPoint3D*	m_points;
};

struct pRTVSTransferReply
{
	uint16			m_pointCount;
	uint16			m_pad;
};

//----------------------------------------------------------------------------
struct pRTVSCenterLineRequest
{
	uint32			m_volumeID;
	pRTVSPoint3D	m_viewVector;
	uint16			m_direction;
	uint16			m_pointCount;
	pRTVSPoint3D*	m_points;
	/* m_pointCount of pRTVSPoint3D, similar to pRTVSSegmentationRequest */
};

struct pRTVSCenterLineReply
{
	float			m_scalefactor;	//used by CPR engine for scale factor
	short			m_processStatus;
	uint16			m_pointCount;
	/* followed by m_pointCount of pRTVSPoint3D */
};

struct pRTVSStenosisRequest
{
	uint32			m_volumeID;
	float			m_percentage;
	pRTVSPoint3D	m_position;
	pRTVSPoint3D	m_normal;
	float			m_size;
	int16			m_lthrd;
	int16			m_hthrd;
	uint32			m_reqType;		// Request type flag
};

struct pRTVSStenosisReply
{
	pRTVSPoint3D	m_center;
	float			m_radius;
	uint16			m_rawvalue;		//only 16 bit used by stenosis interface
	uint16			m_lthrd;
	uint16			m_hthrd;
	uint16			m_pointCount;
	/* followed by m_pointCount of pRTVSPoint3D */
};

// Appropriate values are initialized in the implementation class.
struct pRTVSROIStatisticsRequest
{
	// The request could be a combination of one of these values!
	enum ROIStatRequest	
	{			
		kVolume		=  1,		// volume if multiple ROIs / Area if one ROI.
		kVoxelMin	=  2,		// voxel min value
		kVoxelMax	=  4,		// voxel max value
		kVoxelMean	=  8,		// voxel mean
		kVoxelStdDev= 16,		// std. deviation
		kHistogram  = 32		// 
	};
	
	uint32		m_volumeID;		// Volume ID
	uint32		m_reqType;		// Request type based on ROIStatRequest enum.
	uint16		m_numROIs;		// Num. of ROIs making up this region.
	uint16		m_markVoxels;	// Flag indicating whether the voxels will be masked.
	uint32		m_maskOpType;	// Masking operation type (0 - apply, 1 - clear,  2 - reverse)
										//(used only if m_markVoxels is set).	
};

struct pRTVSROIStatisticsReply
{
	uint32		m_reqType;		// Request type served by this instance.
	float		m_voxelMean;	
	float		m_volume;
	float       m_stdDeviation;			
	uint32		m_isHistogramPresent;	// Flag indicating the presence of a Histogram. 
	uint16		m_voxelMin;		
	uint16		m_voxelMax;		
	/* followed by an instance of the pRTVSHistogram object if the above flag is set*/
};

//----------------------------------------------------------------------------
enum { kpRTVSWorldSpace, kpRTVSImageSpace};
struct pRTVSCMPR
{
	uint16			m_segmentCount;
	uint16			m_space;
	float			m_thickness;
	uint16			m_smooth;
	uint16			m_flag;		//See the flag definition in nvli.h - kVLICMPRflag
	float			m_view[3];
	float			m_up[3];
	uint16			m_fillPreference;
	uint16			m_pad;
//	uint16			m_type;		// 0 - CMPR,	1 - SMPR
//	uint16			m_flip;		// 0 - No flip, 1 - flip
	float			m_angle;	// angle used for  SMPR
	float			m_centershift;	// center offset relative to origin
	float			m_additionalzoom;

	pRTVSSegment* m_segments;
};

//----------------------------------------------------------------------------
enum 
{ 
	kpRTVSRenderNormal, 
	kpRTVSRenderSlicer, 
	kpRTVSRenderCMPR 
};

/* All render request will contain pRTVSRenderRequest.
 * Specific render request will contain more information
 */

struct pRTVSRenderRequest
{
	uint32		m_volumeID;
	uint16		m_bufferID;
	uint16		m_quality;
};


struct pRTVSRenderSlice
{
	uint32		m_volumeID;
	uint16		m_bufferID;
	uint16		m_quality;
	pRTVSSlicer	m_slicer;
};

struct pRTVSRenderCMPR
{
	uint32		m_volumeID;
	uint16		m_bufferID;
	uint16		m_quality;
	pRTVSCMPR	m_cmpr;
};

//----------------------------------------------------------------------------
enum 
{
	kpRTVSBlendNone			= 0,
	kpRTVSBlendFTB			= 1,	// Front to back, standard blending method
	kpRTVSBlendMIP			= 2,	// Maximum Intensity Projection
	kpRTVSBlendMINIP		= 3,	// Minimum Intensity Projection
	kpRTVSBlendFTPPreWeight = 4,	// use opacity weighted color
	kpRTVSBlendReplace		= 5,
	kpRTVSBlendSumAndCount	= 6

};

/* render quality control */
enum 
{ 
	kpRTVSQualitySuperFine = -1,
	kpRTVSQualityDefault, 
	kpRTVSQualityAuto, 
	kpRTVSQualityDraft1, 
	kpRTVSQualityDraft2, 
	kpRTVSQualityFine,
	kpRTVSQualityFineDraft,
	kpRTVSQualityFree,				// no resize, mipmap 0 to N
	kpRTVSQualitySlabDraft			// for slabs
};	

//-----------------------------------------------------------------------
struct pRTVSRenderOption
{
	float		m_rayTerminationValue;
	float		m_backgroundColor[4];	// R,G,B,A 
	float		m_initialColor[4];		// not used yet
	float       m_transparency;	

	uint16		m_blendMode;
	uint16		m_gmom;   // gradient magnitude opacity modulation
	uint16		m_gmimS;  // gradient mag. illumination modu. (specular)
	uint16		m_gmimD;  // gradient mag. illumination modu. (diffuse)
	uint16		m_gmimE;  // gradient mag. illumination modu. (emissive)
	uint16		m_alphaCorrection;
	uint16		m_mode;					// draft or fine
	uint16		m_gradientCorrection;	// true or false
	uint16		m_interpolationOrder;
	uint16		m_opacityWeighting;	
	
#if defined( __DENTAL )
	uint16		m_renderWithGeometry;	
#endif	
};
//-----------------------------------------------------------------------
#if defined( __DENTAL )
struct pRTVSImplantGeometry
{
	int			m_mode;
	int			m_id;
	int			m_color;
	int			m_visible;
	float		m_length;
	float		m_position[3];
	float		m_vector[3];
	int			m_object_id;
};
//-----------------------------------------------------------------------
struct pRTVSMandibularCanal
{
	int			m_mode;
	int			m_id;
	int			m_color;
	int			m_visible;
	float		m_radius;
	int			m_numPoints;
};
//-----------------------------------------------------------------------
struct pRTVSImplantModel
{
	int			m_mode;
	int			m_id;
	int			m_numPoints;
};
//-----------------------------------------------------------------------
struct pRTVSImplantDB
{
	int			m_mode;
	int			m_id;
	int			m_sizeRequest;
	int			m_sizeReply;
};
//-----------------------------------------------------------------------
struct pRTVSImplantSimulatorOption
{
	int			m_mode;
	int			m_enable;
	int			m_view_mode;
	int			m_color_Implant_selected;
	int			m_color_Implant_not_selected;
	int			m_color_MandebularCanal_selected;
	int			m_color_MandebularCanal_not_selected;
	int			m_material_Implant;
	int			m_material_MandibularCanal;
};
#endif	
//-----------------------------------------------------------------------
enum { kpRTVSGradientTableLength = 8 };
struct pRTVSGradientTable
{
	double	m_table[kpRTVSGradientTableLength];
	short	m_type;
	short	m_length;
};

//----------------------------------------------------------------------------
struct pRTVSAnnotation
{
	int16			m_id;			// annotation ID
	int16			m_x;			// relative to image
	int16			m_y;			// relative to image
	uint16			m_fontSize;
	uint16			m_fontStyle;
	uint16			m_length;		// how many bytes
	unsigned char	m_rgba[4];		// text color
	uint16			m_angle;		// 0 horizontal, 90 vertical etc
	uint16			m_alloc;		// application helper/pad
	char*			m_text;
};

//----------------------------------------------------------------------------
struct pRTVSCount
{
	uint16		m_count;
	uint16		m_pad;
	uint16		*m_id;		// reserved space holder
};

//----------------------------------------------------------------------------
enum
{
	kpRTVSCompressNone		= 0,
	kpRTVSCompressJPEGLossy = 1,	// lossy
	kpRTVSCompressJPEG2000  = 2,	// lossy
	kpRTVSCompressDelta		= 3,
	kpRTVSJPEGBaseline		= 4,	// lossy
	kpRTVSJPEG200Lossless   = 5,
	kpRTVSJPEGLossless		= 6,

	kpRTVSUnknownLossless	= 20,
	kpRTVSUnknownLossy		= 21,

	// Sentinel
	kpRTVSCompressInvalid
};

struct pRTVSImageOption
{
	uint16		m_width;		// image width
	uint16		m_height;		// image height
	int16		m_viewportX;
	int16		m_viewportY;
	int16		m_viewportWidth;
	int16		m_viewportHeight;
	int16		m_near;
	int16		m_far;
	float32		m_panX;
	float32		m_panY;
	float32		m_zoomX;
	float32		m_zoomY;
	float32		m_samplingFactor;
	uint16		m_warp;
	uint16		m_compressMethod;
	uint16		m_compressQuality;
	uint16		m_imageType;
	uint16		m_blockSize;
	uint16		m_bitsPerPixel;	// for grayscale only
};

//----------------------------------------------------------------------------
/*
 * WAN Support: complete download a slice. Due to bandwidth consideration,
 * we'll not send the series/study/sop UIDs with the slice. This means
 * the client needs to track it
 */

struct pRTVSDownloadedImageOld
{
	uint16		m_imageNumber;		/* instance number */

	/*	kpRTVSCompressNone	 
		kpRTVSCompressJPEGLossy 
		kpRTVSCompressJPEG2000
	*/
	uint16		m_encoder;		 
	float		m_location;			/* position along axis of acquisition */
	uint16		m_width;
	uint16		m_height;
	uint16		m_bitsPerSample;	/* 8 or 12 or 16 */
	uint16		m_bytesPerPixel;	/* 1,3,4   */
	uint16		m_ratio;			/* compression ratio 10X, 15X etc */
	float		m_orientation[6];	/* directional cosine */
	float		m_dfov;
	uint32		m_pixelByteCount;
	int16		m_recommendedWindowLevel;
	uint16		m_recommendedWindowWidth;
	char*		m_pixels;
};
	
struct pRTVSDownloadedImage
{
	uint32		m_imageNumber;		/* instance number */
	uint16		m_pad;

	/*	kpRTVSCompressNone	 
		kpRTVSCompressJPEGLossy 
		kpRTVSCompressJPEG2000
	*/
	uint16		m_encoder;		 
	float		m_location;			/* position along axis of acquisition */
	uint16		m_width;
	uint16		m_height;
	uint16		m_bitsPerSample;	/* 8 or 12 or 16 */
	uint16		m_bytesPerPixel;	/* 1,3,4   */
	uint16		m_ratio;			/* compression ratio 10X, 15X etc */
	float		m_orientation[6];	/* directional cosine */
	float		m_dfov;
	uint32		m_pixelByteCount;
	int16		m_recommendedWindowLevel;
	uint16		m_recommendedWindowWidth;
	char*		m_pixels;
	float		m_position[3];
	float		m_thickness;
};

//----------------------------------------------------------------------------
#define VP1000ONLY 

// if the image header changes, make sure change the image
// header in nvr.h as well
enum 
{
	kpUnknownImage		 = -1,
	kpRTVSMONOCHROME1	 = 0,	//1 sample/pixel  - grayscale from white to black
	kpRTVSMONOCHROME2	 = 1,	//1 sample/pixel  - grayscale from black to white
	kpRTVSPALETTE_COLOR  = 2,	//1 sample/pixel - needs lut - unsupported yet
	kpRTVSRGB			 = 3,	//3 samples/pixel
	kpRTVSRGBA			 = 4,	// 4 samples/pixel
	kpRTVSLuminanceAlpha = 5,	// 2 samples per pixel luminance+alpha
	kpRTVSYBR_FULL		 = 6,
	kpRTVSYBR_FULL_422   = 7,
	// aliases
	kpRTVSLuminance		 = kpRTVSMONOCHROME2
};


struct pRTVSImageHeader161
{
	uint32		m_volumeID;
	uint32		m_serialNumber;
	uint16		m_bufferID;
	uint16		m_width;
	uint16		m_height;
	int16		m_type;
	uint16		m_bytesPerPixel;	//  
	uint16		m_bitsPerPixel;		// actual bits per pixels
	uint16		m_minY;
	uint16		m_maxY;
	uint16		m_minX;
	uint16		m_maxX;
	uint16		m_stride;
	uint16		m_renderType;

	unsigned char*  m_pixels;
	
	// compression parameters
	uint16			m_encoder;		// None, LossyJPEG, JPEG2000
	uint16			m_numChunks;
	uint32			m_maxChunkSize;
	uint16			m_ratio;
	uint16			m_hasValidInfo; // if the dicom specific stuff is valid

	// dicom specific stuff
	float			m_orientation[6]; 
	int				m_instanceNumber;
	int16			m_recommendedWindowLevel;
	uint16			m_recommendedWindowWidth;
	DICOMUID		m_SOPInstanceUID;
	float			m_position[3];
	float			m_thickness;
	short			m_sliceNumber;	// on vp1000
	short			m_srcCompression; // the encoder/ratio is really from the source
	float			m_xScale;
	float			m_yScale;
	float			m_CPRLength;	  // in image space
};

struct pRTVSImageHeader
{
	uint32		m_volumeID;
	uint32		m_serialNumber;
	uint16		m_bufferID;
	uint16		m_width;
	uint16		m_height;
	int16		m_type;
	uint16		m_bytesPerPixel;	//  
	uint16		m_bitsPerPixel;		// actual bits per pixels
	uint16		m_minY;
	uint16		m_maxY;
	uint16		m_minX;
	uint16		m_maxX;
	uint16		m_stride;
	uint16		m_renderType;

	unsigned char*  m_pixels;
	
	// compression parameters
	uint16			m_encoder;		// None, LossyJPEG, JPEG2000
	uint16			m_numChunks;
	uint32			m_maxChunkSize;
	uint16			m_ratio;
	uint16			m_hasValidInfo; // if the dicom specific stuff is valid

	// dicom specific stuff
	float			m_orientation[6]; 
	int				m_instanceNumber;
	int16			m_recommendedWindowLevel;
	uint16			m_recommendedWindowWidth;
	DICOMUID		m_SOPInstanceUID;
	float			m_position[3];
	float			m_thickness;
	short			m_sliceNumber;	// on vp1000
	short			m_srcCompression; // the encoder/ratio is really from the source
	float			m_xScale;
	float			m_yScale;
	float			m_CPRLength;	  // in image space
	char			m_pixelRepresentation;
	char			m_photometricInterpretation;
	char			m_imageTime[16];
	char			m_imageDate[12];

};

/**  
 *	This header is used only when depth buffer needs to returned following the image buffer.
 */
struct pRTVSImageAndDepthHeader : public pRTVSImageHeader
{
	// Image Depth Buffer: 
	uint32          m_bDepth;		// flag indicating existance of depthe buffer
	uint32			m_depthID;		// ID used for identifying buffer
	int32           m_depthsize;	// buffer size
	void*		    m_depthbuffer;  // Not used for transmission.
};

struct pRTVSImageHeaderOld
{
	uint32		m_volumeID;
	uint32		m_serialNumber;
	uint16		m_bufferID;
	uint16		m_width;
	uint16		m_height;
	uint16		m_type;
	uint16		m_bytesPerPixel;	//  
	uint16		m_bitsPerPixel;		// actual bits per pixels
	uint16		m_minY;
	uint16		m_maxY;
	uint16		m_minX;
	uint16		m_maxX;
	uint16		m_stride;
	uint16		m_renderType;

	unsigned char*  m_pixels;
	
	// compression parameters
	uint16			m_encoder;		// None, LossyJPEG, JPEG2000
	uint16			m_numChunks;
	uint32			m_maxChunkSize;
	uint16			m_ratio;
	uint16			m_hasValidInfo; // if the dicom specific stuff is valid

	// dicom specific stuff
	float			m_orientation[6]; 
	int				m_instanceNumber;
	int16			m_recommendedWindowLevel;
	uint16			m_recommendedWindowWidth;
	DICOMUID		m_SOPInstanceUID;
	float			m_position[3];
	float			m_thickness;
	short			m_sliceNumber;	// on vp1000
	short			m_pad;
};

//----------------------------------------------------------------------------
// a rectangular block
struct pRTVSImageChunkHeader
{
	uint32		m_serialNumber;	
	int32		m_size;					// size in bytes. Negative indicates error
	uint16		m_startY;				// position of first scanline (within the image)
	uint16		m_endY;					// position of last  scanline
	uint16		m_width;				// stride of the chunk
	uint16		m_startX;				// starting position of first pixel in original image
	uint16		m_endX;
	uint16		m_pad;
};


//----------------------------------------------------------------------------
enum 
{ 
	kpRTVSPatientName		= (1<<1),
	kpRTVSPatientID			= (1<<2),
	kpRTVSBirthDate			= (1<<3),
	kpRTVSPatientSex		= (1<<4),
	kpRTVSStudyCount		= (1<<5),   // this patient related

	kpRTVSStudyID			= (1<<6),
	kpRTVSStudyUID			= (1<<7),
	kpRTVSStudyDate			= (1<<8),
	kpRTVSStudyTime			= (1<<9),
	kpRTVSAccessionNumber	= (1<<10),
	kpRTVSPhysician			= (1<<11),	// referrung physician's name
	kpRTVSRadiologist		= (1<<12),	// who reads the study
	kpRTVSSeriesCount		= (1<<13),	// series in this study
	kpRTVSModalitiesInStudy	= (1<<14),
	kpRTVSImagesInStudy		= (1<<15),
	kpRTVSStudyDescription	= (1<<16),


	kpRTVSSeriesNumber		= (1<<17),
	kpRTVSSeriesUID			= (1<<18),
	kpRTVSModality			= (1<<19),
	kpRTVSImageCount		= (1<<20),
	kpRTVSSeriesDescription	= (1<<21),
	kpRTVSBodyPartExamined	= (1<<22),
	
	kpRTVSDataSource		= (1<<23),

	kpRTVSSeriesDaysToLock	= (1<<24),
 	kpRTVSSeriesDate		= (1<<25),
	kpRTVSSeriesTime		= (1<<26),

	kpRTVSDicomInformationBITS = 27
};


enum 
{
	kpRTVSModalityUnknown	= 0,
	kpRTVSModalityCT		= 1,
	kpRTVSModalityMR		= 2,
	kpRTVSModalityXA		= 3,
	kpRTVSModalityXF		= 4,
	kpRTVSModalityUS		= 5,
	kpRTVSModalityCR		= 6,
	kpRTVSModalityDR		= 7,
	kpRTVSModalitySC		= 8, // secondary capture
	kpRTVSModalityOT		= 9, // other
	kpRTVSModalityRF		= 10,
	kpRTVSModalityDX		= 11,
	kpRTVSModalityPT		= 12,
	kpRTVSModalityNM		= 13,
	kpRTVSModalityLast,

	kpRTVSSecondaryCapture	= 64, // mask to indicate secondary capture SOPClassUID

	kpRTVSModalityPET		= kpRTVSModalityPT,
	// setinel
	kpRTVSModalityMax  = (kpRTVSModalityLast+kpRTVSSecondaryCapture)

};

//----------------------------------------------------------------------------
/*
 * status has 16bits
 */
enum 
{
	kpRTVSIsUnknown			= 0,
	kpRTVSIsUnread			= (1<<0),
	kpRTVSIsPartiallyRead	= (1<<1),
	kpRTVSIsRead			= (1<<2),
	kpRTVSIsHiden			= (1<<3),
	kpRTVSReadStatusBits	= 4,
};

/* 
 * auxiliary data associated with a particular study/series 
 */
enum 
{ 
	kpRTVSAuxStartBit		= 0,
	kpRTVSAuxIR				= (1<<(kpRTVSAuxStartBit+0)),	/* interactive report */
	kpRTVSAuxCaScore		= (1<<(kpRTVSAuxStartBit+1)),
	kpRTVSAuxMask			= (1<<(kpRTVSAuxStartBit+2)),
	kpRTVSAuxScene			= (1<<(kpRTVSAuxStartBit+3)),
	kpRTVSAuxTemplate		= (1<<(kpRTVSAuxStartBit+4)),
	kpRTVSAuxCustom			= (1<<(kpRTVSAuxStartBit+5)),
	kpRTVSAuxCaReport		= (1<<(kpRTVSAuxStartBit+6)),
	kpRTVSAuxSC				= (1<<(kpRTVSAuxStartBit+7)), // secondary capture
	kpRTVSAuxNetScene		= (1<<(kpRTVSAuxStartBit+8)),
	kpRTVSAuxDICOM			= (1<<(kpRTVSAuxStartBit+9)),
	kpRTVSAuxFindings		= (1<<(kpRTVSAuxStartBit+10)),
	kpRTVSAuxSimpleReport	= (1<<(kpRTVSAuxStartBit+11)),
	kpRTVSAuxParametricMapEnhancingRatio	= (1<<(kpRTVSAuxStartBit+12)),
	kpRTVSAuxRigidTransform	= (1<<(kpRTVSAuxStartBit+13)),
	kpRTVSAuxCenterline		= (1<<(kpRTVSAuxStartBit+14)),
	kpRTVSAuxCandidates		= (1<<(kpRTVSAuxStartBit+15)),
	kpRTVSAuxParametricMapUptake = (1<<(kpRTVSAuxStartBit+16)),
	kpRTVSAuxCustom3rdParty1	= (1 << (kpRTVSAuxStartBit+17)),    // for 3rd party stuff
	kpRTVSAuxResampledVolume	= (1 << (kpRTVSAuxStartBit+18)),    // SH, 2006.09.25

	kpRTVSIsAux			= (1<<30),

	kpRTVSIsStudyLevel	= (1<<31) // hack for querylevel
};


/* status of a print job */
enum 
{ 
	kpRTVSTransferring = 1,
    kpRTVSQueued, 
	kpRTVSPrinting, 
	kpRTVSDone,
	kpRTVSCancelled,
	kpRTVSPrintError = kpRTVSErrPrint
};

//----------------------------------------------------------------------------
struct pRTVSDICOMPrinter
{
	uint16	m_id;
	uint16	m_port;
	char	m_AETitle[kpRTVSMaxAETitle];
	char	m_IPAddress[kpRTVSMaxIPAddress];
	char	m_name[32];
	char	m_manufacturer[32];
	char	m_model[16];
	char	m_location[16];
	uint16	m_isColor;
	uint16	m_isLandscape;
	char	m_mediaSize[16];
};

//----------------------------------------------------------------------------
struct pRTVSDICOMPrintRequest
{
	uint16		m_printerID;
	uint16		m_printType;		// sc or original series
	char		m_printerName[32];
	int			m_fileCount;
	uint16		m_nrows;
	uint16		m_ncolumns;
	uint16		m_requestID;
	uint16		m_pad;
	DICOMUID	m_seriesUID;
	DICOMUID	m_studyUID;
	DICOMUID	m_description;		// used to display patient name	(only first 64)
	/* to be followed by m_nrows * m_ncolumns files */
};

//----------------------------------------------------------------------------
/* request and reply use the same structure */
struct pRTVSDICOMPrintStatus
{
	uint16		m_printerID;
	int16		m_requestStatus;
	uint16		m_requestID;					/* from client */
	uint16		m_priority;
	char		m_jobID[32];					/* server assigned */
	char		m_userName[kpRTVSMaxUserName];	/* from server */
	char		m_firstFileName[16];
	int			m_reserved[2];
};

//----------------------------------------------------------------------------
struct pRTVSServerDICOMConfiguration
{
	uint16	m_QRSourceCount;
	uint16	m_PrintDestinationCount;
	/* to be followed by actual data */
};

//----------------------------------------------------------------------------
// general patient/study/series queries. We truncate description to 68 chars
struct pRTVSDicomInformation
{
	uint32			m_validFields;
	DICOMPName		m_patientName;
	DICOMUID		m_patientID;
	char			m_patientSex[4];
	DICOMDate		m_patientBirthDate;
	DICOMSNUM		m_studyCount;

	DICOMUID		m_studyUID;
	char			m_studyID[20];	// SH 16 char max
	DICOMDateRange	m_studyDate;	// can contain ranges yyyymmdd-yyyymmdd
	DICOMTime		m_studyTime;
	char			m_accessionNumber[20];   // SH 16 max
	DICOMName		m_radiologistName;
	DICOMName		m_physicianName;
	char			m_modalitiesInStudy[16];
	DICOMName		m_studyDescription;
	DICOMSNUM		m_seriesCount;
	DICOMSNUM		m_imagesInStudy;
	char			m_characterSet[64];

	DICOMUID		m_seriesUID;
	DICOMDate		m_seriesDate;	// yyyymmdd
	DICOMTime		m_seriesTime;	// hhmmss+6
	DICOMNUM		m_seriesNumber;	// IS 12 max
	char			m_modality[8];
	DICOMSNUM		m_imageCount;
	DICOMName		m_seriesDescription;

	char			m_bodyPartExamined[16];
	pRTVSDICOMDevice	m_source;

	uint32		m_auxDataInfo;  /* read, unread, interactiveReport etc */
 	uint16		m_readStatus;
	int16		m_daysToLock;   /* how many days for the lock (autodelete). -1 for forever */
};

#if 0
struct pRTVSDICOMStudyInformation
{
	uint32			m_validFields;
	DICOMName		m_patientName;
	DICOMUID		m_patientID;
	char			m_patientSex[4];
	DICOMDate		m_patientBirthDate;
	DICOMUID		m_studyUID;
	char			m_studyID[20];	// SH 16 char max
	DICOMDate		m_studyDate;	// can contain ranges yyyymmdd-yyyymmdd
	DICOMDate		m_studyTime;
	char			m_accessionNumber[20];   // SH 16 max
	DICOMName		m_radiologistName;
	DICOMName		m_physicianName;
	char			m_modalitiesInStudy[16];
	DICOMName		m_studyDescription;

	pRTVSDICOMDevice	m_source;
	uint16				m_auxDataStatus;  /* read, unread, interactiveReport etc */
	short				m_studyCount;
	uint16				m_readStatus;
};

struct pRTVSDICOMSeriesInformation
{
	pRTVSDICOMStudyInformation m_studyInfo;
	DICOMUID		m_seriesUID;
	short			m_seriesNumber;	
	short			m_imageCount;
	char			m_modality[8];
	DICOMName		m_seriesDescription;
	char			m_bodyPartExamined[16];
};
#endif
//----------------------------------------------------------------------------
// more detailed info about a particular series: all structures should
// have the entries in SerisBasic as the begining entry.
struct pRTVSSeriesBasic
{
	uint16		m_modality;			// modality - enums
	uint16		m_groupCount;		// how many distinct groups
	uint16		m_bytesPerPixel;	// how the pixel is stored
	uint16		m_bitsPerPixel;		// dynamc range of the data
	uint16		m_xSize;
	uint16		m_ySize;
	uint16		m_zSize; 
	uint16		m_groupID;
	uint16		m_referenceGroupID;
	uint16		m_status;
	uint8		m_canFastLoad;
	char		m_groupType[15];
};

//----------------------------------------------------------------------------
struct pRTVSCTSeriesInfo
{
	uint16		m_modality;
	uint16		m_groupCount;
	uint16		m_bytesPerPixel;
	uint16		m_bitsPerPixel;
	uint16		m_xSize;
	uint16		m_ySize;
	uint16		m_zSize; 
	uint16		m_groupID;
	uint16		m_referenceGroupID;
	uint16		m_status;				//kpRTVSNonUniform
	uint8		m_canFastLoad;
	char		m_groupType[15];
	
	// CT specific
	uint16		m_IsEqualSpacing;
	float		m_xResolution;	// in mm
	float		m_yResolution;	// in mm
	float		m_zResolition;	// in mm
};

//----------------------------------------------------------------------------
struct pRTVSMRSeriesInfo
{
	uint16		m_modality;
	uint16		m_groupCount;
	uint16		m_bytesPerPixel;
	uint16		m_bitsPerPixel;
	uint16		m_xSize;
	uint16		m_ySize;
	uint16		m_zSize; 
	uint16		m_groupID;
	uint16		m_referenceGroupID;
	uint16		m_pad;
	uint8		m_canFastLoad;
	char		m_groupType[15];
	
	// MR specific
	uint16		m_IsEqualSpacing; 
	uint16		m_phaseCount;
};

//----------------------------------------------------------------------------
// group IDs - different types/sequences within a series
enum
{
	kpRTVSDefaultGroup,
	kpRTVSXAPlaneA = 128,
	kpRTVSXAPlaneB = 256,
	kpRTVSMRPhase1 = 1024,
	kpRTVSMRPhase2 = 2048,
	kpRTVSMRPhase3 = 4096
};

// XA info also used by CR/DR
struct pRTVSXASeriesInfo
{
	uint16		m_modality;
	uint16		m_groupCount;
	uint16		m_bytesPerPixel;
	uint16		m_bitsPerPixel;
	uint16		m_xSize;
	uint16		m_ySize;
	uint16		m_zSize; 
	uint16		m_groupID;
	uint16		m_referenceGroupID;
	uint16		m_pad;
	uint8		m_canFastLoad;
	char		m_groupType[15]; 

	// XA specific

	DICOMUID	m_SOPInstanceUID;
	DICOMUID	m_ReferenceSOPInstanceUID;
	uint16		m_frameCount;
	uint16		m_xaPad;
};

//----------------------------------------------------------------------------
union pRTVSDICOMSeriesInfo
{
	uint16				m_modality;
	pRTVSSeriesBasic	m_basic;
	pRTVSCTSeriesInfo	m_CTInfo;
	pRTVSMRSeriesInfo	m_MRInfo;
	pRTVSXASeriesInfo	m_XAInfo;
};

//----------------------------------------------------------------------------
// for 12 bit SC
struct pDerivedSeriesInfo
{
	double  m_imageOrientation[6];
	float   m_thickness;
	float	m_pixelSpacing[2];
	float	m_imagePosition[3];
	float	m_location;

	int		m_width;
	int		m_height;
	short	m_bytesPerPixel;
	short	m_hasAnnotation;
	
	uint16	m_window;
	int16	m_level;
	int		m_blendMode; // if blendMode == -1, means this icon is for bmp
	char	m_seriesDescription[68];
	char	m_imageComment[68];
	
	// 2007.05.25 kunikichi Slope Intercept support
	float	m_rescaleSlope;
	float	m_rescaleIntercept;
};

//----------------------------------------------------------------------------

struct pCapturedImage
{
	int16		m_id;
	int16		m_type;
	uint16		m_width;
	uint16		m_height;
	uint16		m_samplesPerPixel;
	uint16		m_bitsPerSample;
	uint16		m_hasAnnotation;
	int16		m_pad;
	char*		m_pixels;
};

struct pRTVSICON
{
	uint16		m_width;
	uint16		m_height;
	uint16		m_samplesPerPixel;
	uint16		m_bitsPerSample;
	uint16		m_type;
	uint16		m_alloc;
	int16		m_id;
	int16		m_pad;
 	pDerivedSeriesInfo m_dsinfo;
	char*		m_pixels;
};

struct pRTVSCaptured
{
	uint16		m_width;
	uint16		m_height;
	uint16		m_samplesPerPixel;
	uint16		m_bitsPerSample;
	uint16		m_type;
	uint16		m_alloc;
	int16		m_id;
	int16		m_pad;
	pDerivedSeriesInfo m_dsinfo;
	char*		m_pixels;
};


//
struct pRTVSAuxSaveReply
{	
	DICOMUID	m_seriesUID;
	uint16		m_seriesNumber;
	uint16		m_auxDataID;
};

//------------------------------------------------------
struct pRTVSAuxDataInfo
{
	uint32		m_ID;
	uint32		m_type;						// Scene, MASK etc
	DICOMUID	m_seriesUID;				/* the AuxSeries UID */
	char		m_name[16 + sizeof DICOMUID];
	char		m_subTypeName[32];				// same as anatomy
	char		m_date[kpRTVSMaxDateLength];	//xxxx.xx.xx
	char		m_time[12]; // hh:mm:ss
	char		m_processType[32];
	char		m_processName[32];
	uint16		m_hasIcon;
	uint16		m_operation;					
	uint32		m_volumeID;						// use for update volume
	char		m_volumesHash[68];
	char		m_reserved[4];
};

// need to consolidate this and AuxDataInfo
struct pRTVSAuxDataMaskReq
{
	uint32		m_ID;						// DB ID
	uint32		m_type;						// Scene, MASK etc
	uint32		m_volumeID;	
	DICOMUID	m_seriesUID;				/* the AuxSeries UID */
	DICOMUID	m_studyUID;
	char		m_name[16 + sizeof DICOMUID];
	char		m_subTypeName[32];				// same as anatomy
	char		m_date[kpRTVSMaxDateLength];	//xxxx.xx.xx
	char		m_partName[32];
	uint16		m_operation;
	uint16		m_padShort;
};

//----------------------------------------------------------------------------
// AuxData request identifies an AuxData uniquely
#define kpRTVSMaxPrivateDataRefereces 4

struct pRTVSAuxDataRequest	// request
{
	uint32				m_ID;			// could be databaseID or volumeID
	uint32				m_type;			// AuxType
	DICOMUID			m_seriesUID[kpRTVSMaxPrivateDataRefereces];	
	DICOMUID			m_studyUID[kpRTVSMaxPrivateDataRefereces];
	DICOMUID			m_auxSeriesUID;	// the auxSeries itself
	char				m_name[16 + sizeof DICOMUID];	// fileName if appropriate
	char				m_partName[32];
	char				m_processName[32];
	uint16				m_hasIcon;
	uint16				m_hasImage;
	uint16				m_imageNumber;	// for SC writing
	uint16				m_padShort;
	pRTVSDICOMDevice	m_server;
	int					m_intReserved;
};

//----------------------------------------------------------------------------
struct pRTVSDICOMTagQuery
{
	DICOMUID	m_seriesUID;
	DICOMUID	m_studyUID;
	DICOMUID	m_sopUID;
	uint16		m_isSOPUID;
	uint16		m_pad;
	uint32		m_tagsCount;
	uint32*		m_tags;		// space holder
	// to be followed by m_tagsCount number of tags, each of which is a uint32
};

//----------------------------------------------------------------------------
// a whole bunch of the following can be sent to the client
struct pRTVSDICOMTagValueReply
{
	uint32		m_tag;
	uint32		m_length;
	/* to be followed by the value, always a string */
};

struct pRTVSDICOMTagQueryReply
{
	int32		m_tagsCount;	/* total number of tags in the reply */
};

//----------------------------------------------------------------------------
struct	pRTVSDICOMImageIDInfo
{
	float		m_position;			/* position along axis of acquisition */
	int32		m_imageNumber;		/* image number */
	uint16		m_type;
	uint16		m_numberOfFrames;
};

//----------------------------------------------------------------------------
struct	pRTVSDICOMImageIDQueryReply
{
	DICOMUID				m_imageUID;			/* SOPInstanceUID	*/
	pRTVSDICOMImageIDInfo	m_info;
};

//----------------------------------------------------------------------------
struct pRTVSCompressedDICOMImageIDQueryReply
{
	DICOMUID	m_rootUID;			/* root for SOPInstanceUID	*/
	int32		m_mysize;			
	uint16		m_numberOfUIDs;		
	uint16		m_pad;

	// to be followed by m_numberOfUIDs of 
	//   pRTVSDICOMImageIDInfo 
	//   compressesedUID (null terminated)
};

//----------------------------------------------------------------------------
// retrieve the original image or a preview image - dependent on Minor setting
enum { kpRTVSNearestNeightbor, kpRTVSBiLinear };
enum { kpRTVSFirst= -1, kpRTVSMiddle = -5, kpRTVSLast = -10 };

struct pRTVSDICOMImageRetrieveRequest
{
	DICOMUID	m_imageUID;			// SOPInstanceUID
	DICOMUID	m_seriesUID;
	DICOMUID	m_studyUID;
	int16		m_frameNumber;		// starting from 1
	uint16		m_bufferID;
	uint16		m_groupID;			// not currently used
	uint16		m_xSize;			// 0 to keep the original size
	uint16		m_ySize;			// 0 to keep the original size
	uint16		m_sampleMethod;
};

//----------------------------------------------------------------------------
// for series related queries, we need studyUID and seriesUID
struct pRTVSDICOMSeriesLevelQueryRequest
{	
	DICOMUID	m_seriesUID;
	DICOMUID	m_studyUID;
	uint32		m_type;
	int32		m_what;
	int16		m_sortMethod;
	int16		m_nphase;
	int16		m_groupID;
	int			m_SOPUIDCount;
	char		m_name[128];		// scene and template stuff
	char		m_filter[64];
	// To be followed by m_SOPInstanceUIDCount of SOPInstanceUIDs
	// Not compressed at the moment
	char*		m_SOPUIDList;		// kpRTVSMaxDICOMUIDLength char each
};

//----------------------------------------------------------------------------
struct pRTVSDICOMUID
{
	DICOMUID	m_seriesUID;
	DICOMUID	m_studyUID;
	DICOMUID	m_SOPUID;
	uint32		m_pad;
};

//----------------------------------------------------------------------------
struct pRTVSDICOMPath
{
	DICOMUID	m_seriesUID;
	DICOMUID	m_studyUID;
	DICOMUID	m_SOPUID;
	char		m_path[kpRTVSMaxFilePath];
	int			m_status;		// offline, compressed etc
};

//----------------------------------------------------------------------------
// changing a series access rights
enum 
{ 
	kpRTVSDeleteSeries		 = 1,
	kpRTVSDeleteSeriesAccess = 2, 
	kpRTVSMark				 = 4,
	kpRTVSUnmark			 = 8,
	kpRTVSImport			 = 16,
	kpRTVSDownload			 = 32,
	kpRTVSPush				 = 64,
	kpRTVSRetrieve			 = 128
};

union SeriesOperand
{
	pRTVSDICOMDevice	m_dicomDevice;

};

struct pRTVSDICOMSeries
{
	DICOMUID		m_seriesUID;
	DICOMUID		m_studyUID;
	uint16			m_options;
	uint16			m_operand;
	char			m_optionArgument[32];
	pRTVSAuthorization	m_manager;
	SeriesOperand	m_operandStruct;
};

//----------------------------------------------------------------------------
enum 
{ 
	kpRTVSReadStatus	= 1,
	kpRTVSArchiveStatus = 2
};

struct pRTVSDICOMSeriesAttributes
{
	DICOMUID	m_seriesUID;
	DICOMUID	m_studyUID;
	uint16		m_attributesField;  // which status to modify   ReadStatus for example
	uint16		m_attributes;		// the status itself		Read       for example
	pRTVSAuthorization	m_manager;
};

//----------------------------------------------------------------------------

struct pRTVSNode
{	
	uint32		m_address;
	uint16		m_port;
	uint16		m_family;
	char		m_hostname[64];
	uint16		m_load;				// system load
	uint16		m_ttl;				// connection load.
	char		m_ID[32];			// a unique ID generated by the server
};

//----------------------------------------------------------------------------
/*
 * How to sort the images
 */
enum 
{ 
	kpRTVSSortImageNone, 
	kpRTVSSortImagePosition	= 1,		/* position */
	kpRTVSSortImageNumber	= 2,		/* by instance number & orientation	*/
	kpRTVSSortImageSOPUID	= 3,		/* by SOPUID				*/
	kpRTVSSortImageAs2D		= 4,		/* by instance number only	*/
	kpRTVSSortImageSkip		= 64,		/* additional sort			*/
};

//----------------------------------------------------------------------------
// structure used to create and update a volume with
// voxel data.
struct pRTVSSubvolume
{
	uint32		m_volumeID;
	uint16		m_format;
	uint16		m_voxelSize;
	uint16		m_x0;
	uint16		m_nx;
	uint16		m_y0;
	uint16		m_ny;
	uint16		m_z0;
	uint16		m_nz;
	uint16		m_field;
	uint16		m_compression;
	uint16		m_save;
	uint16		m_pad;
	unsigned char* m_voxels;
};

//----------------------------------------------------------------------------
// structure used to create and update a mask with masking bits
struct pRTVSMask 
{
	uint32		m_volumeID;		

	uint16		m_op;			// requested operation

	uint16		m_nx;			//	Whole Mask X Size
	uint16		m_ny;			//	Whole Mask Y Size
	uint16		m_nz;			//	Whole Mask Z Size

	uint16		m_sm_x0;		// The sub-mask range originX 
	uint16		m_sm_y0;		// The sub-mask range originY 
	uint16		m_sm_z0;		//					  originZ	
	uint16		m_sm_nx;		//					  size X	
	uint16		m_sm_ny;	    //					  size Y
	uint16		m_sm_nz;		//					  size Z

	uint16		m_sv_x0;		// The sub-volume range originX 
	uint16		m_sv_y0;		// The sub-volume range originY 
	uint16		m_sv_z0;		//						originZ	
	uint16		m_sv_nx;		//						size X	
	uint16		m_sv_ny;	    //						size Y
	uint16		m_sv_nz;		//						size Z

	uint16      m_firstBit;
	uint16      m_bitsUsed;

	uint32		m_length;				
	uint16		m_compression;			// flag indicating compressed buffer
	uint32		m_decompressedlength;	// valid only if data is compressed.
	unsigned char* m_maskBits;
};

//----------------------------------------------------------------------------
struct pRTVSMipMapInfo
{
	uint32		m_volumeID;		

	int16       m_rangeMin;		// MipMap Range	Minimum 
	int16       m_rangeMax;		// MipMap Range Maximum 
	uint16      m_resize;		// flag indicating image Resize after rendering	
};

//----------------------------------------------------------------------------
enum 
{ 
		kpRTVSRead		= (1<<0), 
		kpRTVSWrite		= (1<<1), 
		kpRTVSAppend	= (1<<2),
		kpRTVSOverwrite	= (1<<3),
		kpRTVSList		= (1<<4),
		kpRTVSOpen		= (1<<10), 
		kpRTVSClose		= (1<<11)
};

//----------------------------------------------------------------------------
struct pRTVSServerFileRequest
{
	uint16	m_command;
	uint16	m_recursive;
	uint16	m_overwrite;
	uint16	m_groupRoot;	// reads/writes to group home rather than user home
	char	m_commandString[8];
	uint16	m_clientFileNameLength;
	uint16	m_serverFileNameLength;
};

//--------------------------------------------------------------------------
enum 
{ 
	kpRTVSDirList		= 1, 
	kpRTVSDirCreate		= 2,
	kpRTVSDirDelete		= 3,
	kpRTVSFileDelete	= 4,
	kpRTVSFileStat		= 5,
	kpRTVSFileRead		= 6,
	kpRTVSFileWrite		= 7
};

struct pRTVSFileRequest
{
	uint16	m_type;
	uint16	m_recursive;
	uint16	m_length;
	uint16	m_command; 
	uint32	m_byteDataSize; //
	// these two paddings make coding much simpler
	char*	m_fileName;
	char*	m_byteData;
	/* to be followed by the name of the file, then the byte data*/
};

struct pRTVSDirEntry
{
	uint32		m_size;			// size of the entry
	uint32		m_mtime;		// modification time, in seconds from 2000
	uint32		m_ctime;
	uint32		m_mode;
	uint16		m_length;		// length in bytes of the entry
	// to be followed by the entry itself
};

struct pRTVSDirListReply
{
	uint16	m_totalEntries;
	uint16	m_pad;
	/* to be followed by totalEntries of pRTVSDirEntry */
};

//----------------------------------------------------------------------------
struct pRTVSByteData
{
	uint32		m_ID;		// file Handle or other ID 
	uint32		m_size;		// size of the data
	uint16		m_rw;		// read or write
	uint16		m_pad;		
	char *		m_bytes;
	// real data to follow.
};

//----------------------------------------------------------------------------
// store rendering parameters: fileName is the settings file name
// and settingsName is the name of a particular setting.
struct pRTVSSetting
{
	pRTVSName	m_fileName;
	char		m_settingName[32];
	uint32		m_clientDataSize;
	uint16		m_append;			// for writing only
	uint16		m_excludeVolume;	// only load rendering parameters,
	uint16		m_settingType;		// regular setting or report
	uint16		m_pad;
};

//----------------------------------------------------------------------------
// server replies with the following data
struct pRTVSSettingReply
{
	pRTVSVolume		m_volume;
	pRTVSByteData	m_clientData;
};

//----------------------------------------------------------------------------
// request data to save a file onto disk. Limited support
struct	pRTVSVolumeStore
{
	pRTVSName	m_fileName;
	uint32		m_volumeID;
	uint16		m_overwrite;
	uint16		m_pad;
};

//----------------------------------------------------------------------------
enum 
{ 
	kpRTVSAsIs,
	kpRTVSReport,
	kpRTVSSettings,
	kpRTVSVolume,
	kpRTVSPrint,
	kpRTVSFileData,
};


enum
{
	kpRTVSIsFile	= (1 << 0),
	kpRTVSIsDir		= (1 << 1),
	kpRTVSIsURead	= (1 << 2),
	kpRTVSIsUWrite	= (1 << 3),
};

struct	pRTVSFile
{
	pRTVSName	m_name;
	uint32		m_size;			// data size or total entries in directory
	uint32		m_time;			// last update time
 	uint16		m_mode;			// read/write/append/data
	uint16		m_type;			// file or dir
};

struct pRTVSFileReq
{
	char		m_path[sizeof pRTVSName];
	uint32		m_size;			// data size or total entries in directory
	uint32		m_time;
 	uint16		m_mode;			// read/write/append/data
	uint16		m_type;			// file or dir
};

//----------------------------------------------------------------------------
struct pRTVSSysTime
{
	uint16		m_year;			// 0 to whatever
	uint16		m_month;		// 1 to 12
	uint16		m_day;			// 1 to 31
	uint16		m_hour;			// 0 to 23
	uint16		m_minutes;		// 0 to 59
	uint16		m_seconds;		// 0 to 59
	uint16		m_mseconds;		// 0 to 9999
	uint16		m_timeType;
};

//----------------------------------------------------------------------------
struct pRTVSSlicerOption
{
	uint16	m_width;
	uint16	m_height;
	uint16	m_window;
	int16	m_level;
	uint16	m_precision;
	uint16	m_method;
};

//----------------------------------------------------------------------------
enum 
{ 
	kpRTVSMean		=	1, 
	kpRTVSDistance	=   (1<<1),
	kpRTVSArea		=	(1<<2),
	kpRTVSMinMax	=	(1<<3),
	kpRTVSStdDev	=	(1<<4),
	kpRTVSVoxel		=	(1<<5),
	kpRTVSProfile	=	(1<<6),
	kpRTVSAngle		=	(1<<7),

	kpRTVSVoxelStat	= (kpRTVSMean|kpRTVSMinMax|kpRTVSStdDev|kpRTVSArea)
};

enum 
{
	kpRTVSOval,
	kpRTVSRectangle,
	kpRTVSPolygon,
	kpRTVSLine,
	kpRTVSPoint,
	kpRTVSLines
};

struct pRTVSMeasurementRequest
{
	uint32			m_volumeID;	// volumeID
	pRTVSSlicer		m_slicer;
	uint16			m_request;	// bit-OR of mean/disitance etc.
	uint16			m_shape;
	uint16			m_npoints;
	uint16			m_pad;
	int16			m_start;		// relative to the sagittal/coronal image
	int16			m_end;			// relative to the satiggal/coronal image
	pRTVSPoint2D	*m_points;
};

//----------------------------------------------------------------------------
struct pRTVSMeasurementReply
{
	uint32		m_volumeID;
	int			m_min;
	int			m_max;
	float		m_mean;
	float		m_stddev;
	float		m_area;
	float		m_distance;
	float		m_angle;
	uint16		m_request;
	uint16		m_ndata;		// number of profile points
	int*		m_profile;		// padding
};

//----------------------------------------------------------------------------
enum { kpRTVSServerLog = 1, kpRTVSTransLog = 2};
struct pRTVSLogEntry
{
	uint16		m_type;
	int16		m_level;
	uint32		m_intPad;
	uint32		m_length;
	char*		m_pcharPad;
};

//----------------------------------------------------------------------------
//	BEGIN - Rob Lewis 10/20/03
struct pRTVSDICOMGetServerListRequest
{
	uint16		m_type;
	uint16		m_pad;
};
//	END - Rob Lewis 10/20/03

//----------------------------------------------------------------------------
struct pRTVSDICOMSetServerListRequest
{
	uint16	m_excludeLocalInQuery;
	uint16	m_serverCount;
	// to be followed by m_serverCount of dicom servers

};

//*************************************************************************
// Volume operations
//***********************************************************************
enum 
{ 
	kpRTVSSub  = 0,
	kpRTVSAdd  = 1
};

enum
{
	kpRTVSScaleAndOffset	= (1<<0),
	kpRTVSClamp				= (1<<1),

	kpRTVSSaveAsDICOM		= (1<<12),
	kpRTVSOneSlice			= (1<<13),
	/* alias */
	kpRTVSReMap = kpRTVSScaleAndOffset
};

// linear transforms for a 2D slice
struct pRTVSLinearTransform
{
	float	m_rotation;
	float	m_xscale;
	float	m_yscale;
	float	m_xtranslation;
	float	m_ytranslation;
};

struct pRTVSSlice
{
	uint32				m_volumeID;
	pRTVSSlicer			m_slicer;
	int16				m_delta;
	int16				m_pad;
};

// slice1 - slice2
struct pRTVSVolumeOpsSlicer
{
	int16					m_opCode;
	int16					m_option;
	pRTVSSlice				m_slice1;
	pRTVSSlice				m_slice2;
};

struct pRTVSVolumeOps3DTransform
{
	uint32					m_volumeID1;
	uint32					m_volumeID2;
	int16					m_opCode;
	int16					m_option;
	pRTVSMatrix				m_transform1;
	pRTVSMatrix				m_transform2;
};

struct pRTVSVolumeOps
{
	uint32					m_volumeID1;
	uint32					m_volumeID2;
	int16					m_opCode;
	int16					m_option;
	pRTVSLinearTransform	m_transform1;
	pRTVSLinearTransform	m_transform2;
};

//-----------------------------------------------------------------------
// virtual DICOM tags
enum pRTVSVirtualDICOMTags
{
	kpRTVSPercentRtoR = 0x00790010
};

//-----------------------------------------------------------------------
enum { kpRTVSCommandUnknown, kpCommandSystem, kpRTVSCommandAlgorithm };
struct pRTVSCommand
{
	pRTVSCommand()
	{
		m_type			= kpRTVSCommandUnknown;
		m_volumeID[0]		= 0;
		m_volumeID[1]		= 0;
		m_command[0]	= '\0';
		m_argument[0]	= '\0';
		m_name[0]		= '\0';
		m_numVolumes    = 0;
	}

	uint32	m_volumeID[2];
	uint16	m_type;
	uint16	m_numVolumes;
	char	m_command[64];		// the command 
	char	m_name[32];			// command/algorithm name
	char	m_argument[256];
};

//-------------------------------------------------------------------
struct pRTVSCommandReply
{
	uint32	m_volumeID;
	uint16	m_dataLength;
	uint16	m_pad;
	/* to be followed by m_dataLength of data */
};

//-------------------------------------------------------------------
enum 
{
	kpRTVSTaskStatusUnknown	= 0,
	kpRTVSTaskCompleted		= 1,
	kpRTVSTaskFailed		= 2,
	kpRTVSTaskAborted		= 4,
	kpRTVSTaskRunning		= 8,
	kpRTVSTaskPending		= 16,
	kpRTVSTaskSuspended		= 32
};

struct pRTVSAsyncTaskStatusReply
{
	int16		m_status;
	uint16		m_total;
	uint16		m_completed;
	uint16		m_dataLength;
	char		m_name[32];
	char		m_taskID[16];
	char		m_description[48];
	// to be followed by m_dataLength bytes
};


struct pRTVSAsyncTask
{
	uint16	m_hide;
	uint16	m_type;
};

//---------------------------------------------------------------------------
struct pRTVSDepthOrFrameBuffer
{
	uint32	m_volumeID;		// to identify the location on which the buffer will be created on the server.
	uint16	m_format;
	uint16  m_bufferType;	// 0 -> Depth; 1 -> Frame
	uint16	m_ROIOnly;
	uint16	m_width;
	uint16	m_height;
	uint16	m_xorigin;
	uint16	m_yorigin;
	uint16	m_xsize;
	uint16	m_ysize;

	// depend on the ROIOnly setting, what follows could be the entire buffer
	// or just the ROI (region of interest)
	void*   m_buffer;	// holds the buffer
};

struct pRTVSPingData
{
	uint16	m_ID;
	uint16	m_updateIdleTime;
};

struct pRTVSDisconnectData
{
	int32	m_ID;
	int16	m_status;
	int16	m_pad;
};

//---------------------------------------------------------------------------
// Plug-in stuff
enum { kRTVSMaxPluginVolumes = 2 };

struct pRTVSPlugInInfo
{
	int		m_type;
	char	m_name[64];
	char	m_description[64];
};

//----------------------------------------------------------------------------
#define kpRTVSMaxMipMaps  6
struct pRTVSPluginVolume
{
	uint64		m_mipmapBufferID[kpRTVSMaxMipMaps];
	pRTVSMatrix	m_modelMatrix;
	pRTVSVolume	m_volume;
	int32		m_location;
	int16		m_numMipmaps;
	int16		m_pad;
};

//----------------------------------------------------------------------------
struct pRTVSPluginVolumes
{
	pRTVSPluginVolume m_volumes[kRTVSMaxPluginVolumes];
	uint16	m_numVolumes;
	uint16	m_pad;
};

//----------------------------------------------------------------------------
// both request and reply
struct pRTVSChooseMipmapLevel
{
	uint32	m_volumeID;
	int16	m_minMipmapLevel;
	int16	m_maxMipmapLevel;
	int16	m_chosenMipmapLevel;
	int16	m_pad;
};

//K.Ko 2010/03/03 for Anonymize
#if defined(PXADMIN_SERVER) || defined(PXADMIN_CLIENT) 
struct pRTVSAnonymizeSeries
{
//
DICOMUID	m_seriesUID;
DICOMUID	m_studyUID;
//new 
uint32		m_NewUIDFlag;
DICOMUID	m_NewPatientID;
DICOMPName	m_NewPatientName;
//
int16		m_copyMode; //0: Anonymize, 1: copy all TAG 
pRTVSAuthorization	m_manager;
};
//
//----------------------------------------------------------------------------
// This structure is exposed to the client. Modify nvr.h if
// changes are made here.
// DataServer information
struct pRTVSDataServerStatus  //K.Ko 2010/03/29
{
#define DataServerMediaPointMax (16)
	uint16		m_status;
	uint32		m_errorCode;
	uint16		m_mediaPointTotalNum;		// total number of mediaPoint
	uint32		m_mediaPointTotalSpace[DataServerMediaPointMax];	// total space size (MB)
	uint32		m_mediaPointFreeSpace[DataServerMediaPointMax];	// free space size (MB)
	uint32		m_Ext[4];					// reserved
};
#endif

#endif   // RTVSPROTOCOL_H_
