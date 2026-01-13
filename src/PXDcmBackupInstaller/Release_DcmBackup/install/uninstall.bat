echo off

rem ==========================================
rem setup uninstall LOG
rem  
set LOG_FOLDER=C:\PxDcmSetupLog
if not  EXIST %LOG_FOLDER%  mkdir %LOG_FOLDER%
set LOG_FILE=%LOG_FOLDER%\PXDataSServerUnInstall.log
rem 
rem ==========================================

echo ===== start uninstall ====== >> %LOG_FILE%
echo ===== start uninstall ====== 

date /T  >> %LOG_FILE%
time /T >> %LOG_FILE%
echo ------- >> %LOG_FILE%

 cd ..

rem ==========================================
echo   ---  stop services   ----  >> %LOG_FILE%
echo    stop services    

echo   ---  stop PXDcmSServer  ----  >> %LOG_FILE%
sc Stop PXDcmSServer >> %LOG_FILE%
rem -----
echo   ---  delete PXDcmSServer  ----  >> %LOG_FILE%
sc Delete PXDcmSServer >> %LOG_FILE%

echo   ---  stop PXDcmJobProc ----  >> %LOG_FILE%
sc Stop PXDcmJobProc >> %LOG_FILE%
rem -----
echo   ---  delete PXDcmJobProc ----  >> %LOG_FILE%
sc Delete PXDcmJobProc >> %LOG_FILE%
 
echo --- delete schedular task --- >> %LOG_FILE%
SchTasks.exe /Delete /TN PxDBBackupTask /F  >> %LOG_FILE%
SchTasks.exe /Delete /TN PxDBShrinkTask /F  >> %LOG_FILE%
SchTasks.exe /Delete /TN PxDBManagerTask /F  >> %LOG_FILE%

echo =====   uninstall end  ====== >> %LOG_FILE%
echo pleas check the install Logfile %LOG_FILE%
echo on