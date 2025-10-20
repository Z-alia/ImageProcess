# 编译环境配置指南

## ⚠️ 编译错误解决方案

如果遇到以下错误：
```
Could NOT find PkgConfig (missing: PKG_CONFIG_EXECUTABLE)
NMake does not support parallel builds
Generator: build tool execution failed
```

这说明你的系统缺少必要的编译环境或依赖库。

## 🔧 完整环境配置步骤

### 方案一：MSYS2/MinGW 环境（推荐）

这是最简单且完整的解决方案。

#### 1. 安装 MSYS2

1. 下载 MSYS2：https://www.msys2.org/
2. 运行安装程序，建议安装到 `C:\msys64`
3. 安装完成后，打开 "MSYS2 MinGW 64-bit" 终端

#### 2. 更新 MSYS2

```bash
pacman -Syu
# 如果终端关闭，重新打开后再运行一次
pacman -Su
```

#### 3. 安装编译工具链

```bash
# 基础工具链
pacman -S mingw-w64-x86_64-gcc
pacman -S mingw-w64-x86_64-cmake
pacman -S mingw-w64-x86_64-make
pacman -S mingw-w64-x86_64-pkgconf

# GTK3 图形库
pacman -S mingw-w64-x86_64-gtk3

# 图像处理库
pacman -S mingw-w64-x86_64-libpng
pacman -S mingw-w64-x86_64-libjpeg-turbo

# 可选：OpenCV（用于视频处理）
pacman -S mingw-w64-x86_64-opencv
```

#### 4. 配置环境变量

将 MSYS2 的 MinGW64 bin 目录添加到系统 PATH：

1. 右键"此电脑" → "属性" → "高级系统设置"
2. 点击"环境变量"
3. 在"系统变量"中找到 `Path`，点击"编辑"
4. 添加：`C:\msys64\mingw64\bin`
5. 点击"确定"保存

#### 5. 验证安装

打开**新的** PowerShell 窗口，运行：

```powershell
gcc --version
cmake --version
pkg-config --version
```

如果都能正常显示版本号，说明配置成功！

#### 6. 编译项目

```powershell
cd C:\Users\28693\Desktop\Smart-Car-dx\udp\ImageProcess
.\build.bat
```

或使用 PowerShell 脚本：

```powershell
.\build.ps1
```

---

### 方案二：Visual Studio + vcpkg（备选）

如果你更喜欢使用 Visual Studio：

#### 1. 安装 Visual Studio

下载并安装 Visual Studio 2019 或更新版本：
- 选择"使用 C++ 的桌面开发"工作负载
- 确保包含 CMake 工具

#### 2. 安装 vcpkg

```powershell
git clone https://github.com/microsoft/vcpkg
cd vcpkg
.\bootstrap-vcpkg.bat
.\vcpkg integrate install
```

#### 3. 安装依赖库

```powershell
.\vcpkg install gtk:x64-windows
.\vcpkg install libpng:x64-windows
.\vcpkg install libjpeg-turbo:x64-windows
.\vcpkg install opencv:x64-windows
```

#### 4. 配置 CMake

```powershell
cd C:\Users\28693\Desktop\Smart-Car-dx\udp\ImageProcess
mkdir build
cd build
cmake .. -DCMAKE_TOOLCHAIN_FILE=[vcpkg路径]/scripts/buildsystems/vcpkg.cmake -DBUILD_GUI=ON
cmake --build . --config Release
```

---

### 方案三：仅构建核心库（无GUI）

如果你只需要核心的图像处理功能，不需要图形界面：

#### 1. 安装 MinGW

从 https://sourceforge.net/projects/mingw-w64/ 下载 MinGW-w64

#### 2. 编译（无GUI模式）

```powershell
cd C:\Users\28693\Desktop\Smart-Car-dx\udp\ImageProcess
mkdir build
cd build
cmake .. -G "MinGW Makefiles" -DCMAKE_BUILD_TYPE=Release -DBUILD_GUI=OFF
cmake --build . --config Release
```

这将只构建静态库 `libimage_internal.a`，可用于其他项目。

---

## 🐛 常见问题排查

### 问题 1：找不到 pkg-config

**症状**：
```
Could NOT find PkgConfig (missing: PKG_CONFIG_EXECUTABLE)
```

**解决方案**：
```bash
# 在 MSYS2 MinGW 64-bit 终端中
pacman -S mingw-w64-x86_64-pkgconf
```

### 问题 2：NMake 不支持并行构建

