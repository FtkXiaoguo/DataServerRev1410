@echo off
rem doCreateBigDB2005.bat
rem
rem
rem
rem    Usage: doCreateBigDB2005
rem
rem

IF %0%1 == %0 goto defDB
set DATA_BASE_NAME=-S %1\SQLEXPRESS
goto START_DB
:defDB
set DATA_BASE_NAME=-S localhost\SQLEXPRESS

:START_DB
set SQL_CMD=sqlcmd

ECHO ### setup DB ... using cmd: %SQL_CMD% %DATA_BASE_NAME% 


rem  detect if there is new database AqNETDB
%SQL_CMD% %DATA_BASE_NAME% -b -d AqNETDB -E -n -w 5000 -Q "SELECT * FROM dbinfo"
rem go to install new AqNETDB or update
goto UPDATE%ERRORLEVEL%

echo Can not find AqNETDB 
GOTO END

:UPDATE0
echo create BigDB ...
echo. 
%SQL_CMD% %DATA_BASE_NAME% -i CreateBigDB.sql


:END 

pause




 

