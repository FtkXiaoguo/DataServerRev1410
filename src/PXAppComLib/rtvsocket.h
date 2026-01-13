 /*********************************************************************
 *
 * rtvsocket.h
 *
 *---------------------------------------------------------------------
 *   
 *-------------------------------------------------------------------
 */

#ifndef RTVSOCKET_H_
#define RTVSOCKET_H_

#include "rtvbase.h"

#ifdef kRTVSPort
#define kPort kRTVSPort
#else
#define kPort 1200
#endif

class RTVSocket : public iRTVBase
{
 public:
	enum 
	{		
		REFUSED			= -1,			// server is not running
		INVALID_HOST	= -2,			// bad hostname/ip address
		NoADDRESS		= -3,			// can't lookup the ip from hostname
		BUSY			= -4			// port already bound
	};

    RTVSocket(void);
	RTVSocket(RTVSocket &inSock);
    virtual ~RTVSocket(void);

	static	void	NetInit(void);
	static  void	SetBufferSize(int iSizeInKiloBytes); // default is 64
	
	// connect. Wait for up to iWaitmSec. use -1 to indicate indefinite (system)
    int				Connect(const char *iAddress, int iPort=kPort, int iWaitmSec=-1);

    int				Listen(int iPort=kPort, const char* iIP="");
	void			Init(void);
	void			Reset(void);
    int				Accept(void);
    int				Failed(void);
    void			PrintError(const char *iWhere);
    int				GetFD(void) const { return m_fd;}
	void			CloseFD(void);
    int				Send(const char *iMsg, int iLen, int iMode=0) const;
    int				Receive(char *ioBuf, int iLen, int iFlag=0);
    int				Receive(char *ioBuf, int iLen, int iFlag, bool iReceivePartial);
    void			Close(void);
	int				Punt(unsigned int iNBytes);
	int				Peek(char *iBuf, int iLen);
	unsigned int	PeerIP(void);
	const char*		PeerName(void);
	const char*		PeerIPString(void);
	unsigned int	HostIP(void);
	const char*		HostName(void);
	const char*		HostIPString(void);

	// set all socket options
	int				SetOption(int iLevel, int iOptName, const void* iOptVal, int iOptLen);

	// set to false to prevent reverse lookup on the server
	static void		SetReverseLookup(int iYesNo) { m_doReverseLookup	= iYesNo;}
	static void		SetKeepAlive(int iYesNo)	 { m_keepAlive			= iYesNo;}
	const static int m_true;
	const static int m_false;
	static char*	IPToIPString(unsigned int IP, char* oIPString);
 private:
	int					WaitForConnection(int imSec);
    int                 m_socketID;
    int                 m_fd;
    int                 m_status;
	unsigned int		m_hostIP;
	unsigned int        m_peerIP;
	char				m_hostName[256];
	char				m_peerName[256];
	int                 GetErrorStatus(void) const;
	void				GetHostInfo(void);
	const char*         GetErrorMessage(void);
	void				ConvertIPToHostName(unsigned int iIP, char oName[], int iLen=256);
	int					m_sendBufSize;
	int					m_recvBufSize;
	int					m_sameDomain;
	static int			m_defaultBufSize;
	static int			m_doReverseLookup;
	static int			m_noDNS;
	static int			m_keepAlive;
};

#endif
