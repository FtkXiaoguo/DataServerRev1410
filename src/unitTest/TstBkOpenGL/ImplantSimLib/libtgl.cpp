/*$Id: libtgl.cpp 732 2009-09-09 11:57:21Z yasuyuki $*/
/*********************************************************************/
/*                                                                   */
/*                    Tiny OpenGL Library TGL                        */
/*                                                                   */
/*********************************************************************/

 
#include <crtdbg.h>
#if defined( _DEBUG )
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif
 

#include "libtgl.h"
#include "tglfont.h"
#include <math.h>

#include <windows.h>

#define ENABLE_DOUBLE_BUFFER		1

#include "AqCore/TRLogger.h"
extern TRLogger gLogger;

#define IMP_ERR      gLogger.LogMessage
#define IMP_INF      gLogger.LogMessage
#define IMP_CINF     gLogger.LogMessage
#define IMP_CFLUSH   gLogger.FlushLog

/*********************************************************************/
/*                            global                                 */
/*********************************************************************/
void	imp_error( const char * fmt, ... )
{
}
void	imp_info( const char * fmt, ... ){
}
void	imp_cinfo( const char * fmt, ... ){
}
void	imp_cflush( void ){
}

//#include "mmx.h"
/********************************************************************************/
/* GL 描画用のダミーウィドウ                                                    */
/********************************************************************************/
/*********************************************************************/
/*                              WndProc                              */
/*********************************************************************/
LRESULT CALLBACK WndProc (HWND, UINT, WPARAM, LPARAM);


struct SInitWindowGL
{
	HWND hWnd;
	int sx;
	int sy;
};

/*********************************************************************/
/*                             InitWindowGL                          */
/*********************************************************************/
void InitWindowGL( void * arg )
{
	SInitWindowGL * p = (struct SInitWindowGL *)arg;

    // GLレンダリング領域がＣｌｉｅｎｔ領域になるため多めるとる。
	int sx = p->sx + 8;
	int sy = p->sy + 8+26;

	int win_sx = GetSystemMetrics(SM_CXSCREEN);
	int win_sy = GetSystemMetrics(SM_CYSCREEN);
	
	if ( sx > win_sx ) sx = win_sx;
	if ( sy > win_sy ) sy = win_sy;
	
	


	HWND hWnd;
	MSG  msg;
	WNDCLASSEX  wndclass ;

	wndclass.cbSize        = sizeof(wndclass);        /* 構造体の大きさ */
//	wndclass.style         = CS_HREDRAW | CS_VREDRAW; /* スタイル */
	wndclass.style         = CS_OWNDC|CS_NOCLOSE; /* スタイル */
	wndclass.lpfnWndProc   = WndProc;                 /* メッセージ処理関数 */
	wndclass.cbClsExtra    = 0;
	wndclass.cbWndExtra    = 0;
	wndclass.hInstance     = 0;               /* プログラムのハンドル */
	wndclass.hIcon         = LoadIcon (NULL, IDI_APPLICATION); /* アイコン */
	wndclass.hCursor       = LoadCursor (NULL, IDC_ARROW);     /* カーソル */
	wndclass.hbrBackground = (HBRUSH)GetStockObject(WHITE_BRUSH); /* ブラシ */
	wndclass.lpszMenuName  = NULL;              /* メニュー */
	wndclass.lpszClassName = "OpenGL Window";   /* クラス名 */
	wndclass.hIconSm       = LoadIcon (NULL, IDI_APPLICATION);

	RegisterClassEx (&wndclass); /* ウインドウクラスTest Windowを登録 */

	hWnd = CreateWindow (
            "OpenGL Window",     /* ウインドウクラス名 */
	        "OpenGL Window",     /* ウインドウのタイトル */
            WS_OVERLAPPEDWINDOW, /* ウインドウスタイル */
//            WS_POPUPWINDOW, /* ウインドウスタイル */
            0,0,                 /* ウインドウ表示位置 */
            sx, sy,              /* ウインドウの大きさ */
            NULL,                /* 親ウインドウのハンドル */
            NULL,                /* メニューのハンドル */
            NULL,                /* インスタンスのハンドル */
            NULL);               /* 作成時の引数保存用ポインタ */

	ShowWindow (hWnd,SW_SHOW);   /* ウインドウを表示 */
	UpdateWindow (hWnd);

	p->hWnd = hWnd;

	while (GetMessage (&msg,NULL,0,0)) { /* メッセージループ */

		TranslateMessage(&msg);
		DispatchMessage(&msg);

	}
}

/*********************************************************************/
/*                              WndProc                              */
/*********************************************************************/
LRESULT CALLBACK WndProc (HWND hwnd, UINT iMsg, WPARAM wParam, LPARAM lParam)
{
	switch (iMsg)
		{

		case WM_DESTROY : /* 終了処理 */

			PostQuitMessage(0);
			return 0;

		}
	return DefWindowProc (hwnd, iMsg, wParam, lParam) ;
}

/*********************************************************************/
/*                             InitWindow                            */
/*********************************************************************/
int TGL::InitWindow( int sx, int sy )
{
	m_thd = 0;
	m_tid = 0;
	
	struct SInitWindowGL prm;
	
	prm.hWnd = 0;
	prm.sx   = sx;
	prm.sy   = sy;
	
	m_thd = CreateThread( NULL, 0, (LPTHREAD_START_ROUTINE)InitWindowGL, &prm, 0, &m_tid );

	IMP_INF( "TGL::InitWindow Create Thread %08x:%08x\n", m_thd, m_tid );

    Sleep( 50 );
	for( int i = 0; i < 10; i++ )
		{
	    if ( prm.hWnd ) break;
	    Sleep( 1000 );
		}

	if ( prm.hWnd == 0 ) return -1;

	return Init( prm.hWnd );
}

/*********************************************************************/
/*                              macros                               */
/*********************************************************************/

#define M_PI	 	3.14159265358979323846264338327950288419716939937510

#define for if(0);else for




/*********************************************************************/
/*                               ctor                                */
/*********************************************************************/

TGL::TGL( void )

	{
	first = 1;
	clr_col = 0;
	hWnd = 0;
	
	m_thd = 0;
	m_tid = 0;
	
	m_window_sx = 0;
	m_window_sy = 0;
	
	m_bDoubleBuffer = 0;
	}


/*********************************************************************/
/*                               dtor                                */
/*********************************************************************/

TGL::~TGL( void )

	{
	}


/*********************************************************************/
/*                               Init                                */
/*********************************************************************/
void TGL::CopyFromDIB( HDC * dc, int sx, int sy )
{
	m_CglDIB.Draw( *dc, sx, sy );
}


/*********************************************************************/
/*                               Init                                */
/*********************************************************************/
int TGL::Init( HWND hwnd )
{
	this->hWnd = hwnd;
	
    hdc = ::GetDC( hwnd );
    
   	RECT r;
//    GetWindowRect( *hwnd, &r );
    GetClientRect( hwnd, &r );
    
    winX = r.left;
    winY = r.top;
    winSX = r.right - r.left;
    winSY = r.bottom - r.top;

	int retcd = Init( hdc, winSX, winSY );

	wglMakeCurrent( 0, 0 );
    ::ReleaseDC( hwnd, hdc );

	return retcd;
}


