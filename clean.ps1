# ============================================
# 清理构建文件脚本 (PowerShell)
# ============================================

Write-Host "=====================================" -ForegroundColor Cyan
Write-Host "  清理构建文件" -ForegroundColor Cyan
Write-Host "=====================================" -ForegroundColor Cyan
Write-Host ""

$ScriptDir = Split-Path -Parent $MyInvocation.MyCommand.Path
Set-Location $ScriptDir

if (Test-Path "build") {
    Write-Host "正在删除 build 目录..." -ForegroundColor Yellow
    Remove-Item -Recurse -Force "build"
    Write-Host "✓ build 目录已删除" -ForegroundColor Green
} else {
    Write-Host "build 目录不存在，无需清理" -ForegroundColor Gray
}

Write-Host ""
Write-Host "清理完成！" -ForegroundColor Cyan
Write-Host ""
