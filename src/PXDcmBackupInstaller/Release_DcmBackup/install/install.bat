echo off

rem ==========================================
rem setup install LOG
rem  
set LOG_FOLDER=C:\PxDcmSetupLog
if not  EXIST %LOG_FOLDER%  mkdir %LOG_FOLDER%
set LOG_FILE=%LOG_FOLDER%\PXDataSServerInstall.log
rem 
rem ==========================================

echo ===== start install ====== >> %LOG_FILE%
echo ===== start install ====== 

date /T  >> %LOG_FILE%
time /T >> %LOG_FILE%
echo ------- >> %LOG_FILE%

cd ..

echo ---  start create DB   ----  >> %LOG_FILE%
echo  start create DB 
cd PxSQLDB
call .\CreatePXDcmDB.bat >> %LOG_FILE%
cd ..

rem ==========================================
echo   ---  install service   ----  >> %LOG_FILE%
echo  install service  
cd server
call .\installPXDicomSServer.bat >> %LOG_FILE%
call .\installPXDcmJobProc.bat >> %LOG_FILE%
cd ..
echo   ---  setup depend MSSQL   ----  >> %LOG_FILE%
sc config PXDcmJobProc depend= MSSQL$SQLEXPRESS
sc config PXDcmSServer depend= MSSQL$SQLEXPRESS
sc config PXDcmJobProc obj= .\prexion password= prexion

rem ==========================================
rem echo ---  SetupTaskScheduler  ----  >> %LOG_FILE%
rem echo --- At first try to delete the old version ---- >> %LOG_FILE%
rem SchTasks.exe /Delete /TN PxDBBackupTask /F  >> %LOG_FILE%
rem SchTasks.exe /Delete /TN PxDBShrinkTask /F  >> %LOG_FILE%
rem SchTasks.exe /Delete /TN PxDBManagerTask /F  >> %LOG_FILE%

rem echo  SetupTaskScheduler 
rem cd server
rem call .\PXSetupTaskScheduler.exe >> %LOG_FILE%
rem cd ..

rem ==========================================
rem echo ---  Setup MediaPoint ----  >> %LOG_FILE%
rem echo  Setup MediaPoint 
rem .\PXMediaPoint.exe

mkdir F:\PXDcmBackupMediaP
mkdir F:\PXDcmBackupMediaP\PXDICOM_R

rem ==========================================
echo   ---  start service   ----  >> %LOG_FILE%
echo   ---  start PXDcmSServer  ----  >> %LOG_FILE%
sc Start PXDcmSServer >> %LOG_FILE%

echo   ---  start PXDcmJobProc ----  >> %LOG_FILE%
sc Start PXDcmJobProc >> %LOG_FILE%

rem =========================
rem run setupJPEGDateway
rem =========================
echo --- setupDcmBackup ----  >> %LOG_FILE%
cd PxSQLDB
call .\setupDcmBackup.bat
cd ..

echo =====   install end  ====== >> %LOG_FILE%
echo pleas check the install Logfile %LOG_FILE%
echo on