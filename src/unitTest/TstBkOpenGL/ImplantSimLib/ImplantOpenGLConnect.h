#pragma once

 #include <windows.h>

#define IMP_GL_MAX_ARG_DATA_SIZE (1024*1024)
#define IMP_GL_SHMEM_SIZE 		(64*1024*1024)

#define IMP_GL_PIPE_NAME		"\\\\.\\pipe\\impOpenGL"
#define IMP_GL_SHMEM_NAME_G 	"Global\\impOpenGL"

#define IMP_GL_SHMEM_NAME		"Local\\impOpenGL"
#define PIPE_CONNECT_RETRY		30

enum ImplantOpenGLCmd
{
	ImplantOpenGLCmd_init = 0,
	ImplantOpenGLCmd_render  ,
};
class CImplantOpenGLConServer
{
public:
	CImplantOpenGLConServer(void);
	~CImplantOpenGLConServer(void);

	bool Init(  bool session0Flag);
	bool Term( void );
	 
	bool Exec();
	bool ConnectPipe( void );
	void DisconnectPipe( void );

protected:
	virtual bool exec_init(char *arg_buff,int &size);
	virtual bool exec_render(char *arg_buff,int &size);
	char			m_pipe_name[ MAX_PATH ];	//!< @brief パイプ名称
	char			m_shmem_name[ MAX_PATH ];	//!< @brief 共有メモリ名称

	char*			m_arg_buf;					//!< @brief コマンドバッファ
	
	int				m_disp_loop;				//!< @brief Dipatcher ループ

 
	 HANDLE	m_pipe_hd;				//!< @brief PIPEへのハンドル
	 HANDLE	m_shmem_hd;				//!< @brief SHMEMへのハンドル

	 void* m_shmem_ptr;				 //!< @brief 共有メモリへのポインタ
	
	bool InitPipe( const char* name );	//!< @brief NAME パイプ初期化
	bool TermPipe( void );				//!< @brief パイプの終了処理
	bool InitShmem( const char* name, size_t size );	//!< @brief NAME 共有メモリ初期化
	bool TermShmem( void );				//!< @brief 共有メモリの終了処理

	bool Disp( void );
	//
	bool ReadPipe( LPVOID  lpBuffer,
				  DWORD   nNumberOfBytesToRead,
				  LPDWORD lpNumberOfBytesRead );
	bool WritePipe( LPVOID  lpBuffer,
				  DWORD   nNumberOfBytesToWrite,
				  LPDWORD lpNumberOfBytesWritten );
	
	void* GetPtr( void ){ return m_shmem_ptr; }
	
	bool Send( size_t offset, LPVOID buffer, size_t size, int thNum=1);
	bool Recv( size_t offset, LPVOID buffer, size_t size, int thNum=1 );
};


class CImplantOpenGLConClient
{
public:
	CImplantOpenGLConClient(void);
	~CImplantOpenGLConClient(void);

	bool Init(bool isSession0=false);
	 
	bool exec_cmd(int cmd, void* data, int size);

	bool Term();
protected:
	HANDLE m_mutex;
	HANDLE m_pipe_hd;
	HANDLE m_shmem_hd;
	void*  m_shmem_ptr;
	HANDLE	m_sem_IO;
};
