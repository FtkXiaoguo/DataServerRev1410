echo off

rem ==========================================
rem setup install LOG
rem  
set WorkFolder=%~dp0
echo WorkFolder %WorkFolder%

c:
cd %WorkFolder%

set LOG_FOLDER=C:\PxDcmSetupLog
if not  EXIST %LOG_FOLDER%  mkdir %LOG_FOLDER%
set LOG_FILE=%LOG_FOLDER%\PXDataSServerInstall.log
rem 
rem ==========================================

echo ===== start install (JPEGActionGate) Ver 3.2.1.0 ====== >> %LOG_FILE%
echo ===== start install (JPEGActionGate) Ver 3.2.1.0 ====== 

date /T  >> %LOG_FILE%
time /T >> %LOG_FILE%
echo ------- >> %LOG_FILE%

cd ..

echo ---  start create DB   ----  >> %LOG_FILE%
echo  start create DB 
cd PxSQLDB
call .\CreatePXDcmDB.bat >> %LOG_FILE%
cd ..

rem moved to installShieldle 2018/08/02
rem ==========================================
rem echo   ---  install service   ----  >> %LOG_FILE%
rem echo  install service  
rem cd server
rem call .\installPXDicomSServer.bat >> %LOG_FILE%
rem call .\installPXDcmJobProc.bat >> %LOG_FILE%
rem cd ..
rem echo   ---  setup depend MSSQL   ----  >> %LOG_FILE%

rem sc config PXDcmJobProc depend= MSSQL$SQLEXPRESS
rem sc config PXDcmSServer depend= MSSQL$SQLEXPRESS
rem sc config PXDcmJobProc obj= .\prexion password= prexion

rem ==========================================
echo ---  SetupTaskScheduler  ----  >> %LOG_FILE%
echo --- At first try to delete the old version ---- >> %LOG_FILE%
SchTasks.exe /Delete /TN PxDBBackupTask /F  >> %LOG_FILE%
SchTasks.exe /Delete /TN PxDBShrinkTask /F  >> %LOG_FILE%
SchTasks.exe /Delete /TN PxDBManagerTask /F  >> %LOG_FILE%

echo  SetupTaskScheduler 
cd server
call .\PXSetupTaskScheduler.exe >> %LOG_FILE%
cd ..

rem ==========================================
rem echo ---  Setup MediaPoint ----  >> %LOG_FILE%
rem echo  Setup MediaPoint 
rem .\PXMediaPoint.exe

mkdir C:\PXJPEGGatewayMediaP
mkdir C:\PXJPEGGatewayMediaP\PXDICOM_R

rem ==========================================
echo   ---  start service   ----  >> %LOG_FILE%
echo   ---  start PXDcmSServer  ----  >> %LOG_FILE%
rem ==========================================
rem added 2018/08/02
rem restart 
sc Stop PXDcmJobProc >> %LOG_FILE%
 
sc Stop PXDcmSServer >> %LOG_FILE%
 
sc Start PXDcmSServer >> %LOG_FILE%

echo   ---  start PXDcmJobProc ----  >> %LOG_FILE%
sc Start PXDcmJobProc >> %LOG_FILE%

rem =========================
rem run setupJPEGDateway
rem =========================
echo --- setupJPEGGateway----  >> %LOG_FILE%
cd PxSQLDB
call .\setupJPEGGateway.bat
cd ..

echo =====   install end  ====== >> %LOG_FILE%
echo please check the install Logfile %LOG_FILE%
echo on

rem for installShield custum actions cmd.exe /K
exit