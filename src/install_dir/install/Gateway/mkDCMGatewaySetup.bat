echo Release
rem ------------------------------------------------
rem Common enviroment            -------------------
rem ------------------------------------------------

set Path=C:\Program Files\NSIS;%Path%

echo ################################################
echo Make Installer for new install
echo ################################################
del PXDICOMGateway_setup.exe
makensis.exe /DPACKAGE_DICOMGATEWAY="1" DCMGatewayInstaller > mkDCMGatewaySetup.log

echo ################################################
echo Make Installer for upgrade
echo ################################################
del PXDICOMGateway_upgrade.exe
makensis.exe /DPACKAGE_DICOMGATEWAY="1" /DUPGRADE="1" DCMGatewayInstaller >> mkDCMGatewaySetup.log

