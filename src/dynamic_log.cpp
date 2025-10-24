#include "dynamic_log.h"
#include <sstream>
#include <iomanip>
#include <cstring>
#include <fstream>
#include <algorithm>
#include <ctime>
#include <set>

// ============================================================================
// C++ 实现
// ============================================================================

DynamicLogManager::DynamicLogManager() : current_frame(0), auto_save_enabled(false) {
}

DynamicLogManager::~DynamicLogManager() {
    if (auto_save_enabled && !csv_path.empty()) {
        flushToCsv();
    }
    clearAll();
}

DynamicLogManager& DynamicLogManager::getInstance() {
    static DynamicLogManager instance;
    return instance;
}

std::string DynamicLogManager::valueToString(LogVarType type, const void* var_ptr) {
    if (!var_ptr) return "NULL";
    
    std::ostringstream oss;
    
    switch (type) {
        case LOG_TYPE_INT8:
            oss << static_cast<int>(*(const int8_t*)var_ptr);
            break;
        case LOG_TYPE_UINT8:
            oss << static_cast<unsigned int>(*(const uint8_t*)var_ptr);
            break;
        case LOG_TYPE_INT16:
            oss << *(const int16_t*)var_ptr;
            break;
        case LOG_TYPE_UINT16:
            oss << *(const uint16_t*)var_ptr;
            break;
        case LOG_TYPE_INT32:
            oss << *(const int32_t*)var_ptr;
            break;
        case LOG_TYPE_UINT32:
            oss << *(const uint32_t*)var_ptr;
            break;
        case LOG_TYPE_FLOAT:
            oss << std::fixed << std::setprecision(6) << *(const float*)var_ptr;
            break;
        case LOG_TYPE_DOUBLE:
            oss << std::fixed << std::setprecision(10) << *(const double*)var_ptr;
            break;
        case LOG_TYPE_STRING:
            oss << (const char*)var_ptr;
            break;
        default:
            oss << "UNKNOWN_TYPE";
            break;
    }
    
    return oss.str();
}

void DynamicLogManager::addVariable(const std::string& var_name, LogVarType type, const void* var_ptr, int frame_index) {
    if (frame_index < 0) {
        frame_index = current_frame;
    }
    
    DynamicLogVariable var;
    var.name = var_name;
    var.type = type;
    var.value_str = valueToString(type, var_ptr);
    
    frame_logs[frame_index].push_back(var);
    
    // 如果启用了自动保存，立即写入CSV
    if (auto_save_enabled && !csv_path.empty()) {
        appendFrameToCsv(frame_index);
    }
}

std::vector<DynamicLogVariable> DynamicLogManager::getFrameLogs(int frame_index) const {
    auto it = frame_logs.find(frame_index);
    if (it != frame_logs.end()) {
        return it->second;
    }
    return std::vector<DynamicLogVariable>();
}

void DynamicLogManager::clearAll() {
    frame_logs.clear();
}

void DynamicLogManager::clearFrame(int frame_index) {
    frame_logs.erase(frame_index);
}

std::vector<int> DynamicLogManager::getFrameIndices() const {
    std::vector<int> indices;
    for (const auto& pair : frame_logs) {
        indices.push_back(pair.first);
    }
    return indices;
}

void DynamicLogManager::setCsvPath(const std::string& path) {
    csv_path = path;
    auto_save_enabled = !path.empty();
    
    // 不创建新文件，直接使用现有的CSV文件
    // CSV文件由 CSVReader 加载时已经存在
}

std::vector<std::string> DynamicLogManager::getAllVariableNames() const {
    std::vector<std::string> names;
    std::set<std::string> name_set;
    
    // 收集所有出现过的变量名（保持插入顺序）
    for (const auto& frame_pair : frame_logs) {
        for (const auto& var : frame_pair.second) {
            if (name_set.insert(var.name).second) {
                names.push_back(var.name);
            }
        }
    }
    
    return names;
}

