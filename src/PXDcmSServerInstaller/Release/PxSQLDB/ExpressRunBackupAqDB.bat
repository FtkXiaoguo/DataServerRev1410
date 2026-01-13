echo off
  
set DB_BACKUP_FOLDER=C:\AQNetDBBack

rem ========================================

 
set DATA_BASE_NAME=-S localhost\SQLEXPRESS
 
set SQL_CMD=sqlcmd
 

rem ========================================

echo To backup AQNetDB ...

echo. 
%SQL_CMD% %DATA_BASE_NAME% -i backupAqDB.sql >>%DB_BACKUP_FOLDER%\backup.log
 
rem ========================================

time/t >> %DB_BACKUP_FOLDER%\backup.log
echo -----  end backup AqDB ----- >>%DB_BACKUP_FOLDER%\backup.log
echo ==================================== >>%DB_BACKUP_FOLDER%\backup.log
