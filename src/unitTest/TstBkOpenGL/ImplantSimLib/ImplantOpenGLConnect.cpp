#include "ImplantOpenGLConnect.h"



#include "AqCore/TRLogger.h"
extern TRLogger gLogger;
#define ERR      gLogger.LogMessage
#define WRN      gLogger.LogMessage
#define INF      gLogger.LogMessage
#define CINF     gLogger.LogMessage
#define CFLUSH   gLogger.FlushLog

CImplantOpenGLConServer::CImplantOpenGLConServer(void)
{
	
	strcpy( m_pipe_name , IMP_GL_PIPE_NAME );
	strcpy( m_shmem_name, IMP_GL_SHMEM_NAME );

	m_arg_buf = new char[  IMP_GL_MAX_ARG_DATA_SIZE ];

	
}

CImplantOpenGLConServer::~CImplantOpenGLConServer(void)
{
	 delete [] m_arg_buf;
}
bool CImplantOpenGLConServer::InitPipe( const char* name )
{
	/***********************************/
	/*             prologue            */
	/***********************************/
	INF( "init_pipe: enter.\n" ) ; CFLUSH();

	/***********************************/
	/*          init security          */
	/***********************************/
	SECURITY_DESCRIPTOR secDesc;

	InitializeSecurityDescriptor( &secDesc, SECURITY_DESCRIPTOR_REVISION );
	SetSecurityDescriptorDacl( &secDesc, TRUE, NULL, FALSE );

	SECURITY_ATTRIBUTES secAttr;

	secAttr.nLength              = sizeof( SECURITY_ATTRIBUTES );
	secAttr.bInheritHandle       = FALSE;
	secAttr.lpSecurityDescriptor = &secDesc;


	/***********************************/
	/*            open pipe            */
	/***********************************/
	INF("init_pipe: Create [%s] pipe.\n", name );CFLUSH();

	m_pipe_hd = CreateNamedPipe( name,
								 PIPE_ACCESS_DUPLEX, 
								 PIPE_TYPE_MESSAGE|PIPE_WAIT,
								 1,
								 0,
								 0,
								 100, 
								 &secAttr );

	if ( m_pipe_hd==INVALID_HANDLE_VALUE )
	{
		ERR( "init_pipe: can not create pipe.\n" );CFLUSH();
		return false;
	}


	/***********************************/
	/*             epilogue            */
	/***********************************/
	INF( "init_pipe: leave.\n" );CFLUSH();

	return true;
}

bool CImplantOpenGLConServer::TermPipe( void )
{
	INF( "term_pipe: enter.\n" );CFLUSH();

	CloseHandle( m_pipe_hd );

	m_pipe_hd = INVALID_HANDLE_VALUE;

	INF( "term_pipe: leae.\n" );CFLUSH();

	return true;
}
bool CImplantOpenGLConServer::InitShmem( const char* name, size_t size )
{
	/***********************************/
	/*             prologue            */
	/***********************************/
	INF("init_shmem: enter.\n" );CFLUSH();


	/***********************************/
	/*            open shmem           */
	/***********************************/
	INF("init_shmem: Create [%s] shmem.\n", name );CFLUSH();

	HANDLE hFile = (HANDLE)0xffffffffffffffff;

	m_shmem_hd = CreateFileMapping( hFile,
	        						NULL,
			        				PAGE_READWRITE,
					                0,
							        size,
									name );

	if ( m_shmem_hd==INVALID_HANDLE_VALUE )
	{
		ERR( "init_shmem: can not open shmem.\n" );CFLUSH();
		return false;
	}


	/***********************************/
	/*             map shmem           */
	/***********************************/
	m_shmem_ptr = (void*)MapViewOfFile( m_shmem_hd,
           					            FILE_MAP_ALL_ACCESS,
		           				        0,
				           		        0,
						                0 );

	if ( m_shmem_ptr==NULL || m_shmem_ptr==(void*)-1 )
	{
		ERR( "init_shmem: can not map shmem.\n" );CFLUSH();
		return false;
	}


	/***********************************/
	/*             epilogue            */
	/***********************************/
	INF( "init_shmem: leave.\n" );CFLUSH();


	return true;
}

