@echo off
rem keflings-nx: runs the code generator on app\renut_manifest.toml (app\assets: default.xex + default.xexp).
rem Usage: tools\codegen.bat <log name>   (log in logs\<name>.log)
rem Run it with C:\Windows\System32\cmd.exe: on this PC "cmd" on PATH is devkitPro's MSYS2 shim.
set "PATH=C:\Program Files (x86)\Microsoft Visual Studio\Installer;%PATH%"
call "%~dp0pc_env.bat" || exit /b 1
set "ROOT=%~dp0.."
set "LOG=%ROOT%\logs\%~1.log"
if "%~1"=="" set "LOG=%ROOT%\logs\codegen.log"
cd /d "%ROOT%\app"
"%ROOT%\sdk\out\win-amd64\rexglue.exe" codegen renut_manifest.toml > "%LOG%" 2>&1
echo exit %ERRORLEVEL% >> "%LOG%"
