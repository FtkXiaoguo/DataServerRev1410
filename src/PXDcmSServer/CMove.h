/***********************************************************************
 * CMove.h
 *---------------------------------------------------------------------
 *		Copyright, PreXion 2012, All rights reserved.
 *
 *	PURPOSE:
 *		Processes CStore Associations.
 *
 *	
 *
 *-------------------------------------------------------------------
 */
#ifndef C_MOVE_H
#define C_MOVE_H

#include <vector>

#include "RTVDiCOMService.h"
//#include "vlidicomstatus.h"

#include "RTVDICOMDef.h"
#include "PxNetDB.h"
#include "rtvpoolaccess.h"

typedef RTVMapAccess<std::string, int> ILE_Service_MAP;

//#8 2012/03/16 K.KO
#define MOVE_SERVICE_NAME "TIDICOM_SCU_Service_List"

class CMove : public RTVDiCOMService
{
public:

	CMove(DiCOMConnectionInfo& connectInfo, int iMessageID);
	virtual ~CMove();

	void LogProcessStatus(void) {};		// Required from abstract class RTVDiCOMService
	int Process(void);

	//#8 2012/03/16 K.KO
	//ServiceListŽw’èŽd‘g‚Ý‚Ì’Ç‰Á
	static bool initCMoveServiceList();
private:
	int	ParseCMoveRQMessage();
	int	OpenCStoreAssociation();
	int	CheckCMoveCancellation(int iSOPInstanceIndex, int &oRespStatus);
	int	AddToFailedSOPList(char * iSOPInstanceUID);
	int	ReceiveAndParseCStoreRspMsg(unsigned int  &oCStoreResponse);
	int			BuildSendMessage(const char* iFilePath);  
	int	SendCMoveResponse(int iResponseCode);
	int	AttemptSendResponse(int iResponseMsg, int iResponseCode);
	int	GetSOPInstanceList();
	void		SendInstances();
	void		HandleFailedInstance(std::string iSopUID);
	int	CECHOProcess();
	int	HandleFinalError(const char* iMsg, int iResponseCode);
	int	BuildInstanceFilter(unsigned long iTag, std::vector<std::string>& iFilter);
	int	DecompressPixelsIfNeedTo(int iMessageID, CPxDicomImage* pImage);
	unsigned 	TransferSyntaxToMask(int iTransferSyntax);
	int			IsAccepted(unsigned int iMaskValue, int iTransferSyntax);
	int			GetPathToInstanceList(std::vector<std::string>& oPaths, 
								std::vector<std::string>& oInstanceUIDs, 
								int iLevel);
	int MakeDicomServerObject(void);
	int MakeOutboundAEObject(void);
	int MakeRemoteAEObject(ApplicationEntity& iRemoteAE);
	int MakeLocalAEObject(void);
	int AuditLogSendSeries(const char* iSeriesInstanceUID);

	int		m_cStoreAssociationID;
	int		m_finalCMoveRespMSG;
	int		m_finalStatus;

	//	For keeping track of how many failures, warnings, etc
	int		m_failedSOPListCount;
	int		m_remainingCStoreSubOperations;
	int		m_successfulCStoreSubOperations;
	int		m_failedCStoreSubOperations;
	int		m_warnedCStoreSubOperations;
	int		m_totalCStoreSubOperations;
	int		m_instanceCount;
	int		m_currentCStoreSubOperationIndex;
	int m_cStoreAssociationStatus;
 
    char	m_targetAETitle[kVR_AE];	//	Where should we send to?
	char	m_targetIPAddress[66];
	int		m_targetPort;

	CPxDcmDB	m_db;
	std::vector<std::string> m_studyInstanceUIDs;
	std::vector<std::string> m_seriesInstanceUIDs;
	std::vector<std::string> m_SOPInstanceUIDs;

	std::vector<std::string> m_instanceV;	
	std::vector<std::string> m_pathV;		//	Path to each instance

	bool		m_abort;
	int			m_level;

	ILE_Service_MAP m_ILEServiceMap;
	int m_studyLevelObjectID;
	int m_seriesLevelObjectID;
	int m_requesterObjectID;
	//
	std::vector<DICOMStudy> m_curStudy;  //#27 2012/06/14 K.Ko
};

#endif // C_MOVE_H
