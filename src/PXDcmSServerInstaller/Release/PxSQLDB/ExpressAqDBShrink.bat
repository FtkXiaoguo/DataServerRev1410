@echo off
echo "====================="
Date /T
Time /T

IF %0%1 == %0 goto defDB
set DATA_BASE_NAME=-S %1\SQLEXPRESS
goto START_DB
:defDB
set DATA_BASE_NAME=-S localhost\SQLEXPRESS

:START_DB
set SQL_CMD=sqlcmd


%SQL_CMD% %DATA_BASE_NAME% -b -d PxDcmDB -E  -Q "DBCC SHRINKDATABASE (PxDcmDB)"
%SQL_CMD% %DATA_BASE_NAME% -b -d PxDcmHistDB  -E  -Q "DBCC SHRINKDATABASE (PxDcmHistDB)"

 
Time /T