**症状**：
```
Warning: NMake does not support parallel builds
```

**原因**：CMake 检测到 Visual Studio 的 NMake，但项目需要 MinGW。

**解决方案**：
1. 清理 build 目录：`.\clean.bat`
2. 确保 MinGW 的 bin 目录在 PATH 中，且优先于 VS 工具
3. 使用 `-G "MinGW Makefiles"` 明确指定生成器

### 问题 3：找不到 GTK

**症状**：
```
Could NOT find GTK3
```

**解决方案**：
```bash
# 在 MSYS2 MinGW 64-bit 终端中
pacman -S mingw-w64-x86_64-gtk3
```

### 问题 4：链接错误

**症状**：
```
undefined reference to `gtk_...`
```

**解决方案**：
1. 确保在 MSYS2 MinGW 64-bit 环境中编译
2. 不要在普通的 CMD 或 PowerShell 中直接编译
3. 或者确保 `C:\msys64\mingw64\bin` 在 PATH 最前面

---

## ✅ 推荐工作流程

### 第一次使用：

1. ✅ 安装 MSYS2（见上文）
2. ✅ 在 MSYS2 终端中安装所有依赖
3. ✅ 配置系统环境变量
4. ✅ 在 PowerShell 中运行 `.\build.bat`

### 日常开发：

```powershell
# 修改代码后，直接运行：
.\build.bat

# 如果有重大更改，先清理：
.\clean.bat
.\build.bat
```

---

## 📋 完整的依赖检查清单

在编译前，确保以下命令都能正常运行：

```powershell
# 检查编译器
gcc --version          # 应显示 GCC 版本
g++ --version          # 应显示 G++ 版本

# 检查构建工具
cmake --version        # 应显示 CMake 版本
mingw32-make --version # 应显示 Make 版本

# 检查包管理
pkg-config --version   # 应显示 pkg-config 版本

# 检查库
pkg-config --modversion gtk+-3.0  # 应显示 GTK 版本
pkg-config --modversion libpng    # 应显示 libpng 版本
```

如果任何命令失败，说明对应的工具/库未正确安装。

---

## 🎯 快速诊断

运行以下 PowerShell 脚本快速诊断环境：

```powershell
Write-Host "=== 编译环境诊断 ===" -ForegroundColor Cyan

$tools = @(
    @{Name="GCC"; Cmd="gcc --version"},
    @{Name="G++"; Cmd="g++ --version"},
    @{Name="CMake"; Cmd="cmake --version"},
    @{Name="Make"; Cmd="mingw32-make --version"},
    @{Name="pkg-config"; Cmd="pkg-config --version"}
)

foreach ($tool in $tools) {
    try {
        Invoke-Expression $tool.Cmd | Out-Null
        Write-Host "✓ $($tool.Name) 已安装" -ForegroundColor Green
    } catch {
        Write-Host "✗ $($tool.Name) 未找到" -ForegroundColor Red
    }
}

Write-Host "`n=== 库依赖检查 ===" -ForegroundColor Cyan

$libs = @("gtk+-3.0", "libpng", "libjpeg")

foreach ($lib in $libs) {
    try {
        pkg-config --modversion $lib 2>$null | Out-Null
        if ($LASTEXITCODE -eq 0) {
            $version = pkg-config --modversion $lib
            Write-Host "✓ $lib : $version" -ForegroundColor Green
        } else {
            Write-Host "✗ $lib 未找到" -ForegroundColor Red
        }
    } catch {
        Write-Host "✗ $lib 未找到" -ForegroundColor Red
    }
}
```

将上述脚本保存为 `check_env.ps1` 并运行。

---

## 💡 提示

1. **首选 MSYS2**：这是在 Windows 上编译 GTK 应用的标准方法
2. **环境变量很重要**：确保 MSYS2 的 bin 目录在 PATH 中
3. **重启终端**：修改环境变量后，一定要重新打开 PowerShell
4. **使用正确的终端**：
   - MSYS2 MinGW 64-bit 终端：用于 `pacman` 安装
   - PowerShell/CMD：用于日常编译（配置好 PATH 后）

---

## 📞 还有问题？

如果按照上述步骤仍然无法编译，请检查：

1. 是否在正确的目录？
2. 是否有管理员权限？
3. 防病毒软件是否阻止了编译？
4. 磁盘空间是否充足？
5. MSYS2 是否完全更新到最新版本？

提供编译时的完整错误信息有助于诊断问题。
