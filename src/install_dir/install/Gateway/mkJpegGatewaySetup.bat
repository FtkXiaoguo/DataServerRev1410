echo Release
rem ------------------------------------------------
rem Common enviroment            -------------------
rem ------------------------------------------------

set Path=C:\Program Files\NSIS;%Path%

echo ################################################
echo Make Installer
echo ################################################
del PXJpegGateway_setup.exe
makensis.exe /DPACKAGE_JPEGGATEWAY="1" DCMGatewayInstaller> mkJpegGatewaySetup.log

echo ################################################
echo Make Installer
echo ################################################
del PXJpegGateway_upgrade.exe
makensis.exe /DPACKAGE_JPEGGATEWAY="1" /DUPGRADE="1" DCMGatewayInstaller >> mkJpegGatewaySetup.log