void DynamicLogManager::appendFrameToCsv(int frame_index) {
    if (csv_path.empty()) return;
    
    // 获取该帧的所有变量
    auto it = frame_logs.find(frame_index);
    if (it == frame_logs.end() || it->second.empty()) return;
    
    const auto& vars = it->second;
    
    // 追加模式打开文件（RAII自动关闭）
    std::ofstream file(csv_path, std::ios::app);
    if (!file.is_open()) {
        // 文件打开失败，静默返回（避免干扰主程序）
        return;
    }
    
    // 生成当前时间戳（ISO格式）
    time_t now = time(nullptr);
    struct tm* timeinfo = localtime(&now);
    if (!timeinfo) return;
    
    char timestamp[64];
    strftime(timestamp, sizeof(timestamp), "%Y-%m-%d %H:%M:%S", timeinfo);
    
    // 按照标准CSV日志格式写入：
    // host_recv_iso, log_text_hex, log_text_utf8, 变量1, 变量2, ...
    file << timestamp << ",,"  // 时间戳 + 空hex列
         << "[dynamic]";        // 标记为动态日志
    
    // 写入所有变量值
    for (const auto& var : vars) {
        file << "," << var.value_str;
    }
    
    file << "\n";
    // file.close() 由析构函数自动调用
}

void DynamicLogManager::flushToCsv() {
    // 在合并模式下，每次添加变量时已经立即写入CSV
    // 此函数主要用于兼容性，实际上数据已实时写入
    // 无需额外操作
}

// ============================================================================
// C 接口实现
// ============================================================================

extern "C" {

void log_add_variable(const char* var_name, LogVarType var_type, const void* var_ptr, int frame_index) {
    if (!var_name || !var_ptr) return;
    DynamicLogManager::getInstance().addVariable(var_name, var_type, var_ptr, frame_index);
}

void log_add_int8(const char* var_name, int8_t value, int frame_index) {
    log_add_variable(var_name, LOG_TYPE_INT8, &value, frame_index);
}

void log_add_uint8(const char* var_name, uint8_t value, int frame_index) {
    log_add_variable(var_name, LOG_TYPE_UINT8, &value, frame_index);
}

void log_add_int16(const char* var_name, int16_t value, int frame_index) {
    log_add_variable(var_name, LOG_TYPE_INT16, &value, frame_index);
}

void log_add_uint16(const char* var_name, uint16_t value, int frame_index) {
    log_add_variable(var_name, LOG_TYPE_UINT16, &value, frame_index);
}

void log_add_int32(const char* var_name, int32_t value, int frame_index) {
    log_add_variable(var_name, LOG_TYPE_INT32, &value, frame_index);
}

void log_add_uint32(const char* var_name, uint32_t value, int frame_index) {
    log_add_variable(var_name, LOG_TYPE_UINT32, &value, frame_index);
}

void log_add_float(const char* var_name, float value, int frame_index) {
    log_add_variable(var_name, LOG_TYPE_FLOAT, &value, frame_index);
}

void log_add_double(const char* var_name, double value, int frame_index) {
    log_add_variable(var_name, LOG_TYPE_DOUBLE, &value, frame_index);
}

void log_add_string(const char* var_name, const char* value, int frame_index) {
    if (!value) return;
    log_add_variable(var_name, LOG_TYPE_STRING, value, frame_index);
}

void log_clear_all() {
    DynamicLogManager::getInstance().clearAll();
}

void log_clear_frame(int frame_index) {
    DynamicLogManager::getInstance().clearFrame(frame_index);
}

int log_get_current_frame() {
    return DynamicLogManager::getInstance().getCurrentFrame();
}

void log_set_current_frame(int frame_index) {
    DynamicLogManager::getInstance().setCurrentFrame(frame_index);
}

void log_set_csv_path(const char* csv_path) {
    if (!csv_path) {
        DynamicLogManager::getInstance().setCsvPath("");
    } else {
        DynamicLogManager::getInstance().setCsvPath(csv_path);
    }
}

const char* log_get_csv_path() {
    static std::string path;
    path = DynamicLogManager::getInstance().getCsvPath();
    return path.c_str();
}

void log_flush_to_csv() {
    DynamicLogManager::getInstance().flushToCsv();
}

} // extern "C"
