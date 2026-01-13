/**********************************************************************
 * rtvsocket.cpp
 *---------------------------------------------------------------------
 *	
 *
 *   
 *-------------------------------------------------------------------
 */
#ifndef RTVSOCKET_H_
#include "rtvsocket.h"
#endif


#include <string.h>
#include <stdlib.h>
#include <ctype.h>
#include <assert.h>

#if !defined(_WIN32)
# include <sys/types.h>
# include <sys/socket.h>
# include <netinet/in.h>
# include <unistd.h>
# include <arpa/inet.h>
# include <netdb.h>
# include <errno.h>
#else
# include <winsock.h>
# include <io.h>
# define  close		closesocket
# //define  snprintf	_snprintf
#endif

#include "AqCore/TRPlatform.h"

//-----------------------------------------------------------------------------
const int RTVSocket::m_true = 1;
const int RTVSocket::m_false = 0;

int RTVSocket::m_doReverseLookup = 0;	// auto
int	RTVSocket::m_noDNS = 0;
int	RTVSocket::m_keepAlive = 1;

void RTVSocket::NetInit(void)
{
#ifdef _WIN32
	static int initialized;
	if(!initialized)
	{
		WSADATA wsaData;
		if (WSAStartup(0x202, &wsaData)!= 0) 
		{
			WSACleanup();
			return;
		}
		initialized = 1;
	}
#endif
}

//----------------------------------------------------------------------
RTVSocket::RTVSocket(void)
{
    
//	NetInit();   // Let TRPlatform do it to avoid matching WSACleanup()

	Init();

	if(m_socketID < 0)
	{
		m_status = GetErrorStatus();
		PrintError("socket");
	}
}  


int RTVSocket::m_defaultBufSize = 128; // in Kilobytes

inline int iClamp(int A, int AMin, int AMax) { return (A < AMin ? AMin : (A > AMax ? AMax:A));}
void RTVSocket::SetBufferSize(int iSizeInKBytes)
{
	m_defaultBufSize = iClamp(iSizeInKBytes, 8, 2048);
}

//---------------------------------------------------------------------
void RTVSocket::Init(void)
{
	int type = SOCK_STREAM;
	
	m_fd = m_socketID = socket(AF_INET, type, 0);
	m_hostIP = 0;
	m_peerIP = 0;
	m_hostName[0] = '\0';
	m_peerName[0] = '\0';
	m_sameDomain = 0;
	
#if 1
	int sz;

	m_recvBufSize = m_defaultBufSize * 1024;
	m_sendBufSize = m_defaultBufSize * 1024;

	sz = sizeof  m_recvBufSize;
	setsockopt(m_fd, SOL_SOCKET,SO_RCVBUF, (char*)&m_recvBufSize, sz);
	getsockopt(m_fd, SOL_SOCKET,SO_RCVBUF, (char*)&m_recvBufSize, &sz);
	
	sz = sizeof m_sendBufSize;
	setsockopt(m_fd, SOL_SOCKET,SO_SNDBUF, (char*)&m_sendBufSize, sz);
	getsockopt(m_fd, SOL_SOCKET,SO_SNDBUF, (char*)&m_sendBufSize, &sz);
#else
	m_recvBufSize = m_sendBufSize = 8192;
#endif

	setsockopt(m_fd, SOL_SOCKET, SO_KEEPALIVE, (char *)(m_keepAlive ?&m_true:&m_false), sizeof m_true);
	
	if ( m_recvBufSize < 2048)
		m_recvBufSize = 2048;
	
	if (m_sendBufSize < 2048)
		m_sendBufSize = 2048;
}

//------------------------------------------------------------------------
RTVSocket::RTVSocket(RTVSocket& inSock)
{
	*this = inSock;	
	m_socketID = 0;
}

int RTVSocket::GetErrorStatus(void) const
{
#if !defined(_WIN32)
	return errno;
#else
	return WSAGetLastError();
#endif
}

