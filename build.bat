@echo off
REM ============================================
REM 图像处理程序一键编译脚本 (批处理)
REM ============================================

echo =====================================
echo   图像处理程序编译脚本
echo =====================================
echo.

REM 进入脚本所在目录
cd /d "%~dp0"

REM 1. 检查并创建 build 目录
echo [1/4] 检查构建目录...
if not exist "build" (
    echo       创建 build 目录...
    mkdir build
) else (
    echo       build 目录已存在
)
echo       √ 完成
echo.

REM 2. 运行 CMake 配置
echo [2/4] 配置项目 (CMake)...
cd build
REM 使用 MinGW Makefiles 生成器，并启用 GUI 构建
cmake .. -G "MinGW Makefiles" -DCMAKE_BUILD_TYPE=Release -DBUILD_GUI=ON
if errorlevel 1 (
    echo       × CMake 配置失败
    cd ..
    pause
    exit /b 1
)
echo       √ 配置成功
cd ..
echo.

REM 3. 编译项目
echo [3/4] 编译项目...
cmake --build build --config Release -j 4
if errorlevel 1 (
    echo       × 编译失败
    pause
    exit /b 1
)
echo       √ 编译成功
echo.

REM 4. 检查生成的可执行文件
echo [4/4] 检查输出文件...
if exist "build\bin\imageprocessor.exe" (
    echo       √ 找到可执行文件: build\bin\imageprocessor.exe
    set "FOUND=1"
)
if exist "build\imageprocessor.exe" (
    echo       √ 找到可执行文件: build\imageprocessor.exe
    set "FOUND=1"
)
if exist "build\bin\video_processor.exe" (
    echo       √ 找到可执行文件: build\bin\video_processor.exe
    set "FOUND=1"
)
if exist "build\video_processor.exe" (
    echo       √ 找到可执行文件: build\video_processor.exe
    set "FOUND=1"
)

if not defined FOUND (
    echo       ! 警告: 未找到可执行文件，请检查编译输出
)
echo.

REM 完成
echo =====================================
echo   编译完成！
echo =====================================
echo.
echo 提示: 可以运行以下命令启动程序：
echo   build\bin\imageprocessor.exe
echo   或
echo   run_imageprocessor.bat
echo.

REM 询问是否立即运行
set /p RUN="是否立即运行程序? (Y/N): "
if /i "%RUN%"=="Y" (
    if exist "build\bin\imageprocessor.exe" (
        echo 启动程序...
        start "" "build\bin\imageprocessor.exe"
        goto :end
    )
    if exist "build\imageprocessor.exe" (
        echo 启动程序...
        start "" "build\imageprocessor.exe"
        goto :end
    )
    echo 未找到可执行文件
)

:end
pause
