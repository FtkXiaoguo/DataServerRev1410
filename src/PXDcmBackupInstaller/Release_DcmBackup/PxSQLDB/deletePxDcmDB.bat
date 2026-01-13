echo off
rem deletePxDcmDB.bat
rem
rem   MS-DOS script that Delete the PxDcmDB database and
rem	all of the tables that go with it.  
rem
rem    Gang Li (March 2004)
rem
rem
rem    Usage: deletePxDcmDB I_do_want_to_delete_PxDcmDB
rem
rem

rem T.C. Zhao 07.19.2005 Modified the script so it does not take command argument
rem thus allowing the batch file to be double-clicked

rem if  /I "%1"=="" goto Usage
rem if not "%1" == "I_do_want_to_delete_PxDcmDB" goto Usage

rem Delete the PxDcmDB database
echo Dropping database PxDcmDB. 
echo Press Control-C to cancel database deletion ...
pause

osql -b -E -n -w 5000 -Q "DROP database PxDcmDB"
rem osql -b -E -n -w 5000 -Q "DROP database PxDcmDB2"
rem osql -b -E -n -w 5000 -Q "DROP database PxDcmDB3"
osql -b -E -n -w 5000 -Q "DROP database PxDcmHistDB"

echo AQNetDB deleted

rem :USAGE
rem echo.
rem echo This command will delete entire PxDcmDB Database.
rem echo Make sure this is what you want to do!
rem echo.
rem echo Here is the secrete usage
rem echo. 
rem echo Usage: deletePxDcmDB I_do_want_to_delete_PxDcmDB