bool CImplantOpenGLConServer::TermShmem( void )
{
	INF( "term_shmem: enter.\n" );CFLUSH();

	CloseHandle( m_shmem_hd );

	m_shmem_hd = INVALID_HANDLE_VALUE;

	INF( "term_shmem: leae.\n" );CFLUSH();

	return true;
}

bool CImplantOpenGLConServer::Init( bool session0Flag )
{
	bool retcd = false;
	
	if(session0Flag){
		strcpy( m_shmem_name, IMP_GL_SHMEM_NAME_G );
	}
	/***********************************/
	/*             prologue            */
	/***********************************/
	INF(" enter %s %s.\n", m_pipe_name, m_shmem_name );CFLUSH();
	
	/***********************************/
	/*             init pipe           */
	/***********************************/
	retcd = InitPipe( m_pipe_name );
	if ( !retcd )
	{
		return retcd;
	}

	
	/***********************************/
	/*             init pipe           */
	/***********************************/
	retcd = InitShmem( m_shmem_name, IMP_GL_SHMEM_SIZE );
	if ( !retcd )
	{
		return retcd;
	}
	
	
	/***********************************/
	/*             epilogue            */
	/***********************************/
	INF(   " leave.\n" );CFLUSH();
	
	return retcd;
}

bool CImplantOpenGLConServer::Term( void )
{
	bool retcd = false;
	
	/***********************************/
	/*             prologue            */
	/***********************************/
	INF(   " enter.\n" );CFLUSH();
	
	/***********************************/
	/*             init pipe           */
	/***********************************/
	retcd = TermShmem();
	if ( !retcd )
	{
		return retcd;
	}

	/***********************************/
	/*             init pipe           */
	/***********************************/
	retcd = TermPipe();
	if ( !retcd )
	{
		return retcd;
	}
	
	
	/***********************************/
	/*             epilogue            */
	/***********************************/
	INF(   " leave.\n" );CFLUSH();
	
	return retcd;
}

bool CImplantOpenGLConServer::ConnectPipe( void )
{
	/***********************************/
	/*           wait connect          */
	/***********************************/
	int retcd;
	retcd = ConnectNamedPipe( m_pipe_hd, NULL );
	if ( !retcd ) 
		{
		int err = GetLastError();
		if ( ERROR_PIPE_CONNECTED != err )
			{
			ERR( "exec: can not connect pipe.[%08x]\n", err );CFLUSH();
			return false;
			}
		}
	
	return true;
}
 
void CImplantOpenGLConServer::DisconnectPipe( void )
{
	DisconnectNamedPipe( m_pipe_hd );
}

bool CImplantOpenGLConServer::ReadPipe( LPVOID  lpBuffer, DWORD   nNumberOfBytesToRead, LPDWORD lpNumberOfBytesRead )
{
	return TRUE == ReadFile( m_pipe_hd, lpBuffer, nNumberOfBytesToRead, lpNumberOfBytesRead, NULL );
}

 
bool CImplantOpenGLConServer::WritePipe( LPVOID  lpBuffer, DWORD   nNumberOfBytesToWrite, LPDWORD lpNumberOfBytesWritten )
{
	return TRUE == WriteFile( m_pipe_hd, lpBuffer, nNumberOfBytesToWrite, lpNumberOfBytesWritten, NULL );
}
 
bool CImplantOpenGLConServer::Send( size_t offset, LPVOID buffer, size_t size, int thNum )
{
	char* dp = (char*)m_shmem_ptr + offset;
	
	memcpy( dp, buffer, size );

	return true;
}

 
bool CImplantOpenGLConServer::Recv( size_t offset, LPVOID buffer, size_t size, int thNum )
{
	char* sp = (char*)m_shmem_ptr + offset;
	
	memcpy( buffer, sp, size );

	return true;
}

