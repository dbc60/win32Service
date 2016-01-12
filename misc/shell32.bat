@echo off

REM Note that vcvarsall.bat is local to only the shell in which it is executed.
REM Setup the Visual Studio 2015 x86 build environment
call "C:\Program Files (x86)\Microsoft Visual Studio 14.0\VC\vcvarsall.bat"

REM Be able to run anything in the misc directory
set PROJECT_PATH=%~dp0

REM Remove the trailing directory separator
IF %PROJECT_PATH:~-1%==\ SET PROJECT_PATH=%PROJECT_PATH:~0,-1%

REM This script is one level below the project directory, so remove the
REM subdirectory from the path.
FOR /f "delims=" %%F in ("%PROJECT_PATH%") do (
  set PROJECT_PATH=%%~dpF
)

REM Remove the trailing directory separator
IF %PROJECT_PATH:~-1%==\ SET PROJECT_PATH=%PROJECT_PATH:~0,-1%

TITLE Win32 Service Example %Platform%
set BUILD_PATH=%PROJECT_PATH%\build\cmd

if NOT exist "%BUILD_PATH%" mkdir "%BUILD_PATH%"


REM I can call misc\build.bat. There's no need to keep a copy of build.bat in
REM the build\cmd directory.
REM copy /Y "%PROJECT_PATH%\misc\build.bat" "%BUILD_PATH%" > nul

set PATH=%BUILD_PATH%;%PATH%
goto :EOF
