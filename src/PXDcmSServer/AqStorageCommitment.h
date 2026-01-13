/***********************************************************************
 * AqStorageCommitment.h
 *---------------------------------------------------------------------
 *		Copyright, TeraRecon 2006, All rights reserved.
 *
 *	PURPOSE:
 *		Process Storage Commitment Requests
 *
 *	AUTHOR(S):  Rob Lewis, April 2006
 *
 *-------------------------------------------------------------------
 */
#ifndef STORAGE_COMMITMENT_H
#define STORAGE_COMMITMENT_H

#include <string>
#include <vector>
#include "RTVDiCOMService.h"

typedef std::string TRANSACTION_UID;

//-----------------------------------------------------------------------------
//
struct SOPInstanceDescriptor
{
	std::string m_sopClassUID;
	std::string m_sopInstanceUID;
};

//-----------------------------------------------------------------------------
//
struct NAction
{
public:
	NAction() {}
	virtual ~NAction() {}

	std::string m_remoteAE;
	std::string m_transactionUID;
	std::vector<SOPInstanceDescriptor> m_vSOPInstances;
};

//-----------------------------------------------------------------------------
//
//	Class to handle StorageCommitment SCP role
//	TODO: handle SCU role (for AqGATE).	
//
class AqStorageCommitment : public RTVDiCOMService
{
public:

	enum ProcessState 
	{ 
		kInitialized, 
		kEnterPreprocess, 
		kLeavePreprocess,
		kEnterProcess,
		kLeaveProcess,
		kEnterDestructor, 
		kLeaveDestructor
	};
	
	AqStorageCommitment(DiCOMConnectionInfo& connectInfo, int iMessageID);
	virtual ~AqStorageCommitment();
	
	int PreProcess();
	virtual int Process();
	void LogProcessStatus(void);

protected:
	int ParseNActionMessage(std::string& oTransactionUID);
	int SendNActionRsp(int iStatus);
	int ReceiveNEventResponse();
	int HandleNAction(NAction& iAction);
	int SendNEventMessage(NAction& iAction);
	void CleanUp(void);
	
	ProcessState m_state;
	std::map<TRANSACTION_UID, NAction> m_mapTranactions;
};

#endif // STORAGE_COMMITMENT_H
