@echo off
echo ---- copy_all.bat ----

set COPY_CMD=copy /y
set COPY_SRC_FOLDER=..\bin_v8\Release
set COPY_DEST_FOLDER= ..\install_dir\bin
  
if not  EXIST %COPY_DEST_FOLDER%  mkdir %COPY_DEST_FOLDER%

rem --- server --------------------
set COPY_DEST_FOLDER= ..\install_dir\server
%COPY_CMD% %COPY_SRC_FOLDER%\dcmlib.dll %COPY_DEST_FOLDER%
%COPY_CMD% %COPY_SRC_FOLDER%\PXDcmSServer.exe %COPY_DEST_FOLDER%
%COPY_CMD% %COPY_SRC_FOLDER%\hasp_windows_93834.dll %COPY_DEST_FOLDER%
%COPY_CMD% %COPY_SRC_FOLDER%\PXLicenseManager.dll %COPY_DEST_FOLDER%
%COPY_CMD% %COPY_SRC_FOLDER%\PXDcmJobProc.exe %COPY_DEST_FOLDER%
%COPY_CMD% %COPY_SRC_FOLDER%\PXDbManagerTask.exe %COPY_DEST_FOLDER%
%COPY_CMD% %COPY_SRC_FOLDER%\PXSetupTaskScheduler.exe %COPY_DEST_FOLDER%

rem --- admintool --------------------
set COPY_DEST_FOLDER= ..\install_dir\admintool
%COPY_CMD% %COPY_SRC_FOLDER%\dcmlib.dll %COPY_DEST_FOLDER%
%COPY_CMD% %COPY_SRC_FOLDER%\PXDcmSAdmin.exe %COPY_DEST_FOLDER%
%COPY_CMD% %COPY_SRC_FOLDER%\hasp_windows_93834.dll %COPY_DEST_FOLDER%
%COPY_CMD% %COPY_SRC_FOLDER%\PXLicenseManager.dll %COPY_DEST_FOLDER%

rem --- PxSQLDB --------------------
set COPY_SRC_FOLDER=..\PxSQLDB
set COPY_DEST_FOLDER= ..\install_dir\PxSQLDB
%COPY_CMD% %COPY_SRC_FOLDER%\*.* %COPY_DEST_FOLDER%
 
@echo on