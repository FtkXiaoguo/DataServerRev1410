@echo off
rem doCreateBigDB.bat
rem
rem
rem
rem    Usage: doCreateBigDB
rem
rem


 
rem  detect if there is new database AqNETDB
osql -b -d AqNETDB -E -n -w 5000 -Q "SELECT * FROM dbinfo"
rem go to install new AqNETDB or update
goto UPDATE%ERRORLEVEL%

echo Can not find AqNETDB 
GOTO END

:UPDATE0
echo create BigDB ...

echo. 
osql -E -n -i CreateBigDB.sql
:END 

pause
