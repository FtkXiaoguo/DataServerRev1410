#include <afxcmn.h>
#include <afxwin.h>
#include <shlobj.h>
#ifdef __cplusplus

class CAQBrowseForFolder
{
	public:
		//  Shuji 2005.06.22
		// Added bEnableCreateNewFolder argument to support "New Folder" button
		CAQBrowseForFolder( bool bEnableEditBox = false, 
							bool bEnableCreateNewFolder = false, 
							bool bEnableSubDirButton = false, 
							bool bSubDirState = true );
		virtual	~CAQBrowseForFolder();

	public:
		inline	static	void	SetEnableSubDirButton(bool bFlag) {m_bEnableSubDirButton = bFlag;}
		inline	static	bool	GetEnableSubDirButton(void) {return(m_bEnableSubDirButton);}
		inline	static	void	SetSearchSubDirState(bool bFlag) {m_bSearchSubDirState = bFlag;}
		inline	static	bool	GetSearchSubDirState(void) {return(GetEnableSubDirButton() ? m_bSearchSubDirState : false);}

		bool			Exec(HWND hOwner, const char* sTitle, char* sLocal);
		static	void	GetPathFromEdit(CEdit* pEdit);

		static	void	CloseWindow();

	public:
		static	char	m_strPathName[_MAX_PATH];

	private:
		static	bool	m_bEnableCreateNewFolder;
		static	bool	m_bEnableEditBox;
		static	bool	m_bEnableSubDirButton;
		static	bool	m_bSearchSubDirState;
		static	WNDPROC	m_VSSelectOrgProc;
		static	WNDPROC	m_VSEditOrgProc;

		static	LRESULT CALLBACK VSSelectFolderSubProc(HWND, UINT, WPARAM, LPARAM);
		static	LRESULT CALLBACK VSEditProc(HWND, UINT, WPARAM, LPARAM);
		static	int CALLBACK CallbackSelectDir(HWND, UINT, LPARAM, LPARAM);
};

#endif
