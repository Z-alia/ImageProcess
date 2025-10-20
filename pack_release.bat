@echo off
setlocal
REM 打包 Windows 绿色版运行环境到 release 目录
set "SRC_DIR=%~dp0build\bin"
set "DST_DIR=%~dp0release"
set "MSYS_DLL_DIR=C:\msys64\mingw64\bin"
set "MSYS_SHARE_DIR=C:\msys64\mingw64\share"

if not exist "%SRC_DIR%\imageprocessor.exe" (
    echo [error] 请先编译生成 build\bin\imageprocessor.exe
    exit /b 1
)

REM 创建目标目录
if exist "%DST_DIR%" rmdir /s /q "%DST_DIR%"
mkdir "%DST_DIR%"

REM 复制主程序
copy "%SRC_DIR%\imageprocessor.exe" "%DST_DIR%\"

REM 自动收集依赖 DLL（核心 GTK/GLib/PNG/JPEG 及其依赖）
for %%F in (
    libgtk-3-0.dll libgdk-3-0.dll libatk-1.0-0.dll libcairo-2.dll libgobject-2.0-0.dll libglib-2.0-0.dll libgio-2.0-0.dll libgmodule-2.0-0.dll libgthread-2.0-0.dll libpng16-16.dll libjpeg-8.dll libpango-1.0-0.dll libpangocairo-1.0-0.dll libpangoft2-1.0-0.dll libfontconfig-1.dll libfreetype-6.dll libintl-8.dll libiconv-2.dll zlib1.dll libexpat-1.dll libharfbuzz-0.dll libepoxy-0.dll libpixman-1-0.dll libwinpthread-1.dll libgcc_s_seh-1.dll libstdc++-6.dll
) do (
    if exist "%MSYS_DLL_DIR%\%%F" copy "%MSYS_DLL_DIR%\%%F" "%DST_DIR%\"
)

REM 复制 GTK 运行时资源
xcopy /e /y /i "%MSYS_SHARE_DIR%\themes" "%DST_DIR%\share\themes"
xcopy /e /y /i "%MSYS_SHARE_DIR%\icons" "%DST_DIR%\share\icons"
xcopy /e /y /i "%MSYS_SHARE_DIR%\glib-2.0" "%DST_DIR%\share\glib-2.0"

REM 生成启动脚本
>">%DST_DIR%\run_imageprocessor.bat" echo @echo off
>>"%DST_DIR%\run_imageprocessor.bat" echo setlocal
>>"%DST_DIR%\run_imageprocessor.bat" echo set "PATH=%%~dp0;%%PATH%%"
>>"%DST_DIR%\run_imageprocessor.bat" echo set "XDG_DATA_DIRS=%%~dp0share;%%XDG_DATA_DIRS%%"
>>"%DST_DIR%\run_imageprocessor.bat" echo set "GSETTINGS_SCHEMA_DIR=%%~dp0share\glib-2.0\schemas"
>>"%DST_DIR%\run_imageprocessor.bat" echo start "ImageProcessor" imageprocessor.exe
>>"%DST_DIR%\run_imageprocessor.bat" echo endlocal

endlocal
