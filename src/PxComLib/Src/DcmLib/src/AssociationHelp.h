// AssociationHelp
//////////////////////////////////////////////////////////////////////
 

#if !defined(AFX_ASSOCIATION_HELP_H_)
#define AFX_ASSOCIATION_HELP_H_
 
 
#pragma warning (disable: 4819)

#include "dcmtk/ofstd/ofstdinc.h"


#include "dcmtk/dcmnet/dimse.h"
#include "dcmtk/dcmnet/diutil.h"
#include "dcmtk/dcmdata/dcfilefo.h"
//#include "dcmtk/dcmdata/dcdebug.h"
#include "dcmtk/dcmdata/dcuid.h"
#include "dcmtk/dcmdata/dcdict.h"
#include "dcmtk/dcmdata/cmdlnarg.h"
#include "dcmtk/ofstd/ofconapp.h"
#include "dcmtk/ofstd/ofstd.h"
#include "dcmtk/ofstd/ofdatime.h"
#include "dcmtk/dcmdata/dcuid.h"         /* for dcmtk version name */
#include "dcmtk/dcmnet/dicom.h"         /* for DICOM_APPLICATION_ACCEPTOR */
#include "dcmtk/dcmdata/dcdeftag.h"      /* for DCM_StudyInstanceUID */
#include "dcmtk/dcmdata/dcostrmz.h"      /* for dcmZlibCompressionLevel */
#include "dcmtk/dcmnet/dcasccfg.h"      /* for class DcmAssociationConfiguration */
#include "dcmtk/dcmnet/dcasccff.h"      /* for class DcmAssociationConfigurationFile */


class DcmXTSCPResponseParam
{
public:
	void clear() {
		memset(this,0,sizeof(DcmXTSCPResponseParam));
	}
	T_ASC_PresentationContextID m_presIdCmd;
	
	
	DcmDataset *m_statusDetail;
	//
	T_DIMSE_C_StoreRQ	m_store_request;
	T_DIMSE_C_StoreRSP	m_store_response; 
	//
	T_DIMSE_C_FindRQ	m_find_request;  
	T_DIMSE_C_FindRSP	m_find_response; 
	 
	//
	T_DIMSE_C_MoveRQ	m_move_request;  
	T_DIMSE_C_MoveRSP	m_move_response; 
	//
	DcmDataset*			m_response_datas;
	//

	T_DIMSE_C_EchoRQ	m_echo_request;
	T_DIMSE_C_EchoRSP	m_echo_response; 
	//
	T_DIMSE_Message		m_reqMsg;
};



