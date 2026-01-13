echo off
rem ==========================================
rem upgrade PXJPEGGateway
rem 
set LOG_FOLDER=c:\PXJPEGGatewayLog
set SRC_FOLDER=.
set DST_FOLDER=C:\PXJPEGGateway
set LOG_FILE=%LOG_FOLDER%\PXJPEGGateway_Upgrade.txt

del %LOG_FILE%

echo === upgrade JpegGateway Ver 3.0.2.5 ==== >> %LOG_FILE%

date /t  >> %LOG_FILE%

if not EXIST %LOG_FOLDER% mkdir %LOG_FOLDER%
if not EXIST %DST_FOLDER% echo folder is not existed
if not EXIST %DST_FOLDER% mkdir %DST_FOLDER%
if not EXIST %DST_FOLDER%\admintool mkdir %DST_FOLDER%\admintool
if not EXIST %DST_FOLDER%\server mkdir %DST_FOLDER%\server

rem ==============================
rem check the service status
rem ==============================
setlocal enabledelayedexpansion
FOR /F "delims=" %%L IN ('sc query PXDcmJobProc ') do (
SET CL=%%L
SET CL2=!CL: =!
IF "!CL2:~0,7!"=="STATE:1" SET JobProc_Sts=STOP1
IF "!CL2:~0,7!"=="STATE:2" SET JobProc_Sts=STOP2
IF "!CL2:~0,7!"=="STATE:3" SET JobProc_Sts=STOP3
IF "!CL2:~0,7!"=="STATE:4" SET JobProc_Sts=START
)
rem ===
FOR /F "delims=" %%L IN ('sc query PXDcmSServer') do (
SET CL=%%L
SET CL2=!CL: =!
IF "!CL2:~0,7!"=="STATE:1" SET Server_Sts=STOP1
IF "!CL2:~0,7!"=="STATE:2" SET Server_Sts=STOP2
IF "!CL2:~0,7!"=="STATE:3" SET Server_Sts=STOP3
IF "!CL2:~0,7!"=="STATE:4" SET Server_Sts=START
)
rem ===

if NOT %JobProc_Sts% == START goto Lab_a1
echo to stop PXDcmJobProc >> %LOG_FILE%
sc stop PXDcmJobProc >> %LOG_FILE%

:Lab_a1

if NOT %Server_Sts% == START goto Lab_a2
echo to stop PXDcmSServer >> %LOG_FILE%
sc stop PXDcmSServer >> %LOG_FILE%

:Lab_a2

taskkill /F /IM PXDcmJobProc.exe
taskkill /F /IM PXDcmSServer.exe
taskkill /F /IM PXDcmSAdmin.exe

xcopy /S /Y %SRC_FOLDER%\admintool\*.* %DST_FOLDER%\admintool
xcopy /S /Y %SRC_FOLDER%\server\*.* %DST_FOLDER%\server
xcopy /S /Y %SRC_FOLDER%\utils\*.* %DST_FOLDER%\utils

%DST_FOLDER%\utils\ModifyConfigure.exe %DST_FOLDER%\config\PXDcmSServer.cfg useJPEGGateway_AE 1 >> %LOG_FILE%

echo utils\ntrights.exe +r SeServiceLogonRight -u finecube >> %LOG_FILE%
%DST_FOLDER%\utils\ntrights.exe +r SeServiceLogonRight -u finecube

echo sc config PXDcmJobProc obj= .\finecube password= finecube >> %LOG_FILE%
sc config PXDcmJobProc obj= .\finecube password= finecube

rem ==============================
rem restart the services
rem ==============================
if NOT %JobProc_Sts% == START goto Lab_b1
sc config PXDcmJobProc obj= .\finecube password= finecube >> %LOG_FILE%
echo to start PXDcmJobProc >> %LOG_FILE%
if %JobProc_Sts% == START sc start PXDcmJobProc >> %LOG_FILE%

:Lab_b1

if NOT %Server_Sts% == START goto Lab_b2
echo to start PXDcmSServer >> %LOG_FILE%
sc start PXDcmSServer >> %LOG_FILE%

:Lab_b2

echo ================================================= >> %LOG_FILE%
dir /OD %DST_FOLDER%\admintool >> %LOG_FILE%

echo ================================================= >> %LOG_FILE%
dir /OD %DST_FOLDER%\\server >> %LOG_FILE%

%LOG_FILE%

 