bool CImplantOpenGLConServer::Disp( void )
{
	/***********************************/
	/*             prologue            */
	/***********************************/
	bool 	retcd;
	int 	com;
	int		size;
	DWORD 	bytes;


	try{
	/***********************************/
	/*           recv command          */
	/***********************************/
	retcd = ReadPipe( &com, sizeof( int ), &bytes );
	if ( (retcd)&& bytes==0 )
	{
		WRN(   " disconnected.\n" );CFLUSH();
		throw(-1);
	}

	if ( (!retcd )|| bytes!=sizeof( int ) )
	{
		ERR(   " recv command fail. retcd=%08x bytes=%08x\n", retcd, bytes );CFLUSH();
		throw(-1);
	}


	/***********************************/
	/*          recv data size         */
	/***********************************/
	retcd = ReadPipe( &size, sizeof( int ), &bytes );
	if ( (!retcd) || bytes!=sizeof( int ) )
	{
		ERR(   " recv data size fail.\n" );CFLUSH();
		throw(-1);
	}


	/***********************************/
	/*            recv data            */
	/***********************************/
	retcd = ReadPipe( m_arg_buf, size, &bytes );
	if ( (!retcd) || bytes!=size )
	{
		ERR(   " recv data fail.\n" );CFLUSH();
		throw(-1);
	}

	/***********************************/
	/*             dispatch            */
	/***********************************/
	if ( 1 )
	{
		INF( " recv command %d \n" ,com);CFLUSH();
		CFLUSH();

 
		switch( com )
		{
 
			case ImplantOpenGLCmd_init:   
				exec_init( m_arg_buf,size ); 
				break;
		 
			case ImplantOpenGLCmd_render:   
				exec_render( m_arg_buf,size ); 
				break;
				 
			default:
				ERR(   " unknown command=%08x\n", com );CFLUSH();
				*(int*)m_arg_buf = -1;
				throw(-2);
				break;
		}
 
	}
	
	
	/***********************************/
	/*           send command          */
	/***********************************/
	retcd = WritePipe( &com, sizeof( int ), &bytes );
	if ( (!retcd) || bytes!=sizeof( int ) )
	{
		ERR(   " send command fail.\n" );CFLUSH();
		throw(-1);;
	}


	/***********************************/
	/*          send data size         */
	/***********************************/
	retcd = WritePipe( &size, sizeof( int ), &bytes );
	if ( (!retcd)|| bytes!=sizeof( int ) )
	{
		ERR(   " send data size fail.\n" );CFLUSH();
		throw(-1);
	}


	/***********************************/
	/*            send data            */
	/***********************************/
	retcd = WritePipe( m_arg_buf, size, &bytes );
	if ( (!retcd) || bytes!=size )
	{
		ERR(   " send data fail.\n" );CFLUSH();
		throw(-1);;
	}

	}catch(int error)
	{
		if(error == -1){
			retcd = false;
		}
	}catch(...){
		retcd = false;
	}

	/***********************************/
	/*             epilogue            */
	/***********************************/
	 

	return retcd;
}
bool CImplantOpenGLConServer::Exec( void )
{
	/***********************************/
	/*             prologue            */
	/***********************************/
	bool retcd;

	INF(   " enter.\n" );CFLUSH();


	/***********************************/
	/*            wait loop            */
	/***********************************/
//loop:

	INF(   " wait connection.\n" );CFLUSH();


	/***********************************/
	/*           wait connect          */
	/***********************************/
	retcd = ConnectPipe();
	if ( !retcd )
	{
		return false;
	}

	INF(   " pipe connected.\n" );CFLUSH();

	/***********************************/
	/*             dispatch            */
	/***********************************/
	m_disp_loop = 1;
	while( m_disp_loop )
	{
		retcd = Disp();
		if ( !retcd )
			break;
	}


	/***********************************/
	/*            disconnect           */
	/***********************************/
	DisconnectPipe();


	/***********************************/
	/*          purge buffers          */
	/***********************************/
//	Disconnect();
	
//	goto loop;


	/***********************************/
	/*             epilogue            */
	/***********************************/
	INF(   " leave.\n" );CFLUSH();

	return true;
}

bool CImplantOpenGLConServer::exec_init(char *arg_buff,int &size)
{
	int *data_p = (int*)arg_buff;
	INF( " exec_init data size %d ,data %d\n" ,size,*data_p);CFLUSH();
	CFLUSH();

	return true;
}
bool CImplantOpenGLConServer::exec_render(char *arg_buff,int &size)
{
	int *data_p = (int*)arg_buff;
	INF( " exec_render data size %d ,data %d\n" ,size,*data_p);CFLUSH();
	CFLUSH();

	return true;
}
////////////////////////////
////////////
CImplantOpenGLConClient::CImplantOpenGLConClient(void)
{
	 
}

