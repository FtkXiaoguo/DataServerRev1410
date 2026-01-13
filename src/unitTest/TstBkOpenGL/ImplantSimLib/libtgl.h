/*$Id: libtgl.h 731 2009-09-09 11:55:02Z yasuyuki $*/
/****************************************************************************/
/*                                                                          */
/*                         Tiny OpenGL Library TGL                          */
/*                                                                          */
/****************************************************************************/
 

#ifndef __tgl_h__
#define __tgl_h__




/****************************************************************************/
/*                                 include                                  */
/****************************************************************************/

//#define _WIN32_WINNT 0x0400

//#include "StdAfx.h"
#include <windows.h>

#include <GL/gl.h>
#include <GL/glu.h>
#include <GL/glaux.h>

#include <stdarg.h>

#include "CglDIB.h"

/****************************************************************************/
/*                                 constant                                 */
/****************************************************************************/



/****************************************************************************/
/*                                  macros                                  */
/****************************************************************************/

#define TGL_ARGB( A, R, G, B ) 	( ((A)<<24) | ((R)<<16) | ((G)<<8) | ((B)) )
#define TGL_COLOR( A, R, G, B ) ( ((A)<<24) | ((R)<<16) | ((G)<<8) | ((B)) )

#define TGL_A( X ) 	            (((X)>>24)&0xff)
#define TGL_R( X ) 	            (((X)>>16)&0xff)
#define TGL_G( X ) 	            (((X)>> 8)&0xff)
#define TGL_B( X ) 	            (((X)>> 0)&0xff)

#define TGL_BLACK				TGL_COLOR( 0,   0,   0,   0 )
#define TGL_WHITE				TGL_COLOR( 0, 255, 255, 255 )
#define TGL_BLUE				TGL_COLOR( 0,   0,   0, 255 )
#define TGL_RED					TGL_COLOR( 0, 255,   0,   0 )
#define TGL_GREEN				TGL_COLOR( 0,   0, 255,   0 )
#define TGL_CYAN				TGL_COLOR( 0,   0, 255, 255 )
#define TGL_YELLOW				TGL_COLOR( 0, 255, 255,   0 )
#define TGL_MAGENDA				TGL_COLOR( 0, 255,   0, 255 )


const int CIRCLE_DIV = 45;
const int SPHERE_DIV = 16;


/****************************************************************************/
/*                                  struct                                  */
/****************************************************************************/

struct Lookat
	{
	float	vx;
	float	vy;
	float	vz;
	float	rx;
	float	ry;
	float	rz;
	float 	twist;
	float	fov;
	float   far_plane;
	float	near_plane;
	float	aspect;
	int		used;
	};


