# ============================================
# 编译环境诊断脚本
# ============================================

Write-Host "=========================================" -ForegroundColor Cyan
Write-Host "  编译环境诊断" -ForegroundColor Cyan
Write-Host "=========================================" -ForegroundColor Cyan
Write-Host ""

$allOk = $true

# 检查编译工具
Write-Host ">>> 检查编译工具" -ForegroundColor Yellow
Write-Host ""

$tools = @(
    @{Name="GCC (C 编译器)"; Cmd="gcc"; Args="--version"},
    @{Name="G++ (C++ 编译器)"; Cmd="g++"; Args="--version"},
    @{Name="CMake (构建系统)"; Cmd="cmake"; Args="--version"},
    @{Name="MinGW Make"; Cmd="mingw32-make"; Args="--version"},
    @{Name="pkg-config (包配置)"; Cmd="pkg-config"; Args="--version"}
)

foreach ($tool in $tools) {
    try {
        $output = & $tool.Cmd $tool.Args 2>&1 | Select-Object -First 1
        if ($LASTEXITCODE -eq 0 -or $output) {
            Write-Host "  ✓ $($tool.Name)" -ForegroundColor Green
            Write-Host "    $output" -ForegroundColor Gray
        } else {
            throw "Not found"
        }
    } catch {
        Write-Host "  ✗ $($tool.Name) 未找到" -ForegroundColor Red
        $allOk = $false
    }
}

Write-Host ""

# 检查库依赖
Write-Host ">>> 检查库依赖 (需要 pkg-config)" -ForegroundColor Yellow
Write-Host ""

$libs = @(
    @{Name="GTK+ 3.0"; Package="gtk+-3.0"},
    @{Name="libpng"; Package="libpng"},
    @{Name="libjpeg"; Package="libjpeg"},
    @{Name="OpenCV (可选)"; Package="opencv4"; Optional=$true}
)

$pkgConfigWorks = $false
try {
    pkg-config --version | Out-Null
    if ($LASTEXITCODE -eq 0) {
        $pkgConfigWorks = $true
    }
} catch {}

if ($pkgConfigWorks) {
    foreach ($lib in $libs) {
        try {
            $version = pkg-config --modversion $lib.Package 2>&1
            if ($LASTEXITCODE -eq 0) {
                Write-Host "  ✓ $($lib.Name): $version" -ForegroundColor Green
            } else {
                if ($lib.Optional) {
                    Write-Host "  ⚠ $($lib.Name): 未安装（可选）" -ForegroundColor Yellow
                } else {
                    Write-Host "  ✗ $($lib.Name): 未安装" -ForegroundColor Red
                    $allOk = $false
                }
            }
        } catch {
            if ($lib.Optional) {
                Write-Host "  ⚠ $($lib.Name): 未安装（可选）" -ForegroundColor Yellow
            } else {
                Write-Host "  ✗ $($lib.Name): 未安装" -ForegroundColor Red
                $allOk = $false
            }
        }
    }
} else {
    Write-Host "  ⚠ pkg-config 不可用，跳过库检查" -ForegroundColor Yellow
    $allOk = $false
}

Write-Host ""

# 检查环境变量
Write-Host ">>> 检查环境变量" -ForegroundColor Yellow
Write-Host ""

$pathDirs = $env:Path -split ';'
$msys2Found = $false
$mingw64Found = $false

foreach ($dir in $pathDirs) {
    if ($dir -like "*msys64*mingw64*bin*") {
        $mingw64Found = $true
        Write-Host "  ✓ 找到 MSYS2 MinGW64: $dir" -ForegroundColor Green
        break
    }
}

if (-not $mingw64Found) {
    Write-Host "  ✗ PATH 中未找到 MSYS2 MinGW64" -ForegroundColor Red
    Write-Host "    应该类似：C:\msys64\mingw64\bin" -ForegroundColor Gray
    $allOk = $false
}

Write-Host ""

# 总结
Write-Host "=========================================" -ForegroundColor Cyan
if ($allOk) {
    Write-Host "  ✓ 环境配置完整！" -ForegroundColor Green
    Write-Host "=========================================" -ForegroundColor Cyan
    Write-Host ""
    Write-Host "可以开始编译了：" -ForegroundColor White
    Write-Host "  .\build.bat" -ForegroundColor Cyan
    Write-Host "  或" -ForegroundColor White
    Write-Host "  .\build.ps1" -ForegroundColor Cyan
} else {
    Write-Host "  ⚠ 环境配置不完整" -ForegroundColor Yellow
    Write-Host "=========================================" -ForegroundColor Cyan
    Write-Host ""
    Write-Host "请按照以下步骤配置环境：" -ForegroundColor Yellow
    Write-Host ""
    Write-Host "1. 安装 MSYS2：https://www.msys2.org/" -ForegroundColor White
    Write-Host ""
    Write-Host "2. 在 MSYS2 MinGW 64-bit 终端中运行：" -ForegroundColor White
    Write-Host "   pacman -S mingw-w64-x86_64-gcc" -ForegroundColor Cyan
    Write-Host "   pacman -S mingw-w64-x86_64-cmake" -ForegroundColor Cyan
    Write-Host "   pacman -S mingw-w64-x86_64-make" -ForegroundColor Cyan
    Write-Host "   pacman -S mingw-w64-x86_64-pkgconf" -ForegroundColor Cyan
    Write-Host "   pacman -S mingw-w64-x86_64-gtk3" -ForegroundColor Cyan
    Write-Host "   pacman -S mingw-w64-x86_64-libpng" -ForegroundColor Cyan
    Write-Host "   pacman -S mingw-w64-x86_64-libjpeg-turbo" -ForegroundColor Cyan
    Write-Host ""
    Write-Host "3. 添加到系统 PATH：" -ForegroundColor White
    Write-Host "   C:\msys64\mingw64\bin" -ForegroundColor Cyan
    Write-Host ""
    Write-Host "4. 重新打开 PowerShell 并重新运行此脚本验证" -ForegroundColor White
    Write-Host ""
    Write-Host "详细说明请查看：ENVIRONMENT_SETUP.md" -ForegroundColor Gray
}

Write-Host ""
