
// testDraw.cpp : 実装ファイル
//

#include "stdafx.h"
#include "testdcmInfo.h"
#include "testDraw.h"
#include "afxdialogex.h"
#include "testdcmInfoAPI.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#endif

  

CtestDraw::CtestDraw( ) 
{
	m_drawStr = "xxxxxxxxxxx";
}
BEGIN_MESSAGE_MAP(CtestDraw, CStatic)
 
	ON_WM_PAINT()
	 
END_MESSAGE_MAP()

// ダイアログに最小化ボタンを追加する場合、アイコンを描画するための
//  下のコードが必要です。ドキュメント/ビュー モデルを使う MFC アプリケーションの場合、
//  これは、Framework によって自動的に設定されます。

void CtestDraw::OnPaint()
{
	CRect rect;
	GetClientRect(rect);
	CString drawStr = m_drawStr;
	//CStatic::OnPaint();
	CPaintDC dc(this);
	dc.FillSolidRect(rect, RGB(0, 0, 0));
	CPen newPen;
	newPen.CreatePen(PS_SOLID, 2, RGB( 0, 220, 0));
	CPen* pOldPen = dc.SelectObject(&newPen);


	dc.MoveTo(10, 10);
	dc.LineTo(100, 100);
	CRect TextRect(10, 100, 150, 130);
	dc.SetTextColor(RGB(200,100,0));
	dc.SetBkMode(TRANSPARENT);
	dc.DrawText(drawStr, &TextRect, DT_SINGLELINE | DT_CENTER);

	dc.SelectObject(pOldPen);
}
 