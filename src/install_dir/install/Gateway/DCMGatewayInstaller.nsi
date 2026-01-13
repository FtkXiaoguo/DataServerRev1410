;
;
;  This script is for the NSIS.
;---------------------------------
; DICOM Transfer tool installer
;--------------------------------
;Include Modern UI

!include "MUI2.nsh"
!include "FileFunc.nsh"

!ifdef PACKAGE_DICOMGATEWAY
;---------------------------------------
; For DICOM Gateway
;---------------------------------------
!define PACKAGE_NAME        "PreXion DICOM Gateway Tool"
!define PACKAGE_EXE_NAME    "PXDICOMGateway"
!define PACKAGE_INSTALL_DIR "C:\PXDcmGateway"
!define LOG_FILE_DIR "C:\PXDcmGatewayLog"
!define APP_REG_KEY "Software\PreXion\DCMGateway"

!define PACKAGE_MAIN_EXE_NAME    "admintool\PXDcmSAdmin.exe"
!define DESKTOP_ICON_NAME  "PXDcmSAdmin"

!else ifdef PACKAGE_JPEGGATEWAY
;---------------------------------------
; For Jpeg Gateway
;---------------------------------------
!define PACKAGE_NAME        "PreXion Jpeg Gateway Tool"
!define PACKAGE_EXE_NAME    "PXJPEGGateway"
!define PACKAGE_INSTALL_DIR "C:\PXJPEGGateway"
!define LOG_FILE_DIR "C:\PXJPEGGatewayLog"
!define APP_REG_KEY "Software\PreXion\JpegGateway"

!define PACKAGE_MAIN_EXE_NAME    "admintool\PXDcmSAdmin.exe"
!define DESKTOP_ICON_NAME  "PXDcmSAdmin"

!ifdef UPGRADE
Var  JobProcServiceStartStatus
Var  DcmSServerServiceStartStatus
!endif

!else
!error "please define package"
!endif

!define PACKAGE_SRC_DIR     "..\..\"

!define UNINST_REG_KEY "Software\Microsoft\Windows\CurrentVersion\Uninstall\PreXion ${PACKAGE_NAME}"


!define PACKAGE_MAJOR   '3'
!define PACKAGE_MINOR   '0'
!define PACKAGE_SUB     '2'
!define PACKAGE_MICRO   '3'

!define PACKAGE_VERSION "${PACKAGE_MAJOR}.${PACKAGE_MINOR}.${PACKAGE_SUB}.${PACKAGE_MICRO}"

; The file to write
!ifdef UPGRADE
OutFile ${PACKAGE_EXE_NAME}_upgrade.exe
!else
OutFile ${PACKAGE_EXE_NAME}_setup.exe
!endif
; Request application privileges for Windows Vista/7
RequestExecutionLevel admin

; The default installation directory
InstallDir "${PACKAGE_INSTALL_DIR}"


;--------------------------------

; Pages
  !insertmacro MUI_PAGE_WELCOME
  !insertmacro MUI_PAGE_INSTFILES
  !insertmacro MUI_PAGE_FINISH

  !insertmacro MUI_UNPAGE_WELCOME
  !insertmacro MUI_UNPAGE_CONFIRM
  !insertmacro MUI_UNPAGE_INSTFILES
  !insertmacro MUI_UNPAGE_FINISH

;--------------------------------
;Languages

  !insertmacro MUI_LANGUAGE "English"

!ifdef PACKAGE_DICOMGATEWAY
LangString Name ${LANG_ENGLISH}  "DICOM Gateway Tool"
!else ifdef PACKAGE_JPEGGATEWAY
LangString Name ${LANG_ENGLISH}  "Jpeg Gateway Tool"
!endif

Name "$(Name) v${PACKAGE_VERSION}"

;--------------------------------
;Version Information

  VIProductVersion "${PACKAGE_VERSION}"
  VIAddVersionKey /LANG=${LANG_ENGLISH} "ProductName" "${PACKAGE_NAME}"
  VIAddVersionKey /LANG=${LANG_ENGLISH} "Comments" ""
  VIAddVersionKey /LANG=${LANG_ENGLISH} "CompanyName" "PreXion, Inc."
  VIAddVersionKey /LANG=${LANG_ENGLISH} "LegalTrademarks" ""
  VIAddVersionKey /LANG=${LANG_ENGLISH} "LegalCopyright" "Copyright (C) 2013, PreXion, Inc."

!ifdef UPGRADE
  VIAddVersionKey /LANG=${LANG_ENGLISH} "FileDescription" "${PACKAGE_NAME} Upgrader"
!else
  VIAddVersionKey /LANG=${LANG_ENGLISH} "FileDescription" "${PACKAGE_NAME} Installer"