/*********************************************************************/
/*                               Init DC                             */
/*********************************************************************/
int TGL::Init( HDC _hDC, int sx, int sy, BOOL bWnd )
{
	m_window_sx = sx;
	m_window_sy = sy;
	
	hdc = _hDC;

	{
		// 現在対応しているフォーマットの数を参照する
		int format_count= DescribePixelFormat( _hDC, 0, 0, NULL );

		
		IMP_INF( " >>>>>>>>>format_count %d \n" ,format_count);
		// 列挙する
		for( int fi= 1 ; fi<= format_count ; fi++ ){
			PIXELFORMATDESCRIPTOR   pformat;
			DescribePixelFormat( _hDC, fi, sizeof(PIXELFORMATDESCRIPTOR), &pformat );
			bool b1 = PFD_GENERIC_ACCELERATED & pformat.dwFlags;
			bool b2 = PFD_GENERIC_FORMAT  & pformat.dwFlags;
			bool b3 = PFD_SUPPORT_OPENGL   & pformat.dwFlags;
			if(b3){
				IMP_INF( "dwFlags[0x%x] PFD_GENERIC_ACCELERATED[%d], PFD_GENERIC_FORMAT[%d] PFD_SUPPORT_OPENGL[%d]\n", pformat.dwFlags,b1,b2,b3);
			}

		}

		IMP_INF( " >>>>>>>>>---------\n" );


	}

	/***********************************/
	/*        setup pixel format       */
	/***********************************/
	PIXELFORMATDESCRIPTOR pfd =
		{
		sizeof( PIXELFORMATDESCRIPTOR ),
		1,
		PFD_SUPPORT_OPENGL,
		PFD_TYPE_RGBA,
		24,
		0, 0, 0,
		0, 0, 0,
		0, 0, 
		0, 0, 0, 0, 0,
		32,
		32, 					// Stencil
		0,
		PFD_MAIN_PLANE,
		0, 
		0, 
		0, 
		0
		};
  if ( this->hWnd || bWnd == TRUE)
    {
    pfd.dwFlags |= PFD_DRAW_TO_WINDOW;

#if ENABLE_DOUBLE_BUFFER
    pfd.dwFlags |= PFD_DOUBLEBUFFER;
    m_bDoubleBuffer = 1;
#endif
    }
  else
    {
    pfd.dwFlags |= PFD_DRAW_TO_BITMAP | PFD_SUPPORT_GDI;
    }

	int id = ChoosePixelFormat( _hDC, &pfd );
	
	SetPixelFormat( _hDC, id, &pfd );


	/***********************************/
	/*         create context          */
	/***********************************/
	hrc = wglCreateContext( _hDC );

	wglMakeCurrent( _hDC, hrc );

	IMP_INF( "wglMakeCurrent %08x, %08x %d\n", _hDC, hrc, glGetError());

	{
		bool b1 = PFD_GENERIC_ACCELERATED & pfd.dwFlags;
		bool b2 = PFD_GENERIC_FORMAT  & pfd.dwFlags;
		IMP_INF( "dwFlags[0x%x] PFD_GENERIC_ACCELERATED[%d], PFD_GENERIC_FORMAT[%d] \n", pfd.dwFlags,b1,b2);

//		DescribePixelFormat()

		IMP_INF( " %s, \n %s, \n %s, \n %s \n",
		(const char *)glGetString(GL_VENDOR), 
		(const char *)glGetString(GL_RENDERER), 
		(const char *)glGetString(GL_VERSION),
		(const char *)glGetString(GL_EXTENSIONS)
			); 

	}
	/***********************************/
	/*            epilogue             */
	/***********************************/
// 	wglMakeCurrent( NULL, NULL );
	return 0;
}

/*********************************************************************/
/*                               Init DIB                            */
/*********************************************************************/
int TGL::Init( int sx, int sy )

	/***********************************/
	/*             prologue            */
	/***********************************/
    {
    if ( FALSE == m_CglDIB.CreateGLDIB( sx, sy, 24, 24 ) )
      {
      return -1;
      }

    winX = 0;
    winY = 0;
    winSX = sx;
    winSY = sy;
    
    hdc = m_CglDIB.GetDC();
  
  	return Init( hdc, sx, sy );
	}

/**************************************************************/
/*                          Term                              */
/**************************************************************/

int TGL::Term( void )

	{
	/***********************************/
	/*            epilogue             */
	/***********************************/
#if 0
	BOOL retcd = wglMakeCurrent( NULL, NULL );
	if (false == retcd)
		{
		IMP_INF( "wglMakeCurrent Error %08x\n", GetLastError() );
		}
#endif
	if ( hrc )
    	{
	    wglDeleteContext( hrc );
		hrc = 0;
    	}


	if ( m_thd )
		{
		IMP_INF( "TGL::SendMessage WM_DESTROY %08x\n", hWnd );
		SendMessage( hWnd, WM_DESTROY, 0, 0 );
		IMP_INF( "TGL::WaitForSingleThread %08x:%08x\n", m_thd, m_tid );
		WaitForSingleObject( m_thd, INFINITE );
		IMP_INF( "TGL::Terminate Thread %08x:%08x\n", m_thd, m_tid );
		CloseHandle( m_thd );
		m_thd = 0;
		}

	return 0;
	}

void TGL::SetWgl( void )
{
  if ( hWnd )
    {
  	hdc = ::GetDC( hWnd );
  	}

 	wglMakeCurrent( hdc, hrc );
}

void TGL::ReleaseWgl( void )
{
	wglMakeCurrent( 0, 0 );
	if ( hWnd )
		{
		::ReleaseDC( hWnd, hdc );
		}
}


/**************************************************************/
/*                           Init2D                           */
/**************************************************************/

int TGL::Init2D( void )

	{
  if ( hWnd )
    {
  	/***********************************/
  	/*       get window position       */
  	/***********************************/
    RECT r;
    GetWindowRect( hWnd, &r );
    
    winX = r.left;
    winY = r.top;
    winSX = r.right - r.left;
    winSY = r.bottom - r.top;


  	/***********************************/
  	/*             get DC              */
  	/***********************************/
  	hdc = ::GetDC( hWnd );
  	}

 	wglMakeCurrent( hdc, hrc );


#if ENABLE_DOUBLE_BUFFER
	glDrawBuffer( GL_BACK );
#endif
    glPixelStorei( GL_UNPACK_ALIGNMENT, 1 );

	/***********************************/
	/*         init gl property        */
	/***********************************/
  glDisable( GL_DEPTH_TEST );
  glDisable( GL_BLEND );
  glDisable( GL_ALPHA_TEST );

	glLineStipple( 1, 0xffff );
	glClearColor( 0.0, 0.0, 0.0, 0.0 );
	glClearIndex( 0.0 );

	glShadeModel( GL_FLAT );
	glPixelStorei( GL_UNPACK_ALIGNMENT, 1 );

	glViewport( 0, 0, winSX, winSY );
	glMatrixMode( GL_PROJECTION );
	glLoadIdentity();
	
	gluOrtho2D( 0.0, winSX, 0.0, winSY );

	glMatrixMode( GL_MODELVIEW );
	glLoadIdentity();

	glDisable( GL_LIGHTING );
	glDisable( GL_LIGHT0   );

	return 0;
	}


