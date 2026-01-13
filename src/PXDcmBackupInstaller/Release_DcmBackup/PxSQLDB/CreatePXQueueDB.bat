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

 

echo ---  start ExpressCreatePxQueueDB   ----  
time /T  
echo %0 : %1, %2  
rem cd %1
 
echo Current Dir: %cd% 
rem ---------------------------------

 
set DATA_BASE_NAME=-S localhost\SQLEXPRESS

:START_DB
set SQL_CMD=sqlcmd

ECHO ### setup DB ... using cmd: %SQL_CMD% %DATA_BASE_NAME% 


rem  detect if there is new database PxQueueDB
%SQL_CMD% %DATA_BASE_NAME% -b -d PxQueueDB -E  -w 5000 -Q "SELECT * FROM dbinfo"
rem go to install new PxQueueDB or update

echo detect PxDcmDB ERRORLEVEL=%ERRORLEVEL%   

goto UPDATE%ERRORLEVEL%

 
:UPDATE1
rem install new PxQueueDB

echo. 
echo install new PxQueueDB
 

%SQL_CMD% %DATA_BASE_NAME% -w 5000 -Q "CREATE DATABASE PxQueueDB"
 
rem wait database create finish
%SQL_CMD% %DATA_BASE_NAME% -w 5000 -Q "waitfor delay '00:00:5'"

echo. 
%SQL_CMD% %DATA_BASE_NAME% -i CreatePxQueueDB.sql


GOTO COMMON

 
:UPDATE0
rem there was a database, do update
echo do update 
echo bypass all patching ! 
 
:COMMON 
 
 
:END1

echo ---  end ExpressCreatePxQueueDB   ----  



 

