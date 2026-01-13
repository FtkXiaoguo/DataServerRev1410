/***********************************************************************
 * CStore.h
 *---------------------------------------------------------------------
 *	
 *
 *-------------------------------------------------------------------
 */
#ifndef C_STORE_H
#define C_STORE_H

#include "PxNetDB.h"
#include "RTVDiCOMStore.h"

class CPxWorkQueue;
//-----------------------------------------------------------------------------
class CStore : public RTVDiCOMStore
{
public:
	CStore (DiCOMConnectionInfo& connectInfo, int iMessageID);
	
	~CStore();
	
	int dbg_dumyProc(int level); //2012/03/07 for test
	int dbg_stepByStepProc(int level); //2012/03/07 for test
	int Process();
	void LogProcessStatus(void);

	int PreProcessNoneResponce();//2010/03/16 k.ko #660
	int SuccessResponce(bool error=false);//2010/03/16 k.ko #660

	bool doPostJob(); //#16 2012/04/26 K.Ko

	std::string getSeriesUID() { return m_seriesUID;} //#21 2012/05/29 K.Ko
private:
	///////////
	//#20 SeriesñàÇ…AutoRoutingÇçsÇ§Å@2012/05/23Å@K.KO
	bool registerSeriesOnRouting();
	///////////
//	bool getAutoRoutingAEs(std::vector<std::string> &destAEList/*output*/,CPxWorkQueue * &QueueProc/*output*/); //#16 2012/04/26 K.Ko
	///////////////////////
	int CoerceSOPInstanceUID();
	int HandleTerareconSpecific ();
	int theProcess();
	int GetSingleFrameData (void);
	int HandleCRImage ();
	int HandleCTImage ();
	int HandleMRImage ();
	int HandleSCImage ();
	int HandleXAImage ();
	int HandleUSImage ();
	int HandleNMImage ();
	int HandlePTImage ();
	const char*	GenerateUID(void);
	void SaveForRetry();

	CPxDcmDB		m_db;
	DICOMData	m_dbData;
	char		m_fileName[256];
	char		m_shortfileName[256];
	int			m_imageDBSaved;
};

//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
#endif // C_STORE_H