/*********************************************************************/
/*                                Term2D                             */
/*********************************************************************/

int TGL::Term2D( int flag )

	{
	if ( flag==0 ) 
		{
		glFlush();
		
		if ( m_bDoubleBuffer )
			{
#if ENABLE_DOUBLE_BUFFER
			SwapBuffers( hdc );
#endif
			}
		}

 	BOOL retcd = wglMakeCurrent( 0, 0 );
	if (false == retcd)
		{
		IMP_INF( "Term2D wglMakeCurrent Error %08x\n", GetLastError() );
		}

	if ( hWnd )
	    {
  		::ReleaseDC( hWnd, hdc );
	  	}

	return 0;
	}


/****************************************************************************/
/*                                   Init3D                                 */
/****************************************************************************/

int TGL::Init3D( int X0, int Y0, int SX, int SY, float fa, float fr, float fg, float fb )

	{
	/***********************************/
	/*            init draw            */
	/***********************************/
	glMatrixMode( GL_PROJECTION );
	glLoadIdentity();

  winX  = X0;
  winY  = Y0;
  winSX = SX;
  winSY = SY;

	glViewport( X0, Y0, SX, SY );
	
//	glOrtho( -1.0, 1.0, -1.0, 1.0, -2.0, 2.0 );

	glMatrixMode( GL_MODELVIEW );
	glLoadIdentity();

//	glDrawBuffer( GL_BACK );
//	glClearColor( 0.0f, 0.0f, 0.0f, 0.0 );


	/***********************************/
	/*           setup draw            */
	/***********************************/
	glClearDepth( +1.0 );
	
 	glClearColor( fr, fg, fb, fa );


	glClear( GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT );

	glEnable( GL_DEPTH_TEST );

	glColor3f( 1.0F, 1.0F, 1.0F );


#if 1
	glDepthFunc(GL_ALWAYS);
	glDepthFunc(GL_LESS);
	glDepthMask( GL_TRUE );

#if 0
	glEnable( GL_BLEND );

	glEnable( GL_LINE_SMOOTH );
	glBlendFunc( GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA );
	glShadeModel( GL_FLAT );
#endif

#if 0
	glEnable( GL_BLEND );
//	glBlendFunc( GL_SRC_ALPHA_SATURATE, GL_ONE );
	glBlendFunc( GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA );
//	glEnable( GL_ALPHA_TEST );
	glEnable( GL_POLYGON_SMOOTH );
//	glCullFace( GL_BACK );
//	glEnable (GL_CULL_FACE );

#endif 
	glShadeModel( GL_SMOOTH );
	
	glPolygonMode( GL_FRONT_AND_BACK, GL_FILL );

#endif


	glDisable   ( GL_CLIP_PLANE0 );
	glDisable   ( GL_CLIP_PLANE1 );
	glDisable   ( GL_CLIP_PLANE2 );
	glDisable   ( GL_CLIP_PLANE3 );
	glDisable   ( GL_CLIP_PLANE4 );
	glDisable   ( GL_CLIP_PLANE5 );

	return 0;
	}


/*********************************************************************/
/*                                Term3D                             */
/*********************************************************************/

int TGL::Term3D( void )

	{
	/***********************************/
	/*             matrix              */
	/***********************************/
//	glPopMatrix();
//	glPopMatrix();


	/***********************************/
	/*            disable              */
	/***********************************/
	glDisable( GL_BLEND      );
	glDisable( GL_ALPHA_TEST );
	glDisable( GL_DEPTH_TEST );

	glDisable( GL_LIGHTING );
	glDisable( GL_LIGHT0   );


	/***********************************/
	/*         restore for 2D          */
	/***********************************/
	glViewport( 0, 0, winSX, winSY );

	glMatrixMode( GL_PROJECTION );
	glLoadIdentity();
	gluOrtho2D( 0.0, winSX, 0.0, winSY );

	glMatrixMode( GL_MODELVIEW );
	glLoadIdentity();

	return 0;
	}

int TGL::GetError( void )
{
#if 0
  if ( glGetError() != GL_NO_ERROR )
    {
    return -1;
    }
#endif
  return glGetError();
}
/*********************************************************************/
/*                             SetViewPoint                          */
/*********************************************************************/

int TGL::SetViewPoint( struct Lookat& at )

	{
	this->at = at;

	return 0;
	}

/*********************************************************************/
/*                             SetScale                              */
/*********************************************************************/

int TGL::SetScale(float sx, float sy, float sz)

	{
	sc_x = sx;
	sc_y = sy;
	sc_z = sz;
	return 0;
	}

/*********************************************************************/
/*                             SetOffset                             */
/*********************************************************************/

int TGL::SetOffset(float sx, float sy, float sz)

	{
	of_x = sx;
	of_y = sy;
	of_z = sz;
	return 0;
	}

/*********************************************************************/
/*                         SetWorldMatrix                            */
/*********************************************************************/

int TGL::SetWorldMatrix( float xr, float yr, float zr )

	{
	glRotated( zr, 0.0, 0.0, 1.0 );
	glRotated( yr, 0.0, 1.0, 0.0 );
	glRotated( xr, 1.0, 0.0, 0.0 );
//	glRotated( zr*M_PI/180.0F, 0.0, 0.0, 1.0 );
//	glRotated( yr*M_PI/180.0F, 0.0, 1.0, 0.0 );
//	glRotated( xr*M_PI/180.0F, 1.0, 0.0, 0.0 );

//	glMatrixMode( GL_MODELVIEW );
//	glLoadIdentity();

//	glMultMatrixd( m );
//	glRotated( ot.zr, 0.0, 0.0, 1.0 );
//	glRotated( rot, 0.0, 0.0, 1.0 );
//	glRotated( ot.xr, 1.0, 0.0, 0.0 );
//	glTranslated( ot.x, ot.y, ot.z );

//	rot += 0.5;

	return 0;
	}


/*********************************************************************/
/*                              SetLight                             */
/*********************************************************************/

int TGL::SetLight( float lx, float ly, float lz )

	{
	/***********************************/
	/*              enable             */
	/***********************************/
	glEnable( GL_LIGHTING       );
	glEnable( GL_LIGHT0         );

//	glColorMaterial( GL_FRONT_AND_BACK, GL_DIFFUSE );
//	glEnable( GL_COLOR_MATERIAL );

	glLightModeli( GL_LIGHT_MODEL_TWO_SIDE, GL_TRUE );
//	glLightModeli( GL_LIGHT_MODEL_LOCAL_VIEWER, GL_TRUE );


	/***********************************/
	/*           light model           */
	/***********************************/
//	GLfloat	model_ambient[ 4 ]     = { 0.4F, 0.4F, 0.4F, 1.0F };

//	glLightModelfv( GL_LIGHT_MODEL_AMBIENT, model_ambient );


	/***********************************/
	/*           light param           */
	/***********************************/
	GLfloat light_ambient[ 4 ]  = { 0.2F, 0.2F, 0.2F, 1.0F };
	GLfloat light_specular[ 4 ] = { 1.0F, 1.0F, 1.0F, 1.0F };
	GLfloat light_diffuse[ 4 ]  = { 1.0F, 1.0F, 1.0F, 1.0F };

	glLightfv( GL_LIGHT0, GL_AMBIENT,  light_ambient       );
	glLightfv( GL_LIGHT0, GL_SPECULAR, light_specular      );
	glLightfv( GL_LIGHT0, GL_DIFFUSE,  light_diffuse       );


	/***********************************/
	/*         light position          */
	/***********************************/
	GLfloat	light_position[ 4 ] = { lx, ly, lz, 0.0F };

	glLightfv( GL_LIGHT0, GL_POSITION, light_position );


#if 0
	GLfloat	light_position2[ 4 ] = { lz, lz, lz, 0.0F };
	glEnable( GL_LIGHT1         );
//	glLightf ( GL_LIGHT1, GL_SPOT_CUTOFF,  30 );
//	glLightfv( GL_LIGHT1, GL_SPOT_DIRECTION,  spot_direction );
//	glLightf ( GL_LIGHT1, GL_SPOT_EXPONENT,  2.0 );
	glLightfv( GL_LIGHT1, GL_AMBIENT,  light_ambient       );
	glLightfv( GL_LIGHT1, GL_SPECULAR, light_specular      );
	glLightfv( GL_LIGHT1, GL_DIFFUSE,  light_diffuse       );
	glLightfv( GL_LIGHT1, GL_POSITION, light_position2 );
#endif
	return 0;
	}


