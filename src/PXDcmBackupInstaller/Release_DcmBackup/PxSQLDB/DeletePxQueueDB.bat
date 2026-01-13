echo off
rem DeletePxQueueDB.bat
rem
rem
rem


IF %0%1 == %0 goto defDB
set DATA_BASE_NAME=-S %1\SQLEXPRESS
goto START_DB
:defDB
set DATA_BASE_NAME=-S localhost\SQLEXPRESS

:START_DB
set SQL_CMD=sqlcmd

ECHO ### delete DB using cmd:  %SQL_CMD% %DATA_BASE_NAME%


rem Delete the PxQueueDB database
echo Dropping database PxQueueDB. 
echo Press Control-C to cancel database deletion ...
pause

%SQL_CMD% %DATA_BASE_NAME% -b -E -n -w 5000 -Q "DROP database PxQueueDB"


echo PxQueueDB deleted

 
pause