//--------------------------------------------------------------------------
void RTVSocket::Close(void)
{
	int ret = 0, err = 0;
	if(m_fd > 0)
	{
		if ((ret = close(m_fd)) != 0)
		{
			err = GetErrorStatus();
			if (err != WSAENOTSOCK) // will cleanup this
				assert(ret == 0);
		}
	}
	
    if(m_socketID > 0 && m_socketID != m_fd)
	{
		if ((ret = close(m_socketID)) != 0)
		{
			err = GetErrorStatus();
			assert(ret == 0);
		}
	}
	
	m_fd = m_socketID = 0;

	m_peerIP = 0;
	m_peerName[0] = '\0';
}

//--------------------------------------------------------------------------
void RTVSocket::Reset(void)
{
	static struct linger l;
	static unsigned long sFalse = 0;
	l.l_linger	= 0;
	l.l_onoff	= 1;

	if (m_fd > 0)
	{
		setsockopt(m_fd,SOL_SOCKET,SO_LINGER,(char *)&l,sizeof l);
		Close(); 
		Init();
	}
}

//--------------------------------------------------------------------------
void RTVSocket::CloseFD(void)
{
	if(m_fd > 0)
	{
		int ret = 0, err = 0;
		if ((ret = close(m_fd)) != 0)
		{
			err = GetErrorStatus();
			assert(ret == 0);
		}
	}

	m_fd = 0;
}

//--------------------------------------------------------------------------------------
int RTVSocket::SetOption(int iLevel, int iOptName, const void *iOptVal, int iOptValLen)
{
	return setsockopt(m_fd, iLevel, iOptName, (char *)iOptVal, iOptValLen);
}

//--------------------------------------------------------------------------------------
RTVSocket::~RTVSocket(void)
{
    Close();   
}

//--------------------------------------------------------------------------------------
int RTVSocket::Failed(void)
{
   return m_status;
}

struct EMsg
{
   int err;
   const char *reason;
};

//--------------------------------------------------------------------------------------
static EMsg sMsg[] =
{
#if !defined(_WIN32)
   {EACCES,     "permission denied"},
   {EADDRINUSE, "address already in use"},
   {EADDRNOTAVAIL, "address not available"},
   {EAFNOSUPPORT, "bad family address"},
   {EALREADY   ,  "non-blocking socket connection attempted"},
   {EBADF,       "bad fd"},
   {ECONNREFUSED,"connection refused"},
   {EINVAL,      "already bound or bad namelen"},
   {EINPROGRESS, "delayed - in progress"},
   {EINTR,       "interrupted by signal"},
   {EISCONN,     "already connected"},
   {EIO   ,      "I/O error"},
   {EMFILE,      "descriptor table full"},
   {ENOMEM,      "out of memory"},
   {ENOSR,       "out of stream resources"},
   {ENOTSOCK,    "not a socket"},
   {EPROTONOSUPPORT, "protocol not supported"},
   {ESTALE, "stale NFS handle"},
   {EWOULDBLOCK, "blocking non-blocking socket"},
#else
   {WSAEACCES, "permission denied"},
   {WSAEADDRINUSE,"address already in use"},
   {WSAEALREADY,"operation already in progress"},
   {WSAECONNREFUSED,"connection refused"},
   {WSAEFAULT,"Bad address"},
   {WSAEHOSTDOWN ,"host down"},
   {WSAEHOSTUNREACH,"no route to host"},
   {WSAEMSGSIZE,"message to long"},
   {WSAENOBUFS,"no buffer space available"},
   {WSAENOTCONN,"not connected"},
#endif
   // sentinel 
   {-1000}
};

//--------------------------------------------------------------------------------------
void RTVSocket::PrintError(const char *s)
{
    const char *reason = "Unknown";
    int found;
    EMsg *m = sMsg;
	
    for ( found = 0 ;!found && m->err != -1000; m++)
    {
		if ((found = (m->err == m_status)))
			reason = m->reason;
    }
	
    if(m_verbose)
		ErrMessage("%s: %s(%d)\n", (s ? s:"?"), reason, m_status);
}
          
//--------------------------------------------------------------------------------------
#define sLOOPBACK "127.0.0.1"