/*********************************************************************/
/*                          SetColorMaterial                         */
/*********************************************************************/

int TGL::SetColorMaterial( int col, float dif, float amb, float spc )

	{
	float a = TGL_A( col )/255.0F;
	float r = TGL_R( col )/255.0F;
	float g = TGL_G( col )/255.0F;
	float b = TGL_B( col )/255.0F;

	GLfloat Ambient1[]   = { r*amb, g*amb, b*amb, a };
//	GLfloat Diffuse1[]   = { r*dif, g*dif, b*dif, 1.0F };
	GLfloat Diffuse1[]   = { r*dif, g*dif, b*dif, a  };
	GLfloat Specular1[]  = { spc,   spc,   spc,   a };
	GLfloat Shininess1[] = { 64.0f }; 

#if 0
	glMaterialfv( GL_FRONT , GL_AMBIENT   , Ambient1   );
	glMaterialfv( GL_FRONT , GL_DIFFUSE   , Diffuse1   );
	glMaterialfv( GL_FRONT , GL_SPECULAR  , Specular1  );
	glMaterialfv( GL_FRONT , GL_SHININESS , Shininess1 );
#else
	glMaterialfv( GL_FRONT_AND_BACK , GL_AMBIENT   , Ambient1   );
	glMaterialfv( GL_FRONT_AND_BACK , GL_DIFFUSE   , Diffuse1   );
	glMaterialfv( GL_FRONT_AND_BACK , GL_SPECULAR  , Specular1  );
	glMaterialfv( GL_FRONT_AND_BACK , GL_SHININESS , Shininess1 );
#endif
	return 0;
	}


/*********************************************************************/
/*                                SwapBuffer                         */
/*********************************************************************/

int TGL::SwapBuffer( void )

	{
#if ENABLE_DOUBLE_BUFFER
	SwapBuffers( hdc );
#endif
	return 0;
	}


/*********************************************************************/
/*                                Flush                              */
/*********************************************************************/

int TGL::Flush( void )

	{
	glFlush();
#if ENABLE_DOUBLE_BUFFER
	SwapBuffers( hdc );
#endif
	return 0;
	}


/*********************************************************************/
/*                                Clear                              */
/*********************************************************************/

int TGL::Clear( void )

	{
	glClear( GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT );

	return 0;
	}


/*********************************************************************/
/*                           ClearBackBuffer                         */
/*********************************************************************/

int TGL::ClearBackBuffer( void )

	{
#if ENABLE_DOUBLE_BUFFER
	glDrawBuffer( GL_BACK );
#endif
	glClearColor( 0.25f, 0.25f, 0.35f, 0.0 );
	glClear( GL_COLOR_BUFFER_BIT );

	return 0;
	}


/*********************************************************************/
/*                              InitFont                             */
/*********************************************************************/

int TGL::InitFont( void )

	{
    GLuint i;

    glPixelStorei( GL_UNPACK_ALIGNMENT, 1 );

    fontOffset = glGenLists( 128 );

    for ( i=32; i<127; i++ ) 
    	{
		glNewList( i+fontOffset, GL_COMPILE );
		glBitmap( 8, 13, 0.0, 2.0, 10.0, 0.0, rasters[ i-32 ] );
		glEndList();
    	}

	return 0;
	}


/****************************************************************************/
/*                           SetModelViewMatrix3D                           */
/****************************************************************************/

int TGL::SetModelViewMatrix3D( double m[ 16 ] )

	{
	glMatrixMode( GL_MODELVIEW );
	glLoadIdentity();

	glMultMatrixd( m );

	return 0;
	}

/****************************************************************************/
/*                           SetProjectionMatrix3D                           */
/****************************************************************************/

int TGL::SetProjectionMatrix3D( double m[ 16 ] )

	{
	glMatrixMode( GL_PROJECTION );
	glLoadIdentity();

	glViewport( winX, winY, winSX, winSY );
	glMultMatrixd( m );

	return 0;
	}

/****************************************************************************/
/*                                 SetupLight                               */
/****************************************************************************/

int TGL::SetupLight( void )

	{
  float s = -2.0;
  SetLight( s * 0.5f, s * 0.5f, s * 1.0f );
  
  return 0;
#if 1
	glEnable( GL_LIGHTING       );
	glEnable( GL_LIGHT0         );
	glEnable( GL_COLOR_MATERIAL );

	glColorMaterial( GL_FRONT_AND_BACK, GL_AMBIENT_AND_DIFFUSE );

	GLfloat	modelAmbient[]     = { 0.2F, 0.2F, 0.2F, 1.0F };
	GLfloat	materialSpecular[] = { 0.0F, 0.0F, 0.0F, 1.0F };
	GLfloat light_diffuse[ 4 ]  = { 1.0F, 1.0F, 1.0F, 1.0F };
	GLfloat	lightPosition[]    = { 0.5F, 0.5F, 1.0F, 0.0F };

	glLightModelfv( GL_LIGHT_MODEL_AMBIENT, modelAmbient );
	glLightfv( GL_LIGHT0, GL_DIFFUSE,  light_diffuse       );

	glMaterialfv( GL_FRONT_AND_BACK, GL_SPECULAR, materialSpecular );
	glMaterialf( GL_FRONT_AND_BACK, GL_SHININESS, 0.0 );

	glLightfv( GL_LIGHT0, GL_POSITION, lightPosition );
#endif
	return 0;
	}

/*********************************************************************/
/*                            SetTextColor                           */
/*********************************************************************/

int TGL::SetTextColor( int fg, int bg )

	{
	tfga = TGL_A( fg )/255.0F;
	tfgr = TGL_R( fg )/255.0F;
	tfgg = TGL_G( fg )/255.0F;
	tfgb = TGL_B( fg )/255.0F;

	tbga = TGL_A( bg )/255.0F;
	tbgr = TGL_R( bg )/255.0F;
	tbgg = TGL_G( bg )/255.0F;
	tbgb = TGL_B( bg )/255.0F;

	return 0;
	}

#include <gl/gl.h>
//#include "glut.h"
#include <gl/glaux.h>
/*********************************************************************/
/*                               PutText                             */
/*********************************************************************/

