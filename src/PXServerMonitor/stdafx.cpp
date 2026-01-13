// stdafx.cpp : 標準インクルード PXServerMonitor.pch のみを
// 含むソース ファイルは、プリコンパイル済みヘッダーになります。
// stdafx.obj にはプリコンパイルされた型情報が含まれます。

#include "stdafx.h"


void LOGOUT(LPCSTR format, ...)
{
	va_list va;
	va_start(va, format);
	CString str;
	str.FormatV(format, va);
	va_end(va);
	::OutputDebugString(str);
}
