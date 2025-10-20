# 编译脚本使用说明

## 📦 一键编译脚本

本项目提供了多个编译脚本，方便快速构建程序。

## 🚀 快速开始

### 方式一：使用批处理脚本（推荐）

双击运行 `build.bat` 或在命令行中执行：

```cmd
build.bat
```

### 方式二：使用 PowerShell 脚本

在 PowerShell 中执行：

```powershell
.\build.ps1
```

**注意**：如果遇到权限问题，需要先设置执行策略：

```powershell
Set-ExecutionPolicy -ExecutionPolicy RemoteSigned -Scope CurrentUser
```

### 方式三：手动编译

```cmd
# 创建构建目录
mkdir build
cd build

# 配置项目
cmake .. -DCMAKE_BUILD_TYPE=Release

# 编译（使用 4 个并行任务）
cmake --build . --config Release -j 4
```

## 📝 脚本功能说明

### 编译脚本 (build.bat / build.ps1)

自动完成以下步骤：

1. ✅ 检查并创建 `build` 目录
2. ✅ 运行 CMake 配置（Release 模式）
3. ✅ 并行编译项目（使用 4 个线程）
4. ✅ 检查生成的可执行文件
5. ✅ 提示运行程序（可选）

**特点**：
- 彩色输出，清晰展示编译进度
- 自动错误检测和提示
- 编译成功后可选择立即运行程序
- 适用于首次编译和重新编译

### 清理脚本 (clean.bat / clean.ps1)

删除 `build` 目录，清理所有编译生成的文件。

**使用场景**：
- 需要完全重新编译
- 切换编译配置前
- 清理磁盘空间

## 📂 生成的文件位置

编译成功后，可执行文件通常位于：

```
build/bin/imageprocessor.exe        # 图像处理主程序
build/bin/video_processor.exe       # 视频处理程序
```

或者：

```
build/imageprocessor.exe
build/video_processor.exe
```

## 🔧 编译选项

### 修改并行编译线程数

编辑脚本中的 `-j 4` 参数：

```cmd
cmake --build build --config Release -j 8   # 使用 8 个线程
```

### 切换编译模式

**Debug 模式**（包含调试信息）：

修改脚本中的：
```cmd
cmake .. -DCMAKE_BUILD_TYPE=Debug
cmake --build build --config Debug -j 4
```

**Release 模式**（默认，性能优化）：
```cmd
cmake .. -DCMAKE_BUILD_TYPE=Release
cmake --build build --config Release -j 4
```

## ⚠️ 常见问题

### 1. 提示 "cmake 不是内部或外部命令"

**解决方案**：安装 CMake 并添加到系统 PATH

- 下载地址：https://cmake.org/download/
- 安装时勾选 "Add CMake to system PATH"

### 2. 编译错误：缺少 GTK 或 OpenCV

**解决方案**：确保已安装依赖库

```bash
# MSYS2/MinGW 环境
pacman -S mingw-w64-x86_64-gtk3
pacman -S mingw-w64-x86_64-opencv
pacman -S mingw-w64-x86_64-libpng
```

### 3. PowerShell 脚本无法运行

**错误信息**：`无法加载文件，因为在此系统上禁止运行脚本`

**解决方案**：
```powershell
Set-ExecutionPolicy -ExecutionPolicy RemoteSigned -Scope CurrentUser
```

### 4. 找不到可执行文件

检查编译输出是否有错误，或者查看 CMakeLists.txt 中的输出目录配置。

## 🎯 完整工作流程

### 首次编译

```cmd
# 1. 编译项目
build.bat

# 2. 运行程序
build\bin\imageprocessor.exe
```

### 修改代码后重新编译

```cmd
# 直接运行编译脚本即可（增量编译）
build.bat
```

### 完全重新编译

```cmd
# 1. 清理旧文件
clean.bat

# 2. 重新编译
build.bat
```

## 📊 脚本输出示例

```
=====================================
  图像处理程序编译脚本
=====================================

[1/4] 检查构建目录...
      build 目录已存在
      ✓ 完成

[2/4] 配置项目 (CMake)...
      ✓ 配置成功

[3/4] 编译项目...
      ✓ 编译成功

[4/4] 检查输出文件...
      ✓ 找到可执行文件: build\bin\imageprocessor.exe

=====================================
  编译完成！
=====================================

是否立即运行程序? (Y/N):
```

## 🎨 优化建议

### 加快编译速度

1. 增加并行线程数（根据 CPU 核心数）：
   ```cmd
   cmake --build build --config Release -j 8
   ```

2. 使用 Ninja 构建系统（需要先安装 Ninja）：
   ```cmd
   cmake .. -G Ninja -DCMAKE_BUILD_TYPE=Release
   ninja -C build
   ```

### 缩小可执行文件大小

在 Release 模式下，可执行文件会自动优化。如需进一步减小：

```cmd
# 编译后使用 strip 工具
strip build\bin\imageprocessor.exe
```

## 📖 相关文档

- [IMPROVEMENTS.md](IMPROVEMENTS.md) - 最新优化说明
- [README.md](README.md) - 项目说明
- CMakeLists.txt - 构建配置

## 💡 提示

- 首次编译推荐使用 `build.bat`（简单易用）
- 开发过程中可以只运行 `build.bat` 进行增量编译
- 遇到奇怪的编译问题时，先运行 `clean.bat` 清理后重新编译
