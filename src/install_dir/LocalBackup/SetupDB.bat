echo off

rem ==========================================
rem setup PXDcmBackup DB 
rem 
echo ===== setup PXDcmBackup DB  ======

set WorkFolder=%~dp0

echo WorkFolder %WorkFolder%
cd %WorkFolder%

echo changed folder to %CD%

rem ==========================================
set LOG_FOLDER=C:\PxDcmSetupLog
if not  EXIST %LOG_FOLDER%  mkdir %LOG_FOLDER%
set LOG_FILE=%LOG_FOLDER%\PXSetupPXDcmBackupDB.log

cd ..

echo ---  start create DB   ----  >> %LOG_FILE%
echo  start create DB 
cd PxSQLDB
call .\CreatePXDcmDB2.bat >> %LOG_FILE%

echo ---  Setup AE
echo ---  Setup AE   ----  >> %LOG_FILE%
call .\setupDcmBackup.bat >> %LOG_FILE%

cd ..


echo =====   start create DB end  ====== >> %LOG_FILE%
echo .
echo ===== setup PXDcmBackup DB is finished ======
echo pleas check the install Logfile %LOG_FILE%
echo on

pause