static inline int IsIPAddress(const char* iName)
{
	//	-- - 11/12/04 - Without this check, NULL string 
	//		is considered a valid IP address
	if (!iName || strlen(iName) < 3)
		return 0;

	int yes;
	
	for ( yes = 1; yes && *iName; iName++)
		yes = (isdigit(*iName) || *iName=='.');
	
	return yes;
}

//--------------------------------------------------------------------------------------
int RTVSocket::Connect(const char *name, int port, int iWaitinMsec)
{
    struct sockaddr_in server;
    struct hostent *hostent;
	int noDNS = 0;
	
    if(!name || !*name )
	{
		return INVALID_HOST;
	}
	
	gethostname(m_hostName, sizeof(m_hostName)-1);
	
	memset(&server, 0, sizeof(server));
	server.sin_family = AF_INET;
    server.sin_port = htons(port);

	/* for some reason, localhost and 127.0.0.1 does not work */
	if (stricmp(name,"localhost") == 0 || strcmp(name,sLOOPBACK) == 0)
		name = m_hostName;
	
	// figure out the server IP address
	if (IsIPAddress(name))
	{
		server.sin_addr.s_addr = inet_addr(name);
	}
	else 
	{
		if (!(hostent = gethostbyname(name)))
		{
			PrintError("GetHostByname()");
			return NoADDRESS;
		}
		memcpy((char *)&(server.sin_addr.s_addr),*hostent->h_addr_list, hostent->h_length);
	}
	
	
	m_peerIP = server.sin_addr.s_addr;
	snprintf(m_peerName, sizeof m_peerName, "%s", name);
	
	if (iWaitinMsec <= 0)
	{
		m_status = connect(m_socketID, (struct sockaddr *)&server, sizeof(server));
	}
	else
	{
		static unsigned long sFalse = 0, sTrue = 1;
		int ret = ioctlsocket(m_socketID,FIONBIO,&sTrue);
		if (ret != 0)
		{
			int err = WSAGetLastError();
			assert(ret == 0);
		}
		
		m_status = connect(m_socketID, (struct sockaddr *)&server, sizeof(server));
		
		if (m_status == SOCKET_ERROR)
		{
			int err = WSAGetLastError();
			if (err == WSAEWOULDBLOCK)
				m_status = WaitForConnection(iWaitinMsec);
			else
				assert(0);
		}
		ioctlsocket(m_socketID,FIONBIO,&sFalse);
	}
	
    if(m_status < 0)
    {
		m_status = GetErrorStatus();
		PrintError("connect");
		return REFUSED;
    }
	
    return m_status  ? REFUSED:0;
}

//--------------------------------------------------------------------------------------
int RTVSocket::WaitForConnection(int imSec)
{
	fd_set wfds;
	struct timeval sTimeOut;
	int ret = 0;
	
	FD_ZERO(&wfds);
	sTimeOut.tv_sec		= imSec/1000;
	sTimeOut.tv_usec	= (imSec - sTimeOut.tv_sec*1000)*1000;
	FD_SET(m_socketID, &wfds);
    ret = select(m_socketID+1, 0, &wfds, 0, &sTimeOut);
	return (ret <=0 ? -1:0);
}


//--------------------------------------------------------------------------------------
#define kMaxAttempt (1024*1024) // should be bigger than 800K, i.e., one image
 
int RTVSocket::Send(const char *msg, int len, int flag) const
{
	int sent = 0;
	
	if (len > kMaxAttempt)
	{
		int left = kMaxAttempt, last = 1;
		for ( ; sent < len && last > 0 ; )
		{
			last = send(m_fd, msg + sent, left, flag);
			if (last > 0)
			{
				sent += last;
				left = len - sent;
				if (left > kMaxAttempt)
					left = kMaxAttempt;
			}
		}
	}
	else
	{	
		sent = send(m_fd, msg, len, flag);
	}
    
	if(sent != len || sent < 0)
	{
		RTVSocket *s = const_cast<RTVSocket *>(this);
        s->m_status = s->GetErrorStatus();
		s->ErrMessage("Sent %d bytes expected %d\n", sent, len);
        s->PrintError("Send");
        return -1;
    }
	
    return sent;
} 

//--------------------------------------------------------------------------------------
// this is of limited use as the value returned is limited by the buflen
int RTVSocket::Peek(char *buf, int len)
{
	return recv(m_fd, buf, len, MSG_PEEK);
}

