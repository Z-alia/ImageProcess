# ImageProcess 完整使用指南

[![C++](https://img.shields.io/badge/C++-17-blue.svg)](https://isocpp.org/)
[![CMake](https://img.shields.io/badge/CMake-3.10+-green.svg)](https://cmake.org/)
[![GTK](https://img.shields.io/badge/GTK-3.0-orange.svg)](https://www.gtk.org/)

> **文档版本**: v3.0
> **最后更新**: 2025年10月24日

---

## 📚 目录

1. [项目简介](#1-项目简介)
2. [功能特点](#2-功能特点)
3. [安装与编译](#3-安装与编译)
4. [快速开始](#4-快速开始)
5. [CSV格式说明](#5-csv格式说明)
6. [示波器使用指南](#6-示波器使用指南)
7. [问题修复记录](#7-问题修复记录)
8. [技术文档](#8-技术文档)
9. [常见问题](#9-常见问题)
10. [相关项目](#10-相关项目)

---

## 1. 项目简介

### 1.1 概述

ImageProcess 是一个用于处理和分析二值图像的桌面应用程序，特别适合与STM32 UDP图传系统配合使用。支持智能CSV读取、可调整窗口布局、平铺帧选择器、实时数据示波器等专业功能。

> **✨ 最新更新**: 
> - ✅ 示波器功能完全支持中文显示
> - ✅ 修复进度条回拖时波形错乱问题
> - ✅ 智能CSV列检测，兼容任意列顺序
> - ✅ Binary模式读取，容错特殊字符

### 1.2 核心特性

#### 🖼️ 图像处理
- 支持PNG、JPEG格式的二值图像
- 自动缩放至120×188标准尺寸
- 自动二值化处理
- 二维数组存储供后续处理

#### 📊 CSV数据分析
- **智能列检测**: 自动识别任意列顺序
- **Binary模式读取**: 容错NULL/EOF等特殊字符
- **完整记录保证**: 从旧版84行→新版309行完整读取
- **灵活列扩展**: 支持任意数量自定义变量

#### 🎨 UI界面增强
- **可拖动布局**: GtkPaned实现三区域可调整
- **自适应缩放**: 图像自动适应窗口大小
- **平铺帧选择器**: 1400×900大窗口网格显示
- **实时日志显示**: 动态显示所有日志变量

#### 📈 数据可视化
- **实时示波器**: 多通道波形显示
- **8种配色方案**: 自动分配不同颜色
- **自动缩放**: Y轴自动适应数据范围
- **时间窗口可调**: 1-60秒可调节

### 1.3 重要说明

本程序处理的是**以PNG或JPEG格式存储的二值图像**，而非必须是1位深度的图像。这意味着：

- ✅ 支持8位、24位等PNG格式，只要内容是二值化的
- ✅ 支持标准JPEG格式图像
- ✅ 自动转换彩色或灰度图像为二值数据
- ✅ 所有图像缩放至120×188尺寸（不裁切）

---

## 2. 功能特点

### 2.1 图像处理功能

#### 支持的图像格式
- **PNG格式**: 1位、8位、24位、32位深度
- **JPEG格式**: 灰度、RGB彩色
- **自动转换**: 彩色→灰度→二值化

#### 图像处理流程
1. **加载图像** → 检测格式并读取
2. **缩放** → 双线性插值至120×188
3. **二值化** → 阈值128转换为0/255
4. **存储** → 二维uint8_t数组

### 2.2 CSV数据功能

#### 智能列检测
自动识别以下列（不区分大小写）：

| 列类型 | 关键词 | 示例 |
|--------|--------|------|
| 时间戳 | time, iso, timestamp | host_recv_iso |
| Hex数据 | hex | log_text_hex |
| UTF-8文本 | utf, text | log_text_utf8 |
| 自定义变量 | 任意 | 温度, 速度, 电压 |

#### Binary模式读取
- 使用`std::ios::binary`标志
- 移除`\r`, `\0`, `\x1A`等特殊字符
- Try-catch保护，单行错误不影响整体
- Debug输出：totalLines, skippedLines, errorLines

### 2.3 示波器功能

#### 核心特性
- 📈 **多通道显示**: 最多支持8个通道
- 🎨 **配色方案**: 红、青、黄、紫、绿、橙、蓝、粉
- 🔄 **实时更新**: 随视频播放自动更新
- 📐 **自动缩放**: Y轴自动适应数据范围
- ⏱️ **时间窗口**: 1-60秒可调节
- 👁️ **通道开关**: 单独显示/隐藏通道

#### 中文支持
- ✅ 使用Pango库渲染中文
- ✅ Microsoft YaHei字体
- ✅ 完美显示中文变量名
- ✅ 图例、坐标轴标签全部支持

#### 窗口管理
- ✅ 关闭后可重新打开
- ✅ 保持通道和设置
- ✅ 隐藏而不是销毁窗口

### 2.4 UI交互功能

#### 可拖动布局
```
主窗口
├── 水平分隔条
│   ├── 左侧区域（垂直分隔）
│   │   ├── 原始图像显示
│   │   └── IMO数组显示
│   └── 右侧区域
│       └── 平铺帧选择器
```

#### 平铺帧选择器
- **窗口大小**: 1400×900像素
- **缩略图**: 每帧120×188像素
- **网格布局**: 自动计算行列数
- **交互**:
  - 鼠标悬停高亮
  - 点击切换帧
  - 当前帧红色边框
  - 滚动浏览支持

#### 实时日志显示
- 动态创建标签显示变量
- 100×100像素画布
- 黑色背景
- 白色文字
- 自动换行

---

## 3. 安装与编译

### 3.1 编译前提醒

⚠️ **在尝试编译之前，必须安装所需的开发库！**

否则会出现类似以下错误：
```
fatal error: gtk/gtk.h: 没有那个文件或目录
Package gtk+-3.0 was not found in the pkg-config search path
```

### 3.2 依赖库安装

#### Ubuntu/Debian系统
```bash
sudo apt-get update
sudo apt-get install libgtk-3-dev libpng-dev libjpeg-dev build-essential cmake
```

#### CentOS/RHEL系统
```bash
sudo yum install gtk3-devel libpng-devel libjpeg-devel cmake
```

#### Fedora系统
```bash
sudo dnf install gtk3-devel libpng-devel libjpeg-devel cmake
```

#### Windows系统（MSYS2）
```bash
pacman -S mingw-w64-x86_64-gtk3 \
          mingw-w64-x86_64-libpng \
          mingw-w64-x86_64-libjpeg-turbo \
          mingw-w64-x86_64-opencv \
          mingw-w64-x86_64-cmake \
          mingw-w64-x86_64-gcc
```

### 3.3 项目结构

```
ImageProcess/
├── src/                    # 源代码
│   ├── main.cpp           # GUI入口
│   ├── oscilloscope.cpp   # 示波器实现 ⭐
│   ├── csv_reader.cpp     # CSV读取器 ⭐
│   ├── processor.c        # 图像处理核心
│   ├── image.c            # 图像加载
│   └── video_processor.cpp # 视频工具
├── build/                  # 构建临时文件
├── install/                # 输出目录
│   ├── bin/               # 可执行文件
│   └── lib/               # 库文件
├── docs/                   # 文档目录 ⭐
│   └── COMPLETE_GUIDE.md  # 完整指南
├── cmake/                  # CMake脚本
├── CMakeLists.txt          # CMake配置
├── build.bat               # 一键构建（Windows）
├── clean.bat               # 清理脚本
├── run.bat                 # 一键运行
└── README.md               # 简要说明
```

### 3.4 编译程序

#### 方式一：使用一键构建脚本（推荐）

**Windows系统**:
```batch
.\build.bat
```

该脚本会自动：
1. ✅ 检查并配置CMake
2. ✅ 使用MinGW Makefiles生成器
3. ✅ 编译项目
4. ✅ 输出到`install/bin/`目录

**Linux/Mac系统**:
```bash
./build.sh
```

#### 方式二：手动使用CMake

```bash
# 配置
cmake -S . -B build -G "MinGW Makefiles"

# 编译
cmake --build build

# Release模式（可选）
cmake --build build --config Release
```

#### pkg-config配置（如需要）

如果遇到pkg-config错误：
```bash
export PKG_CONFIG_PATH=/usr/lib/x86_64-linux-gnu/pkgconfig:/usr/share/pkgconfig:/usr/lib/pkgconfig
```

### 3.5 运行程序

#### 方式一：使用运行脚本（推荐）

**Windows系统**:
```batch
.\run.bat
```

该脚本会自动：
1. ✅ 检查程序是否存在
2. ✅ 配置MSYS2 MinGW64环境
3. ✅ 启动程序

#### 方式二：手动运行

**Linux/Mac**:
```bash
./install/bin/imageprocessor
```

**Windows PowerShell**:
```powershell
# 设置环境变量
$env:PATH = "C:\msys64\mingw64\bin;$env:PATH"
$env:XDG_DATA_DIRS = "C:\msys64\mingw64\share;$env:XDG_DATA_DIRS"
$env:GSETTINGS_SCHEMA_DIR = "C:\msys64\mingw64\share\glib-2.0\schemas"

# 运行程序
./install/bin/imageprocessor.exe
```

### 3.6 清理构建文件

```batch
.\clean.bat
```

该脚本会删除：
- `build/` 目录
- `install/` 目录
- `log/` 目录

---

## 4. 快速开始

### 4.1 基本操作流程

#### 步骤1：加载CSV数据
1. 点击 **"加载日志CSV"** 按钮
2. 选择UDP上位机生成的CSV文件
   - `aligned.csv`（推荐，包含完整数据）
   - `logs.csv`（日志数据）
   - `frames_index.csv`（帧索引）
3. 程序自动识别列顺序和自定义变量
4. 右侧平铺帧选择器显示所有帧缩略图

#### 步骤2：查看图像帧
- **方式一**: 点击平铺帧选择器中的缩略图
- **方式二**: 使用"上一帧"/"下一帧"按钮
- **方式三**: 使用进度条拖动

**显示区域**:
- 左上区域：原始图像（保持原始尺寸）
- 左下区域：IMO数组（处理后结果）
- 右侧区域：平铺帧选择器

#### 步骤3：使用示波器
1. 点击 **"📊 示波器"** 按钮
2. 从下拉框选择要监控的变量（如"温度"）
3. 点击 **"添加通道"** 按钮
4. 播放视频或切换帧，波形自动更新

#### 步骤4：处理图像（可选）
1. 点击 **"处理图像 (original→imo)"** 按钮
2. 程序执行从原始数组到IMO数组的转换
3. 左下区域自动更新显示处理结果

#### 步骤5：查看数组数据（可选）
- **查看原始数组**: 点击"查看原始数组"，显示二值数据
- **查看IMO数组**: 点击"查看IMO数组"，彩色显示结果

### 4.2 示例CSV文件

创建 `test_data.csv` 测试：

```csv
host_recv_iso,log_text_hex,log_text_utf8,温度,速度,电压
2025-10-24 10:00:00.001,48656C6C6F,Hello,25.5,120,3.3
2025-10-24 10:00:00.034,576F726C64,World,26.1,125,3.2
2025-10-24 10:00:00.067,54657374,Test,25.8,118,3.4
2025-10-24 10:00:00.101,44617461,Data,27.2,130,3.5
2025-10-24 10:00:00.134,496E666F,Info,26.5,122,3.3
```

### 4.3 窗口布局调整

#### 使用分隔条
- **水平分隔条**: 拖动调整左侧图像区和右侧帧选择器比例
- **垂直分隔条**: 拖动调整左上原始图像和左下IMO数组比例
- 布局自动保存，下次打开恢复

#### 最佳布局建议
- **主窗口**: 1400×900像素（推荐）
- **左侧区域**: 占60-70%宽度
- **原始图像**: 占左侧50%高度
- **IMO数组**: 占左侧50%高度
- **右侧选择器**: 占30-40%宽度

---

## 5. CSV格式说明

### 5.1 智能列检测

程序支持**自动检测CSV列格式**，无需固定列顺序！

#### 识别规则（不区分大小写）

| 列类型 | 关键词 | 示例 |
|--------|--------|------|
| 时间戳 | time, iso, timestamp | host_recv_iso, timestamp |
| Hex数据 | hex | log_text_hex, data_hex |
| UTF-8文本 | utf, text | log_text_utf8, text |
| 自定义变量 | （其他任意） | 温度, 速度, 电压, frame_id |

### 5.2 支持的CSV格式

#### 格式1：标准格式（UDP上位机默认）
```csv
host_recv_iso,log_text_hex,log_text_utf8,温度,速度,电压
2025-10-24 10:00:00.001,48656C6C6F,Hello,25.5,120,3.3
```
- ✅ 包含时间戳、hex、utf8、自定义变量
- ✅ 最完整的日志信息

#### 格式2：对齐格式（align命令生成）
```csv
frame_id,png_path,frame_host_iso,h,w,log_host_iso,log_text_utf8,温度
1,frames_png/frame_000001.png,2025-10-24 10:00:00.001,120,188,2025-10-24 10:00:00.001,Hello,25.5
```
- ✅ 图像和日志对齐
- ✅ 包含PNG路径
- ✅ 支持视频播放

#### 格式3：简化格式
```csv
timestamp,log_message,value1,value2
2025-10-24 10:00:00.001,Hello,100,200
```
- ✅ 最小化格式
- ✅ 仅时间戳和自定义变量

#### 格式4：扩展格式
```csv
recv_time,hex,utf8,温度,湿度,速度,电压,状态
2025-10-24 10:00:00.001,48656C6C6F,Hello,25.5,60,120,3.3,OK
```
- ✅ 任意数量自定义变量
- ✅ 自动识别所有列

### 5.3 CSV读取特性

#### Binary模式读取
```cpp
std::ifstream file(filename, std::ios::binary);
```
- 防止EOF字符(0x1A)截断
- 防止NULL字符(0x00)中断
- 完整读取所有行

#### 特殊字符清理
自动移除以下字符：
- `\r` - 回车符（CR）
- `\0` - NULL字符
- `\x1A` - EOF/SUB字符（Ctrl+Z）

#### 错误处理
- Try-catch保护每行解析
- 单行错误不影响整体
- Debug输出统计信息：
  ```
  总行数: 309
  跳过行数: 0
  错误行数: 0
  ```

### 5.4 使用建议

#### 推荐格式选择
1. **最佳**: `aligned.csv` - 图像+日志完整对齐
2. **次选**: `logs.csv` - 完整日志信息
3. **可用**: `frames_index.csv` - 仅帧索引

#### 列名建议
- ✅ 时间戳列包含 "time"、"iso" 或 "timestamp"
- ✅ Hex列包含 "hex"
- ✅ 文本列包含 "utf" 或 "text"
- ✅ 自定义变量使用描述性名称（如"温度"、"速度"）

#### 注意事项
- ⚠️ 确保至少有一个时间戳列
- ⚠️ 避免列名重复
- ⚠️ 数值列应包含可解析的数字
- ⚠️ 文本使用UTF-8编码

---

## 6. 示波器使用指南

### 6.1 功能特性

- 📈 **多通道显示**: 最多支持8个通道同时显示
- 🎨 **8种配色方案**: 自动为每个通道分配不同颜色
- 🔄 **实时更新**: 随视频播放自动更新波形
- 📐 **自动缩放**: Y轴可自动调整范围适应数据
- ⏱️ **可调时间窗口**: 1-60秒可调节显示范围
- 👁️ **通道开关**: 可单独显示/隐藏每个通道
- 📊 **实时数值显示**: 图例中显示当前值
- 🌐 **完美中文支持**: 使用Pango渲染中文

### 6.2 使用步骤

#### 步骤1：打开示波器
1. 确保已加载CSV文件
2. 点击主窗口的 **"📊 示波器"** 按钮
3. 示波器窗口弹出

#### 步骤2：添加通道
1. 从 **"选择变量添加通道"** 下拉框选择变量
   - 例如：温度、速度、电压
2. 点击 **"添加通道"** 按钮
3. 通道列表中出现新添加的通道
4. 波形区域显示对应颜色的波形

#### 步骤3：播放和观察
1. 在主窗口播放视频或切换帧
2. 示波器自动更新显示对应时刻的数据
3. 观察各通道波形的变化趋势

#### 步骤4：调整设置
- **时间窗口**: 调整滑块改变显示的时间范围
- **自动缩放**: 勾选/取消勾选切换Y轴缩放模式
- **显示/隐藏**: 勾选通道列表中的复选框
- **删除通道**: 选中通道后点击"删除选中"

### 6.3 使用场景

#### 场景1：监控传感器数据
```
变量: 温度、湿度、气压
用途: 观察环境参数随时间变化趋势
颜色: 红色、青色、黄色
```

#### 场景2：分析运动数据
```
变量: 速度、加速度、角度
用途: 分析机器人或车辆的运动状态
颜色: 紫色、绿色、橙色
```

#### 场景3：调试控制系统
```
变量: 目标值、实际值、误差
用途: 观察PID控制效果
颜色: 蓝色、粉色、红色
```

#### 场景4：电源监控
```
变量: 电压、电流、功率
用途: 监控电源系统稳定性
颜色: 青色、黄色、紫色
```

### 6.4 界面说明

#### 波形显示区域
- **网格线**: 10×10网格方便读数
- **颜色编码**: 每个通道不同颜色
- **图例**: 右上角显示通道名和当前值
- **坐标轴**: 
  - X轴：时间（秒）
  - Y轴：数值（自动缩放）

#### 控制面板
- **变量选择**: 下拉框列出所有可用变量
- **通道列表**: 显示已添加的通道，支持开关控制
- **时间窗口**: 滑块调节1-60秒
- **自动缩放**: 复选框开关
- **按钮**:
  - 添加通道
  - 删除选中
  - 清空全部

### 6.5 配色方案

系统预设8种颜色，循环使用：

| 编号 | 颜色 | RGB值 | 用途示例 |
|------|------|-------|---------|
| 1 | 🔴 红色 | (1.0, 0.2, 0.2) | 温度、错误 |
| 2 | 🔵 青色 | (0.2, 0.8, 1.0) | 湿度、状态 |
| 3 | 🟡 黄色 | (1.0, 0.8, 0.2) | 速度、警告 |
| 4 | 🟣 紫色 | (0.6, 0.4, 1.0) | 角度、模式 |
| 5 | 🟢 绿色 | (0.4, 0.9, 0.5) | 电压、正常 |
| 6 | 🟠 橙色 | (1.0, 0.6, 0.3) | 电流、中等 |
| 7 | 🔷 蓝色 | (0.3, 0.6, 0.9) | 压力、深度 |
| 8 | 🌸 粉色 | (1.0, 0.4, 0.6) | 其他变量 |

### 6.6 技术细节

#### 数据解析
- 支持整数、浮点数、科学计数法
- 自动提取字符串中的数字部分
- 容错处理：无法解析的值显示为0

#### 时间轴计算
```cpp
// 假设视频帧率为30fps
double current_time = frame_index / 30.0;

// 时间窗口
double time_min = current_time - time_window;
double time_max = current_time;
```

#### Y轴范围计算
```cpp
// 自动模式：找到所有可见通道的最小最大值
if (auto_scale) {
    y_min = min(all_visible_channels);
    y_max = max(all_visible_channels);
    
    // 添加10%边距
    double range = y_max - y_min;
    if (range < 0.001) {
        // 范围太小，使用固定范围
        y_min = center - 0.5;
        y_max = center + 0.5;
    } else {
        y_min -= range * 0.1;
        y_max += range * 0.1;
    }
}
```

#### 进度条回拖处理
```cpp
// 检测时间倒退（用户往回拖动进度条）
if (!channel.times.empty() && current_time < channel.times.back()) {
    // 清除所有当前时间之后的数据
    while (!channel.times.empty() && channel.times.back() > current_time) {
        channel.times.pop_back();
        channel.values.pop_back();
    }
}
```

### 6.7 性能优化

#### 数据管理
- 使用`std::deque`高效管理数据点
- 仅保留时间窗口内的数据
- 自动清理超出范围的旧数据

#### 渲染优化
- 按需刷新绘图，避免过度渲染
- 使用Cairo硬件加速
- Pango文字缓存

### 6.8 故障排除

#### 问题1：示波器窗口是空的
**原因**: 未加载CSV或未添加通道
**解决**:
1. 确认已加载CSV文件（主窗口显示日志信息）
2. 确认CSV中有数值变量
3. 尝试添加通道

#### 问题2：波形不更新
**原因**: 视频未播放或通道被隐藏
**解决**:
1. 确认视频正在播放或切换帧
2. 检查通道复选框是否勾选
3. 检查时间窗口设置是否合理

#### 问题3：数值显示为0
**原因**: CSV中对应列不包含数值
**解决**:
1. 检查CSV中对应列是否包含数值
2. 确认变量名称选择正确
3. 使用文本编辑器查看CSV原始数据

#### 问题4：中文显示为方块
**原因**: 系统字体缺失
**解决**:
1. 确认系统已安装Microsoft YaHei或SimHei字体
2. 修改`oscilloscope.cpp`中的字体设置
3. 重新编译程序

#### 问题5：关闭后无法再打开
**状态**: ✅ 已修复
**原理**: 窗口隐藏而不是销毁

---

## 7. 问题修复记录

### 7.1 示波器修复历史

#### 修复版本1（初版）
**日期**: 2025-10-24

**修复的问题**:
1. ✅ 中文乱码
   - 添加UTF-8编码设置
   - 使用`g_setenv("LANG", "zh_CN.UTF-8", TRUE)`

2. ✅ 无法添加变量
   - 添加CSV路径保存功能
   - 改进变量加载逻辑
   - 添加友好提示信息

#### 修复版本2（Pango）
**日期**: 2025-10-24

**修复的问题**:
1. ✅ **中文乱码彻底解决**
   - **原因**: Cairo的`cairo_show_text()`不支持中文
   - **方案**: 使用Pango库替代Cairo绘制文字
   - **实现**:
     ```cpp
     // 旧方案（乱码）
     cairo_show_text(cr, "温度");
     
     // 新方案（正常）
     PangoLayout *layout = pango_cairo_create_layout(cr);
     PangoFontDescription *desc = pango_font_description_from_string("Microsoft YaHei 10");
     pango_layout_set_font_description(layout, desc);
     pango_layout_set_text(layout, "温度", -1);
     pango_cairo_show_layout(cr, layout);
     ```

2. ✅ **关闭后无法再打开**
   - **原因**: 窗口被`gtk_widget_destroy()`销毁
   - **方案**: 隐藏窗口而不是销毁
   - **实现**:
     ```cpp
     // 窗口关闭事件：返回TRUE阻止默认销毁行为
     gboolean onWindowDelete(GtkWidget *widget, GdkEvent *event, gpointer user_data) {
         gtk_widget_hide(widget);  // 隐藏而不是销毁
         return TRUE;  // 阻止默认行为
     }
     ```

**修改的文件**:
- `src/oscilloscope.cpp` - 所有文字渲染函数
- `src/oscilloscope.h` - 添加窗口事件处理声明

**修改的函数**:
- `buildUI()` - CSS字体设置
- `drawLegend()` - Pango渲染图例
- `drawAxisLabels()` - Pango渲染坐标轴
- `drawChannels()` - Pango渲染提示文字
- `onWindowDelete()` - 新增窗口关闭事件处理

#### 修复版本3（多通道）
**日期**: 2025-10-24

**修复的问题**:
1. ✅ **多通道Y轴范围错乱**
   - **原因**: 不同通道数值范围差异大，Y轴计算不当
   - **方案**: 改进Y轴范围计算逻辑
   - **实现**:
     ```cpp
     double range = y_max - y_min;
     if (range < 0.001) {
         // 范围太小，使用固定范围
         double center = (y_max + y_min) / 2.0;
         y_min = center - 0.5;
         y_max = center + 0.5;
     } else {
         // 添加10%边距
         y_min -= range * 0.1;
         y_max += range * 0.1;
     }
     
     // 确保最终范围有效
     double y_range = y_max - y_min;
     if (y_range < 0.001) {
         y_range = 1.0;
         y_max = y_min + y_range;
     }
     ```

2. ✅ **进度条回拖波形错乱**
   - **原因**: 时间倒退时新旧数据混合
   - **方案**: 检测时间倒退并清除未来数据
   - **实现**:
     ```cpp
     // 检测时间倒退（用户往回拖动进度条）
     if (!channel.times.empty() && current_time < channel.times.back()) {
         // 清除所有当前时间之后的数据
         while (!channel.times.empty() && channel.times.back() > current_time) {
             channel.times.pop_back();
             channel.values.pop_back();
         }
     }
     ```

**修改的文件**:
- `src/oscilloscope.cpp` - Y轴计算和数据管理逻辑

**修改的函数**:
- `drawChannels()` - Y轴范围计算
- `drawAxisLabels()` - 同步Y轴范围
- `updateChannelData()` - 时间倒退检测

### 7.2 CSV读取器改进

#### 改进版本1（Binary模式）
**日期**: 2025-10-23

**解决的问题**:
- ✅ CSV只能读取部分记录（84/306行）
- ✅ 特殊字符导致读取中断

**改进内容**:
1. **Binary模式读取**
   ```cpp
   // 旧方案（文本模式，遇到EOF截断）
   std::ifstream file(filename);
   
   // 新方案（二进制模式，完整读取）
   std::ifstream file(filename, std::ios::binary);
   ```

2. **特殊字符清理**
   ```cpp
   // 移除会导致解析问题的字符
   line.erase(std::remove(line.begin(), line.end(), '\r'), line.end());  // CR
   line.erase(std::remove(line.begin(), line.end(), '\0'), line.end());  // NULL
   line.erase(std::remove(line.begin(), line.end(), '\x1A'), line.end()); // EOF
   ```

3. **错误容忍**
   ```cpp
   try {
       // 解析CSV行
       parseCSVLine(line);
   } catch (const std::exception& e) {
       errorLines++;
       continue;  // 单行错误不影响整体
   }
   ```

**结果**:
- 从84行 → 309行完整读取
- 容错能力增强
- 兼容性提升

#### 改进版本2（智能列检测）
**日期**: 2025-10-24

**新增功能**:
- ✅ 自动识别任意列顺序
- ✅ 中英文列名匹配
- ✅ 任意数量自定义变量

**实现原理**:
```cpp
// 扫描表头，识别列类型
for (size_t i = 0; i < headers.size(); ++i) {
    std::string lower_header = toLowerCase(headers[i]);
    
    if (contains(lower_header, "time") || 
        contains(lower_header, "iso") ||
        contains(lower_header, "timestamp")) {
        time_col = i;  // 时间戳列
    }
    else if (contains(lower_header, "hex")) {
        hex_col = i;  // Hex数据列
    }
    else if (contains(lower_header, "utf") || 
             contains(lower_header, "text")) {
        utf8_col = i;  // UTF-8文本列
    }
    else {
        var_names.push_back(headers[i]);  // 自定义变量
        var_cols.push_back(i);
    }
}
```

### 7.3 其他修复

#### UI响应速度优化
- 使用`gtk_widget_queue_draw()`替代强制刷新
- 减少不必要的重绘次数
- 优化图像缩放算法

#### 内存泄漏修复
- 正确释放GdkPixbuf对象
- 使用RAII管理Cairo资源
- 避免Pango对象泄漏

#### 编译警告消除
- 修复所有`-Wdeprecated-declarations`警告
- 替换过时的GTK API
- 更新到GTK3标准用法

---

## 8. 技术文档

### 8.1 架构设计

#### 模块划分
```
ImageProcess
├── GUI模块 (main.cpp)
│   ├── 窗口管理
│   ├── 事件处理
│   └── 界面更新
├── 示波器模块 (oscilloscope.cpp)
│   ├── 波形绘制
│   ├── 通道管理
│   └── 数据更新
├── CSV读取模块 (csv_reader.cpp)
│   ├── 文件解析
│   ├── 列检测
│   └── 数据提取
├── 图像处理模块 (processor.c)
│   ├── 二值化
│   ├── 缩放
│   └── 数组转换
└── 视频工具 (video_processor.cpp)
    ├── 视频解码
    ├── 帧提取
    └── 批处理
```

#### 数据流
```
CSV文件 → CSVReader → LogRecord
                          ↓
                    OscilloscopeWindow
                          ↓
                    Channel Data (deque)
                          ↓
                    Cairo Drawing → GTK显示
```

### 8.2 核心数据结构

#### LogRecord（日志记录）
```cpp
struct LogRecord {
    std::string timestamp;           // 时间戳
    std::string hex_data;            // Hex数据
    std::string utf8_text;           // UTF-8文本
    std::map<std::string, std::string> variables;  // 自定义变量
};
```

#### ChannelData（示波器通道）
```cpp
struct ChannelData {
    std::string name;                // 通道名称
    std::string variable_name;       // 变量名
    std::deque<double> times;        // 时间点
    std::deque<double> values;       // 数值
    double min_value;                // 最小值
    double max_value;                // 最大值
    Color color;                     // 颜色
    bool visible;                    // 是否可见
};
```

#### 图像数组
```c
uint8_t** original_bi_image;  // 原始二值图像 [120][188]
uint8_t** imo;                // 处理后的IMO数组 [120][188]
```

### 8.3 关键算法

#### CSV解析算法
```cpp
std::vector<std::string> parseCSVLine(const std::string& line) {
    std::vector<std::string> fields;
    std::string field;
    bool in_quotes = false;
    
    for (char c : line) {
        if (c == '"') {
            in_quotes = !in_quotes;
        } else if (c == ',' && !in_quotes) {
            fields.push_back(field);
            field.clear();
        } else {
            field += c;
        }
    }
    fields.push_back(field);
    
    return fields;
}
```

#### 数值解析算法
```cpp
double parseValue(const std::string& str) {
    // 尝试直接转换
    try {
        return std::stod(str);
    } catch (...) {
        // 提取字符串中的数字
        std::regex number_regex("[+-]?\\d+\\.?\\d*([eE][+-]?\\d+)?");
        std::smatch match;
        if (std::regex_search(str, match, number_regex)) {
            return std::stod(match[0]);
        }
        return 0.0;
    }
}
```

#### 图像缩放算法（双线性插值）
```cpp
void resizeImage(const uint8_t* src, int src_w, int src_h,
                 uint8_t* dst, int dst_w, int dst_h) {
    for (int y = 0; y < dst_h; y++) {
        for (int x = 0; x < dst_w; x++) {
            // 映射到源图像坐标
            float src_x = x * (float)src_w / dst_w;
            float src_y = y * (float)src_h / dst_h;
            
            // 双线性插值
            int x0 = (int)src_x;
            int y0 = (int)src_y;
            int x1 = std::min(x0 + 1, src_w - 1);
            int y1 = std::min(y0 + 1, src_h - 1);
            
            float dx = src_x - x0;
            float dy = src_y - y0;
            
            uint8_t v00 = src[y0 * src_w + x0];
            uint8_t v01 = src[y0 * src_w + x1];
            uint8_t v10 = src[y1 * src_w + x0];
            uint8_t v11 = src[y1 * src_w + x1];
            
            float v0 = v00 * (1 - dx) + v01 * dx;
            float v1 = v10 * (1 - dx) + v11 * dx;
            float v = v0 * (1 - dy) + v1 * dy;
            
            dst[y * dst_w + x] = (uint8_t)v;
        }
    }
}
```

### 8.4 性能优化

#### 内存管理
```cpp
// 使用智能指针管理资源
std::unique_ptr<uint8_t[]> buffer(new uint8_t[size]);

// RAII包装GTK对象
struct GObjectDeleter {
    void operator()(GObject* obj) {
        if (obj) g_object_unref(obj);
    }
};
using GObjectPtr = std::unique_ptr<GObject, GObjectDeleter>;
```

#### 绘图优化
```cpp
// 只在数据变化时刷新
if (data_changed) {
    gtk_widget_queue_draw(drawing_area);
}

// 使用双缓冲
cairo_surface_t* surface = cairo_image_surface_create(
    CAIRO_FORMAT_ARGB32, width, height);
cairo_t* cr = cairo_create(surface);
// ... 绘制到surface
cairo_set_source_surface(window_cr, surface, 0, 0);
cairo_paint(window_cr);
cairo_destroy(cr);
cairo_surface_destroy(surface);
```

#### 数据结构选择
```cpp
// 使用deque高效管理时序数据
std::deque<double> times;    // O(1)头尾插入删除
std::deque<double> values;

// 只保留时间窗口内的数据
while (!times.empty() && 
       (current_time - times.front()) > time_window) {
    times.pop_front();
    values.pop_front();
}
```

### 8.5 错误处理

#### 异常安全
```cpp
// RAII确保资源释放
class FileGuard {
    std::FILE* fp;
public:
    FileGuard(const char* path, const char* mode)
        : fp(std::fopen(path, mode)) {}
    ~FileGuard() { if (fp) std::fclose(fp); }
    std::FILE* get() { return fp; }
};

// 使用
FileGuard file("data.csv", "r");
if (!file.get()) {
    throw std::runtime_error("Failed to open file");
}
```

#### 错误传播
```cpp
// 使用返回码
bool loadCSV(const char* filename) {
    try {
        // ... 加载逻辑
        return true;
    } catch (const std::exception& e) {
        std::cerr << "Error: " << e.what() << std::endl;
        return false;
    }
}

// 调用处检查
if (!loadCSV(path)) {
    showErrorDialog("Failed to load CSV");
}
```

### 8.6 调试技巧

#### 启用调试输出
```cpp
#define DEBUG_MODE 1

#if DEBUG_MODE
    #define DEBUG_LOG(fmt, ...) \
        fprintf(stderr, "[DEBUG] " fmt "\n", ##__VA_ARGS__)
#else
    #define DEBUG_LOG(fmt, ...)
#endif

// 使用
DEBUG_LOG("Loading CSV: %s", filename);
DEBUG_LOG("Found %d records", count);
```

#### GDB调试
```bash
# 编译时添加调试符号
cmake -DCMAKE_BUILD_TYPE=Debug -S . -B build
cmake --build build

# 使用GDB调试
gdb ./install/bin/imageprocessor

# GDB命令
(gdb) break oscilloscope.cpp:123
(gdb) run
(gdb) print variable_name
(gdb) backtrace
```

#### Valgrind内存检查
```bash
# 检查内存泄漏
valgrind --leak-check=full --show-leak-kinds=all \
    ./install/bin/imageprocessor

# 检查内存错误
valgrind --tool=memcheck ./install/bin/imageprocessor
```

---

## 9. 常见问题

### 9.1 编译问题

#### Q1: 提示"gtk/gtk.h: 没有那个文件或目录"
**原因**: 未安装GTK开发库

**解决方案**:
```bash
# Ubuntu/Debian
sudo apt-get install libgtk-3-dev

# Windows (MSYS2)
pacman -S mingw-w64-x86_64-gtk3
```

#### Q2: 编译时出现"undefined reference to `cv::imread'"
**原因**: 未找到OpenCV库

**解决方案**:
```bash
# Ubuntu/Debian
sudo apt-get install libopencv-dev

# Windows (MSYS2)
pacman -S mingw-w64-x86_64-opencv

# CMake配置
cmake -DOpenCV_DIR=/path/to/opencv -S . -B build
```

#### Q3: MinGW编译卡住不动
**原因**: 并行编译导致资源不足

**解决方案**:
```bash
# 限制并行任务数
cmake --build build -j2

# 或使用单线程
cmake --build build -j1
```

### 9.2 CSV加载问题

#### Q4: CSV只加载了部分记录（如84/309行）
**状态**: ✅ 已修复（v2.0版本）

**解决方案**:
1. 更新到最新版本
2. 重新编译程序
3. 确认使用Binary模式读取

#### Q5: CSV列名不识别
**原因**: 列名不符合识别规则

**解决方案**:
- 时间戳列包含"time"、"iso"或"timestamp"
- Hex列包含"hex"
- UTF-8列包含"utf"或"text"
- 或手动修改列名

#### Q6: 自定义变量不显示
**检查**:
1. CSV第一行是否为列名（标题行）
2. 列名是否与数据对齐
3. 控制台是否输出"成功加载CSV"

**调试**:
```cpp
// 添加DEBUG输出查看识别的列
std::cout << "Time column: " << time_col << std::endl;
std::cout << "Hex column: " << hex_col << std::endl;
std::cout << "UTF8 column: " << utf8_col << std::endl;
std::cout << "Variable columns: ";
for (size_t i : var_cols) {
    std::cout << i << " ";
}
std::cout << std::endl;
```

### 9.3 示波器问题

#### Q7: 示波器窗口是空的
**原因**: 未加载CSV或未添加通道

**解决方案**:
1. 确认主窗口已加载CSV
2. 点击"添加通道"按钮
3. 查看通道列表是否有内容

#### Q8: 波形不更新
**原因**: 视频未播放或通道被隐藏

**解决方案**:
1. 播放视频或切换帧
2. 检查通道复选框是否勾选
3. 检查时间窗口设置（建议5-10秒）

#### Q9: 中文显示为方块（□□□）
**状态**: ✅ 已修复（v2.0版本）

**如仍有问题**:
1. 确认系统已安装Microsoft YaHei字体
2. 检查编译版本是否为v2.0+
3. 尝试修改字体设置：
   ```cpp
   // oscilloscope.cpp中修改
   PangoFontDescription *desc = 
       pango_font_description_from_string("SimHei 10");
   ```

#### Q10: 关闭后无法再打开
**状态**: ✅ 已修复（v2.0版本）

**确认修复**:
1. 检查是否使用最新版本
2. 重新编译程序
3. 测试：打开→关闭→再打开

#### Q11: 进度条回拖波形错乱
**状态**: ✅ 已修复（v3.0版本）

**原理**: 检测时间倒退并清除未来数据

#### Q12: 多通道显示时Y轴范围异常
**状态**: ✅ 已修复（v3.0版本）

**原理**: 改进Y轴范围计算，处理边界情况

### 9.4 图像处理问题

#### Q13: 图像显示模糊
**原因**: 缩放算法导致

**说明**: 
- 使用双线性插值保证图像质量
- 小图放大时自然会有模糊
- 建议使用原始尺寸接近120×188的图像

#### Q14: 图像颜色反转
**原因**: 二值化阈值问题

**解决方案**:
修改`image.c`中的阈值：
```c
// 当前阈值128
if (pixel > 128) {
    binary_value = 255;  // 白色
} else {
    binary_value = 0;    // 黑色
}
```

#### Q15: PNG路径找不到
**原因**: 相对路径问题

**解决方案**:
1. 使用绝对路径
2. 或将PNG文件放在程序同目录下
3. 确认CSV中的路径正确

### 9.5 性能问题

#### Q16: 程序运行卡顿
**可能原因**:
1. 图像文件过大
2. CSV记录过多
3. 示波器通道过多

**优化建议**:
1. 减小图像分辨率
2. 减少示波器通道数量
3. 减小时间窗口（5-10秒）
4. 关闭不必要的窗口

#### Q17: 内存占用过高
**原因**: 大量图像数据缓存

**解决方案**:
1. 不要同时打开过多帧选择器
2. 定期关闭不使用的窗口
3. 减少视频帧数

### 9.6 平台特定问题

#### Q18: Windows下DLL缺失
**错误**: "无法找到msys-2.0.dll"等

**解决方案**:
```powershell
# 添加MSYS2路径到PATH
$env:PATH = "C:\msys64\mingw64\bin;$env:PATH"

# 或使用run.bat脚本
.\run.bat
```

#### Q19: Linux下字体渲染问题
**解决方案**:
```bash
# 安装中文字体
sudo apt-get install fonts-wqy-microhei fonts-wqy-zenhei

# 刷新字体缓存
fc-cache -fv
```

#### Q20: Mac下编译失败
**原因**: Mac使用Clang编译器，部分选项不同

**解决方案**:
```bash
# 使用Homebrew安装依赖
brew install gtk+3 opencv pkg-config

# 设置PKG_CONFIG_PATH
export PKG_CONFIG_PATH="/usr/local/lib/pkgconfig:$PKG_CONFIG_PATH"

# 编译
cmake -S . -B build
cmake --build build
```

### 9.7 获取帮助

如遇到文档未覆盖的问题：

1. **查看控制台输出**: 程序会输出详细的调试信息
2. **检查日志文件**: `log/`目录（如有）
3. **GitHub Issues**: 提交问题到项目仓库
4. **邮件联系**: 包含错误信息和环境描述

---

## 10. 相关项目

### 10.1 UDP上位机

**项目地址**: [https://github.com/Dengdxx/udp](https://github.com/Dengdxx/udp)

**功能特点**:
- 实时UDP图像和日志传输
- 自动生成PNG序列和CSV数据
- 支持自定义协议和变量配置
- v2.2版本已修复CSV数据完整性问题

**与ImageProcess的关系**:
- ImageProcess是UDP上位机的配套分析工具
- 读取UDP生成的CSV和PNG文件
- 提供更强大的数据可视化功能

### 10.2 完整工作流程

```
┌──────────────┐
│  STM32设备   │
│              │
│ • 图像采集   │
│ • 压缩传输   │
│ • 日志打包   │
└──────┬───────┘
       │ WiFi/UDP
       ↓
┌──────────────┐
│ UDP上位机    │
│              │
│ • UDP接收    │
│ • PNG保存    │
│ • CSV记录    │
│ • 实时显示   │
└──────┬───────┘
       │ aligned.csv
       │ frames_png/
       ↓
┌──────────────┐
│ ImageProcess │
│              │
│ • CSV智能读取│
│ • 平铺帧选择 │
│ • 数据分析   │
│ • 波形显示   │
└──────────────┘
```

### 10.3 推荐配置

#### STM32端
```c
// 图像配置
#define IMAGE_FORMAT  COMPRESSED_BINARY  // 压缩二值(1位)
#define IMAGE_HEIGHT  120
#define IMAGE_WIDTH   188
#define COMPRESSION   8_TO_1  // 8:1压缩比

// 通信配置
#define SPI_SPEED     4000000  // 4Mbps（推荐）
#define UDP_PORT      8080
#define PACKET_SIZE   1024
```

#### UDP上位机
```python
# 配置
IMAGE_FORMAT = "压缩二值(1位)"
FIXED_H = 120
FIXED_W = 188
FRAME_HEADER = "A0FFFFA0"
FRAME_FOOTER = "B0B00A0D"

# 输出
PNG_DIR = "frames_png"
LOG_CSV = "logs.csv"
FRAME_INDEX_CSV = "frames_index.csv"
ALIGNED_CSV = "aligned.csv"  # 推荐用于ImageProcess
```

#### ImageProcess
```
加载文件: aligned.csv
窗口大小: 1400×900（推荐）
示波器:
  - 时间窗口: 5-10秒
  - 通道数: 3-5个
  - 刷新率: 10Hz
```

### 10.4 数据流说明

#### 1. STM32 → UDP上位机
```
UDP数据包格式:
[帧头: A0FFFFA0]
[图像数据: 900字节（120×188÷8）]
[帧尾: B0B00A0D]

或

[帧头: BB66]
[日志数据: 变长]
[帧尾: 0D0A]
```

#### 2. UDP上位机 → ImageProcess
```
PNG文件:
frames_png/
├── frame_000001.png  (120×188)
├── frame_000002.png
└── ...

CSV文件:
aligned.csv
├── frame_id
├── png_path
├── frame_host_iso
├── log_text_utf8
├── 温度
├── 速度
└── ...（自定义变量）
```

#### 3. ImageProcess处理
```
1. CSV智能读取
   ↓
2. 自动识别列
   ↓
3. 加载PNG图像
   ↓
4. 显示+分析
   ↓
5. 示波器可视化
```

### 10.5 版本兼容性

| ImageProcess | UDP上位机 | 说明 |
|-------------|-----------|------|
| v3.0 | v2.2+ | ✅ 完全兼容（推荐） |
| v2.0 | v2.1+ | ✅ 兼容，建议升级 |
| v1.0 | v2.0 | ⚠️ CSV可能不完整 |

**升级建议**:
1. 优先升级UDP上位机到v2.2
2. 删除旧的CSV文件
3. 重新采集数据
4. 使用ImageProcess v3.0分析

---

## 附录

### A. 快速参考

#### A.1 快捷键（未来计划）
```
Ctrl+O    - 打开CSV文件
Ctrl+S    - 保存设置
Ctrl+W    - 关闭窗口
Ctrl+Q    - 退出程序
Space     - 播放/暂停
←/→       - 上一帧/下一帧
Ctrl+Z    - 撤销
Ctrl+Y    - 重做
```

#### A.2 命令行参数（未来计划）
```bash
# 直接加载CSV
./imageprocessor --csv=aligned.csv

# 指定帧
./imageprocessor --csv=data.csv --frame=100

# 批处理模式
./imageprocessor --batch --input=input/ --output=output/
```

### B. 示例数据

#### B.1 测试CSV（完整版）
```csv
frame_id,host_recv_iso,png_path,h,w,log_text_utf8,温度,速度,电压,湿度
1,2025-10-24 10:00:00.001,frames_png/frame_000001.png,120,188,Hello,25.5,120,3.3,60
2,2025-10-24 10:00:00.034,frames_png/frame_000002.png,120,188,World,26.1,125,3.2,62
3,2025-10-24 10:00:00.067,frames_png/frame_000003.png,120,188,Test,25.8,118,3.4,61
4,2025-10-24 10:00:00.101,frames_png/frame_000004.png,120,188,Data,27.2,130,3.5,65
5,2025-10-24 10:00:00.134,frames_png/frame_000005.png,120,188,Info,26.5,122,3.3,63
```

#### B.2 Python生成测试数据
```python
import csv
import random
from datetime import datetime, timedelta

# 生成测试CSV
def generate_test_csv(filename, num_records=100):
    with open(filename, 'w', newline='', encoding='utf-8') as f:
        writer = csv.writer(f)
        
        # 表头
        writer.writerow([
            'frame_id', 'host_recv_iso', 'png_path', 'h', 'w',
            'log_text_utf8', '温度', '速度', '电压', '湿度'
        ])
        
        # 数据
        base_time = datetime.now()
        for i in range(1, num_records + 1):
            timestamp = base_time + timedelta(milliseconds=i*33)
            writer.writerow([
                i,
                timestamp.strftime('%Y-%m-%d %H:%M:%S.%f')[:-3],
                f'frames_png/frame_{i:06d}.png',
                120, 188,
                f'Log{i}',
                round(25 + random.uniform(-2, 2), 1),  # 温度
                random.randint(100, 150),              # 速度
                round(3.3 + random.uniform(-0.2, 0.2), 2),  # 电压
                random.randint(50, 70)                 # 湿度
            ])

if __name__ == '__main__':
    generate_test_csv('test_data.csv', 100)
    print('Generated test_data.csv with 100 records')
```

### C. 致谢

感谢以下开源项目：
- **GTK+**: 跨平台GUI工具包
- **Cairo**: 2D图形库
- **Pango**: 国际化文本渲染
- **OpenCV**: 计算机视觉库
- **CMake**: 跨平台构建系统

### D. 许可证

本项目采用 [MIT License](LICENSE)。

### E. 贡献指南

欢迎提交Issue和Pull Request！

**提交Issue时请包含**:
- 操作系统和版本
- 编译器版本
- 详细的错误信息
- 复现步骤

**提交PR时请确保**:
- 代码风格一致
- 添加必要的注释
- 通过编译测试
- 更新相关文档

### F. 更新日志

#### v3.0 (2025-10-24)
- ✅ 修复示波器进度条回拖波形错乱
- ✅ 修复多通道Y轴范围计算
- ✅ 改进时间倒退检测
- ✅ 优化坐标映射算法

#### v2.0 (2025-10-24)
- ✅ 完美支持中文显示（Pango）
- ✅ 修复示波器关闭后无法再打开
- ✅ Binary模式CSV读取
- ✅ 智能列检测

#### v1.0 (2025-10-23)
- 初始版本发布
- 基本图像处理功能
- CSV读取功能
- 示波器基础功能

---

## 联系方式

- **GitHub**: [https://github.com/Dengdxx/ImageProcess](https://github.com/Dengdxx/ImageProcess)
- **Issues**: [提交问题](https://github.com/Dengdxx/ImageProcess/issues)
- **Wiki**: [查看Wiki](https://github.com/Dengdxx/ImageProcess/wiki)

---

**文档结束** | 最后更新: 2025年10月24日 | 版本: v3.0

---