!endif

  VIAddVersionKey /LANG=${LANG_ENGLISH} "ProductVersion" "${PACKAGE_VERSION}"
  VIAddVersionKey /LANG=${LANG_ENGLISH} "FileVersion" "${PACKAGE_VERSION}"


;--------------------------------

Function execshell_error
  DetailPrint "Cannot execute command"
  MessageBox MB_OK "Cannot execute command."
  Quit ; for debug
FunctionEnd

Function execshell_error_and_quit
  DetailPrint "Cannot execute command"
  MessageBox MB_OK "Cannot execute command. Installer will quit."
  Quit
FunctionEnd

; The stuff to install
Section "Install" ;No components page, name is not important

; set registry setting target (current user or all)
SetShellVarContext all

SetDetailsView show
LogSet on

  ; need to check the previous version is installed

!ifndef UPGRADE
  ; newly install, there must not be file before install.
IfFileExists $INSTDIR\server\PXDcmSServer.exe 0 newinstall
  MessageBox MB_OK|MB_ICONSTOP "$(Name) is already installed. Please use upgrade tool."
  Quit
newinstall:

!else

  ; upgrade install. there must be file before install
IfFileExists $INSTDIR\server\PXDcmSServer.exe check_version 0
  MessageBox MB_OK|MB_ICONSTOP "$(Name) is not installed. This is upgrade tool. Please install it by installer."
  Quit

check_version:
  ; version check
  ${GetFileVersion} "$INSTDIR\server\PXDcmSServer.exe" $R0

  ; compare version
  StrCmp $R0 "${PACKAGE_VERSION}" 0 +3
    MessageBox MB_YESNO|MB_ICONQUESTION|MB_DEFBUTTON2 "Version $R0 is already installed. Do you want to overwrite it?" IDYES overwrite IDNO 0
    Quit
overwrite:

!ifdef PACKAGE_JPEGGATEWAY
; check exist service config (start or stop)
  SimpleSC::GetServiceStatus PXDcmJobProc
  Pop $0	; error code
  Pop $JobProcServiceStartStatus	; status

  IntCmp $0 0 success1
   ;error
  MessageBox MB_OK "Cannot get PXDcmJobProc status"
  Quit

success1:

  ; debug
  MessageBox MB_OK "PXDcmJobProc status is $JobProcServiceStartStatus"

;--------------------------
  SimpleSC::GetServiceStatus PXDcmSServer
  Pop $0	; error code
  Pop $DcmSServerServiceStartStatus	; status

  IntCmp $0 0 success2
   ;error
  MessageBox MB_OK "Cannot get PXDcmJobProc status"
  Quit

success2:

  ; debug
  MessageBox MB_OK "PXDcmSServer status is $DcmSServerServiceStartStatus"
  Quit

!endif

  ;--------------------------
  ; Stop service
  ExecWait '"sc" stop PXDcmJobProc' $0
  ExecWait '"sc" stop PXDcmSServer' $0

  ExecWait '"taskkill" /F /IM PXDcmJobProc.exe' $0
  ExecWait '"taskkill" /F /IM PXDcmSServer.exe' $0
  ExecWait '"taskkill" /F /IM PXDcmSAdmin.exe' $0
!endif

  ; create directories
  CreateDirectory ${LOG_FILE_DIR}

  ; Set output path to the installation directory.
  SetOutPath $INSTDIR

!ifndef UPGRADE
  File /x .svn ${PACKAGE_SRC_DIR}\PXMediaPoint.exe
!endif

  ; admintool
  File /r /x .svn ${PACKAGE_SRC_DIR}\admintool

  ; Server
  File /r /x .svn ${PACKAGE_SRC_DIR}\server

  ; utils
  File /r /x .svn ${PACKAGE_SRC_DIR}\utils

  SetOutPath $INSTDIR\PxSQLDB
  File /x .svn ${PACKAGE_SRC_DIR}\PxSQLDB\*.*
!ifdef PACKAGE_DICOMGATEWAY
  File /x .svn ${PACKAGE_SRC_DIR}\gateway\PxSQLDB\*.*
!else ifdef PACKAGE_JPEGGATEWAY
  File /x .svn ${PACKAGE_SRC_DIR}\jpeg_gateway\PxSQLDB\*.*
!endif

!ifndef UPGRADE
  SetOutPath $INSTDIR\config
!ifdef PACKAGE_DICOMGATEWAY
  File /x .svn ${PACKAGE_SRC_DIR}\gateway\config\*.*
!else ifdef PACKAGE_JPEGGATEWAY
  File /x .svn ${PACKAGE_SRC_DIR}\jpeg_gateway\config\*.*
!endif
!endif	; UPGRADE


 # Sleep 5000 # for capurturing image (for debug)

!ifndef UPGRADE
   ; newly install
!ifdef PACKAGE_DICOMGATEWAY
!else ifdef PACKAGE_JPEGGATEWAY
!endif

!else	; UPGRADE
  ; change configuration
