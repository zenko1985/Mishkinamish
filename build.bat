@echo off
cd /d "%~dp0"
call "C:\Program Files (x86)\Intel\oneAPI\setvars.bat" >nul 2>&1
set IPP_BIN=C:\Program Files (x86)\Intel\oneAPI\ipp\2026.0\bin
if not exist ipp_dlls mkdir ipp_dlls
copy /y "%IPP_BIN%\ipps.dll" ipp_dlls\ >nul
copy /y "%IPP_BIN%\ippcore.dll" ipp_dlls\ >nul
copy /y "%IPP_BIN%\ippsd1.dll" ipp_dlls\ >nul
copy /y "%IPP_BIN%\ippsk0.dll" ipp_dlls\ >nul
copy /y "%IPP_BIN%\ippsl9.dll" ipp_dlls\ >nul
copy /y "%IPP_BIN%\ippsy8.dll" ipp_dlls\ >nul
"C:\Program Files\Microsoft Visual Studio\MSBuild\Current\Bin\MSBuild.exe" mishkinamish.vcxproj -p:Configuration=Release -p:Platform=x64 -t:Rebuild
pause
