# CSV 格式说明

## ✨ 新特性：自动列检测

ImageProcess 现在支持**自动检测 CSV 列格式**，无需固定列顺序！

## 🔍 智能列检测规则

程序会自动识别以下列（不区分大小写）：

### 1. 时间戳列
包含以下关键词之一的列会被识别为时间戳：
- `time`
- `iso`
- `timestamp`

示例：`host_recv_iso`, `timestamp`, `recv_time`

### 2. Hex 数据列
包含 `hex` 关键词的列：
- `log_text_hex`
- `hex_data`
- `data_hex`

### 3. UTF-8 文本列
包含以下关键词之一的列：
- `utf`
- `text`

示例：`log_text_utf8`, `text`, `utf8_data`

### 4. 自定义变量列
所有其他列都会被识别为自定义变量，在日志显示区域展示。

## 📋 支持的 CSV 格式

### 格式 1：标准格式（UDP 上位机默认）
```csv
host_recv_iso,log_text_hex,log_text_utf8,温度,速度
2025-10-23 22:19:26.075,48656C6C6F,Hello,25.5,120
```

### 格式 2：任意列顺序
```csv
frame_id,host_recv_iso,png_path,h,w
1,2025-10-23 22:19:26.075,path/to/image.png,120,188
```
- `host_recv_iso` → 时间戳
- `frame_id`, `png_path`, `h`, `w` → 自定义变量

### 格式 3：只有部分列
```csv
timestamp,log_message
2025-10-23 22:19:26.075,Hello World
```

### 格式 4：扩展格式
```csv
recv_time,hex,utf8,温度,湿度,速度,电压,状态
2025-10-23 22:19:26.075,48656C6C6F,Hello,25.5,60,120,3.3,OK
```

## ⚙️ 工作原理

1. **表头扫描**：读取第一行，识别列名
2. **智能匹配**：根据关键词匹配时间戳、hex、utf8列
3. **自动提取**：其余列作为自定义变量
4. **容错处理**：
   - 如果某列缺失，该字段为空
   - 不再要求最少3列
   - 支持任意列数

## 🎯 实际应用

### 场景 1：UDP 采集的 logs.csv
```csv
host_recv_iso,log_text_hex,log_text_utf8,温度,速度
```
✅ 完美支持！

### 场景 2：frames_index.csv（不推荐但能加载）
```csv
frame_id,host_recv_iso,png_path,h,w
```
✅ 可以加载，`host_recv_iso` 作为时间戳，其他列作为变量

### 场景 3：aligned.csv（现在支持！）
```csv
frame_id,png_path,frame_host_iso,h,w,log_host_iso,log_stm32_ts_us,log_text_utf8,host_dt_diff_ms
```
✅ 可以加载：
- `frame_host_iso` 或 `log_host_iso` → 时间戳
- `log_text_utf8` → UTF-8文本
- 其他列 → 自定义变量

## 💡 使用建议

1. **推荐使用 `logs.csv`**（最完整的日志信息）
2. **确保至少有一个时间戳列**（否则使用第1列）
3. **列名尽量包含关键词**（time/hex/utf8）以便自动识别
4. **自定义变量列会自动显示**在日志面板

## 🔧 技术细节

- 不区分大小写匹配
- 支持部分匹配（例如 `host_recv_iso` 包含 `iso`）
- 如果没有检测到关键列，使用前3列作为默认
- 空字段不会导致错误
- 支持带引号的CSV字段

## ⚠️ 注意事项

虽然现在支持任意格式，但为了获得最佳显示效果，建议 CSV 包含：
- ✅ 时间戳信息（用于显示"⏰ 时间戳"）
- ✅ UTF-8文本（用于显示"📝 UTF-8文本"）
- ✅ Hex数据（用于显示"🔧 Hex数据"）
- ✅ 自定义变量（用于显示"📊 日志变量"）
