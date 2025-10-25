#ifndef DYNAMIC_LOG_H
#define DYNAMIC_LOG_H

#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

// 日志变量类型
typedef enum {
    LOG_TYPE_INT8,
    LOG_TYPE_UINT8,
    LOG_TYPE_INT16,
    LOG_TYPE_UINT16,
    LOG_TYPE_INT32,
    LOG_TYPE_UINT32,
    LOG_TYPE_FLOAT,
    LOG_TYPE_DOUBLE,
    LOG_TYPE_STRING,
    // 数组类型
    LOG_TYPE_INT8_ARRAY,
    LOG_TYPE_UINT8_ARRAY,
    LOG_TYPE_INT16_ARRAY,
    LOG_TYPE_UINT16_ARRAY,
    LOG_TYPE_INT32_ARRAY,
    LOG_TYPE_UINT32_ARRAY,
    LOG_TYPE_FLOAT_ARRAY,
    LOG_TYPE_DOUBLE_ARRAY
} LogVarType;

// C接口：添加日志变量
// 参数：
//   var_name: 变量名（字符串）
//   var_type: 变量类型
//   var_ptr: 变量指针（对于字符串，传入char*；对于数组，传入数组指针）
//   frame_index: 对应的帧索引（-1表示当前帧）
void log_add_variable(const char* var_name, LogVarType var_type, const void* var_ptr, int frame_index);

// 数组版本的日志函数（需要传入数组长度）
// array_ptr: 指向数组首元素的指针
// count: 数组元素个数
void log_add_array(const char* var_name, LogVarType array_type, const void* array_ptr, int count, int frame_index);

// C接口：添加基本类型变量的便捷函数
void log_add_int8(const char* var_name, int8_t value, int frame_index);
void log_add_uint8(const char* var_name, uint8_t value, int frame_index);
void log_add_int16(const char* var_name, int16_t value, int frame_index);
void log_add_uint16(const char* var_name, uint16_t value, int frame_index);
void log_add_int32(const char* var_name, int32_t value, int frame_index);
void log_add_uint32(const char* var_name, uint32_t value, int frame_index);
void log_add_float(const char* var_name, float value, int frame_index);
void log_add_double(const char* var_name, double value, int frame_index);
void log_add_string(const char* var_name, const char* value, int frame_index);

// C接口：添加数组的便捷函数
void log_add_int8_array(const char* var_name, const int8_t* array, int count, int frame_index);
void log_add_uint8_array(const char* var_name, const uint8_t* array, int count, int frame_index);
void log_add_int16_array(const char* var_name, const int16_t* array, int count, int frame_index);
void log_add_uint16_array(const char* var_name, const uint16_t* array, int count, int frame_index);
void log_add_int32_array(const char* var_name, const int32_t* array, int count, int frame_index);
void log_add_uint32_array(const char* var_name, const uint32_t* array, int count, int frame_index);
void log_add_float_array(const char* var_name, const float* array, int count, int frame_index);
void log_add_double_array(const char* var_name, const double* array, int count, int frame_index);

// 清空所有动态日志
void log_clear_all();

// 清空指定帧的日志
void log_clear_frame(int frame_index);

// 获取当前帧索引
int log_get_current_frame();

// 设置当前帧索引（通常由视频播放器自动设置）
void log_set_current_frame(int frame_index);

// 设置CSV自动保存路径（设置后会自动写入）
void log_set_csv_path(const char* csv_path);

// 获取当前CSV路径
const char* log_get_csv_path();

// 立即刷新所有日志到CSV（通常自动调用）
void log_flush_to_csv();

#ifdef __cplusplus
}

// C++ 接口
#include <string>
#include <map>
#include <vector>

// 单条日志变量记录
struct DynamicLogVariable {
    std::string name;
    LogVarType type;
    std::string value_str;  // 所有类型都转换为字符串存储（数组格式：[1,2,3]）
    int array_count;        // 数组长度（非数组类型为0）
    
    DynamicLogVariable() : type(LOG_TYPE_INT8), array_count(0) {}
};

// 动态日志管理器（C++单例）
class DynamicLogManager {
public:
    static DynamicLogManager& getInstance();
    
    // 添加日志变量
    void addVariable(const std::string& var_name, LogVarType type, const void* var_ptr, int frame_index);
    
    // 添加数组日志变量
    void addArray(const std::string& var_name, LogVarType array_type, const void* array_ptr, int count, int frame_index);
    
    // 获取指定帧的所有日志变量
    std::vector<DynamicLogVariable> getFrameLogs(int frame_index) const;
    
    // 清空所有日志
    void clearAll();
    
    // 清空指定帧
    void clearFrame(int frame_index);
    
    // 获取/设置当前帧
    int getCurrentFrame() const { return current_frame; }
    void setCurrentFrame(int frame) { current_frame = frame; }
    
    // 获取所有有日志的帧索引
    std::vector<int> getFrameIndices() const;
    
    // CSV自动保存
    void setCsvPath(const std::string& path);
    std::string getCsvPath() const { return csv_path; }
    void flushToCsv();
    
    // 获取所有变量名（用于CSV表头）
    std::vector<std::string> getAllVariableNames() const;
    
private:
    DynamicLogManager();
    ~DynamicLogManager();
    DynamicLogManager(const DynamicLogManager&) = delete;
    DynamicLogManager& operator=(const DynamicLogManager&) = delete;
    
    // 将变量值转换为字符串
    std::string valueToString(LogVarType type, const void* var_ptr);
    
    // 将数组转换为字符串（格式：[1,2,3]）
    std::string arrayToString(LogVarType array_type, const void* array_ptr, int count);
    
    // CSV辅助函数
    std::vector<std::string> parseLine(const std::string& line);
    std::string escapeCSV(const std::string& str);
    
    // 存储结构: frame_index -> vector of variables
    std::map<int, std::vector<DynamicLogVariable>> frame_logs;
    int current_frame;
    std::string csv_path;
    bool auto_save_enabled;
    
    // 内部函数：写入单帧到CSV（已废弃）
    void appendFrameToCsv(int frame_index);
};

#endif // __cplusplus

#endif // DYNAMIC_LOG_H
