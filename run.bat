@echo off
REM 运行图像处理程序

echo =====================================
echo   启动图像处理器
echo =====================================
echo.

if exist "build\bin\imageprocessor.exe" (
    echo 找到程序: build\bin\imageprocessor.exe
    echo 正在启动...
    echo.
    start "" "build\bin\imageprocessor.exe"
    echo 程序已启动！
) else (
    echo 错误: 未找到可执行文件
    echo 请先运行 build.bat 编译程序
    pause
)
