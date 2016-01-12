@echo off

REM Added _UNICODE and UNICODE so I can use Unicode strings in structs and whatnot.
REM Need user32.lib to link MessageBox(), which es used on branch DAY001
REM Need gdi32.lib to link PatBlt(), which es used on branch DAY002
REM 2015.01.25 (Day004) I added /FC to get full path names in diagnostics. It's
REM helpful when using Emacs to code and launch the debugger and executable

REM /GS- turn off security checks because that compile-time option relies on
REM the C runtime library, which we are not using.

REM /Gs[size] The number of bytes that local variables can occupy before
REM a stack probe is initiated. If the /Gs option is specified without a
REM size argument, it is the same as specifying /Gs0

REM /Gm- disable minimal rebuild. We want to build everything. It won't
REM take long.

REM /GR- disable C++ RTTI. We don't need runtime type information.

REM /EHsc enable C++ EH (no SEH exceptions) (/EHs),
REM and  extern "C" defaults to nothrow (/EHc)

REM /EHa- disable C++ Exception Handling, so we don't have stack unwind code.
REM Casey says we don't need it.


REM /W3 set warning level 3.
REM /W4 set warning level 4. It's better
REM /WX warnings are errors
REM /wd turns off a particular warning
REM   /wd4201 - nonstandard extension used : nameless struct/union
REM   /wd4100 - 'identifier' : unreferenced formal parameter (this happens a lot while developing code)
REM   /wd4189 - 'identifier' : local variable is initialized but not referenced
REM   /wd4127 - conditional expression is constant ("do {...} while (0)" in macros)

REM /FC use full pathnames in diagnostics

REM /Od - disable optimizations. The debug mode is good for development

REM /Oi Generate intrinsic functions. Replaces some function calls with
REM intrinsic or otherwise special forms of the function that help your
REM application run faster.

REM /GL whole program optimization. Use the /LTCG linker option to create the
REM output file. /ZI cannot be used with /GL.

REM /I<dir> add to include search path

REM /Fe:<file> name executable file

REM /D<name>{=|#}<text> define macro

REM /Zi enable debugging information
REM /Z7 enable debugging information

REM /link [linker options and libraries] The linker options are
REM documented here: https://msdn.microsoft.com/en-us/library/y0zzbyt4.aspx

REM /nodefaultlib t

REM Note that subsystem version number 5.1 only works with 32-bit builds.
REM The minimum subsystem version number for 64-bit buils is 5.2.
REM /subsystem:windows,5.1 - enable compatibility with Windows XP (5.1)

REM /LTCG link time code generation

REM /STACK:reserve[,commit] stack allocations. The /STACK option sets the size
REM of the stack in bytes. Use this option only when you build an .exe file.

REM DEFINITIONS
REM   _UNICODE - 16-bit wide characters
REM   UNICODE  - 16-bit wide characters
REM   HANDMADE_INTERNAL - 0 = build for public release, 1 = build for developers only
REM   HANDMADE_SLOW - 0 = No slow code (like assertion checks) allowed!, 1 = Slow code welcome
REM   __ISO_C_VISIBLE - the version of C we are targeting for the math library.
REM                     1995 = C95, 1999 = C99, 2011 = C11.

REM BUILD PROPERTIES
REM It's possible to set build properties from the command line using the /p:<Property>=<value>
REM command-line option. For example, to set TargetPlatformVersion to 10.0.10240.0, you would
REM add "/p:TargetPlatformVersion=10.0.10240.0" and possibly
REM "/p:WindowsTargetPlatformVersion=10.0.10240.0". Note that the TargetPlatformVersion setting
REM is optional and allows you to specify the kit version to build with. The default is to use
REM the latest kit.

REM Building Software Using the Universal CRT (VS2015)
REM Use the UniversalCRT_IncludePath property to find the Universal CRT SDK header files.
REM Use one of the following properties to find the linker/library files:
REM    UniversalCRT_LibraryPath_x86
REM    UniversalCRT_LibraryPath_x64
REM    UniversalCRT_LibraryPath_arm

REM Common compiler flags
set CommonCompilerFlags=/nologo /Zc:wchar_t /Zc:forScope /Zc:inline /Gd /Gm- /GR- /EHa- /EHsc /Oi /WX /W4 /wd4201 /wd4100 ^
    /wd4189 /wd4127 /wd4505 /FC /D _UNICODE /D UNICODE
    
REM Debug and optimized compiler flags
set CommonCompilerFlagsDEBUG=/MTd  /Zi /Od %CommonCompilerFlags%
set CommonCompilerFlagsOPTIMIZE=/MT /Zo /O2 /Oi /favor:INTEL64 %CommonCompilerFlags%

REM Choose either Debug or Optimized Compiler Flags
set CommonCompilerFlagsFinal=%CommonCompilerFlagsDEBUG%
REM set CommonCompilerFlagsFinal=%CommonCompilerFlagsOPTIMIZE%


REM Common linker flags
REM set CommonLinkerFlags=/incremental:no /opt:ref user32.lib gdi32.lib winmm.lib
set CommonLinkerFlags=/incremental:no /MANIFESTUAC
set CommonLinkerFlagsX64=/MACHINE:X64 %CommonLinkerFlags%
set CommonLinkerFlagsX86=/MACHINE:X86 %CommonLinkerFlags%

REM Choose 32-bit or 64-bit build
REM set CommonLinkerFlagsFinal=%CommonLinkerFlagsX86%
set CommonLinkerFlagsFinal=%CommonLinkerFlagsX64%


REM Visual Studio Librarian Options
set CommonLibrarianFlags=/LTCG /NOLOGO


REM It seems that the minimum subsystem is 5.02 for 64-bit Windows XP. Both "/subsystem:windows,5.1" and /subsystem:windows,5.01"
REM failed with linker warning "LNK4010: invalid subsystem version number 5.1"
REM 32-bit build
REM cl %CommonCompilerFlags% "%PROJECT_PATH%\src\win32_all.cpp" /link /subsystem:windows,5.02 %CommonLinkerFlagsFinal%

REM 64-bit build
REM set datetime=%date:~-4,4%%date:~-10,2%%date:~-7,2%_%time:~0,2%%time:~3,2%%time:~6,2%
REM set datetime=%datetime: =0%
REM Optimization switches /O2 /Oi /fp:fast


REM PROJECT_PATH must exist. It is set in shell.bat and shell32.bat, so run one
REM of them first. BUILD_PATH should have been set in those script files, too.
if NOT DEFINED PROJECT_PATH GOTO :ERR_PATH
goto :EOF

:ERR_PATH
echo "ERROR: The project path (PROJECT_PATH) is not defined."
