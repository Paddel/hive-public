@echo off
color 0b

@REM Check for Visual Studio
call set "VSPATH="
if exist "%ProgramFiles(x86)%\Microsoft Visual Studio\Installer\vswhere.exe" ( if not defined VSPATH (
	for /f "usebackq tokens=*" %%i in (`"%ProgramFiles(x86)%\Microsoft Visual Studio\Installer\vswhere.exe" -latest -products * -requires Microsoft.Component.MSBuild -property installationPath`) do (
		set VSPATH=%%i
	)
) )
if defined VS140COMNTOOLS ( if not defined VSPATH (
	call set "VSPATH=%%VS140COMNTOOLS%%"
) )
if defined VS120COMNTOOLS ( if not defined VSPATH (
	call set "VSPATH=%%VS120COMNTOOLS%%"
) )
if defined VS110COMNTOOLS ( if not defined VSPATH (
	call set "VSPATH=%%VS110COMNTOOLS%%"
) )
if defined VS100COMNTOOLS ( if not defined VSPATH (
	call set "VSPATH=%%VS100COMNTOOLS%%"
) )
if defined VS90COMNTOOLS ( if not defined VSPATH (
	call set "VSPATH=%%VS90COMNTOOLS%%"
) )
if defined VS80COMNTOOLS ( if not defined VSPATH (
	call set "VSPATH=%%VS80COMNTOOLS%%"
) )

@REM check if we already have the tools in the environment
if exist "%VCINSTALLDIR%" (
	goto compile
)

if not defined VSPATH (
	echo You need Microsoft Visual Studio 8, 9, 10, 11, 12, 13 or 15 installed
	pause
	exit
)

@REM set up the environment
if exist "%VSPATH%vsvars32.bat" (
	call "%VSPATH%vsvars32.bat"
	goto compile
)
if exist "%VSPATH%\VC\Auxiliary\Build\vcvarsall.bat" (
	call "%%VSPATH%%\VC\Auxiliary\Build\vcvarsall.bat" x86
	goto compile
)

echo Unable to set up the environment
pause
exit

:compile
set /p answer=Compiling n/tw/lp/c/d/f/fd:
cls
if "%answer%"=="n" (GOTO normal)
if "%answer%"=="tw" (GOTO tower)
if "%answer%"=="lp" (GOTO laptop)
if "%answer%"=="c" (GOTO clear)
if "%answer%"=="d" (GOTO debug)
if "%answer%"=="f" (GOTO file)
if "%answer%"=="fd" (GOTO filedebug)
goto normal

:normal
@echo Mode: Normal
@call bam.exe config="release" dummy="1" -j 8
goto compile

:tower
@echo Mode: Tower
@call bam.exe config="release" device="tower" nohive="1" -j 8
goto compile

:laptop
@echo Mode: Laptop
@call bam.exe config="release" device="laptop" nohive="1" -j 8
goto compile

:male
@echo Mode: Male
@call bam.exe config="release" device="male" nohive="1" -j 8
goto compile

:clear
@echo Mode: Clear
@call bam.exe dummy="1" -c all -j 8
@call bam.exe config="debug" dummy="1" -c all -j 8
@call bam.exe config="debug" device="tower" dummy="1" -c all -j 8
if exist "compile_out.txt" (del "compile_out.txt")
goto compile

:debug
@echo Mode: Debug
@call bam.exe config="debug" dummy="1" -j 8
goto compile

:file
@echo Mode: File
@call bam.exe config="release" dummy="1" -j 8 > compile_out.txt
goto compile

:filedebug
@echo Mode: FileDebug
@call bam.exe config="debug" dummy="1" -d -j 8 > compile_out.txt
goto compile