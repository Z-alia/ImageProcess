# 动态日志系统 - 数组支持使用示例

## ✅ 新增功能

动态日志系统现在支持完整输出数组！数组会以 JSON 格式存储在 CSV 文件中：`[1,2,3,4,5]`

## 📋 支持的数组类型

- `LOG_TYPE_INT8_ARRAY` / `LOG_TYPE_UINT8_ARRAY`
- `LOG_TYPE_INT16_ARRAY` / `LOG_TYPE_UINT16_ARRAY`
- `LOG_TYPE_INT32_ARRAY` / `LOG_TYPE_UINT32_ARRAY`
- `LOG_TYPE_FLOAT_ARRAY`
- `LOG_TYPE_DOUBLE_ARRAY`

## 💡 使用方法

### C 接口

```c
#include "dynamic_log.h"

// 示例1：记录 uint8_t 数组（边界坐标）
uint8_t left_borders[120];  // 左边界数组
for (int i = 0; i < 120; i++) {
    left_borders[i] = get_left_boundary(i);
}
log_add_uint8_array("left_borders", left_borders, 120, -1);

// 示例2：记录 int16_t 数组（传感器数据）
int16_t sensor_data[8] = {100, 250, 340, 280, 190, 150, 120, 100};
log_add_int16_array("sensors", sensor_data, 8, -1);

// 示例3：记录 float 数组（权重参数）
float weights[5] = {0.1f, 0.25f, 0.3f, 0.25f, 0.1f};
log_add_float_array("filter_weights", weights, 5, -1);

// 示例4：记录部分数组（只记录前N个元素）
uint8_t large_buffer[1000];
// 只记录前50个
log_add_uint8_array("buffer_preview", large_buffer, 50, -1);
```

### C++ 接口

```cpp
#include "dynamic_log.h"

// 使用 DynamicLogManager 单例
auto& logger = DynamicLogManager::getInstance();

// 记录数组
std::vector<float> data = {1.5f, 2.3f, 4.7f, 3.2f};
logger.addArray("float_data", LOG_TYPE_FLOAT_ARRAY, data.data(), data.size(), -1);

// 或使用通用接口
int32_t values[] = {10, 20, 30, 40, 50};
logger.addArray("int_values", LOG_TYPE_INT32_ARRAY, values, 5, -1);
```

## 📊 CSV 输出格式

假设你的边界跟踪代码这样使用：

```c
// 在 image.c 中
void process_original_to_imo(uint8_t* original, uint8_t* imo, int w, int h) {
    log_set_current_frame(g_frame_index);
    
    uint8_t left_bounds[120];
    uint8_t right_bounds[120];
    
    // ... 边界提取逻辑 ...
    
    // 记录完整的边界数组
    log_add_uint8_array("left_bounds", left_bounds, 120, -1);
    log_add_uint8_array("right_bounds", right_bounds, 120, -1);
    
    // 同时记录单个关键点（兼容之前的代码）
    log_add_uint8("left_top", left_bounds[0], -1);
    log_add_uint8("right_top", right_bounds[0], -1);
}
```

生成的 CSV 文件会包含：

```csv
frame_id,host_recv_iso,png_path,h,w,breakk,left_bounds,right_bounds,left_top,right_top
1,2025-10-25T10:30:00,frame_000001.png,120,188,0,"[0,1,2,3,4,...]","[187,186,185,184,...]",0,187
2,2025-10-25T10:30:01,frame_000002.png,120,188,0,"[0,0,1,2,3,...]","[187,187,186,185,...]",0,187
```

## 🎯 实际应用场景

### 1. 完整边界可视化
```c
// 记录整行的左右边界
log_add_uint8_array("left_borders_full", left_array, 120, -1);
log_add_uint8_array("right_borders_full", right_array, 120, -1);
```

### 2. 多传感器数据
```c
// 记录所有传感器读数
uint16_t adc_values[16];
read_all_sensors(adc_values);
log_add_uint16_array("all_sensors", adc_values, 16, -1);
```

### 3. 灰度直方图
```c
// 记录图像直方图
uint32_t histogram[256];
calculate_histogram(image, histogram);
log_add_uint32_array("gray_histogram", histogram, 256, -1);
```

### 4. 路径规划轨迹
```c
// 记录规划的路径点
int16_t path_x[50], path_y[50];
plan_path(path_x, path_y, 50);
log_add_int16_array("path_x", path_x, 50, -1);
log_add_int16_array("path_y", path_y, 50, -1);
```

## ⚠️ 注意事项

1. **数组大小限制**：建议数组长度不超过 1000 个元素，避免 CSV 单元格过大
2. **CSV 兼容性**：数组内容会被转义为字符串，用引号包围：`"[1,2,3]"`
3. **性能考虑**：大数组会增加 CSV 写入时间，建议只在需要时记录
4. **数据类型**：确保使用正确的数组类型函数（如 `uint8_t` 用 `log_add_uint8_array`）

## 🔍 查看和分析数组数据

### 在 Python 中解析
```python
import pandas as pd
import json

df = pd.read_csv('frames_index.csv')

# 解析数组列
df['left_bounds'] = df['left_bounds'].apply(json.loads)

# 绘制第一帧的边界
import matplotlib.pyplot as plt
plt.plot(df.loc[0, 'left_bounds'], label='Left')
plt.plot(df.loc[0, 'right_bounds'], label='Right')
plt.legend()
plt.show()
```

### 在 Excel 中查看
数组会显示为 `[1,2,3,4,...]`，可以复制到文本编辑器查看完整内容。

## 📝 更新日志

- **2025-10-25**: 添加完整数组支持
  - 新增 8 种数组类型
  - 新增 `log_add_*_array()` 系列函数
  - 数组以 JSON 格式存储在 CSV 中
  - 支持 C/C++ 双接口
  - 完全向后兼容现有代码

## 🚀 快速开始

1. **在 image.c 中使用**：
```c
log_add_uint8_array("my_array", array_data, array_length, -1);
```

2. **编译项目**：
```bash
.\build.bat
```

3. **运行 GUI 并加载 CSV**：
- 数组数据会自动保存到 CSV
- 可以在示波器中查看数组的各个元素（需手动解析）

4. **使用 Python 分析**：
```python
import pandas as pd
df = pd.read_csv('frames_index.csv')
print(df['left_bounds'])  # 查看数组内容
```
