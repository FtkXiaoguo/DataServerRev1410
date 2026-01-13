// TstDumyCls.h: CTstDumyCls クラスのインターフェイス
//
//////////////////////////////////////////////////////////////////////

#if !defined(AFX_TSTDUMYCLS_H__84ABB4F7_8190_4C17_AC45_3998F889AC5C__INCLUDED_)
#define AFX_TSTDUMYCLS_H__84ABB4F7_8190_4C17_AC45_3998F889AC5C__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#ifdef MakeTstDumyLib 
	#define ITstDumyLibDefDllCls __declspec(dllexport)
	 
#else 
	#define ITstDumyLibDefDllCls __declspec(dllimport)
	 
#endif

 class ITstDumyLibDefDllCls CTstDumyCls  
{
public:
	CTstDumyCls();
	virtual ~CTstDumyCls();

	int getTestID();
};

#endif // !defined(AFX_TSTDUMYCLS_H__84ABB4F7_8190_4C17_AC45_3998F889AC5C__INCLUDED_)
