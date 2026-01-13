set DB_BACKUP_FOLDER=C:\AQNetDBBack
IF EXIST %DB_BACKUP_FOLDER% GOTO START_L
echo ----- create backup folder  %DB_BACKUP_FOLDER% -----
mkdir %DB_BACKUP_FOLDER%
:START_L
echo ----- backup older data -----

rem FOR %%B IN (%DB_BACKUP_FOLDER%\*.dat) DO copy %%B  %%B.bak

echo ----- start backup AqDB ----- >>%DB_BACKUP_FOLDER%\backup.log
date/t >> %DB_BACKUP_FOLDER%\backup.log
time/t >> %DB_BACKUP_FOLDER%\backup.log

rem ========================================
 
set DATA_BASE_NAME=-E -n

 
set SQL_CMD=osql
 
echo To backup AQNetDB ...

echo. 
%SQL_CMD% %DATA_BASE_NAME% -i "C:\Program Files\AQNet\bin\backupAqDB.sql"
 
rem ========================================

echo -----  end backup AqDB ----- >>%DB_BACKUP_FOLDER%\backup.log
echo ==================================== >>%DB_BACKUP_FOLDER%\backup.log