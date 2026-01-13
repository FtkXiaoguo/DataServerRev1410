echo off
 
IF %0%1 == %0 goto defDB
set DATA_BASE_NAME=-S %1\SQLEXPRESS
goto START_DB
:defDB
set DATA_BASE_NAME=-S localhost\SQLEXPRESS

:START_DB
set SQL_CMD=sqlcmd
 
echo To restore AQNetDB ...

echo. 
%SQL_CMD% %DATA_BASE_NAME% -i restoreAqDB.sql
 

pause
