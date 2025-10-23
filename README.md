# ImageProcess - 图像处理与分析工具

[![C++](https://img.shields.io/badge/C++-17-blue.svg)](https://isocpp.org/)
[![CMake](https://img.shields.io/badge/CMake-3.10+-green.svg)](https://cmake.org/)
[![GTK](https://img.shields.io/badge/GTK-3.0-orange.svg)](https://www.gtk.org/)

一个用于处理和分析二值图像的桌面应用程序，特别适合与STM32 UDP图传系统配合使用。支持智能CSV读取、可调整窗口布局、平铺帧选择器等专业功能。

> **✨ 最新更新**: 支持智能CSV列检测、可拖动窗口布局、Binary模式读取，完美兼容UDP上位机生成的所有CSV格式。

## 功能特点

### 🖼️ 图像处理
- 支持上传PNG或JPEG格式的二值图像文件
- 自动缩放至120行×188列标准尺寸
- 验证图像内容并自动二值化处理
- 支持多种位深度的图像格式（8位、24位、32位等）
- 将图像数据存储为二维uint8_t数组供后续处理

### 📊 CSV数据分析（配合UDP上位机）
- **智能列检测**: 自动识别任意列顺序的CSV文件
  - 支持3种必需列: `host_recv_iso`、`png_path`、`frame_id`
  - 自动匹配中英文列名（如`host_recv_iso`/`主机接收时间`）
  - 兼容所有自定义日志变量列
- **Binary模式读取**: 容错特殊字符（NULL/EOF/控制字符）
  - 自动清理`\r`、`\0`、`\x1A`等字符
  - Try-catch保护，单行错误不影响整体
  - Debug输出：totalLines、skippedLines、errorLines
- **完整记录保证**: 从旧版84行→新版309行完整读取
- **灵活列扩展**: 支持任意数量的自定义变量列

### 🎨 UI界面增强
- **可拖动布局**: 使用GtkPaned实现三图像区域可调整
  - 左侧：当前图像（原图+IMO数组）
  - 右侧：平铺帧选择器
  - 拖动分隔条自由调整各区域大小
- **自适应缩放**: 图像自动适应窗口大小
  - 保持宽高比
  - 支持滚动查看
  - 居中显示
- **平铺帧选择器**: 1400x900大窗口，网格显示所有帧
  - 每帧显示缩略图
  - 鼠标悬停高亮
  - 点击切换查看
  - 支持滚动浏览

### 🔍 数据可视化
- **原始数组查看**: 图形化显示120×188二值数组
  - 黑白像素点直观显示
  - 完整平铺，无需滚动
- **IMO数组查看**: 彩色显示处理结果
  - 6种颜色映射（0-5对应黑/红/橙/黄/绿/青）
  - 独立窗口显示
  - 支持缩放和滚动

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
│   ├── main.cpp           # GUI入口（GTK + OpenCV）
│   ├── processor.c/.h     # 图像处理核心逻辑
│   ├── csv_reader.cpp/.h  # 智能CSV读取器 ⭐
│   ├── image.c/.h         # 图像加载和转换
│   ├── video_processor.cpp # 命令行视频工具
│   └── global_image_buffer.c/.h  # 图像缓冲区管理
├── build/                  # CMake 构建产物（临时文件）
├── install/                # 编译输出目录
│   ├── bin/               # 可执行程序
│   │   └── imageprocessor.exe
│   └── lib/               # 库文件（如有）
├── log/                    # 日志文件（预留）
├── cmake/                  # CMake 辅助脚本
├── CMakeLists.txt          # CMake 配置文件
├── build.bat               # 一键构建脚本
├── clean.bat               # 清理脚本
├── run.bat                 # 一键运行脚本
└── README.md               # 本文档
```

**重点文件说明**：
- `csv_reader.cpp`: 智能CSV读取器，支持任意列顺序、Binary模式、特殊字符清理
- `main.cpp`: GTK界面，可拖动布局、平铺帧选择器、自适应缩放
- `processor.c`: C语言核心处理逻辑，接口稳定
- `video_processor.cpp`: OpenCV视频逐帧处理工具

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

### 基本操作流程

#### 1. 加载CSV数据（配合UDP上位机）
1. 点击"Load CSV"按钮
2. 选择UDP上位机生成的CSV文件（如`aligned.csv`、`frames_index.csv`）
3. 程序自动识别列顺序和自定义变量
4. 右侧平铺帧选择器显示所有帧的缩略图

#### 2. 查看图像帧
- **方式一**: 点击平铺帧选择器中的缩略图
- **方式二**: 使用"上一帧"/"下一帧"按钮导航
- 左上区域显示原始图像（保持原始尺寸）
- 左下区域显示IMO数组（处理后的结果）

#### 3. 处理图像
1. 点击"处理图像 (original->imo)"按钮
2. 程序执行从原始数组到IMO数组的转换
3. 左下区域自动更新显示处理结果

#### 4. 查看数组数据
- **查看原始数组**: 点击"查看原始数组"按钮，图形化显示二值数据
- **查看IMO数组**: 点击"查看IMO数组"按钮，彩色显示处理结果

### 高级功能

#### CSV格式兼容性

程序支持多种CSV格式，自动识别列顺序：

**标准格式（UDP上位机生成）**:
```csv
frame_id,host_recv_iso,png_path,h,w,temperature,speed
0,2025-10-24 10:30:45.123,frames_png/frame_000000.png,60,120,25.5,100
```

**对齐格式（align命令生成）**:
```csv
frame_id,host_recv_iso,png_path,log_text_hex,voltage1,voltage2
0,2025-10-24 10:30:45.123,frames_png/frame_000000.png,aabbcc,3.30,4.20
```

**自定义格式**:
```csv
时间戳,图像路径,帧号,自定义变量1,自定义变量2,...
2025-10-24 10:30:45.123,images/img1.png,0,100,200,...
```

**必需列（三选一名称即可）**:
- **时间戳列**: `host_recv_iso` 或 `主机接收时间` 或 `timestamp`
- **图像路径列**: `png_path` 或 `图像路径` 或 `image_path`
- **帧号列**: `frame_id` 或 `帧号` 或 `id`

#### 窗口布局调整

使用GtkPaned实现的可拖动布局：
- **水平分隔条**: 拖动调整左侧图像区和右侧帧选择器的比例
- **垂直分隔条**: 拖动调整左上原始图像和左下IMO数组的比例
- 布局自动保存，下次打开恢复

#### 平铺帧选择器

1400x900大窗口，网格显示所有帧：
- **缩略图大小**: 每帧120x188像素
- **网格布局**: 自动计算列数，行数根据总帧数调整
- **鼠标交互**:
  - 悬停时高亮显示边框
  - 点击切换到对应帧
  - 当前帧显示红色边框
- **滚动浏览**: 支持鼠标滚轮和滚动条

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

## 常见问题与故障排除

### CSV加载问题

**Q1: CSV只加载了部分记录（如84/306行）？**
- **原因**: 旧版本CSV包含NULL(0x00)或EOF(0x1A)特殊字符
- **解决**: 
  1. 更新UDP上位机到v2.2版本
  2. 删除旧的CSV文件
  3. 重新采集数据
  4. 当前版本已支持容错，但建议使用清理后的数据

**Q2: CSV列名不识别？**
- **检查**: 确保包含3个必需列（时间戳、图像路径、帧号）
- **列名**: 支持中英文多种别名，不区分大小写
- **格式**: CSV第一行必须是列名（标题行）

**Q3: 自定义变量列显示不全？**
- **支持**: 程序支持任意数量的自定义变量列
- **检查**: 确保CSV格式正确，列名与数据对齐
- **Debug**: 查看控制台输出的列名列表

### 图像显示问题

**Q4: 图像显示模糊或变形？**
- **原因**: 自动缩放算法保持宽高比
- **解决**: 调整窗口大小或拖动分隔条
- **查看**: 点击"查看原始数组"查看原始数据

**Q5: 平铺帧选择器缩略图太小？**
- **调整**: 拖动水平分隔条增加右侧区域宽度
- **滚动**: 使用鼠标滚轮查看更多帧
- **窗口**: 主窗口默认1400x900，可调整

## 技术细节

### CSV读取器（csv_reader.cpp）

**智能列检测**:
```cpp
// 自动识别以下列名（不区分大小写）:
- 时间戳: "host_recv_iso", "主机接收时间", "timestamp"
- 图像路径: "png_path", "图像路径", "image_path"
- 帧号: "frame_id", "帧号", "id"
```

**Binary模式读取**:
```cpp
std::ifstream file(filename, std::ios::binary);  // 防止EOF字符截断
```

**特殊字符清理**:
```cpp
// 移除可能破坏CSV解析的字符
line.erase(std::remove(line.begin(), line.end(), '\r'), line.end());  // CR
line.erase(std::remove(line.begin(), line.end(), '\0'), line.end());  // NULL
line.erase(std::remove(line.begin(), line.end(), '\x1A'), line.end()); // EOF
```

**错误处理**:
```cpp
// 单行错误不影响整体
try {
    // 解析CSV行
} catch (...) {
    errorLines++;
    continue;
}
```

### 图像数据结构

程序使用以下全局变量管理图像数据：

**原始数组**:
```c
uint8_t** original_bi_image;  // 120行×188列二值数组
int image_width = 188;        // 图像宽度
int image_height = 120;       // 图像高度
```

**处理结果**:
```c
uint8_t** imo;  // 120行×188列IMO数组
```

**颜色映射（IMO数组显示）**:
```c
0 → 黑色 (0x000000)
1 → 红色 (0xFF0000)
2 → 橙色 (0xFF8800)
3 → 黄色 (0xFFFF00)
4 → 绿色 (0x00FF00)
5 → 青色 (0x00FFFF)
其他 → 白色 (0xFFFFFF)
```

### UI组件架构

**主窗口布局**:
```
GtkWindow
├── GtkPaned (水平)
│   ├── GtkPaned (垂直)
│   │   ├── GtkScrolledWindow (原始图像)
│   │   └── GtkScrolledWindow (IMO数组)
│   └── GtkScrolledWindow (平铺帧选择器)
└── 按钮工具栏
```

**自适应缩放算法**:
```cpp
// 保持宽高比适应窗口
double scale_w = viewport_w / (double)pixbuf_w;
double scale_h = viewport_h / (double)pixbuf_h;
double scale = std::min(scale_w, scale_h);
```

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

---

## 🔗 相关项目

- **[UDP上位机](https://github.com/Dengdxx/udp)**: 配套的STM32 UDP图传系统
  - 实时图像和日志传输
  - 自动生成PNG序列和CSV数据
  - 支持自定义协议和变量配置
  - v2.2版本已修复CSV数据完整性问题

### 完整工作流程

```
STM32设备              UDP上位机           ImageProcess
   ↓                      ↓                    ↓
图像采集           UDP接收显示          CSV智能读取
压缩传输           PNG序列保存          平铺帧选择
日志打包           CSV记录生成          数据分析
   ↓                      ↓                    ↓
WiFi模块           实时监控界面         可调整布局
SPI传输            变量配置显示         图像处理
   ↓                      ↓                    ↓
完整数据流 ──────────────────────────→ 完整分析流
```

### 推荐配置

**STM32端**:
- 图像格式: 压缩二值(1位) - 8:1压缩比
- 图像尺寸: 60x120 或 120x188
- 日志协议: 标准格式（无需时间戳）
- SPI速率: 4Mbps（推荐，避免黑线问题）

**UDP上位机**:
- 版本: v2.2+ （支持数据清理）
- 图像格式: 压缩二值(1位)
- CSV格式: 包含自定义变量
- 输出: PNG序列 + aligned.csv

**ImageProcess**:
- 加载: aligned.csv（完整数据）
- 窗口: 1400x900（推荐）
- 布局: 可拖动分隔条调整

---

**最后更新**: 2025年10月24日
**版本**: v2.0