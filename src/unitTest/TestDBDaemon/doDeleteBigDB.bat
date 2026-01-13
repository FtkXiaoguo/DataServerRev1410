@echo off
rem doDeleteBigDB.bat
rem
rem
rem
rem    Usage: doDeleteBigDB
rem
rem


 
rem  detect if there is new database AqNETDB
osql -b -d AqNETDB -E -n -w 5000 -Q "SELECT * FROM dbinfo"
goto UPDATE%ERRORLEVEL%

echo Can not find AqNETDB 
GOTO END

:UPDATE0
echo delete BigDB ...

echo. 
osql -E -n -i deleteBigDB.sql
:END 

pause
