/***********************************************************************
 * AutoRoutingMonitor.h
 *---------------------------------------------------------------------
 *		 
 *
 *	
 *  
 *-------------------------------------------------------------------
 */
#ifndef AUTO_ROUTING_MONITOR_H
#define AUTO_ROUTING_MONITOR_H

#include "PxNetDB.h"
#include "rtvPoolAccess.h"
//#include "RTVDiCOMService.h"
#include "IntervalProcessor.h"
 
class  AutoRoutingAEMan ;//#17 2012/05/11 K.Ko

class CAutoRoutingMonitor : public RTVInactiveManager
{
public:
	CAutoRoutingMonitor();
	~CAutoRoutingMonitor();
	 static CAutoRoutingMonitor& theAutoRoutingMonitor();
	 
	 AutoRoutingAEMan * getInactiveHandler(const std::string &id);

	virtual int Process(void);
	//#17 2012/05/11 K.Ko
	void setAutoRoutingMan(AutoRoutingAEMan *AEMan)  {  m_AutoRoutingMan = AEMan ;};
	
	//#20 SeriesñàÇ…AutoRoutingÇçsÇ§Å@2012/05/23Å@K.KO
//	bool tryAutoRouting();
protected:
	
	virtual int doProcess();
	AutoRoutingAEMan *m_AutoRoutingMan;//#17 2012/05/11 K.Ko
};

#endif // AUTO_ROUTING_MONITOR_H

