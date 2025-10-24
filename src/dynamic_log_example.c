/*
 * 动态日志接口使用示例
 * 
 * 本示例展示如何在图像处理代码中添加动态日志
 * 这些日志会显示在 ImageProcess GUI 的日志窗口中
 */

#include "dynamic_log.h"
#include "processor.h"
#include "global_image_buffer.h"
#include <stdio.h>

// 示例1: 在二值化处理中添加日志
void process_frame_with_logs(int frame_index) {
    // 假设已经有了图像数据
    uint8_t **image = get_original_bi_image();
    int width = get_image_width();
    int height = get_image_height();
    
    // 添加基本信息
    log_add_int32("帧索引", frame_index, frame_index);
    log_add_int32("图像宽度", width, frame_index);
    log_add_int32("图像高度", height, frame_index);
    
    // 统计白色像素数量
    int white_count = 0;
    for (int y = 0; y < height; y++) {
        for (int x = 0; x < width; x++) {
            if (image[y][x] > 127) {
                white_count++;
            }
        }
    }
    
    int total_pixels = width * height;
    float white_ratio = (float)white_count / total_pixels * 100.0f;
    
    // 添加统计结果
    log_add_int32("白色像素", white_count, frame_index);
    log_add_float("白色占比%", white_ratio, frame_index);
    
    // 添加其他分析结果
    log_add_string("处理状态", "完成", frame_index);
}

// 示例2: 在边缘检测中添加日志
void edge_detection_with_logs(int frame_index) {
    // 执行边缘检测...
    int edge_count = 1234;  // 假设检测到的边缘点数
    
    log_add_int32("边缘点数", edge_count, frame_index);
    
    // 检测阈值
    uint8_t threshold = 128;
    log_add_uint8("检测阈值", threshold, frame_index);
}

// 示例3: 在循环中为每一帧添加日志
void batch_process_video() {
    for (int frame = 1; frame <= 100; frame++) {
        // 处理每一帧...
        
        // 为每一帧添加日志
        log_add_int32("当前帧", frame, frame);
        
        // 假设计算了某些特征
        int feature_value = frame * 10;
        log_add_int32("特征值", feature_value, frame);
        
        // 检测到某个事件
        if (frame % 10 == 0) {
            log_add_string("事件", "关键帧", frame);
        }
    }
}

// 示例4: 使用当前帧索引（自动）
void process_current_frame() {
    // 如果不指定帧索引，使用 -1 表示当前帧
    // 当前帧由 log_set_current_frame() 设置（GUI自动调用）
    
    log_add_string("处理方法", "自动二值化", -1);
    log_add_float("执行时间ms", 12.5f, -1);
}

// 示例5: 多类型数据演示
void demo_all_types(int frame_index) {
    // 整数类型
    int8_t temp = -25;
    uint8_t brightness = 200;
    int16_t angle = -3600;  // -36.00度 (假设缩放100倍)
    uint16_t distance = 5000;  // 5000mm
    int32_t large_num = -1234567;
    uint32_t timestamp = 123456789;
    
    log_add_int8("温度°C", temp, frame_index);
    log_add_uint8("亮度", brightness, frame_index);
    log_add_int16("角度x100", angle, frame_index);
    log_add_uint16("距离mm", distance, frame_index);
    log_add_int32("大整数", large_num, frame_index);
    log_add_uint32("时间戳", timestamp, frame_index);
    
    // 浮点类型
    float voltage = 3.3f;
    double pi = 3.141592653589793;
    
    log_add_float("电压V", voltage, frame_index);
    log_add_double("圆周率", pi, frame_index);
    
    // 字符串类型
    log_add_string("算法", "形态学处理", frame_index);
    log_add_string("状态", "运行中", frame_index);
}

// 示例6: 清理日志
void cleanup_example() {
    // 清空某一帧的日志
    log_clear_frame(10);
    
    // 清空所有日志
    log_clear_all();
}

// 示例7: 在 processor.c 中集成（假设）
// 修改 processor.c 中的函数，添加日志输出
void morphological_process_with_log(int frame_index) {
    // 执行形态学处理...
    
    // 记录处理参数
    int kernel_size = 3;
    int iterations = 2;
    
    log_add_int32("卷积核大小", kernel_size, frame_index);
    log_add_int32("迭代次数", iterations, frame_index);
    
    // 记录处理结果
    int removed_noise = 150;
    log_add_int32("去除噪点", removed_noise, frame_index);
}

/* ========================================
 * 集成到现有代码的建议：
 * ========================================
 * 
 * 1. 在 processor.c 的关键函数中添加日志：
 *    - binary_opening_bitpacked()
 *    - binary_closing_bitpacked()
 *    - process_image()
 * 
 * 2. 在 main.cpp 的图像加载回调中添加：
 *    - load_png_image()
 *    - show_cv_frame()
 * 
 * 3. 在自定义图像分析算法中：
 *    - 添加特征提取结果
 *    - 添加检测结果
 *    - 添加统计信息
 * 
 * 4. 显示方式：
 *    - 动态日志会自动显示在日志窗口
 *    - 与CSV日志同时显示
 *    - 动态日志显示在前面（优先级更高）
 * 
 * 5. 性能考虑：
 *    - 日志数据存储在内存中
 *    - 建议在处理完成后清理不需要的帧
 *    - 使用 log_clear_frame() 释放内存
 */
