# ============================================
# 图像处理程序一键编译脚本 (PowerShell)
# ============================================

Write-Host "=====================================" -ForegroundColor Cyan
Write-Host "  图像处理程序编译脚本" -ForegroundColor Cyan
Write-Host "=====================================" -ForegroundColor Cyan
Write-Host ""

# 设置错误处理
$ErrorActionPreference = "Stop"

# 获取脚本所在目录
$ScriptDir = Split-Path -Parent $MyInvocation.MyCommand.Path
Set-Location $ScriptDir

# 1. 检查并创建 build 目录
Write-Host "[1/4] 检查构建目录..." -ForegroundColor Yellow
if (-Not (Test-Path "build")) {
    Write-Host "      创建 build 目录..." -ForegroundColor Gray
    New-Item -ItemType Directory -Path "build" | Out-Null
} else {
    Write-Host "      build 目录已存在" -ForegroundColor Gray
}
Write-Host "      ✓ 完成" -ForegroundColor Green
Write-Host ""

# 2. 运行 CMake 配置
Write-Host "[2/4] 配置项目 (CMake)..." -ForegroundColor Yellow
try {
    Push-Location build
    # 尝试使用 MinGW Makefiles 生成器，并启用 GUI 构建
    Write-Host "      使用 MinGW Makefiles 生成器..." -ForegroundColor Gray
    cmake .. -G "MinGW Makefiles" -DCMAKE_BUILD_TYPE=Release -DBUILD_GUI=ON 2>&1 | Out-Null
    
    if ($LASTEXITCODE -ne 0) {
        Write-Host "      MinGW 配置失败，尝试默认生成器..." -ForegroundColor Yellow
        cmake .. -DCMAKE_BUILD_TYPE=Release -DBUILD_GUI=ON 2>&1 | Out-Null
        if ($LASTEXITCODE -ne 0) {
            Write-Host "      尝试不使用 GUI 模式（仅构建静态库）..." -ForegroundColor Yellow
            cmake .. -G "MinGW Makefiles" -DCMAKE_BUILD_TYPE=Release -DBUILD_GUI=OFF 2>&1 | Out-Null
            if ($LASTEXITCODE -ne 0) {
                throw "CMake 配置失败"
            } else {
                Write-Host "      ⚠ 警告：GUI 依赖缺失，仅构建核心库" -ForegroundColor Yellow
            }
        }
    }
    Write-Host "      ✓ 配置成功" -ForegroundColor Green
} catch {
    Write-Host "      ✗ CMake 配置失败: $_" -ForegroundColor Red
    Write-Host ""
    Write-Host "可能的原因：" -ForegroundColor Yellow
    Write-Host "  1. 未安装 MSYS2/MinGW 环境" -ForegroundColor White
    Write-Host "  2. 未安装 GTK3 开发库 (mingw-w64-x86_64-gtk3)" -ForegroundColor White
    Write-Host "  3. 未安装 pkg-config (mingw-w64-x86_64-pkgconf)" -ForegroundColor White
    Write-Host ""
    Write-Host "安装方法（在 MSYS2 终端中）：" -ForegroundColor Cyan
    Write-Host "  pacman -S mingw-w64-x86_64-gtk3" -ForegroundColor Green
    Write-Host "  pacman -S mingw-w64-x86_64-pkgconf" -ForegroundColor Green
    Write-Host "  pacman -S mingw-w64-x86_64-libpng" -ForegroundColor Green
    Write-Host "  pacman -S mingw-w64-x86_64-libjpeg-turbo" -ForegroundColor Green
    Write-Host ""
    Pop-Location
    Read-Host "按 Enter 键退出"
    exit 1
} finally {
    Pop-Location
}
Write-Host ""

# 3. 编译项目
Write-Host "[3/4] 编译项目..." -ForegroundColor Yellow
try {
    cmake --build build --config Release -j 4
    if ($LASTEXITCODE -ne 0) {
        throw "编译失败"
    }
    Write-Host "      ✓ 编译成功" -ForegroundColor Green
} catch {
    Write-Host "      ✗ 编译失败: $_" -ForegroundColor Red
    exit 1
}
Write-Host ""

# 4. 检查生成的可执行文件
Write-Host "[4/4] 检查输出文件..." -ForegroundColor Yellow
$ExeFiles = @(
    "build\bin\imageprocessor.exe",
    "build\bin\video_processor.exe",
    "build\imageprocessor.exe",
    "build\video_processor.exe"
)

$FoundExe = $false
foreach ($ExePath in $ExeFiles) {
    if (Test-Path $ExePath) {
        $FullPath = Resolve-Path $ExePath
        Write-Host "      ✓ 找到可执行文件: $FullPath" -ForegroundColor Green
        $FoundExe = $true
    }
}

if (-Not $FoundExe) {
    Write-Host "      ⚠ 警告: 未找到可执行文件，请检查编译输出" -ForegroundColor Yellow
}
Write-Host ""

# 完成
Write-Host "=====================================" -ForegroundColor Cyan
Write-Host "  编译完成！" -ForegroundColor Cyan
Write-Host "=====================================" -ForegroundColor Cyan
Write-Host ""
Write-Host "提示: 可以运行以下命令启动程序：" -ForegroundColor White
Write-Host "  .\build\bin\imageprocessor.exe" -ForegroundColor Cyan
Write-Host "  或" -ForegroundColor White
Write-Host "  .\run_imageprocessor.bat" -ForegroundColor Cyan
Write-Host ""

# 询问是否立即运行
$Run = Read-Host "是否立即运行程序? (Y/N)"
if ($Run -eq "Y" -or $Run -eq "y") {
    foreach ($ExePath in $ExeFiles) {
        if (Test-Path $ExePath) {
            Write-Host "启动程序..." -ForegroundColor Green
            & $ExePath
            break
        }
    }
}
