@echo off
rem keflings-nx: the PGO + LTO Switch builds (docs/banjo-nx/pgo-lto.md), each in a folder of its own.
rem   tools\build_switch_pgo.bat generate   instrumented NRO in app\out\switch-pgo-gen (streams counts to pgo/ next to
rem                                         the NRO; merge them with tools\pgo_merge.py into pgo\ at the root)
rem   tools\build_switch_pgo.bat use        release NRO in app\out\switch-pgo-use: -DBANJO_PGO=use -DBANJO_SWITCH_LTO=ON
rem Never run two of these (or any other Switch build) at once: they share sdk\out\switch-arm64.
set "VSROOT=C:\Program Files (x86)\Microsoft Visual Studio\18\BuildTools"
set "PATH=C:\Python314;%VSROOT%\Common7\IDE\CommonExtensions\Microsoft\CMake\CMake\bin;%VSROOT%\Common7\IDE\CommonExtensions\Microsoft\CMake\Ninja;%PATH%"
set "DEVKITPRO=C:/devkitPro"
set "ROOT=%~dp0.."
if "%NVK_SDK%"=="" set "NVK_SDK=C:/devkitPro/mesa-switch-main-nfsmw-source/mesa-switch-main/mesa-unified-install/opt/devkitpro/portlibs/switch"
if "%JOBS%"=="" set "JOBS=10"
if /i "%~1"=="generate" (
  set "BUILD_DIR=%ROOT%\app\out\switch-pgo-gen"
  set "PGO_ARGS=-DBANJO_PGO=generate"
) else if /i "%~1"=="use" (
  set "BUILD_DIR=%ROOT%\app\out\switch-pgo-use"
  set "PGO_ARGS=-DBANJO_PGO=use -DBANJO_SWITCH_LTO=ON"
) else (
  echo usage: build_switch_pgo.bat generate^|use
  exit /b 1
)
if not exist "%BUILD_DIR%\CMakeCache.txt" (
  cmake -S "%ROOT%\app" -B "%BUILD_DIR%" -G Ninja -DCMAKE_BUILD_TYPE=Release ^
    "-DCMAKE_TOOLCHAIN_FILE=%ROOT%\tools\switch\cmake\switch-devkitA64.cmake" ^
    "-DREXSDK_DIR=%ROOT%\sdk" "-DREXGLUE_SWITCH_NVK_SDK=%NVK_SDK%" %PGO_ARGS% %PGO_EXTRA% ^
    -DPYTHON_EXECUTABLE=C:/Python314/python.exe -DPython3_EXECUTABLE=C:/Python314/python.exe || exit /b 1
)
cmake --build "%BUILD_DIR%" -j %JOBS% %2 %3
