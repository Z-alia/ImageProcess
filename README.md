# 二值图处理器

这是一个用C语言编写的简单程序，允许用户上传以PNG或JPEG格式存储的二值图像并为后续处理做准备。

## 功能特点

- 使用GTK创建图形用户界面
- 支持上传以PNG或JPEG格式存储的二值图像文件
- 验证图像内容是否为二值化（黑白）内容
- 将图像数据缩放至120行×188列并存储为二维uint8_t数组供后续处理
- 支持查看完整原始数组数据（图形化显示）
- 支持处理图像并生成IMO数组
- 支持查看IMO数组数据（彩色图形化显示）
- UI支持自适应窗口大小和完整内容平铺显示

## 重要说明

本程序处理的是**以PNG或JPEG格式存储的二值图像**，而非必须是1位深度的图像。这意味着：

- 支持8位、24位等PNG格式，只要图像内容是二值化的（只有黑白两种颜色）
- 支持标准JPEG格式图像，只要图像内容是二值化的
- 程序会自动将彩色或灰度图像转换为二值数据存储
- 对于非标准二值图像，会进行自动转换处理
- 所有图像都会被缩放至120行×188列尺寸，而不是裁切

## 编译前重要提醒

**在尝试编译之前，必须安装所需的开发库！否则会出现类似以下的错误：**
```
fatal error: gtk/gtk.h: 没有那个文件或目录
Package gtk+-3.0 was not found in the pkg-config search path
```

## 编译依赖

程序需要以下库：
- GTK+ 3.0
- libpng
- libjpeg

在Ubuntu/Debian系统上安装依赖：
```bash
sudo apt-get update
sudo apt-get install libgtk-3-dev libpng-dev libjpeg-dev build-essential
```

在CentOS/RHEL/Fedora系统上安装依赖：
```bash
# CentOS/RHEL
sudo yum install gtk3-devel libpng-devel libjpeg-devel

# Fedora
sudo dnf install gtk3-devel libpng-devel libjpeg-devel
```

## 项目结构（ROS2风格）

本项目采用 ROS2 风格的目录结构，清晰地分离源码、构建产物和输出文件：

```
ImageProcess/
├── src/                    # 源代码目录
│   ├── *.cpp              # C++ 源文件
│   ├── *.c                # C 源文件
│   └── *.h                # 头文件
├── build/                  # CMake 构建产物（临时文件）
├── install/                # 编译输出目录
│   ├── bin/               # 可执行程序
│   └── lib/               # 库文件（如有）
├── log/                    # 日志文件（预留）
├── cmake/                  # CMake 辅助脚本
├── CMakeLists.txt          # CMake 配置文件
├── build.bat               # 一键构建脚本
├── clean.bat               # 清理脚本（删除 build/, install/, log/）
├── run.bat                 # 一键运行脚本（自动启动 imageprocessor）
└── README.md               # 本文档
```

**目录说明**：
- `src/`：所有源代码文件（.cpp/.c/.h）
- `build/`：CMake 生成的中间文件、Makefile、对象文件等
- `install/`：最终的可执行程序和库文件
- `log/`：运行日志（预留，当前未使用）

## 编译程序

### 方式一：使用一键构建脚本（推荐）

Windows 用户可以直接运行：
```batch
build.bat
```

该脚本会自动：
1. 检查并配置 CMake（使用 MinGW Makefiles 生成器）
2. 编译项目
3. 将可执行文件输出到 `install/bin/` 目录

### 方式二：手动使用 CMake

```bash
cmake -S . -B build
cmake --build build
```

> Windows 用户请根据本地工具链选择生成器，例如 `cmake -S . -B build -G "MinGW Makefiles"`。多配置生成器（Visual Studio / Ninja Multi-Config）下可使用 `cmake --build build --config Release` 构建特定配置。

如果遇到 pkg-config 相关错误，请确认 pkg-config 能找到 GTK、libpng 和 libjpeg 的 `.pc` 文件：

```bash
export PKG_CONFIG_PATH=/usr/lib/x86_64-linux-gnu/pkgconfig:/usr/share/pkgconfig:/usr/lib/pkgconfig
```

## 运行程序

### 方式一：使用一键运行脚本（推荐）

Windows 用户可以直接运行：
```batch
run.bat
```

该脚本会自动：
1. 检查 `install/bin/imageprocessor.exe` 是否存在
2. 配置 MSYS2 MinGW64 环境变量
3. 启动程序

### 方式二：手动运行

```bash
./install/bin/imageprocessor
```

在 Windows PowerShell 中运行时，请先临时加入 MSYS2 MinGW64 DLL 路径：

