@echo off
rem AqNETDBUpdateUnicode.bat
rem
rem
rem
rem =========================================
rem == setup sqlcmd ==
set WorkFolder=%~dp0
call "%WorkFolder%getSQLCmd.bat" sql_cmd_line
rem =========================================
set SQL_CMD=%sql_cmd_line%
set DATA_BASE_NAME=-S localhost\SQLEXPRESS

%SQL_CMD% %DATA_BASE_NAME% -i PxDBUnicode.sql

%SQL_CMD% %DATA_BASE_NAME% -i PxDcmDBFunc.sql





 