class AssociationHelpBase
{
public:
	AssociationHelpBase(){
	 init();
	};
	virtual ~AssociationHelpBase(){};
	void setupLogLevel(int logLevel) ;
virtual void init()
	{
		m_networkTransferSyntax = EXS_Unknown;
		opt_verbose =  OFTrue;//OFFalse;// OFTrue;
		opt_debug =  OFTrue;//OFFalse;//OFTrue;
		opt_dimse_timeout = 0;
		opt_blockMode = DIMSE_BLOCKING;
	}

void setNetworkTransferSyntax(E_TransferSyntax newXfer) { 
	m_networkTransferSyntax = newXfer;
};
 E_TransferSyntax m_networkTransferSyntax  ;
 OFBool opt_verbose;
 OFBool opt_debug ;
 int opt_dimse_timeout  ;
 T_DIMSE_BlockingMode opt_blockMode  ;
protected:
};
class AssociationHelpClient  : public AssociationHelpBase
{
public:
	AssociationHelpClient();
virtual ~AssociationHelpClient();
virtual void init();
///
OFBool isaListMember(OFList<OFString>& lst, OFString& s);
///
OFCondition addPresentationContext(T_ASC_Parameters *params,
    int presentationContextId, const OFString& abstractSyntax,
    const OFString& transferSyntax,
    T_ASC_SC_ROLE proposedRole = ASC_SC_ROLE_DEFAULT);
OFCondition addPresentationContext(T_ASC_Parameters *params,
    int presentationContextId, const OFString& abstractSyntax,
    const OFList<OFString>& transferSyntaxList,
    T_ASC_SC_ROLE proposedRole = ASC_SC_ROLE_DEFAULT);
///
OFCondition addStoragePresentationContexts(T_ASC_Parameters *params, OFList<OFString>& sopClasses);

static void StoreProgressCallback(void * /*callbackData*/,
    T_DIMSE_StoreProgress *progress,
    T_DIMSE_C_StoreRQ * /*req*/);
static void FindProgressCallback(
    void *callbackData,
        T_DIMSE_C_FindRQ *request,      /* original find request */
        int responseCount,
        T_DIMSE_C_FindRSP *response,    /* pending response received */
        DcmDataset *responseIdentifiers /* pending response identifiers */);

static void setProgressCallbackVerbose(OFBool verbose);

///
static OFCondition
My_DIMSE_storeUser(
	T_ASC_Association *assoc, 
	DcmXTSCPResponseParam *ResponseParam,
//	T_ASC_PresentationContextID presId,
//	T_DIMSE_C_StoreRQ *request,
//	const char *imageFileName, 
	DcmDataset *imageDataSet,
	DIMSE_StoreUserCallback callback, void *callbackData,
//	T_DIMSE_BlockingMode blockMode, 
//	int timeout,
//	T_DIMSE_C_StoreRSP *response,
//	DcmDataset **statusDetail,
    T_DIMSE_DetectedCancelParameters *checkForCancelParams,
    long imageFileTotalBytes);
//////////
static OFCondition
My_DIMSE_findUser(
        T_ASC_Association *assoc,
		DcmXTSCPResponseParam *ResponseParam,
  //      T_ASC_PresentationContextID presID,
  //      T_DIMSE_C_FindRQ *request, 
		DcmDataset *requestIdentifiers,
  //      DIMSE_FindUserCallback callback, void *callbackData,
  //      T_DIMSE_BlockingMode blockMode, int timeout,
   //     T_DIMSE_C_FindRSP *response, DcmDataset **statusDetail
   T_DIMSE_DetectedCancelParameters *checkForCancelParams
		);
/////////////
static OFCondition
My_DIMSE_moveUser(
        /* in */
        T_ASC_Association *assoc,
		DcmXTSCPResponseParam *ResponseParam,
   //     T_ASC_PresentationContextID presID,
  //      T_DIMSE_C_MoveRQ *request,
        DcmDataset *requestIdentifiers
     //   DIMSE_MoveUserCallback callback, void *callbackData
        /* blocking info for response */
    //    T_DIMSE_BlockingMode blockMode, int timeout,
        /* sub-operation provider callback */
    //    T_ASC_Network *net,
     //   DIMSE_SubOpProviderCallback subOpCallback, void *subOpCallbackData,
        /* out */
    //    T_DIMSE_C_MoveRSP *response, DcmDataset **statusDetail,
     //   DcmDataset **responseIdentifers,
     //   OFBool ignorePendingDatasets = OFFalse
			);
//////////////
static OFCondition 
My_DIMSE_readCStoreResponse(T_ASC_Association * assoc,T_DIMSE_Message *msg_out,DcmDataset **dataset_out,
					  DcmXTSCPResponseParam *ResponseParam,
					  T_DIMSE_BlockingMode blockMode, 
					  int timeout,
					  T_DIMSE_DetectedCancelParameters *checkForCancelParams);

///
static OFCondition 
My_DIMSE_readCFindResponse(T_ASC_Association * assoc,T_DIMSE_Message *msg_out,DcmDataset **dataset_out,
					  DcmXTSCPResponseParam *ResponseParam,
					  T_DIMSE_BlockingMode blockMode, 
					  int timeout,
					  T_DIMSE_DetectedCancelParameters *checkForCancelParams);
/////////
static OFCondition 
My_DIMSE_readCMoveResponse(T_ASC_Association * assoc,T_DIMSE_Message *msg_out,
					  DcmXTSCPResponseParam *ResponseParam,
//					  DIMSE_MoveUserCallback callback, void *callbackData,
					  T_DIMSE_BlockingMode blockMode, int timeout,
					  /* sub-operation provider callback */
						T_ASC_Network *net,
//						DIMSE_SubOpProviderCallback subOpCallback, void *subOpCallbackData,
						/* out */
					//    T_DIMSE_C_MoveRSP *response, DcmDataset **statusDetail,
						DcmDataset **rspIds,
						OFBool ignorePendingDatasets);

/////////
//#32 2012/08/06 for Echo-SCU
static OFCondition
My_DIMSE_echoUser(T_ASC_Association * assoc, DcmXTSCPResponseParam *ResponseParam);

static OFCondition 
My_DIMSE_readEchoResponse(T_ASC_Association * assoc,T_DIMSE_Message *msg_out,DcmDataset **dataset_out,
					  DcmXTSCPResponseParam *ResponseParam,
					  T_DIMSE_BlockingMode blockMode, 
					  int timeout,int &command);

////////////////
 
OFBool opt_combineProposedTransferSyntaxes  ;
OFBool opt_showPresentationContexts  ;
OFBool opt_correctUIDPadding;

protected:
};
 


class AssociationHelpServer  : public AssociationHelpBase
{
public:
	AssociationHelpServer();
	~AssociationHelpServer();
 virtual void init();

 void setAcceptStorageSOPClassUIDs(const char **sopclssList,int num);
	 
