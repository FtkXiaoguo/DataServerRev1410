#ifndef __SingleInstance_H
#define __SingleInstance_H


#pragma once

class CSingleInstance
{
public:
	CSingleInstance(void);
	~CSingleInstance(void);
public:
	BOOL Initialize(CString strAppName);
	BOOL SetProp(HWND hWnd);
	HANDLE RemoveProp(HWND hWnd);
public:
	CString m_strAppName;
private:
	HANDLE m_hSem;
};



#endif //__SingleInstance_H