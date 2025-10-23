param(
    [Parameter(Mandatory=$true)][string]$Input,
    [Parameter(Mandatory=$true)][string]$OutDir,
    [switch]$ExportImo
)

# 可选：设置 OpenCV DLL 搜索路径（若你用的是 vcpkg 或本地 OpenCV 安装，请按需修改）
# $env:PATH = "C:\opencv\build\x64\vc16\bin;$env:PATH"

$exe = Join-Path $PSScriptRoot 'install\bin\video_processor.exe'
if (-not (Test-Path $exe)) {
    Write-Error "未找到 $exe，请先构建工程。"
    exit 1
}

$argList = @($Input, $OutDir)
if ($ExportImo) { $argList += '--export-imo' }
& $exe @argList
