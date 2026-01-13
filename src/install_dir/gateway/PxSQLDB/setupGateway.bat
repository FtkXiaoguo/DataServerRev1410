@echo off
 
 

echo ---  start setupGateway   ----  
time /T  
echo %0 : %1, %2  
rem cd %1
 
echo Current Dir: %cd% 
rem ---------------------------------

 
set DATA_BASE_NAME=-S localhost\SQLEXPRESS

:START_DB
set SQL_CMD=sqlcmd

 
 
echo. 
%SQL_CMD% %DATA_BASE_NAME% -i setupGateway.sql

 
 
:END1

echo ---  end setupGateway    ----  



 

