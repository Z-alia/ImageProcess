@echo off
REM ============================================
REM 清理构建文件脚本
REM ============================================

echo =====================================
echo   清理构建文件
echo =====================================
echo.

cd /d "%~dp0"

if exist "build" (
    echo 正在删除 build 目录...
    rmdir /s /q build
    echo √ build 目录已删除
) else (
    echo build 目录不存在，无需清理
)

echo.
echo 清理完成！
echo.
pause
