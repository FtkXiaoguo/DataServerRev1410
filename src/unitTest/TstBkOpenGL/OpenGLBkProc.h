
#define _WIN32_WINNT   0x0501

#include <stdio.h>
#include <memory>
#include <windows.h>

class OpenGLProc_COM
{
public:
	HANDLE g_mutex;
	HANDLE g_pipe_hd;
	HANDLE g_shmem_hd;
	void*  g_shmem_ptr;
	HANDLE	g_sem_IO;
	STARTUPINFO 		g_si;
	PROCESS_INFORMATION g_pi;
	HANDLE g_token;					// ÉçÉOÉIÉì
	char	g_full[ MAX_PATH ];
	int		g_log_level;
};

 
class COpenGLBkProc
{
public:
	COpenGLBkProc();
	~COpenGLBkProc();
	
	int startMyProc(OpenGLProc_COM* p,  const char* full_arg);

	DWORD getSessionID() { return m_sessionId;};
protected:
	DWORD m_sessionId;
	
int CreateProcsseSession( OpenGLProc_COM* p, int newSessionId, const char* full_arg) ;
 
};