int TGL::PutText( int x, int y, const char* s )
					
	{
	glColor4f( tfgr, tfgg, tfgb, tfga );

#if 1
	glRasterPos3f( float( x ), float( winSY - y - 1 ), 0.0F );

    glPushAttrib( GL_LIST_BIT );
    glListBase( fontOffset );
    glCallLists( strlen( s ), GL_UNSIGNED_BYTE, (GLubyte*) s );
    glPopAttrib();
#else
	drawBitmapString(float( x ), float( winSY - y - 1 ), GLUT_BITMAP_TIMES_ROMAN_24, (char *)s);
#endif
	return 0;
	}


/********************************************************************/
/*                              Printf                              */
/********************************************************************/

int TGL::Printf( int x, int y, const char* fmt, ... )

	{
	char buf[ 1024 ];

	va_list ap;

	va_start( ap, fmt );

	vsprintf( buf, fmt, ap );

	PutText( x, y, buf );

	return 0;
	}


/*********************************************************************/
/*                               GetImage                            */
/*********************************************************************/

int TGL::GetImage( int* data, int sx, int sy, int ssx, int ssy )

	{
	glPixelZoom( 1.0F, 1.0F );

	if ( hWnd )
		{
#if ENABLE_DOUBLE_BUFFER
		glReadBuffer( GL_BACK );
#endif
		}

	glReadPixels( sx, sy, ssx, ssy, GL_RGBA, GL_UNSIGNED_BYTE, data );

	return 0;
	}
/*********************************************************************/
/*                               GetImage                            */
/*********************************************************************/
#define ARGB_TO_ABGR(X)		(((X)&0xff00ff00)|(((X)>>16)&0x0000ff)|(((X)<<16)&0xff0000))

int TGL::GetImageABGR( int* data, int sx, int sy, int ssx, int ssy )

	{
	GetImage( data, sx, sy, ssx, ssy );
#if 1
	int size = ssx * ssy;
	for( int i = 0; i < size; i++ )
		{
		data[i] = ARGB_TO_ABGR(data[i]);
		}
#endif
	return 0;
	}

//#include "mmintrin.h"
//#include "emmintrin.h"


/****************************************************************************/
/*                                   GetDepth                               */
/****************************************************************************/

int TGL::GetDepth( int* data, int sx, int sy, int ssx, int ssy )

	{
//	ALIGN16 float conv[ 4 ] = { 16777215.0F, 16777215.0F, 16777215.0F, 16777215.0F };

	glPixelZoom( 1.0F, 1.0F );

	glPixelTransferf( GL_DEPTH_SCALE, 1.0F );

	if ( hWnd )
		{
#if ENABLE_DOUBLE_BUFFER
		glReadBuffer( GL_BACK );
#endif
		}

	glReadPixels( sx, sy, ssx, ssy, GL_DEPTH_COMPONENT, GL_FLOAT, data );


	/***********************************/
	/*            conv f2i             */
	/***********************************/
	int * p = data;
	int sz = ssx * ssy;
	
	if ( (int(p) & 0x3) == 0 )
	  {
    const int BLK = 4*4;
    // sse
  	while( sz > 0 && ((int(p) & (4*BLK - 1)) != 0) )
  	  {
  	  *p = 1 * int( ((GLfloat*)( p ))[ 0 ] * 0xffffff );
  	  p++;
      sz--;
      }

#if 0
    const float fg = 16777215.0F;
    __m128 xm_cv = _mm_set1_ps( fg );
    while( sz > BLK )
      {
      __m128 xm_d0 = _mm_load_ps((float*)&p[ 0]);
      __m128 xm_d1 = _mm_load_ps((float*)&p[ 4]);
      __m128 xm_d2 = _mm_load_ps((float*)&p[ 8]);
      __m128 xm_d3 = _mm_load_ps((float*)&p[12]);
      
      xm_d0 = _mm_mul_ps( xm_d0, xm_cv );
      xm_d1 = _mm_mul_ps( xm_d1, xm_cv );
      xm_d2 = _mm_mul_ps( xm_d2, xm_cv );
      xm_d3 = _mm_mul_ps( xm_d3, xm_cv );
      
      __m128i xm_i_d0 = _mm_cvttps_epi32(xm_d0);
      __m128i xm_i_d1 = _mm_cvttps_epi32(xm_d1);
      __m128i xm_i_d2 = _mm_cvttps_epi32(xm_d2);
      __m128i xm_i_d3 = _mm_cvttps_epi32(xm_d3);
      
      _mm_store_si128((__m128i *)&p[ 0], xm_i_d0);
      _mm_store_si128((__m128i *)&p[ 4], xm_i_d1);
      _mm_store_si128((__m128i *)&p[ 8], xm_i_d2);
      _mm_store_si128((__m128i *)&p[12], xm_i_d3);
      
      p  += BLK;
      sz -= BLK;
      }
#endif

    }
	
	while( sz > 0 )
	  {
	  *p = 1 * int( ((GLfloat*)( p ))[ 0 ] * 0xffffff );
	  p++;
    sz--;
    }

	return 0;

#if 0
	int len = ssx*ssy/16/2;

	_asm
	{
	mov			ecx, len				;
	mov			esi, data				;

loop_0:

	movaps		xmm0, [ esi+0x00 ]		;
	movaps		xmm1, [ esi+0x10 ]		;
	movaps		xmm2, [ esi+0x20 ]		;
	movaps		xmm3, [ esi+0x30 ]		;
	movaps		xmm4, [ esi+0x40 ]		;
	movaps		xmm5, [ esi+0x50 ]		;
	movaps		xmm6, [ esi+0x60 ]		;
	movaps		xmm7, [ esi+0x70 ]		;

	mulps		xmm0, conv				;
	mulps		xmm1, conv				;
	mulps		xmm2, conv				;
	mulps		xmm3, conv				;
	mulps		xmm4, conv				;
	mulps		xmm5, conv				;
	mulps		xmm6, conv				;
	mulps		xmm7, conv				;

	cvtps2pi	mm0, xmm0				; 
	cvtps2pi	mm2, xmm1				; 
	cvtps2pi	mm4, xmm2				; 
	cvtps2pi	mm6, xmm3				; 

	psrldq		xmm0, 8					;
	psrldq		xmm1, 8					;
	psrldq		xmm2, 8					;
	psrldq		xmm3, 8					;

	cvtps2pi	mm1, xmm0				; 
	cvtps2pi	mm3, xmm1				; 
	cvtps2pi	mm5, xmm2				; 
	cvtps2pi	mm7, xmm3				; 

	movntq		[ esi+0x00 ], mm0		;
	movntq		[ esi+0x08 ], mm1		;
	movntq		[ esi+0x10 ], mm2		;
	movntq		[ esi+0x18 ], mm3		;
	movntq		[ esi+0x20 ], mm4		;
	movntq		[ esi+0x28 ], mm5		;
	movntq		[ esi+0x30 ], mm6		;
	movntq		[ esi+0x38 ], mm7		;

	cvtps2pi	mm0, xmm4				; 
	cvtps2pi	mm2, xmm5				; 
	cvtps2pi	mm4, xmm6				; 
	cvtps2pi	mm6, xmm7				; 

	psrldq		xmm0, 8					;
	psrldq		xmm1, 8					;
	psrldq		xmm2, 8					;
	psrldq		xmm3, 8					;

	cvtps2pi	mm1, xmm4				; 
	cvtps2pi	mm3, xmm5				; 
	cvtps2pi	mm5, xmm6				; 
	cvtps2pi	mm7, xmm7				; 

	movntq		[ esi+0x40 ], mm0		;
	movntq		[ esi+0x48 ], mm1		;
	movntq		[ esi+0x50 ], mm2		;
	movntq		[ esi+0x58 ], mm3		;
	movntq		[ esi+0x60 ], mm4		;
	movntq		[ esi+0x68 ], mm5		;
	movntq		[ esi+0x70 ], mm6		;
	movntq		[ esi+0x78 ], mm7		;

	lea			esi, [ esi+0x80 ]		;

	dec			ecx						;
	jnz			loop_0					;

	emms								;
	}

	return 0;
#endif
	}