```powershell
$env:PATH = "C:\msys64\mingw64\bin;$env:PATH"
$env:XDG_DATA_DIRS = "C:\msys64\mingw64\share;$env:XDG_DATA_DIRS"
$env:GSETTINGS_SCHEMA_DIR = "C:\msys64\mingw64\share\glib-2.0\schemas"
./install/bin/imageprocessor.exe
```

也可以直接双击仓库根目录下的 `run_imageprocessor.bat`，脚本会自动配置 PATH 并启动程序。

## 清理构建文件

如需清理所有构建产物和输出文件，运行：
```batch
clean.bat
```

该脚本会删除 `build/`、`install/` 和 `log/` 目录。

## 使用方法

1. 点击"上传PNG/JPEG二值图"按钮
2. 在文件选择对话框中选择一个PNG或JPEG图像文件
3. 图像将在左侧"当前图像"区域中显示（保持原始尺寸以便查看）
4. 程序会将图像缩放至120行×188列并验证图像内容是否为二值化内容
5. 缩放后的图像数据存储为二维uint8_t数组
6. 点击"处理图像 (original->imo)"按钮将原始数组处理为IMO数组（用户可自定义处理逻辑）
7. 点击"查看原始数组"按钮可打开新窗口查看原始数组数据的图形化表示
8. 点击"查看IMO数组"按钮可打开新窗口查看IMO数组数据的图形化表示

## UI自适应特性

程序界面支持自适应窗口大小：
- 主窗口可以自由调整大小
- 图像显示区域会自动适应窗口尺寸
- 两个图像显示区域（当前图像和IMO数组）均匀分配空间
- 滚动条会在图像太大时自动出现
- 所有窗口内容支持自适应缩放

## 查看原始数组功能

新增"查看原始数组"按钮，用于图形化查看存储在内存中的完整二值图像数据：
- 以图形化方式显示二维数组的完整内容（120行×188列尺寸）
- 使用黑白像素点直观显示数组内容（黑点表示0，白点表示1）
- 完整平铺显示整个数组，无需滚动即可查看全部内容
- 图像会自动适应窗口大小，支持缩放查看
- 每个数组元素用4x4像素方块表示，整体图像尺寸为752x480像素
- 支持滚动查看，便于仔细观察细节

## IMO数组功能

新增IMO数组相关功能，用于处理和显示最终输出结果：

- **处理图像按钮**：执行从原始数组到IMO数组的转换处理
- **查看IMO数组按钮**：以图形化方式显示IMO二维数组的完整内容（120行×188列尺寸）
- **处理逻辑**：默认处理逻辑是将原始数组直接复制到IMO数组，用户可根据需要修改处理函数实现自定义处理逻辑
- **彩色显示**：根据IMO数组中的值显示不同颜色：
  - 0：黑色
  - 1：红色
  - 2：橙色
  - 3：黄色
  - 4：绿色
  - 5：青色
  - 其他值：白色
- **完整平铺**：整个120行×188列的数组一次性完整显示，无需滚动即可查看全部内容
- **图形化显示**：每个数组元素用4x4像素方块表示，整体图像尺寸为752x480像素
- **独立窗口**：在独立窗口中显示，支持滚动查看和窗口缩放

## 图像缩放功能

程序现在会自动将所有输入图像缩放至120行×188列尺寸：
- 使用双线性插值算法进行高质量缩放
- 缩放后的图像数据存储在二维数组中
- 显示在左侧的图像保持原始尺寸以便查看
- 显示在右侧"IMO数组"区域的图像为120行×188列尺寸
- 查看数组时显示的是120行×188列的数组数据

## 图像处理兼容性

程序支持多种格式的二值图像：

### PNG格式支持
1. **1位深度PNG二值图** - 直接读取位数据
2. **8位深度灰度PNG** - 检查并转换为二值数据
3. **24位RGB PNG** - 转换为灰度再二值化
4. **32位RGBA PNG** - 考虑透明度通道转换为二值数据

### JPEG格式支持
1. **灰度JPEG图像** - 检查并转换为二值数据
2. **RGB JPEG图像** - 转换为灰度再二值化

对于非标准二值图像（即像素值不完全是0和255），程序会自动进行二值化处理（阈值为128）。

## 图像格式检测

程序现在可以自动检测文件的真实格式，不再依赖文件扩展名：

- 检查PNG文件的签名（文件开头的8字节）
- 检查JPEG文件的签名（文件开头的2字节为0xFFD8）
- 对于声称是PNG但实际为JPEG（或其他格式）的文件，提供明确的错误提示

## PNG图像加载问题故障排除

如果程序无法正确打开PNG图像，可能的原因包括：

### 1. 文件相关问题
- **文件不存在或路径错误**：检查文件是否确实存在
- **权限不足**：确保程序有权限读取该文件
- **文件损坏**：尝试用其他图像查看器打开文件确认完整性

