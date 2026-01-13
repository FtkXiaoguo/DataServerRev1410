echo off
rem ==========================================
rem upgrade PXDcmGateway
rem echo
set LOG_FOLDER=c:\PxDcmSetupLog
set SRC_FOLDER=.
set DST_FOLDER=C:\PXDataServerV3
set LOG_FILE=%LOG_FOLDER%\PXDcmGateway_upgrade.txt

if EXIST %LOG_FILE% del %LOG_FILE%

echo === upgrade DicomGateway Ver 3.0.2.3 ==== >> %LOG_FILE%
date /t  >> %LOG_FILE%

if not EXIST %LOG_FOLDER% mkdir %LOG_FOLDER%
if not EXIST %DST_FOLDER% echo folder is not existed
if not EXIST %DST_FOLDER% mkdir %DST_FOLDER%
if not EXIST %DST_FOLDER%\admintool mkdir %DST_FOLDER%\admintool
if not EXIST %DST_FOLDER%\server mkdir %DST_FOLDER%\server
sc stop PXDcmJobProc >> %LOG_FILE%
sc stop PXDcmSServer >> %LOG_FILE%

taskkill /F /IM PXDcmJobProc.exe
taskkill /F /IM PXDcmSServer.exe
taskkill /F /IM PXDcmSAdmin.exe

 
xcopy /S /Y %SRC_FOLDER%\admintool\*.* %DST_FOLDER%\admintool
xcopy /S /Y %SRC_FOLDER%\server\*.* %DST_FOLDER%\server
xcopy /S /Y %SRC_FOLDER%\utils\*.* %DST_FOLDER%\utils
 

sc start PXDcmJobProc >> %LOG_FILE%
sc start PXDcmSServer >> %LOG_FILE%

echo ================================================= >> %LOG_FILE%
dir /OD %DST_FOLDER%\admintool >> %LOG_FILE%

echo ================================================= >> %LOG_FILE%
dir /OD %DST_FOLDER%\\server >> %LOG_FILE%

%LOG_FILE%