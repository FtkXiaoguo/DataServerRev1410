@echo off
rem setlocal
if "%~1"=="" goto :end
set strlower=%~1
for %%i in (a b c d e f g h i j k l m n o p q r s t u v w x y z) do call set strlower=%%strlower:%%i=%%i%%
echo %strlower%
:end
rem endlocal
exit /b