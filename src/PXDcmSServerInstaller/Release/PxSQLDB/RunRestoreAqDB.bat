echo off
 
 
set DATA_BASE_NAME=-E -n

 
set SQL_CMD=osql
 
 
echo To restore AQNetDB ...

echo. 
%SQL_CMD% %DATA_BASE_NAME% -i restoreAqDB.sql
 

pause
