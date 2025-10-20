# 图像处理器快速诊断

Write-Host "=====================================" -ForegroundColor Cyan
Write-Host "  图像处理器诊断工具" -ForegroundColor Cyan
Write-Host "=====================================" -ForegroundColor Cyan
Write-Host ""

$ErrorActionPreference = "SilentlyContinue"

# 1. 检查程序
Write-Host "[1/5] 检查程序文件..." -ForegroundColor Yellow
if (Test-Path "build\bin\imageprocessor.exe") {
    $prog = Get-Item "build\bin\imageprocessor.exe"
    Write-Host "  ✓ 程序存在" -ForegroundColor Green
    Write-Host "    大小: $([math]::Round($prog.Length/1KB, 2)) KB" -ForegroundColor Gray
    Write-Host "    修改: $($prog.LastWriteTime)" -ForegroundColor Gray
} else {
    Write-Host "  ✗ 程序不存在，请先编译" -ForegroundColor Red
    exit 1
}

# 2. 检查 MSYS2 环境
Write-Host "`n[2/5] 检查 MSYS2 环境..." -ForegroundColor Yellow
if (Test-Path "C:\msys64\mingw64\bin") {
    Write-Host "  ✓ MSYS2 已安装" -ForegroundColor Green
    
    # 检查关键 DLL
    $dlls = @("libgtk-3-0.dll", "libgdk_pixbuf-2.0-0.dll", "libpng16-16.dll", "libjpeg-8.dll")
    $missing = @()
    foreach ($dll in $dlls) {
        if (Test-Path "C:\msys64\mingw64\bin\$dll") {
            Write-Host "    ✓ $dll" -ForegroundColor Green
        } else {
            Write-Host "    ✗ $dll" -ForegroundColor Red
            $missing += $dll
        }
    }
    
    if ($missing.Count -gt 0) {
        Write-Host "  ⚠ 缺少 $($missing.Count) 个 DLL" -ForegroundColor Yellow
    }
} else {
    Write-Host "  ✗ MSYS2 未安装或路径不正确" -ForegroundColor Red
}

# 3. 检查 PATH
Write-Host "`n[3/5] 检查环境变量 PATH..." -ForegroundColor Yellow
$pathEntries = $env:Path -split ';'
$msys2Found = $false
foreach ($entry in $pathEntries) {
    if ($entry -like "*msys64*mingw64*bin*") {
        Write-Host "  ✓ 找到 MSYS2: $entry" -ForegroundColor Green
        $msys2Found = $true
        break
    }
}
if (-not $msys2Found) {
    Write-Host "  ⚠ PATH 中未找到 MSYS2，程序可能缺少 DLL" -ForegroundColor Yellow
}

# 4. 创建测试图片
Write-Host "`n[4/5] 创建测试图片..." -ForegroundColor Yellow
try {
    # 使用 .NET 创建简单的测试图片
    Add-Type -AssemblyName System.Drawing
    $bmp = New-Object System.Drawing.Bitmap(188, 120)
    $g = [System.Drawing.Graphics]::FromImage($bmp)
    $g.Clear([System.Drawing.Color]::White)
    
    # 画一些黑色像素
    for ($y = 40; $y -lt 80; $y++) {
        for ($x = 60; $x -lt 128; $x++) {
            $bmp.SetPixel($x, $y, [System.Drawing.Color]::Black)
        }
    }
    
    $bmp.Save("test_image.png", [System.Drawing.Imaging.ImageFormat]::Png)
    $g.Dispose()
    $bmp.Dispose()
    
    Write-Host "  ✓ 测试图片已创建: test_image.png" -ForegroundColor Green
} catch {
    Write-Host "  ⚠ 无法创建测试图片" -ForegroundColor Yellow
}

# 5. 总结
Write-Host "`n[5/5] 诊断总结" -ForegroundColor Yellow
Write-Host "=====================================" -ForegroundColor Cyan

if (Test-Path "build\bin\imageprocessor.exe") {
    Write-Host "程序已准备就绪！" -ForegroundColor Green
    Write-Host ""
    Write-Host "下一步：" -ForegroundColor White
    Write-Host "  1. 运行程序: .\run.bat" -ForegroundColor Cyan
    Write-Host "  2. 或直接双击: build\bin\imageprocessor.exe" -ForegroundColor Cyan
    Write-Host ""
    
    if (Test-Path "test_image.png") {
        Write-Host "  3. 尝试导入测试图片: test_image.png" -ForegroundColor Cyan
    }
    
    Write-Host ""
    $run = Read-Host "是否现在启动程序? (Y/N)"
    if ($run -eq "Y" -or $run -eq "y") {
        Write-Host ""
        Write-Host "正在启动..." -ForegroundColor Green
        Start-Process "build\bin\imageprocessor.exe"
    }
} else {
    Write-Host "请先运行 build.bat 编译程序" -ForegroundColor Red
}

Write-Host ""
