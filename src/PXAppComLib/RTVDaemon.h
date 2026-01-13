/***********************************************************************
 * RTVDaemon.h
 *---------------------------------------------------------------------
 *
 */

#ifndef RTVDAEMON_H_
#define RTVDAEMON_H_

#pragma warning (disable: 4786)

#include <windows.h>
#include <vector>
class RTVDaemonProcessor;

enum
{
	kDaemonAutoStart = 0,
	kDaemonManualStart,
	kDaemonDisabled
};

class RTVDaemon
{
public:
	RTVDaemon(RTVDaemonProcessor *iP);
	int		Start(int argc, char *argv[]);
	
public:
	int		ReportStatusToSCMgr(unsigned long, unsigned long, unsigned long);

	static char*	GetLastErrorText( char* oBuf, long bufSize );
	static char*	GetLastErrorText(void );
	int				OnConsole(void) const { return m_debug;}
	unsigned long	m_error;
	SERVICE_STATUS_HANDLE	m_statusHandle;
	SERVICE_STATUS			m_sStatus;
	RTVDaemonProcessor*		m_processor;
	std::vector<std::string>m_dependencies;
	static std::string		m_action;	// what happens after error


	// other daemon related services 
	static BOOL		ReStartService(const char *iServiceName);
	static int		StopService(const char *iServiceName);
	static int		MyStartService(const char *iServiceName);
	static BOOL		GetServiceState(const char *iServiceName, unsigned long& oState);

	//	iStart type can be one of: kDaemonAutoStart, kDaemonManualStart, kDaemonDisabled
	static BOOL		SetServiceStartType(const char* iServiceName, int iStartType);
	static BOOL		GetServiceStartType(const char* iServiceName, int& oStartType);
	
private:
	void	AddToMessageLog(char *);
	/* for debugging only */
	int		m_debug;
	void	CmdInstallService(int iAutoStart=0);
	void	CmdRemoveService(void);
	void	CmdDebugService(int, char **);
	void	CmdStopService(void);
	static void	HandleFailureAction(SC_HANDLE shandle, const char* action);
};


/* Daemon processor */
class RTVDaemonProcessor
{
public:
	RTVDaemonProcessor(const char *iName=0) : m_name(0)
	{
		if (iName)
			Create(iName);
	}

	virtual ~RTVDaemonProcessor(void)
	{
		if(m_name)
			free(m_name);
		m_name = 0;
	}

	void	Create(const char *iName)
	{
		m_name = strdup(iName);
	}

	const char*		GetName(void)        const  { return m_name;}
	const char*		GetDisplayName(void) const  { return m_name;}
	virtual void	Stop(void) {}
	virtual int		PreProcess(void) {return 0;}
	virtual int		Process(int, char **) = 0;
	virtual void	PostProcess(void) {}
	virtual void	OnSystemShutDown(void) {}
	int				OnConsole(void);
private:
	char *m_name;
};


#endif