CImplantOpenGLConClient::~CImplantOpenGLConClient(void)
{
	 
}
bool CImplantOpenGLConClient::Init(bool isSession0)
{
	char pipe_name[ MAX_PATH ];
	char shmem_name[ MAX_PATH ];

	strcpy( pipe_name, IMP_GL_PIPE_NAME );

	if ( isSession0)
	{
		strcpy( shmem_name, IMP_GL_SHMEM_NAME_G );
	}
	else
	{
		strcpy( shmem_name, IMP_GL_SHMEM_NAME );
	}

	/***********************************/
	/*             init pipe           */
	/***********************************/
	
	int retry = 0;

	Sleep( 500 );
	
	while( 1 )
		{
		m_pipe_hd = CreateFile( pipe_name,
							    GENERIC_READ | GENERIC_WRITE, 
							    FILE_SHARE_WRITE | FILE_SHARE_READ, 
							    NULL, 
							    OPEN_EXISTING,
							    FILE_ATTRIBUTE_NORMAL, 
							    NULL );
	
		if ( m_pipe_hd==INVALID_HANDLE_VALUE ) 
			{
			int errcode = GetLastError();
			if ( retry++==PIPE_CONNECT_RETRY )
				{
				ERR( "vpsoft_init: can not open pipe[%s] %08x.\n", pipe_name, errcode );CFLUSH();
				return false;
				}
			}
		else
			{
			break;
			}
		Sleep( 1000 );
		}


	/***********************************/
	/*            init shmem           */
	/***********************************/
	m_shmem_hd = OpenFileMapping( FILE_MAP_ALL_ACCESS,
		        				  TRUE,
				        		  shmem_name );

	if ( m_shmem_hd==INVALID_HANDLE_VALUE ) 
		{
		ERR( "vpsoft_init: can not open shmem.\n" );CFLUSH();
		return false;
		}

	m_shmem_ptr = (void*)MapViewOfFile( m_shmem_hd,
							            FILE_MAP_ALL_ACCESS,
									    0,
									    0,
									    0 );

	if ( m_shmem_ptr==NULL || m_shmem_ptr==(void*)-1 )
		{
		ERR( "vpsoft_init: can not map shmem.\n" );CFLUSH();
		return false;
		}


	/***********************************/
	/*          init semaphore         */
	/***********************************/
	m_sem_IO = CreateMutex( NULL, FALSE, NULL );
	if (m_sem_IO == NULL)
		{
		ERR( "vpsoft_init: can not create mutex." );CFLUSH();
		return false;
		}


	return true;
}

