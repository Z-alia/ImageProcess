@echo off
chcp 65001 >nul
REM ============================================
REM 清理脚本 (ROS2风格)
REM ============================================

echo.
echo ========================================
echo    清理构建文件
echo ========================================
echo.

cd /d "%~dp0"

REM 删除 build 目录
if exist "build" (
    echo [清理] 删除 build/ 目录...
    rmdir /s /q build
    echo       √ 已删除 build/
) else (
    echo       - build/ 不存在
)

REM 删除 install 目录
if exist "install" (
    echo [清理] 删除 install/ 目录...
    rmdir /s /q install
    echo       √ 已删除 install/
) else (
    echo       - install/ 不存在
)

REM 删除 log 目录（如果有）
if exist "log" (
    echo [清理] 删除 log/ 目录...
    rmdir /s /q log
    echo       √ 已删除 log/
)

echo.
echo ========================================
echo    清理完成！
echo ========================================
pause
