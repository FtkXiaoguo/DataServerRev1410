/*$Id: CglDIB.cpp 282 2009-07-03 06:49:19Z yasuyuki $*/

// 
// win_offscreen_rendering
// CglDIB.cpp
// 
// The MIT License
// 
// Copyright (c) 2009 sonson, sonson@Picture&Software
// 
// Permission is hereby granted, free of charge, to any person obtaining a copy
// of this software and associated documentation files (the "Software"), to deal
// in the Software without restriction, including without limitation the rights
// to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
// copies of the Software, and to permit persons to whom the Software is
// furnished to do so, subject to the following conditions:
// 
// The above copyright notice and this permission notice shall be included in
// all copies or substantial portions of the Software.
// 
// THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
// IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
// FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
// AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
// LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
// OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
// THE SOFTWARE.
/////////////////////////////////////////////////////////////////////////////////
//OpenGL用DIBクラス実装

 

#include <crtdbg.h>
#if defined( _DEBUG )
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

#include "CglDIB.h"


CglDIB::CglDIB( void )
{
  hDC = 0;
  hBitmap = 0;
  

  BitmapInfo = 0;
  data = 0;
  width = 0;
  height = 0;
  color_bit = 0;
  depth_bit = 0;
}

///////////////////////////////////////////////////////////////////////////////
//デストラクタ
CglDIB::~CglDIB(){
  Term();
};

void CglDIB::Term( void )
{
	//カレントコンテキストを解放
	//以前のビットマップとむすびつける
  if ( hDC )
    {
  	SelectObject(hDC, hBitmapOld);
  	}
	//ビットマップオブジェクトを削除
	if ( hBitmap )
	  {
  	DeleteObject(hBitmap);
    hBitmap = 0;
  	}
	//DCを開放
  if ( hDC )
    {
  	DeleteDC(hDC);
    hDC = 0;
  	}
	//ビットマップヘッダのメモリ開放
	delete BitmapInfo;
	BitmapInfo = 0;
}

///////////////////////////////////////////////////////////////////////////////
//描画
BOOL CglDIB::Draw(HDC destDC,			//コピー先のデバイスコンテキスト
				  int destWidth,		//コピー先領域の範囲
				  int destHeight
				  ){
	//転送モード設定
	SetStretchBltMode(destDC, COLORONCOLOR);
	//転送（コピー）
	StretchDIBits(
		destDC,
		0,0,			//コピー先の座標
		destWidth,destHeight,
		0,0,			//ソースの座標
		width, height,
		data,
		(BITMAPINFO*)BitmapInfo,
		DIB_RGB_COLORS,
		SRCCOPY
	);
  return TRUE;
};

///////////////////////////////////////////////////////////////////////////////
//ビットマップメモリの作成
BOOL CglDIB::CreateGLDIB(int inputWidth,		//メモリのサイズ
						 int inputHeight,
						 int inputColor,		//メモリの色数
						 int inputDepth			//メモリのデプスバッファ
						 ){
	width = inputWidth;
	height = inputHeight;
	
	//ビットマップ情報
	delete BitmapInfo;
	BitmapInfo = new BITMAPINFOHEADER;
	if ( BitmapInfo == NULL ) return FALSE;
	
	int iSize = sizeof(BITMAPINFOHEADER) ;
	memset(BitmapInfo, 0, iSize);
	//
	BitmapInfo->biSize = iSize;
	BitmapInfo->biWidth = width;
	BitmapInfo->biHeight = height;
	BitmapInfo->biPlanes = 1;
	BitmapInfo->biBitCount = inputColor;
	BitmapInfo->biCompression = BI_RGB;
	//互換デバイスコンテキストの作成
	hDC = CreateCompatibleDC(NULL);
	if ( hDC == NULL )
	  {
    return FALSE;
    }
	
	//ビットマップメモリ作成
	hBitmap = CreateDIBSection(hDC,
		(BITMAPINFO*)BitmapInfo,
		DIB_RGB_COLORS,
		&data,
		NULL,
		0
		);
    //以前のビットマップメモリ情報を退避させる
	if(hBitmap){
		hBitmapOld = (HBITMAP)SelectObject(hDC,hBitmap);
	}

	return TRUE;
};

///////////////////////////////////////////////////////////////////////////////
HDC	CglDIB::GetDC(void){
	//ビットマップメモリクラスのコンテキストを返す
	return hDC;
}
///////////////////////////////////////////////////////////////////////////////
HBITMAP CglDIB::GetBitmap(void){
	//ビットマップメモリクラスのビットマップハンドルを返す
	return hBitmap;
}

 
