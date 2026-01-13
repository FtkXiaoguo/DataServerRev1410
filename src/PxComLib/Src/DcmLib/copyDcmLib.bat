
set COPY_CMD=copy /y
set COPY_SRC_FOLDER="E:\app\FXViewerProj\FxComLib\Src\DcmLib"
set COPY_DEST_FOLDER="F:\NewViewerProj\Apps\MiniPACS\Src\C++\Common\DcmXT\include"

rem copy *.h
%COPY_CMD% %COPY_SRC_FOLDER%\include\*.h %COPY_DEST_FOLDER%

rem copy *.lib *.dll
set COPY_DEST_FOLDER="F:\NewViewerProj\Apps\MiniPACS\Src\AllLibsDebug"
%COPY_CMD% %COPY_SRC_FOLDER%\lib\debug\*.lib  %COPY_DEST_FOLDER%
%COPY_CMD% %COPY_SRC_FOLDER%\bin\debug\*.dll  %COPY_DEST_FOLDER%

pause