//--------------------------------------------------------------------------------------
int RTVSocket::Receive(char *buf, int len, int flag, bool iReceivePartial)
{
    int nbytes = 0, last, clen = len;
	
	do 
	{
		if (clen > kMaxAttempt)
			clen = kMaxAttempt;

		last = recv(m_fd, buf+nbytes, clen, flag);
		if (last > 0)
		{
			nbytes += last;
			clen = len - nbytes;
		}

		if (iReceivePartial && last < len)
			break;

#ifdef _DEBUG
		Message2("Attempting %d (Read %d bytes. %d remaining)\n", len, nbytes, clen < 0 ? 0:clen);
#endif
	}
	while (nbytes < len && last > 0);
	
    if(last <=0  || nbytes != len)
    {
		m_status = GetErrorStatus();
		Message("Recv: received %d bytes expected %d bytes. Status=%d\n", nbytes, len, m_status);
		PrintError("recv");
		return last;
    }
	
    return nbytes;
}

//--------------------------------------------------------------------------------------
int RTVSocket::Receive(char *buf, int len, int flag)
{
    int nbytes = 0, last, clen = len;
	
	do 
	{
		if (clen > kMaxAttempt)
			clen = kMaxAttempt;

		last = recv(m_fd, buf+nbytes, clen, flag);
		if (last > 0)
		{
			nbytes += last;
			clen = len - nbytes;
		}
#ifdef _DEBUG
		Message2("Attempting %d (Read %d bytes. %d remaining)\n", len, nbytes, clen < 0 ? 0:clen);
#endif
	}
	while (nbytes < len && last > 0);
	
    if(last <=0  || nbytes != len)
    {
		m_status = GetErrorStatus();
		Message("Recv: received %d bytes expected %d bytes. Status=%d\n", nbytes, len, m_status);
		PrintError("recv");
		return last;
    }
	
    return nbytes;
}

//--------------------------------------------------------------------------------------
int RTVSocket::Punt(unsigned int iNBytes)
{
	char buf[1024];
	int n, last, remaining = sizeof(buf);

	for (n = 0, last = 1; last > 0 && remaining > 0; )
	{
		n += (last = Receive(buf, remaining));
		if ((remaining = iNBytes - n) > sizeof(buf))
			remaining = sizeof(buf);
	}
	return n;
}


//--------------------------------------------------------------------------------------
#if !defined(_WIN32)
extern "C" { gethostname(char *, int); }
#endif

int RTVSocket::Listen(int port, const char* iP)
{
    struct hostent *hostent;
	struct sockaddr_in server;
	
    gethostname(m_hostName, sizeof(m_hostName)-1);
	
    Message("Server: %s\n", m_hostName);
	
    if(!(hostent = gethostbyname(m_hostName)))
	{
		ErrMessage("Can't get hostname: %s\n", m_hostName);
		return -1;
	}
	
    server.sin_family = AF_INET;
    server.sin_port = htons(port);
	
    memcpy((char *)&(server.sin_addr.s_addr),*hostent->h_addr_list, hostent->h_length);
	
    Message("ServerResolved: %s (%s, 0x%x)\n", hostent->h_name, inet_ntoa(server.sin_addr),server.sin_addr);
	
	
	m_hostIP = server.sin_addr.s_addr;

	if (!iP)
		iP = "";

	if (stricmp(iP,"Any") == 0)
	{
		server.sin_addr.s_addr = INADDR_ANY;
	}
	else if (!*iP)
	{
		server.sin_addr.s_addr = m_hostIP;
	}
	else
	{
		server.sin_addr.s_addr  = inet_addr(iP);
	}
	 

    m_status = bind(m_socketID, (struct sockaddr *)&server, sizeof(server));
	
    if(m_status < 0)
    {
		m_status = GetErrorStatus();
		PrintError("bind");
		return BUSY;
    }
	
    if((m_status = listen(m_socketID, 5)) < 0)
    {
		m_status = GetErrorStatus();
		PrintError("listen");
		return -1;
    }
    return 0;
	
}

