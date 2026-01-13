#include "StdAfx.h"
#include "SingleInstance.h"

CSingleInstance::CSingleInstance(void)
{
	m_hSem = NULL;
}


CSingleInstance::~CSingleInstance(void)
{
	if (m_hSem){
		CloseHandle(m_hSem);
		m_hSem = NULL;
	}
}

BOOL CSingleInstance::SetProp(HWND hWnd)
{
	BOOL bRtn = FALSE;
	if (hWnd && !m_strAppName.IsEmpty())
		bRtn = ::SetProp(hWnd, m_strAppName, (HANDLE)1);
	return bRtn;
}

HANDLE CSingleInstance::RemoveProp(HWND hWnd)
{
	HANDLE hRtn = NULL;
	if (hWnd && !m_strAppName.IsEmpty())
		hRtn = ::RemoveProp(hWnd, m_strAppName);
	return hRtn;
}

BOOL CSingleInstance::Initialize(CString strAppName)
{
	BOOL bRtn = TRUE;
	m_strAppName = strAppName;
	CString strSemName = _T("###_") + strAppName;
	m_hSem=CreateSemaphore(NULL,1,1,strSemName);
	if(m_hSem){
		if(ERROR_ALREADY_EXISTS==GetLastError()){	
			bRtn = FALSE;
			CloseHandle(m_hSem);
			m_hSem = NULL;
			HWND hWndPrev=::GetWindow(::GetDesktopWindow(),GW_CHILD);

			while(::IsWindow(hWndPrev))
			{
				if(::GetProp(hWndPrev, strAppName))
				{
					if (::IsIconic(hWndPrev))
						::ShowWindow(hWndPrev,SW_RESTORE);

					::ShowWindow(hWndPrev, SW_SHOW);
					::SetForegroundWindow(hWndPrev);
	
				}
				hWndPrev = ::GetWindow(hWndPrev,GW_HWNDNEXT);
			}
			DebugLog(strAppName + _T(" is running!\n"));
		}
	}else{
		DebugLog(_T("Create CreateSemaphore %s Failed!\n"), strSemName);
		bRtn = FALSE;
	} 
	return bRtn;
}