### 2. PNG格式问题
- **非PNG文件**：确认文件确实是PNG格式（不仅仅是扩展名）
- **不支持的PNG变种**：极少数PNG压缩方式可能不被支持

### 3. 文件扩展名与实际格式不符
根据测试发现，有些文件虽然扩展名为.png，但实际可能是JPEG格式。可以通过以下命令检查：
```bash
file filename.png
```
如果输出显示"JPEG image data"而非"PNG image data"，则该文件实际为JPEG格式。

### 4. 图像类型问题
- **非二值图像内容**：虽然程序可以处理任意图像并转换为二值，但结果可能不符合预期
- **透明度处理**：RGBA图像中的透明区域将被处理为白色

### 5. 系统资源问题
- **内存不足**：非常大的图像可能导致内存分配失败
- **图像尺寸过大**：超过系统处理能力的图像尺寸

### 6. 程序诊断
程序现在包含详细的诊断输出，可以通过终端运行程序来查看详细错误信息：
```bash
./imageprocessor
```
在加载图像时，程序会输出详细的处理步骤和任何遇到的错误。

## 技术细节

程序现在可以将加载的图像数据存储为二维uint8_t数组：
- 数组名称为 `original_bi_image`（原始输入数组）
- 数组名称为 `imo`（处理后输出数组）
- 所有图像都会被缩放至120行×188列尺寸
- 支持各种位深度的PNG图像和JPEG图像，只要内容是二值化的
- 自动分配适当大小的二维数组内存
- 正确处理PNG位顺序和各种颜色格式
- 正确处理JPEG的颜色空间转换

全局变量用于访问图像数据：
- `original_bi_image` - 二维uint8_t数组指针（原始输入数组）
- `imo` - 二维uint8_t数组指针（处理后输出数组）
- `image_width` - 图像宽度（固定为188）
- `image_height` - 图像高度（固定为120）

UI功能：
- 左侧显示原始尺寸图像（便于查看）
- 右侧显示IMO数组的图形化表示（120行×188列）
- "处理图像"按钮用于执行从原始数组到IMO数组的处理
- "查看原始数组"按钮用于查看原始数组数据
- "查看IMO数组"按钮用于查看IMO数组数据

## 程序架构

- GUI 入口：`main.cpp`（C++，使用 GTK + OpenCV 进行图像/视频导入与显示）
- 处理逻辑：`processor.c/.h`（C 语言实现，接口稳定，供 GUI 与 CLI 复用）
- 命令行视频工具：`video_processor.cpp`（C++，OpenCV 读取 mp4 并逐帧导出/处理）
- 构建配置与说明文档

## 后续处理扩展

当前程序已经实现了图像加载和基本验证功能，并将图像数据存储为二维uint8_t数组。要添加具体的后续处理功能，可以直接访问数组，例如：
- 图像分析
- 特征提取
- 数据导出等

---

## 新增：视频逐帧导出工具（OpenCV）

为满足“把 mp4 的每一帧导出为 PNG 并逐帧处理”的需求，新增命令行工具：`video_processor`。

功能：
- 读取输入视频（如 .mp4），将每一帧导出为 `frame_XXXXXX.png`。
- 将每帧缩放为 188x120 并二值化为 original 数组，调用 `processor.c` 中的 `process_original_to_imo` 生成 imo；若附加 `--export-imo` 则导出 `imo_XXXXXX.png` 可视化。

构建说明：
- 本仓库的 CMake 已将 GUI 与视频工具拆分为可选目标：
  - `-DBUILD_VIDEO_TOOL=ON`（默认 ON）构建 `video_processor`
  - `-DBUILD_GUI=ON` 构建原有 GTK 图形界面 `imageprocessor`
- `video_processor` 依赖 OpenCV（imgcodecs、imgproc、videoio）。若 CMake 输出“OpenCV 未找到，将跳过 video_processor 目标的构建”，请先安装/配置 OpenCV。

Windows（MinGW + MSYS2）示例：
1) 在 MSYS2 中安装 OpenCV（示例）：`pacman -S mingw-w64-x86_64-opencv`
2) 在 PowerShell 生成与构建（根据本地路径调整 OpenCV_DIR）：
   - cmake -S . -B build -G "MinGW Makefiles" -DOpenCV_DIR=C:\\msys64\\mingw64\\lib\\cmake\\opencv4
   - cmake --build build --target video_processor

运行：
- 直接运行可执行文件（PowerShell）：
  - ./build/bin/video_processor.exe C:\\path\\to\\input.mp4 C:\\path\\to\\frames --export-imo
- 或使用脚本：
  - ./run_video_processor.ps1 -Input C:\\path\\to\\input.mp4 -OutDir C:\\path\\to\\frames -ExportImo