/*********************************************************************/
/*                              PutImage8                            */
/*********************************************************************/

int TGL::PutImage8( char* data, 
					   int dx, int dy, int dsx, int dsy,
					   int sx, int sy, int ssx, int ssy )

	{
	glPixelStorei( GL_UNPACK_ALIGNMENT,    0 );

	glPixelStorei( GL_UNPACK_ROW_LENGTH, ssx );
	glPixelStorei( GL_UNPACK_SKIP_PIXELS, sx );
	glPixelStorei( GL_UNPACK_SKIP_ROWS,   sy );

	glPixelZoom( 1.0F, -1.0F );
	glRasterPos2i( dx, winSY-dy-1 );

	glDrawPixels( ssx, ssy, GL_LUMINANCE, GL_UNSIGNED_BYTE, data );

	return 0;
	}


/*********************************************************************/
/*                               PutImage                            */
/*********************************************************************/

int TGL::PutImage( int* data, 
					   int dx, int dy, int dsx, int dsy,
					   int sx, int sy, int ssx, int ssy )

	{
	glPixelStorei( GL_UNPACK_ALIGNMENT,    1 );

	glPixelStorei( GL_UNPACK_ROW_LENGTH, ssx );
	glPixelStorei( GL_UNPACK_SKIP_PIXELS, sx );
	glPixelStorei( GL_UNPACK_SKIP_ROWS,   sy );

	glPixelZoom( 1.0F, -1.0F );
	glRasterPos2i( dx, winSY-dy-1 );

	glDrawPixels( dsx, dsy, GL_RGBA, GL_UNSIGNED_BYTE, data );

	return 0;
	}


/*********************************************************************/
/*                              SetPenColor                          */
/*********************************************************************/

int TGL::SetPenColor( int fg )

	{
	pa = TGL_A( fg )/255.0F;
	pr = TGL_R( fg )/255.0F;
	pg = TGL_G( fg )/255.0F;
	pb = TGL_B( fg )/255.0F;

	return 0;
	}


/*********************************************************************/
/*                              DotLine                              */
/*********************************************************************/

int TGL::DotLine( int x0, int y0, int x1, int y1, int thick )

	{
	glColor4f( pr, pg, pb, pa );

	glLineStipple( 1, 0xf0f0 );
	glLineWidth( float( thick ) );
	glEnable( GL_LINE_STIPPLE );

	glBegin( GL_LINE_STRIP );

	glVertex2i( x0, winSY - y0 - 1 );
	glVertex2i( x1, winSY - y1 - 1 );

	glEnd();

	glDisable( GL_LINE_STIPPLE );
	glLineStipple( 1, 0xffff );

	return 0;
	}


/*********************************************************************/
/*                                 Line                              */
/*********************************************************************/

int TGL::Line( int x0, int y0, int x1, int y1, int thick )

	{
	glColor4f( pr, pg, pb, pa );

	glLineWidth( float( thick ) );

	glBegin( GL_LINE_STRIP );

	glVertex2i( x0, winSY - y0 - 1 );
	glVertex2i( x1, winSY - y1 - 1 );

	glEnd();

	return 0;
	}


/*********************************************************************/
/*                                 Rect                              */
/*********************************************************************/

int TGL::Rect( int x0, int y0, int x1, int y1, int thick )

	{
	glColor4f( pr, pg, pb, pa );

	glLineWidth( float( thick ) );

	glBegin( GL_LINE_STRIP );

	glVertex2i( x0, winSY - y0 - 1 );
	glVertex2i( x1, winSY - y0 - 1 );
	glVertex2i( x1, winSY - y1 - 1 );
	glVertex2i( x0, winSY - y1 - 1 );
	glVertex2i( x0, winSY - y0 - 1 );

	glEnd();

	return 0;
	}

#if 0
/*********************************************************************/
/*                                 Circle                            */
/*********************************************************************/

int TGL::Circle( int x0, int y0, int r, int thick )

	{
	glColor4f( pr, pg, pb, pa );

	glLineWidth( float( thick ) );

	glBegin( GL_LINE_STRIP );

	for ( int i=0; i<=CIRCLE_DIV; i++ )
		{
		float x = cirtbl[ i*2+0 ]*r + x0;
		float y = cirtbl[ i*2+1 ]*r + winSY - y0 - 1;

		glVertex2f( x, y );
		}

	glEnd();

	return 0;
	}


/*********************************************************************/
/*                             FillCircle                            */
/*********************************************************************/

int TGL::FillCircle( int x0, int y0, int r )

	{
	glColor4f( pr, pg, pb, pa );

	glBegin( GL_TRIANGLE_STRIP );

	y0 = winSY - y0 - 1;

	for ( int i=0; i<=CIRCLE_DIV; i++ )
		{
		float x = cirtbl[ i*2+0 ]*r + x0;
		float y = cirtbl[ i*2+1 ]*r + y0;

		glVertex2i( x0, y0 );
		glVertex2f( x,  y  );
		}

	glEnd();

	return 0;
	}
#endif

/*********************************************************************/
/*                                Line3D                             */
/*********************************************************************/

int TGL::Line3D( float x0, float y0, float z0, float x1, float y1, float z1, int t )

	{
	glColor4f( pr, pg, pb, pa );

	glLineWidth( float( t ) );

	glBegin( GL_LINES );

	glVertex3f( x0, y0, z0 );
	glVertex3f( x1, y1, z1 );

	glEnd();

	return 0;
	}


/*********************************************************************/
/*                                Rect3D                             */
/*********************************************************************/

int TGL::Rect3D( float x[ 4 ], float y[ 4 ], float z[ 4 ], int t )

	{
	glColor4f( pr, pg, pb, pa );

	glLineWidth( float( t ) );

	glBegin( GL_LINES );

	glVertex3f( x[ 0 ], y[ 0 ], z[ 0 ] );
	glVertex3f( x[ 1 ], y[ 1 ], z[ 1 ] );
	glVertex3f( x[ 2 ], y[ 2 ], z[ 2 ] );
	glVertex3f( x[ 3 ], y[ 3 ], z[ 3 ] );
	glVertex3f( x[ 0 ], y[ 0 ], z[ 0 ] );

	glEnd();

	return 0;
	}


/*********************************************************************/
/*                             FillRectX3D                           */
/*********************************************************************/