/****************************************************************************/
/*                                  class                                   */
/****************************************************************************/
class TGL

	{
	CglDIB m_CglDIB;

	HANDLE m_thd;
	DWORD  m_tid;
	
	public:
		void CopyFromDIB( HDC * dc, int sx, int sy );
	/***********************************/
	/*         member variable         */
	/***********************************/
	protected:
		int 	retcd;

	public:
		int		first;
//		CWnd* 	cwnd;
		HWND	hWnd;
		
		int		m_bDoubleBuffer;

	protected:
		HGLRC	hrc;
		HDC		hdc;

		int 	flag;

		GLuint fontOffset;

		float 	tfga;
		float 	tfgr;
		float 	tfgg;
		float 	tfgb;

		float 	tbga;
		float 	tbgr;
		float 	tbgg;
		float 	tbgb;

		float 	pa;
		float 	pr;
		float 	pg;
		float 	pb;

#if 0
		float	cirtbl[ (CIRCLE_DIV+1)*2 ];
		float*	sphtbl;
		float*	sphtbl2;

		GLuint	lstSph;
		GLuint	lstSph2;
#endif



		struct Lookat at;

		float	sc_x;
		float	sc_y;
		float	sc_z;

		float	of_x;
		float   of_y;
		float   of_z;

		int		clr_col;
	public:
		int 	winX;
		int 	winY;
		int 	winSX;
		int 	winSY;
		
		int		m_window_sx;
		int		m_window_sy;
		
		int GetWidth( void ) { return m_window_sx; }
		int GetHeight( void ) { return m_window_sy; }


	/***********************************/
	/*         member function         */
	/***********************************/
	protected:
		int InitFont( void );

	public:
		 TGL( void );
		~TGL( void );
		
		void	SetWgl( void );
		void	ReleaseWgl( void );
		
		unsigned int glGetNewList( unsigned int );

//		int IsEnable(void){return (int)cwnd->m_hWnd;}
		int IsEnable(void){return (int)hdc;}

//		int Init( CWnd& hwnd, int flag );
//		int Init( HWND * hwnd, int sx, int sy );
		int Init( HWND hwnd );
		int Init( HDC _hDC, int sx, int sy, BOOL bWnd = FALSE );
		int Init( int sx, int sy );
		int InitWindow( int sx, int sy );

		int Term( void );

		int Init2D( void );
		int Term2D( int flag=0 );

		int Init3D( int X0, int Y0, int SX, int SY,
		     float fa = 0, float fr=0, float fg=0, float fb=0 );
		int Term3D( void );

		int Flush( void );
		int SwapBuffer( void );

		int Clear( void );
		int ClearBackBuffer( void );
    
    
    int GetError( void );
    
		/***********************************/
		/*             for 2D              */
		/***********************************/
		int SetTextColor( int fg, int bg );
		int PutText( int x, int y, const char* str );
		int Printf( int x, int y, const char* fmt, ... );

		int SetPenColor( int fg );

		int Line( int x0, int y0, int x1, int y1, int thick=1 );
		int Rect( int x0, int y0, int x1, int y1, int thick=1 );
		int DotLine( int x0, int y0, int x1, int y1, int thick=1 );

#if 0
		int Circle( int x0, int y0, int r, int thick );
		int FillCircle( int x0, int y0, int r );
#endif

		int PutImage( int* data, int dx, int dy, int dsx, int dsy,
					  int sx, int sy, int ssx, int ssy );

		int PutImage8( char* data, int dx, int dy, int dsx, int dsy,
					  int sx, int sy, int ssx, int ssy );

		int GetImage( int* data, int sx, int sy, int ssx, int ssy );
		int GetImageABGR( int* data, int sx, int sy, int ssx, int ssy );


		/***********************************/
		/*             for 3D              */
		/***********************************/
		int GetDepth( int* data, int sx, int sy, int ssx, int ssy );
		int SetupLight( void );
		int SetModelViewMatrix3D( double m[ 16 ] );
		int SetProjectionMatrix3D( double m[ 16 ] );
		void SetClearColor(int col){clr_col = col;}
		int SetScale( float sx, float sy, float sz );
		int SetOffset( float sx, float sy, float sz );
		int SetLight( float lx, float ly, float lz );
		int SetViewPoint( struct Lookat& at );
		int SetWorldMatrix( float rx, float ry, float rz );

		int SetColorMaterial( int col, float dif=1.0F, float amb=0.2F, float spc=1.0F );

		void CalcVector( GLfloat* Px, GLfloat* Py, GLfloat* Pz, GLfloat& Nx, GLfloat& Ny, GLfloat& Nz );

		int Line3D( float x0, float y0, float z0, float x1, float y1, float z1, int thick );
		int Rect3D( float x[ 4 ], float y[ 4 ], float z[ 4 ], int t );

		int FillRectX3D( float x, float y, float z, float s );
		int FillRectY3D( float x, float y, float z, float s );
		int FillRectZ3D( float x, float y, float z, float s );
		int FillRect3D( float x[ 4 ], float y[ 4 ], float z[ 4 ] );

		int Cube3D( float x, float y, float z, float c );

#if 0
		int MakeSphere3D( float x, float y, float z, float r );
		int InitSphere3D( void );
		int Sphere3D( float x, float y, float z, float r );
#endif
//		int drawBitmapString(GLfloat x, GLfloat y, void *font, char *string);
	};


#endif

 