//--------------------------------------------------------------------------------------
// get the hostname from IP: needs DNS
void RTVSocket::ConvertIPToHostName(unsigned int IP, char *oName, int iLen)
{
	struct hostent *hostent = 0;
	in_addr in;
	
	in.s_addr = IP;
	
	if (m_doReverseLookup == 1 || (m_doReverseLookup < 0 && m_sameDomain && !m_noDNS))
	{
		hostent = gethostbyaddr((char *)&IP, 4, AF_INET);
		// this turns off reverse lookup permenantly
		if (!hostent)
			m_noDNS = 1;
	}
	
	snprintf(oName, iLen, "%s",  hostent ? hostent->h_name : inet_ntoa(in));
}

//--------------------------------------------------------------------------------------
int RTVSocket::Accept(void)
{
	sockaddr_in		client;
	sockaddr_in		server;

    int k = sizeof(client);
	
    m_fd = accept(m_socketID, (struct sockaddr *)&client, &k);
	
    if(m_fd < 0)
    {
		m_status = GetErrorStatus();
        PrintError("accept");
        return -1;
    }
	
	// reset origin
	m_peerIP = client.sin_addr.s_addr;

	// check my name
	getsockname(m_fd, (struct sockaddr *)&server, &k);

	if (m_hostIP != server.sin_addr.s_addr)
	{
		m_hostIP  = server.sin_addr.s_addr;
		Message("Re-homed. IP=0x%x (%s)\n",m_hostIP,inet_ntoa(server.sin_addr));
	}

	// not exact - just a reasonable guess
	m_sameDomain = ((m_hostIP & 0x0000ffff) == (m_peerIP & 0x0000ffff));
	ConvertIPToHostName(m_peerIP, m_peerName);
	
	Message("accepted from %s (%s 0x%x)\n",m_peerName, inet_ntoa(client.sin_addr), m_peerIP);
	
    return m_fd;
}

//--------------------------------------------------------------------------------------
char*  RTVSocket::IPToIPString(unsigned int iP, char* oIPString)
{
	in_addr addr;
	addr.s_addr = iP;
	strcpy(oIPString, inet_ntoa(addr));
	return oIPString;
}

//--------------------------------------------------------------------------------------
unsigned int RTVSocket::PeerIP(void)
{
	return m_peerIP;
}

//--------------------------------------------------------------------------------------
const char* RTVSocket::PeerIPString(void)
{
	static char ipsBuf[6][32];
	static int ind = 0;
	char *p = ipsBuf[ind = (ind + 1)%6];

	IPToIPString(m_peerIP, p);
	return p;
}
	
//--------------------------------------------------------------------------------------
unsigned int RTVSocket::HostIP(void)
{
	if (m_hostIP == 0)
		GetHostInfo();
	 
	return m_hostIP;
}

//--------------------------------------------------------------------------------------
const char* RTVSocket::HostIPString(void)
{
	static char ipsBuf[6][32];
	static int ind = 0;
	char *p = ipsBuf[ind = (ind + 1)%6];

	IPToIPString(m_hostIP, p);
	return p;
}

//--------------------------------------------------------------------------------------
const char *RTVSocket::PeerName(void)
{	 
	if ( m_peerName[0]=='\0')
		ConvertIPToHostName(m_peerIP, m_peerName);

	return m_peerName;
}

//--------------------------------------------------------------------------------------
void RTVSocket::GetHostInfo(void)
{
	char buf[256];
	struct hostent *hostent = 0;
	
	gethostname(buf, sizeof(buf)-1);
	
	if(!(hostent = gethostbyname(buf)))
	{
		ErrMessage("Can't get host(%s) IP\n", buf);
		m_hostIP = inet_addr(sLOOPBACK);
		strcpy(m_hostName,"localhost");
		return;
	}
	
	in_addr in;
	memcpy(&in, *hostent->h_addr_list, hostent->h_length);
	m_hostIP = in.s_addr;
	strcpy(m_hostName,hostent->h_name);
}

//--------------------------------------------------------------------------------------
const char *RTVSocket::HostName(void)
{
	if (m_hostName[0] == '\0')
		GetHostInfo();
	
	return m_hostName;
}

