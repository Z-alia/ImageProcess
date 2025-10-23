@echo off
setlocal

rem ========================================
rem 一键运行脚本 (ROS2风格)
rem ========================================

rem Ensure MSYS2 MinGW64 runtime DLLs are discoverable at execution time
if not defined MSYS2_PATH_TYPE set "MSYS2_PATH_TYPE=inherit"
set "MSYS2_MINGW64_BIN=C:\msys64\mingw64\bin"
if not exist "%MSYS2_MINGW64_BIN%" (
    echo [error] Expected MSYS2 MinGW64 directory "%MSYS2_MINGW64_BIN%" not found.
    echo         Install MSYS2 and the MinGW64 toolchain, or update this script with the correct path.
    exit /b 1
)

set "PATH=%MSYS2_MINGW64_BIN%;%PATH%"
set "XDG_DATA_DIRS=C:\msys64\mingw64\share;%XDG_DATA_DIRS%"
set "GSETTINGS_SCHEMA_DIR=C:\msys64\mingw64\share\glib-2.0\schemas"

set "APP_DIR=%~dp0install\bin"
if not exist "%APP_DIR%\imageprocessor.exe" (
    echo [error] imageprocessor.exe not found in "%APP_DIR%".
    echo         Build the project first using: build.bat
    exit /b 1
)

pushd "%APP_DIR%"
start "ImageProcessor" imageprocessor.exe
popd

endlocal