int TGL::FillRectX3D( float x, float y, float z, float s )

	{
	GLfloat	  px[ 4 ];
	GLfloat   py[ 4 ];
	GLfloat	  pz[ 4 ];

	GLfloat	  Nx, Ny, Nz;

	s = s / 2.0F;

	px[ 0 ] =  0.0F; py[ 0 ] = -s; pz[ 0 ] = -s;
	px[ 1 ] =  0.0F; py[ 1 ] =  s; pz[ 1 ] = -s;
	px[ 2 ] =  0.0F; py[ 2 ] =  s; pz[ 2 ] =  s;
	px[ 3 ] =  0.0F; py[ 3 ] = -s; pz[ 3 ] =  s;

	glBegin( GL_POLYGON );
	
	CalcVector( px, py, pz, Nx, Ny, Nz );

	for ( int cnt=0; cnt<4 ; cnt++ )
		{
		glNormal3d( Nx, Ny, Nz );
		glVertex3f( px[ cnt ]+x, py[ cnt ]+y, pz[ cnt ]+z );
		}

	glEnd();


	return 0;
	}


/*********************************************************************/
/*                            FillRectY3D                            */
/*********************************************************************/

int TGL::FillRectY3D( float x, float y, float z, float s )

	{
	GLfloat	  px[ 4 ];
	GLfloat   py[ 4 ];
	GLfloat	  pz[ 4 ];

	GLfloat	  Nx, Ny, Nz;

	s = s / 2.0F;

	px[ 0 ] = -s; py[ 0 ] =  0.0F; pz[ 0 ] = -s;
	px[ 3 ] =  s; py[ 3 ] =  0.0F; pz[ 3 ] = -s;
	px[ 2 ] =  s; py[ 2 ] =  0.0F; pz[ 2 ] =  s;
	px[ 1 ] = -s; py[ 1 ] =  0.0F; pz[ 1 ] =  s;

	glBegin( GL_POLYGON );
	
	CalcVector( px, py, pz, Nx, Ny, Nz );

	for ( int cnt=0; cnt<4 ; cnt++ )
		{
		glNormal3d( Nx, Ny, Nz );
		glVertex3f( px[ cnt ]+x, py[ cnt ]+y, pz[ cnt ]+z );
		}

	glEnd();

	return 0;
	}


/*********************************************************************/
/*                             FillRectZ3D                           */
/*********************************************************************/

int TGL::FillRectZ3D( float x, float y, float z, float s )

	{
	GLfloat	  px[ 4 ];
	GLfloat   py[ 4 ];
	GLfloat	  pz[ 4 ];

	GLfloat	  Nx, Ny, Nz;

	s = s / 2.0F;

	px[ 0 ] = -s; py[ 0 ] = -s; pz[ 0 ] =  0.0F;
	px[ 1 ] =  s; py[ 1 ] = -s; pz[ 1 ] =  0.0F;
	px[ 2 ] =  s; py[ 2 ] =  s; pz[ 2 ] =  0.0F;
	px[ 3 ] = -s; py[ 3 ] =  s; pz[ 3 ] =  0.0F;

	glBegin( GL_POLYGON );
	
	CalcVector( px, py, pz, Nx, Ny, Nz );

	for ( int cnt=0; cnt<4 ; cnt++ )
		{
		glNormal3d( Nx, Ny, Nz );
		glVertex3f( px[ cnt ]+x, py[ cnt ]+y, pz[ cnt ]+z );
		}

	glEnd();

	return 0;
	}


/*********************************************************************/
/*                              FillRect3D                           */
/*********************************************************************/

int TGL::FillRect3D( float x[ 4 ], float y[ 4 ], float z[ 4 ] )

	{
	GLfloat	  Nx, Ny, Nz;

	glBegin( GL_POLYGON );
	
	CalcVector( x, y, z, Nx, Ny, Nz );

	for ( int cnt=0; cnt<4 ; cnt++ )
		{
		glNormal3d( Nx, Ny, Nz );
		glVertex3f( x[ cnt ], y[ cnt ], z[ cnt ] );
		}

	glEnd();

	return 0;
	}


/*********************************************************************/
/*                             CalcVector                            */
/*********************************************************************/

void TGL::CalcVector( GLfloat* Px, GLfloat* Py, GLfloat* Pz, GLfloat& Nx, GLfloat& Ny, GLfloat& Nz )

	{
	GLfloat x0 = Px[ 0 ] - Px[ 1 ]; 
	GLfloat y0 = Py[ 0 ] - Py[ 1 ]; 
	GLfloat z0 = Pz[ 0 ] - Pz[ 1 ];
	
	GLfloat x1 = Px[ 1 ] - Px[ 2 ]; 
	GLfloat y1 = Py[ 1 ] - Py[ 2 ]; 
	GLfloat z1 = Pz[ 1 ] - Pz[ 2 ];

	GLfloat va = y0 * z1 - y1 * z0; 
	GLfloat vb = z0 * x1 - z1 * x0; 
	GLfloat vc = x0 * y1 - x1 * y0;

	GLfloat r = (GLfloat)(sqrt( va*va + vb*vb + vc*vc ));

	if ( r <= 0.0 ) r = 0.00000000000001f;

	Nx = va / r;  
	Ny = vb / r; 
	Nz = vc / r;
	}


/*********************************************************************/
/*                               Cube3D                              */
/*********************************************************************/

