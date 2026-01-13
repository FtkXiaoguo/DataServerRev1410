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

 

echo ---  start ExpressSetupPxQueueDB   ----  
time /T  
echo %0 : %1, %2  
rem cd %1
 
echo Current Dir: %cd% 
rem ---------------------------------

 
set DATA_BASE_NAME=-S localhost\SQLEXPRESS

:START_DB
set SQL_CMD=sqlcmd

 
 
echo. 
%SQL_CMD% %DATA_BASE_NAME% -i PxQueueDBFunc.sql

 
 
:END1

echo ---  end ExpressSetupPxQueueDB   ----  



 

