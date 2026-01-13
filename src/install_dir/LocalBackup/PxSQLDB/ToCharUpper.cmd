@echo off
rem setlocal
if "%~1"=="" goto :end
set strupper=%~1
for %%i in (A B C D E F G H I J K L M N O P Q R S T U V W X Y Z) do call set strupper=%%strupper:%%i=%%i%%
echo %strupper%
:end
rem endlocal
exit /b