int TGL::Cube3D( float x, float y, float z, float len )
	
	{
	GLfloat	  px[ 6 ][ 4 ];
	GLfloat   py[ 6 ][ 4 ];
	GLfloat	  pz[ 6 ][ 4 ];

	GLfloat	  Nx, Ny, Nz;

	len = len / 2.0F;

	px[ 0 ][ 0 ] =  len; py[ 0 ][ 0 ] = -len; pz[ 0 ][ 0 ] =  len;
	px[ 0 ][ 1 ] = -len; py[ 0 ][ 1 ] = -len; pz[ 0 ][ 1 ] =  len;
	px[ 0 ][ 2 ] = -len; py[ 0 ][ 2 ] = -len; pz[ 0 ][ 2 ] = -len;
	px[ 0 ][ 3 ] =  len; py[ 0 ][ 3 ] = -len; pz[ 0 ][ 3 ] = -len;

	px[ 1 ][ 0 ] = -len; py[ 1 ][ 0 ] =  len; pz[ 1 ][ 0 ] =  len;
	px[ 1 ][ 1 ] =  len; py[ 1 ][ 1 ] =  len; pz[ 1 ][ 1 ] =  len;
	px[ 1 ][ 2 ] =  len; py[ 1 ][ 2 ] =  len; pz[ 1 ][ 2 ] = -len;
	px[ 1 ][ 3 ] = -len; py[ 1 ][ 3 ] =  len; pz[ 1 ][ 3 ] = -len;

	px[ 2 ][ 0 ] = -len; py[ 2 ][ 0 ] = -len; pz[ 2 ][ 0 ] =  len;
	px[ 2 ][ 1 ] =  len; py[ 2 ][ 1 ] = -len; pz[ 2 ][ 1 ] =  len;
	px[ 2 ][ 2 ] =  len; py[ 2 ][ 2 ] =  len; pz[ 2 ][ 2 ] =  len;
	px[ 2 ][ 3 ] = -len; py[ 2 ][ 3 ] =  len; pz[ 2 ][ 3 ] =  len;

	px[ 3 ][ 0 ] =  len; py[ 3 ][ 0 ] = -len; pz[ 3 ][ 0 ] = -len;
	px[ 3 ][ 1 ] = -len; py[ 3 ][ 1 ] = -len; pz[ 3 ][ 1 ] = -len;
	px[ 3 ][ 2 ] = -len; py[ 3 ][ 2 ] =  len; pz[ 3 ][ 2 ] = -len;
	px[ 3 ][ 3 ] =  len; py[ 3 ][ 3 ] =  len; pz[ 3 ][ 3 ] = -len;

	px[ 4 ][ 0 ] =  len; py[ 4 ][ 0 ] = -len; pz[ 4 ][ 0 ] =  len;
	px[ 4 ][ 1 ] =  len; py[ 4 ][ 1 ] = -len; pz[ 4 ][ 1 ] = -len;
	px[ 4 ][ 2 ] =  len; py[ 4 ][ 2 ] =  len; pz[ 4 ][ 2 ] = -len;
	px[ 4 ][ 3 ] =  len; py[ 4 ][ 3 ] =  len; pz[ 4 ][ 3 ] =  len;

	px[ 5 ][ 0 ] = -len; py[ 5 ][ 0 ] = -len; pz[ 5 ][ 0 ] = -len;
	px[ 5 ][ 1 ] = -len; py[ 5 ][ 1 ] = -len; pz[ 5 ][ 1 ] =  len;
	px[ 5 ][ 2 ] = -len; py[ 5 ][ 2 ] =  len; pz[ 5 ][ 2 ] =  len;
	px[ 5 ][ 3 ] = -len; py[ 5 ][ 3 ] =  len; pz[ 5 ][ 3 ] = -len;

	for( int index = 0 ; index < 6 ; index++ )
		{
		glBegin( GL_POLYGON );
	
		CalcVector( px[ index ], py[ index ], pz[ index ], Nx, Ny, Nz );

		for( int cnt = 0 ; cnt < 4 ; cnt++ )
			{
			glNormal3d( Nx, Ny, Nz );
			glVertex3f( px[ index ][ cnt ] + x, py[ index ][ cnt ] + y,  pz[ index ][ cnt ] + z );
			}

		glEnd();
		}

	return 0;
	}


/*********************************************************************/
/*                           MakeSphere3D                            */
/*********************************************************************/
#if 0
int TGL::MakeSphere3D( float x, float y, float z, float r )

	{
	glEnable( GL_NORMALIZE );

	int d = 0;

	for ( int h=0; h<SPHERE_DIV; h++ )
		{

		for( int v=0; v<SPHERE_DIV; v++ )
			{
			glBegin( GL_POLYGON ) ;

			float x0 = sphtbl[ d++ ];
			float y0 = sphtbl[ d++ ];
			float z0 = sphtbl[ d++ ];

			float x1 = sphtbl[ d++ ];
			float y1 = sphtbl[ d++ ];
			float z1 = sphtbl[ d++ ];

			float x2 = sphtbl[ d++ ];
			float y2 = sphtbl[ d++ ];
			float z2 = sphtbl[ d++ ];

			float x3 = sphtbl[ d++ ];
			float y3 = sphtbl[ d++ ];
			float z3 = sphtbl[ d++ ];

			glNormal3f( x0, y0, z0 );
			glVertex3f( x0*r+x, y0*r+y, z0*r+z );

			glNormal3f( x1, y1, z1 );
			glVertex3f( x1*r+x, y1*r+y, z1*r+z );

			glNormal3f( x2, y2, z2 );
			glVertex3f( x2*r+x, y2*r+y, z2*r+z );

			glNormal3f( x3, y3, z3 );
			glVertex3f( x3*r+x, y3*r+y, z3*r+z );

			glEnd();
			}

		}

	glDisable( GL_NORMALIZE );

	return 0;
	}


/*********************************************************************/
/*                           InitSphere3D                            */
/*********************************************************************/

int TGL::InitSphere3D( void )

	{
	lstSph = glGenLists( 1 );

	glNewList( lstSph, GL_COMPILE );

	MakeSphere3D( 0.0F, 0.0F, 0.0F, 1.0F );

	glEndList();

	return 0;
	}


/*********************************************************************/
/*                             Sphere3D                              */
/*********************************************************************/

int TGL::Sphere3D( float x, float y, float z, float r )

	{
	if ( first )
		{
		InitSphere3D();
		first = 0;
		}

	glPushMatrix();

	glTranslatef( x, y, z );
	glScalef( r, r, r );

	glCallList( lstSph );

	glPopMatrix();

	return 0;
	}

#endif
#if 0
/*********************************************************************/
/*                             MakeSphere3D                          */
/*********************************************************************/

int TGL::MakeSphere3D( float x, float y, float z, float r )

	{
	int d = 0;

	glEnable( GL_NORMALIZE );

   	for ( int i=0; i<SPHERE_DIV; i++ )
		{
      	glBegin( GL_QUAD_STRIP );

      	for ( int j=0; j<=SPHERE_DIV; j++ )
			{
			GLfloat xx0 = sphtbl2[ d++ ];
			GLfloat yy0 = sphtbl2[ d++ ];
			GLfloat zz0 = sphtbl2[ d++ ];

			GLfloat xx1 = sphtbl2[ d++ ];
			GLfloat yy1 = sphtbl2[ d++ ];
			GLfloat zz1 = sphtbl2[ d++ ];

			glNormal3f( xx0, yy0, zz0 );
			glVertex3f( xx0*r+x, yy0*r+y, zz0*r+z );

			glNormal3f( xx1, yy1, zz1 );
			glVertex3f( xx1*r+x, yy1*r+y, zz1*r+z );
      		}

      	glEnd();
   		}

	glDisable( GL_NORMALIZE );

	return 0;
	}


/*********************************************************************/
/*                           InitSphere3D                            */
/*********************************************************************/

int TGL::InitSphere3D( void )

	{
	lstSph2 = glGenLists( 1 );

	glNewList( lstSph2, GL_COMPILE );

	MakeSphere3D( 0.0F, 0.0F, 0.0F, 1.0F );

	glEndList();

	return 0;
	}


/*********************************************************************/
/*                              Sphere3D                             */
/*********************************************************************/

int TGL::Sphere3D( float x, float y, float z, float r )

	{
	if ( first )
		{
		InitSphere3D();
		first = 0;
		}

	glPushMatrix();

	glTranslatef( x, y, z );
	glScalef( r, r, r );

	glCallList( lstSph2 );

	glPopMatrix();

	return 0;
	}
#endif

#if 0
//#include "glut.h"


int TGL::drawBitmapString(GLfloat x, GLfloat y, void *font, char *string)
{
  /* .$B8=:_$N%i%9%?!<%]%8%7%g%s$NJ]B8.(B */
  glPushAttrib(GL_CURRENT_BIT);
    
  glRasterPos2f(x, y);

  /* .$B%S%C%H%^%C%WJ8;zNs$NIA2h.(B */
//  while (*string) 
//    glutBitmapCharacter(font, *string++);

  /* .$BJ]B8$7$?%i%9%?!<%]%8%7%g%s$N%m!<%I.(B */
  glPopAttrib();

  return 0;
}

#endif
 