!ifdef PACKAGE_DICOMGATEWAY
  ClearErrors
  ExecWait '"$INSTDIR\utils\ModifyConfigure.exe" $INSTDIR\config\PXDcmJobProc.cfg EnableAutoDeleteStudy 1' $0
  IfErrors 0 +2
     Call execshell_error

  ClearErrors
  ExecWait '"$INSTDIR\utils\ModifyConfigure.exe" $INSTDIR\config\PXDcmJobProc.cfg AutoDeleteStudyInterval 5' $0

  ClearErrors
  ExecWait '"$INSTDIR\utils\ModifyConfigure.exe" $INSTDIR\config\PXDcmJobProc.cfg AutoDeleteKeepDays 90' $0

!else ifdef PACKAGE_JPEGGATEWAY
  ClearErrors
  ExecWait '"$INSTDIR\utils\ModifyConfigure.exe" $INSTDIR\config\PXDcmSServer.cfg useJPEGGateway_AE 1' $0

  ClearErrors
  ExecWait '"$INSTDIR%\utils\ntrights.exe" +r SeServiceLogonRight -u finecube' $0

  ClearErrors
  ExecWait '"sc" config PXDcmJobProc obj= .\finecube password= finecube' $0
!endif
!endif	; UPGRADE

!ifdef PACKAGE_JPEGGATEWAY && UPGRADE
  IntCmp $JobProcServiceStartStatus 4 start_JobProc
  Goto DcmServer1
start_JobProc:
  ; start service
  ExecWait '"sc" start PXDcmJobProc' $0
  IfErrors 0 +2
     Call execshell_error

DcmServer1:
  IntCmp $DcmSServerServiceStartStatus 4 start_DcmServer
  Goto otherService

start_DcmServer:
  ExecWait '"sc" start PXDcmSServer' $0
  IfErrors 0 +2
     Call execshell_error

otherService:

!else
  ; start service
  ExecWait '"sc" start PXDcmJobProc' $0
  IfErrors 0 +2
     Call execshell_error
  ExecWait '"sc" start PXDcmSServer' $0
  IfErrors 0 +2
     Call execshell_error
!endif

; write registry for application

; Add Registry for uninstall
  WriteRegExpandStr HKLM "${UNINST_REG_KEY}" "UninstallString" '"$INSTDIR\uninstall.exe"'
  WriteRegExpandStr HKLM "${UNINST_REG_KEY}" "InstallLocation" "$INSTDIR"
  WriteRegStr HKLM "${UNINST_REG_KEY}" "DisplayName" "PreXion $(Name)"
  WriteRegStr HKLM "${UNINST_REG_KEY}" "DisplayIcon" "$INSTDIR\${PACKAGE_MAIN_EXE_NAME},0"
  WriteRegStr HKLM "${UNINST_REG_KEY}" "Publisher" "PreXion, Inc."
  WriteRegStr HKLM "${UNINST_REG_KEY}" "DisplayVersion" "${PACKAGE_VERSION}"
  WriteRegDWORD HKLM "${UNINST_REG_KEY}" "NoModify" 1

  WriteUninstaller $INSTDIR\uninstall.exe


; open install folder for check
  ExecShell "open" "$INSTDIR"

SectionEnd ; end the main section

; Create shortcut
Section "Create Shortcuts"
  ; this may affect as work directory
  SetOutPath $INSTDIR
  CreateShortCut "$DESKTOP\${DESKTOP_ICON_NAME}.lnk" "$INSTDIR\${PACKAGE_MAIN_EXE_NAME}"

;  CreateDirectory "$SMPROGRAMS\PreXion\${PACKAGE_NAME}"
;  CreateShortCut "$SMPROGRAMS\PreXion\${PACKAGE_NAME}\${DESKTOP_ICON_NAME}.lnk" "$INSTDIR\${PACKAGE_MAIN_EXE_NAME}"
SectionEnd


;--------------------------------
; Uninstaller
Section "Uninstall"

  ; Stop service
  ExecWait '"sc" stop PXDcmSServer' $0
  ExecWait '"sc" delete PXDcmSServer' $0
  ExecWait '"sc" stop PXDcmJobProc' $0
  ExecWait '"sc" delete PXDcmJobProc' $0

   ; Remove desktop shortcut
    Delete "$DESKTOP\${DESKTOP_ICON_NAME}.lnk"

   ; Remove Dir
    RMDir /r "$INSTDIR\admintool"
    RMDir /r "$INSTDIR\server"
    RMDir /r "$INSTDIR\utils"
    RMDir /r "$INSTDIR\PxSQLDB"

    DELETE /REBOOTOK "$INSTDIR\uninstall.exe"

SectionEnd


#PageEx license
#   LicenseForceSelection checkbox
#PageExEnd

;------------------------------------------------------------------
;			End of files
;------------------------------------------------------------------