 void setMaxPDUSize(int size) { 
	 m_maxPDU = size;
 };
OFString replaceChars( const OFString &srcstr, const OFString &pattern, const OFString &substitute );
//void executeCommand( const OFString &cmd ); //never use it

 

//void executeEndOfStudyEvents(); //never use it
 

DUL_PRESENTATIONCONTEXT *
findPresentationContextID(LST_HEAD * head,
                          T_ASC_PresentationContextID presentationContextID);


OFCondition acceptUnknownContextsWithTransferSyntax(
    T_ASC_Parameters * params,
    const char* transferSyntax,
    T_ASC_SC_ROLE acceptedRole);

OFCondition acceptUnknownContextsWithPreferredTransferSyntaxes(
    T_ASC_Parameters * params,
    const char* transferSyntaxes[], int transferSyntaxCount,
    T_ASC_SC_ROLE acceptedRole= ASC_SC_ROLE_DEFAULT);

///
//2012/03/23 K.Ko 
OFCondition waitCommands(T_ASC_Association * assoc,T_DIMSE_Message *msg_out,T_ASC_PresentationContextID &out_presID,int timeout);
OFCondition processCommands(T_ASC_Association * assoc,T_DIMSE_Message *new_msg,DcmDataset *dataset_out,DcmXTSCPResponseParam *ResponseParam);

///

OFCondition receiveAssociation(T_ASC_Network *net, T_ASC_Association * &assocReceive,int endOfStudyTimeout=-1);

OFCondition acceptAssociation(  T_ASC_Association * &assocReceive);

//
OFCondition CStoreSCP(T_ASC_Association *assoc,T_DIMSE_Message *msg,DcmDataset *dataset_out,DcmXTSCPResponseParam *ResponseParam);

//
OFCondition CFindSCP(T_ASC_Association *assoc,T_DIMSE_Message *msg,DcmDataset *dataset_out,DcmXTSCPResponseParam *ResponseParam);
//
OFCondition CMoveSCP(T_ASC_Association *assoc,T_DIMSE_Message *msg,DcmDataset *dataset_out,DcmXTSCPResponseParam *ResponseParam);


//
OFCondition echoSCP(T_ASC_Association *assoc,T_DIMSE_Message *msg,DcmDataset *dataset_out,DcmXTSCPResponseParam *ResponseParam);

//
static OFCondition
My_DIMSE_storeProvider( 
	T_ASC_Association *assoc, 
	DcmXTSCPResponseParam *ResponseParam,
	const char* imageFileName, int writeMetaheader,
	DcmDataset **imageDataSet,
//	DIMSE_StoreProviderCallback callback, void *callbackData,
	T_DIMSE_BlockingMode blockMode, int timeout);

static OFCondition
My_DIMSE_sendStoreResponse(   
	T_ASC_Association *assoc, 
	DcmXTSCPResponseParam *ResponseParam
	);
///////////////
static OFCondition
My_DIMSE_findProvider(
        T_ASC_Association *assoc, 
		DcmXTSCPResponseParam *ResponseParam,
		DcmDataset **reqIds,//output
        T_DIMSE_BlockingMode blockMode, int timeout);

static OFCondition
My_DIMSE_sendFindResponse(T_ASC_Association * assoc,
        DcmXTSCPResponseParam *ResponseParam);

///////////////
static OFCondition
My_DIMSE_moveProvider(
        /* in */
        T_ASC_Association *assoc,
		DcmXTSCPResponseParam *ResponseParam,
        DcmDataset **reqIds,//output
 //       DIMSE_MoveProviderCallback callback, void *callbackData,
        T_DIMSE_BlockingMode blockMode, int timeout);

static OFCondition
My_DIMSE_sendMoveResponse(T_ASC_Association * assoc,
        DcmXTSCPResponseParam *ResponseParam);


protected:
 OFCmdUnsignedInt   m_maxPDU ;
OFBool      opt_secureConnection ;    // default: no secure connection
 
OFList<OFString>   outputFileNameArray;
//const char *opt_execOnReception ;        // default: don't execute anything on reception
const char *opt_execOnEndOfStudy ;       // default: don't execute anything on end of study


OFBool             opt_timeNames ;

OFString           callingaetitle;  // calling AE title will be stored here
OFString           calledaetitle;   // called AE title will be stored here

 
OFBool             opt_rejectWithoutImplementationUID ;

 
#ifdef _WIN32
 
//OFBool             opt_execSync ;            // default: execute in background
#endif



/////

OFBool             opt_promiscuous ;
const char *       opt_respondingaetitle ;

///
OFBool             opt_ignore  ;
OFBool             opt_uniqueFilenames ;
OFString    opt_outputDirectory ;          // default: output directory equals "."
int            timeNameCounter; // "serial number" to differentiate between files with same timestamp
OFString           opt_fileNameExtension;
//OFBool             opt_bitPreserving ;
OFBool             opt_useMetaheader ;
//OFCmdUnsignedInt   opt_sleepAfter ; //never use it
const char *opt_sortConcerningStudies  ;  // default: no sorting
///////////////
const char** m_AcceptStorageSOPClassUIDs ;
int m_NumAcceptStorageSOPClassUIDs ;
protected:
//	void executeOnReception(); //never use it
};
#endif // !defined(AFX_ASSOCIATION_HELP_H_)