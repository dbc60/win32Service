@echo off
setlocal ENABLEDELAYEDEXPANSION enableextensions

call %PROJECT_PATH%\misc\config.bat
REM @echo on

goto :PROJECTS

:BEGIN_SET_PROJECT
setlocal
set value=U
set project=%1
shift
:BEGIN_PROJECTS_LOOP
if /I "%1" == "%project%" (
  set value=T
  goto :END_PROJECTS_LOOP
)
shift /1
if "%1" == "" goto :END_PROJECTS_LOOP
goto :BEGIN_PROJECTS_LOOP
:END_PROJECTS_LOOP
endlocal&set result=%value%
goto :EOF
:END_SET_PROJECT


:BEGIN_UNSET_PROJECT
setlocal
set value=%2
set project=-%1
shift /3
:BEGIN_UNSET_PROJECTS_LOOP
if /I "%3" == "%project%" (
  set value=F
  goto :END_UNSET_PROJECTS_LOOP
)
shift /3
if "%3" == "" goto :END_UNSET_PROJECTS_LOOP
goto :BEGIN_UNSET_PROJECTS_LOOP
:END_UNSET_PROJECTS_LOOP
endlocal&set result=%value%
goto :EOF
:END_UNSET_PROJECT


:dequote
setlocal
REM The tilde in the next line is the important part
set thestring=%~1%
endlocal&set result=%thestring%
goto :EOF


:PROJECTS
REM A project will build if its variable is set to T (True). It's initialized to U (Undefined), and three-
REM valued logic is used to set it. This way, buildAll can be set on the command line and the project name
REM can be preceeded by a hyphen to prevent the project from being built. For example, "buildALL -unitTest"
REM will build all of the projects except for unitTest.

REM The help, clean, buildAll and verbose projects are pseudo-projects. They don't build a project, but
REM they control this programs output and effect which projects will be built.

call :BEGIN_SET_PROJECT help %*
set help=!result!
call :BEGIN_SET_PROJECT clean %*
set clean=!result!
call :BEGIN_SET_PROJECT buildAll %*
set buildAll=!result!
call :BEGIN_SET_PROJECT verbose %*
set verbose=!result!
call :BEGIN_SET_PROJECT win32PipeClient %*
set win32PipeClient=!result!
call :BEGIN_SET_PROJECT win32Service %*
set win32Service=!result!

call :BEGIN_UNSET_PROJECT clean %clean% %*
set clean=!result!
call :BEGIN_UNSET_PROJECT win32PipeClient %win32PipeClient% %*
set win32PipeClient=!result!
call :BEGIN_UNSET_PROJECT win32Service %win32Service% %*
set win32Service=!result!

:SET_BUILD_ALL
if /I "%buildAll%" == "T" (
  if /I "%clean%" == "U" set clean=T
  if /I "%win32PipeClient%" == "U" set win32PipeClient=T
  if /I "%win32Service%" == "U" set win32Service=T
)


:DISPLAY_PROJECTS
if "%verbose%" == "T" (
  echo clean=%clean%
  echo buildAll=%buildAll%
  echo verbose=%verbose%
  echo win32PipeClient=%win32PipeClient%
  echo win32Service=%win32Service%
  echo.
)


:DISPLAY_HELP
set helpHelp="  help: Display this message."
set cleanHelp="  clean: Clean all projects. Note, if clean follows project names, then the build files from those projects will be deleted!"
set buildAllHelp="  buildAll: Build all of the projects."
set verboseHelp="  verbose: Display the project being built."
set win32PipeClientHelp="  win32PipeClient:"
set win32ServiceHelp="  win32Service:"

if /I "%help%" == "T" (
  echo Valid command-line arguments are:
  call :dequote %helpHelp%
  echo !result!
  call :dequote %cleanHelp%
  echo !result!
  call :dequote %buildAllHelp%
  echo !result!
  call :dequote %verboseHelp%
  echo !result!
  call :dequote %win32PipeClientHelp%
  echo !result!
  call :dequote %win32ServiceHelp%
  echo !result!
  echo.
)

REM goto :CHECK_NO_ARGS


:CHECK_NO_ARGS
REM If there are no arguments, clean all projects and build a couple of them.
if /I "%1" == "" (
  set clean=T
  REM set verbose=T
  set win32Service=T
  set win32PipeClient=T
)

REM Build starts with cleaning out the files generated from the previous build
pushd "%BUILD_PATH%"


:CLEAN
if "%clean%"=="T" (
  if "%verbose%"=="T" echo "Cleaning all projects"
  del *.obj *.pdb *.map *.exe *.res *.lib "%PROJECT_PATH%\src\app_messages.h" > NUL 2> NUL
)


:WIN32_SERVICE
if "%win32Service%"=="T" (
  if "%verbose%"=="T" echo "Building the Win32 service"
  REM Build the message header
  mc -h ..\..\src "%PROJECT_PATH%\src\app_messages.mc"

  REM Build the service along with its built-in control tool that runs from the command line. When run from
  REM the command line, you have the option to  register, unregister, start or stop the service.
  cl %CommonCompilerFlagsFinal%  "%PROJECT_PATH%\src\win32_service.cpp" ^
     /Fmservice.map /Feservice.exe /link %CommonLinkerFlagsFinal%
)


:WIN32_PIPE_CLIENT
if "%win32PipeClient%"=="T" (
  if "%verbose%"=="T" echo "Building the Win32 GUI application"
  REM Build the Win32 GUI app that reports statistics to the user and enables the user to configure the
  REM service. By using both the linker options "/SUBSYSTEM:WINDOWS" and "/ENTRY:mainCRTStartup" I can write
  REM the main function as "int main(int /*argc*/, char* /*argv*/[])" and have a Windows GUI application - no
  REM console. The argc and argv parameters are ignored, but the command-line is still processed by calling
  REM ::GetCommandLineW() and processing the result a little bit. See the definition of
  REM WinMainParameters::GetLPCmdLine() in win32PipeClient.cpp.

  REM Verbose invocation of the resource compiler.
  REM rc /D "_UNICODE" /D "UNICODE" /v /l 0x0409 /nologo /fo"win32PipeClient.res" "%PROJECT_PATH%\src\win32PipeClient.rc"
  REM A more quiet invocation of the resource compiler.
  rc /D "_UNICODE" /D "UNICODE" /l 0x0409 /nologo /fo"win32PipeClient.res" "%PROJECT_PATH%\src\win32PipeClient.rc"

  REM Build the App UI Tool
  cl %CommonCompilerFlagsFinal% "%PROJECT_PATH%\src\win32PipeClient.cpp" "%PROJECT_PATH%\src\win32_ipc_app.cpp" ^
     "%PROJECT_PATH%\src\win32_utility.cpp" ^
     /Fmwin32PipeClient.map /Fewin32PipeClient.exe /link %CommonLinkerFlagsFinal% /ENTRY:mainCRTStartup /SUBSYSTEM:WINDOWS,5.02 ^
     win32PipeClient.res user32.lib gdi32.lib
)


:END
popd
goto :EOF


:ERR_PATH
echo You must run one of shell.bat or shell32.bat to set the build environment.
goto :EOF
