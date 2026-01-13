@echo off
rem PxDcmDBSetup.bat
rem
rem   MS-DOS script that creates and empty PxDcmDB database and
rem     all of the tables that go with it.  
rem
rem    Gang Li (July 2003)
rem
rem
rem    Usage: PxDcmDBSetup
rem
rem
rem =========================================
rem == setup sqlcmd ==
set WorkFolder=%~dp0
call %WorkFolder%getSQLCmd.bat sql_cmd_line
rem =========================================
echo %sql_cmd_line%

echo ---  start ExpressCreatePxDcmDB   ----  
time /T  
echo %0 : %1, %2  
rem cd %1
 
echo Current Dir: %cd% 
rem ---------------------------------

SET DB_FOLDER=F:\PxDcmBackupDB

IF NOT EXIST %DB_FOLDER%  (
  echo mkdir %DB_FOLDER%
  mkdir %DB_FOLDER%
)
 
set DATA_BASE_NAME=-S localhost\SQLEXPRESS

:START_DB
set SQL_CMD=%sql_cmd_line%

ECHO ### setup DB ... using cmd: %SQL_CMD% %DATA_BASE_NAME% 


rem  detect if there is new database PxDcmDB
%SQL_CMD% %DATA_BASE_NAME% -b -d PxDcmDB -E  -w 5000 -Q "SELECT * FROM dbinfo"
rem go to install new PxDcmDB or update

echo detect PxDcmDB ERRORLEVEL=%ERRORLEVEL%   

goto UPDATE%ERRORLEVEL%

 
:UPDATE1
rem install new PxDcmDB

echo. 
echo install new PxDcmDB
 

%SQL_CMD% %DATA_BASE_NAME% -w 5000 -Q "CREATE DATABASE PxDcmDB ON ( NAME=PxDcmDB, FILENAME = '%DB_FOLDER%\PxDcmDB.mdf') LOG ON ( NAME=PxDcmDB_log, FILENAME = '%DB_FOLDER%\PxDcmDB_log.ldf') "
%SQL_CMD% %DATA_BASE_NAME% -w 5000 -Q "CREATE DATABASE PxDcmHistDB ON ( NAME=PxDcmHistDB, FILENAME = '%DB_FOLDER%\PxDcmHistDB.mdf') LOG ON ( NAME=PxDcmHistDB_log, FILENAME = '%DB_FOLDER%\PxDcmHistDB_log.ldf') "

rem wait database create finish
%SQL_CMD% %DATA_BASE_NAME% -w 5000 -Q "waitfor delay '00:00:5'"

echo. 
%SQL_CMD% %DATA_BASE_NAME% -i CreatePxDcmDB.sql


GOTO COMMON

 
:UPDATE0
rem there was a database, do update
echo do update 
echo bypass all patching ! 
 
:COMMON 
 
 
echo. 
%SQL_CMD% %DATA_BASE_NAME% -i PxDcmDBFunc.sql

goto END%ERRORLEVEL%

:END0

rem Limit SQL Server's memory usage to not exceed 128 MB
echo. 
%SQL_CMD% %DATA_BASE_NAME% -Q "sp_configure 'show advanced option', '1'"
rem osql -E  -Q "sp_configure reconfigure with override"
%SQL_CMD% %DATA_BASE_NAME% -Q "sp_configure 'min server memory', '32'"
%SQL_CMD% %DATA_BASE_NAME% -Q "sp_configure 'max server memory', '256'"
%SQL_CMD% %DATA_BASE_NAME% -Q "sp_configure reconfigure with override"

:END1

echo ---  end ExpressCreatePxDcmDB   ----  

exit /b

 

