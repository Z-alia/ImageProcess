# 🔧 示波器修复 - 版本2

## ✅ 已修复的问题

### 1. 中文乱码问题
**修复方法**：使用Pango库替代Cairo直接绘制文字

**改进内容**：
- ✅ 设置全局CSS样式，使用Microsoft YaHei字体
- ✅ 使用Pango渲染所有中文文字（标签、图例、坐标轴）
- ✅ 正确处理中文字符编码

**修改的函数**：
- `buildUI()` - 添加CSS字体设置
- `drawLegend()` - 使用Pango渲染图例
- `drawAxisLabels()` - 使用Pango渲染坐标轴标签
- `drawChannels()` - 使用Pango渲染提示文字

### 2. 关闭后无法再打开问题
**修复方法**：隐藏窗口而不是销毁窗口

**改进内容**：
- ✅ 添加`onWindowDelete`事件处理
- ✅ 点击关闭按钮时隐藏窗口（`gtk_widget_hide`）
- ✅ 再次点击"示波器"按钮可以重新显示窗口
- ✅ 保持窗口状态和数据（通道、设置等）

**修改的代码**：
```cpp
// 窗口关闭事件：返回TRUE阻止默认销毁行为
gboolean onWindowDelete(GtkWidget *widget, GdkEvent *event, gpointer user_data) {
    gtk_widget_hide(widget);
    return TRUE;
}
```

## 🎯 使用说明

### 测试中文显示

1. 运行程序：`.\run.bat`
2. 加载CSV文件（确保包含中文变量名）
3. 打开示波器
4. 添加包含中文的通道名

**预期结果**：所有中文应该正常显示，不再是"口口口"

### 测试关闭/重开

1. 打开示波器窗口
2. 添加几个通道
3. 点击窗口的关闭按钮（X）
4. 再次点击主窗口的"📊 示波器"按钮

**预期结果**：
- ✅ 示波器窗口再次显示
- ✅ 之前添加的通道仍然存在
- ✅ 所有设置保持不变

## 📋 测试用CSV

创建 `test_chinese.csv`：

```csv
host_recv_iso,log_text_hex,log_text_utf8,温度,速度,电压,湿度
2025-10-24 10:00:00.000,48656C6C6F,Hello,25.5,120,3.3,60
2025-10-24 10:00:00.033,576F726C64,World,26.1,125,3.2,62
2025-10-24 10:00:00.067,54657374,Test,25.8,118,3.4,61
2025-10-24 10:00:00.100,44617461,Data,27.2,130,3.5,65
2025-10-24 10:00:00.133,496E666F,Info,26.5,122,3.3,63
```

## 🔍 技术细节

### Pango文字渲染

**为什么使用Pango？**
- Cairo的`cairo_show_text()`不能很好地处理中文
- Pango是GTK的文字布局引擎，完全支持Unicode
- Pango自动处理字体回退（font fallback）

**代码示例**：
```cpp
// 创建Pango布局
PangoLayout *layout = pango_cairo_create_layout(cr);
PangoFontDescription *desc = pango_font_description_from_string("Microsoft YaHei 10");
pango_layout_set_font_description(layout, desc);

// 设置文字并渲染
pango_layout_set_text(layout, "温度", -1);
pango_cairo_show_layout(cr, layout);

// 清理
pango_font_description_free(desc);
g_object_unref(layout);
```

### 窗口生命周期管理

**之前的问题**：
```cpp
// 析构函数中销毁窗口
gtk_widget_destroy(window);
// 问题：窗口被销毁后无法再次显示
```

**现在的方案**：
```cpp
// 关闭时只是隐藏
gtk_widget_hide(window);
// 优点：窗口仍然存在，可以再次show
```

## 🐛 如果还有问题

### 中文仍然显示为方块

**检查系统字体**：
```
检查 C:\Windows\Fonts 中是否有：
- Microsoft YaHei (微软雅黑)
- SimHei (黑体)
```

**修改字体设置**（在oscilloscope.cpp中）：
```cpp
// 如果YaHei不可用，尝试其他字体
PangoFontDescription *desc = pango_font_description_from_string("SimHei 10");
// 或
PangoFontDescription *desc = pango_font_description_from_string("SimSun 10");
```

### 窗口关闭后行为异常

**检查事件连接**：
确保`buildUI()`中有：
```cpp
g_signal_connect(window, "delete-event", G_CALLBACK(onWindowDelete), this);
```

### 性能问题

如果绘制中文时卡顿：
- 减少通道数量
- 减少时间窗口大小
- 降低刷新频率

## 📊 性能优化建议

1. **Pango布局缓存**：如果文字不变，可以缓存布局对象
2. **减少重绘**：只在数据更新时调用`gtk_widget_queue_draw()`
3. **异步更新**：大量数据时考虑使用后台线程

## 🎨 样式定制

### 修改字体
在`buildUI()`中修改CSS：
```cpp
const char *css_data = 
    "window, label, button {"
    "  font-family: 'SimHei', 'Microsoft YaHei', 'Sans';"  // 修改这里
    "  font-size: 11pt;"  // 修改字体大小
    "}";
```

### 修改颜色方案
在构造函数中修改：
```cpp
color_palette = {
    {1.0, 0.0, 0.0, 1.0},  // 纯红
    {0.0, 1.0, 0.0, 1.0},  // 纯绿
    // ... 添加更多颜色
};
```

---

**现在可以测试了！** 🎉

所有中文应该正常显示，关闭窗口后也能再次打开。
