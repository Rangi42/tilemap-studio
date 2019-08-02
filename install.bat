@echo off

net session >nul 2>&1
if not %errorLevel% == 0 (
	echo Administrator privileges required.
	echo Right-click and select "Run as administrator".
	pause
	exit /b
)

cd /d %~dp0

set "INSTALLDIR=%APPDATA%\Rangi\Tilemap Studio"
if exist bin\Release\tilemapstudio.exe (
	set "SOURCEFILE=bin\Release\tilemapstudio.exe"
) else (
	set "SOURCEFILE=tilemapstudio.exe"
)
set "INSTALLPATH=%INSTALLDIR%\tilemapstudio.exe"

if not exist %SOURCEFILE% (
	echo tilemapstudio.exe does not exist.
	echo Download it to the same location as install.bat and retry.
	pause
	exit /b
)

mkdir "%INSTALLDIR%" >nul 2>&1
copy /b /y %SOURCEFILE% "%INSTALLPATH%" >nul

reg add "HKCR\.tilemap" /ve /f /d "Tilemap Studio" >nul
reg add "HKCR\.rle" /ve /f /d "Tilemap Studio" >nul
reg add "HKCR\Tilemap Studio" /ve /f /d "Tilemap File" >nul
rem reg add "HKCR\Tilemap Studio\DefaultIcon" /ve /f /d "%%SystemRoot%%\system32\imageres.dll,127" >nul
reg add "HKCR\Tilemap Studio\shell\open\command" /ve /f /d "\"%INSTALLPATH%\" \"%%1\"" >nul

set LNKSCRIPT="%TEMP%\tilemapstudio-lnk-%RANDOM%.vbs"
echo Set oWS = WScript.CreateObject("WScript.Shell") >> %LNKSCRIPT%
echo sLinkFile = "%USERPROFILE%\Desktop\Tilemap Studio.lnk" >> %LNKSCRIPT%
echo Set oLink = oWS.CreateShortcut(sLinkFile) >> %LNKSCRIPT%
echo oLink.TargetPath = "%INSTALLPATH%" >> %LNKSCRIPT%
echo oLink.Save >> %LNKSCRIPT%
cscript /nologo %LNKSCRIPT%
del %LNKSCRIPT%

echo Installed Tilemap Studio to:
echo %INSTALLPATH%
echo Shortcut created on Desktop.
echo You may now delete the downloaded copy of tilemapstudio.exe.
pause