bool CImplantOpenGLConClient::exec_cmd(int cmd, void* data, int size)
{
	/***********************************/
	/*             prologue            */
	/***********************************/
	DWORD 	bytes;
	int 	retcd;

	bool ret_b = true;
	try{
	/***********************************/
	/*      enter critical section     */
	/***********************************/
	retcd = WaitForSingleObject( m_sem_IO, INFINITE );
	if (retcd !=  WAIT_OBJECT_0 )
		{
			ERR( "vpsoft_ioctl: g_sem_IO not signal\n" );CFLUSH();
			throw(-1);
		}

	/***********************************/
	/*           send command          */
	/***********************************/
	retcd = WriteFile( m_pipe_hd, &cmd, sizeof( int ), &bytes, NULL );
	if ( !retcd || bytes!=sizeof( int ) )
		{
		ERR( "vpsoft_ioctl: write command error.\n" );CFLUSH();
		throw(-2);
		}


	/***********************************/
	/*          send data size         */
	/***********************************/
	retcd = WriteFile( m_pipe_hd, &size, sizeof( int ), &bytes, NULL );
	if ( !retcd || bytes!=sizeof( int ) )
		{
		ERR( "vpsoft_ioctl: write data size error.\n" );CFLUSH();
		throw(-2);
		}


	/***********************************/
	/*            send data            */
	/***********************************/
	retcd = WriteFile( m_pipe_hd, data, size, &bytes, NULL );
	if ( !retcd || bytes!=size )
		{
		ERR( "vpsoft_ioctl: write data error.\n" );CFLUSH();
		throw(-2);
		}


	/***********************************/
	/*           recv command          */
	/***********************************/
	retcd = ReadFile( m_pipe_hd, &cmd, sizeof( int ), &bytes, NULL );
	if ( !retcd || bytes!=sizeof( int ) )
		{
		ERR( "vpsoft_ioctl: read command error.\n" );CFLUSH();
		throw(-2);
		}


	/***********************************/
	/*          recv data size         */
	/***********************************/
	retcd = ReadFile( m_pipe_hd, &size, sizeof( int ), &bytes, NULL );
	if ( !retcd || bytes!=sizeof( int ) )
		{
		ERR( "vpsoft_ioctl: read data size error.\n" );CFLUSH();
		throw(-2);
		}


	/***********************************/
	/*            recv data            */
	/***********************************/
	retcd = ReadFile( m_pipe_hd, data, size, &bytes, NULL );
	if ( !retcd || bytes!=size )
		{
		ERR( "vpsoft_ioctl: read data error.\n" );CFLUSH();
		throw(-2);
		}


	/***********************************/
	/*      leave critical section     */
	/***********************************/
	ReleaseMutex( m_sem_IO );

	 
	}catch(int error){
		if(error == -2){
			ReleaseMutex( m_sem_IO );
		}
		ret_b = false;
	}catch(...){
		ret_b = false;
	}

 
	if(!ret_b){
		ERR(   " CMD %08x DATA %08x SIZE %08x.\n", cmd, data, size );CFLUSH();

	}

	return ret_b ;

 
}

bool CImplantOpenGLConClient::Term()
{
	/***********************************/
	/*             prologue            */
	/***********************************/
	INF( "CImplantOpenGLConClient::Term: enter.\n" );CFLUSH();

	/***********************************/
	/*             say term            */
	/***********************************/
 
	if ( m_pipe_hd != INVALID_HANDLE_VALUE )
		{
		INF( "CImplantOpenGLConClient::Term: say term.\n" );CFLUSH();
#if 0
		Vp1000_dr_hello hello;

		hello.proc_id = GetCurrentProcessId();

		vpsoft_ioctl( p, VP1000_DR_IOCTL_VPSW_TERM, &hello, sizeof( hello ) );
#endif
		INF( "CImplantOpenGLConClient::Term: term pipe.\n" );CFLUSH();
		}
 
	/***********************************/
	/*             term pipe           */
	/***********************************/
	if ( m_pipe_hd != INVALID_HANDLE_VALUE )
		CloseHandle( m_pipe_hd );

	m_pipe_hd = INVALID_HANDLE_VALUE;


	/***********************************/
	/*            term shmem           */
	/***********************************/
	if ( m_shmem_hd != INVALID_HANDLE_VALUE )
		CloseHandle( m_shmem_hd );

	m_shmem_hd  = INVALID_HANDLE_VALUE;
	m_shmem_ptr = NULL;


	/***********************************/
	/*       close mutex               */
	/***********************************/
	if ( m_mutex )
		{
		CloseHandle( m_mutex );
		m_mutex = NULL;
		}

	/***********************************/
	/*       DeleteCriticalSection     */
	/***********************************/
	if ( m_sem_IO )
		{
		CloseHandle( m_sem_IO );
		m_sem_IO = NULL;							// 2008.4.23 Y.Kimura INVALID_HANDLE_CLOSE
		}

#if 0
	/***********************************/
	/*       terminate vpsw.exe        */
	/***********************************/
	int retcd = vpsoft_term_vpsw( p );
	if ( retcd )
		{
		ERR( "CImplantOpenGLConClient::Term: can not terminate vpsw.exe\n" );
		return -1;
		}
#endif
	/***********************************/
	/*             epilogue            */
	/***********************************/
//	if (g_opened > 0) g_opened--;
//
	INF( "CImplantOpenGLConClient::Term: leave.\n" );CFLUSH();


	return true;
}