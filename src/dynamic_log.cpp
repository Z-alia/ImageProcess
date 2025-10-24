#include "dynamic_log.h"
#include "utils.h"
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
    
    // 检查该帧是否已有同名变量，如果有则更新，否则添加
    auto& frame_vars = frame_logs[frame_index];
    bool found = false;
    for (auto& existing_var : frame_vars) {
        if (existing_var.name == var_name) {
            // 更新已存在的变量值
            existing_var.value_str = var.value_str;
            existing_var.type = var.type;
            found = true;
            break;
        }
    }
    
    if (!found) {
        // 添加新变量
        frame_vars.push_back(var);
    }
    
    // 不再立即写入，而是等待flushToCsv统一写入
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
    // 此函数已废弃，不再使用
    // 改用 flushToCsv() 统一写入整个CSV
}

void DynamicLogManager::flushToCsv() {
    if (csv_path.empty()) {
        fprintf(stderr, "[动态日志警告] CSV路径未设置，跳过写入\n");
        return;
    }
    
    // 1. 读取现有CSV文件的所有内容
    std::vector<std::vector<std::string>> existing_data;
    std::vector<std::string> headers;
    
#ifdef _WIN32
    // Windows下使用宽字符路径
    std::wstring wpath = utf8_to_wstring(csv_path);
    FILE* fp = _wfopen(wpath.c_str(), L"rb");
    if (fp) {
        // 使用C风格文件读取来支持宽字符路径
        fseek(fp, 0, SEEK_END);
        long file_size = ftell(fp);
        fseek(fp, 0, SEEK_SET);
        
        if (file_size > 0) {
            std::vector<char> buffer(file_size + 1);
            size_t bytes_read = fread(buffer.data(), 1, file_size, fp);
            buffer[bytes_read] = '\0';
            fclose(fp);
            
            // 解析内容
            std::istringstream iss(buffer.data());
            std::string line;
            bool first_line = true;
            
            while (std::getline(iss, line)) {
                // 移除\r
                if (!line.empty() && line.back() == '\r') {
                    line.pop_back();
                }
                
                if (line.empty()) continue;
                
                std::vector<std::string> fields = parse_csv_line(line);
                
                if (first_line) {
                    headers = fields;
                    first_line = false;
                } else {
                    existing_data.push_back(fields);
                }
            }
        } else {
            fclose(fp);
        }
    }
#else
    // Linux/Unix 使用标准文件流
    std::ifstream infile(csv_path, std::ios::binary);
    if (infile.is_open()) {
        std::string line;
        bool first_line = true;
        
        while (std::getline(infile, line)) {
            // 移除\r
            if (!line.empty() && line.back() == '\r') {
                line.pop_back();
            }
            
            if (line.empty()) continue;
            
            std::vector<std::string> fields = parse_csv_line(line);
            
            if (first_line) {
                headers = fields;
                first_line = false;
            } else {
                existing_data.push_back(fields);
            }
        }
        infile.close();
    }
#endif
    
    // 2. 获取所有动态日志变量名
    std::vector<std::string> new_var_names = getAllVariableNames();
    
    if (new_var_names.empty()) {
        fprintf(stderr, "[动态日志] 没有新变量需要写入\n");
        return;
    }
    
    // 2.5. 检测是否存在 frame_id 列
    int frame_id_col = -1;
    for (size_t i = 0; i < headers.size(); i++) {
        std::string lower_header = headers[i];
        std::transform(lower_header.begin(), lower_header.end(), lower_header.begin(), ::tolower);
        if (lower_header == "frame_id" || lower_header == "frameid" || lower_header == "frame") {
            frame_id_col = static_cast<int>(i);
            break;
        }
    }
    
    // 3. 更新表头（添加新变量列）
    size_t original_col_count = headers.size();
    for (const auto& var_name : new_var_names) {
        // 检查是否已存在
        if (std::find(headers.begin(), headers.end(), var_name) == headers.end()) {
            headers.push_back(var_name);
        }
    }
    
    // 4. 为每一行数据填充新列的值
    for (size_t row_idx = 0; row_idx < existing_data.size(); row_idx++) {
        auto& row = existing_data[row_idx];
        
        // 根据 frame_id 列或行索引确定帧号
        int frame_index;
        if (frame_id_col >= 0 && frame_id_col < static_cast<int>(row.size())) {
            // 如果有 frame_id 列，使用它
            try {
                frame_index = std::stoi(row[frame_id_col]);
            } catch (...) {
                // 如果解析失败，使用行索引 + 1
                frame_index = static_cast<int>(row_idx + 1);
            }
        } else {
            // 没有 frame_id 列，默认：第一行 = 帧1
            frame_index = static_cast<int>(row_idx + 1);
        }
        
        // 扩展行到新的列数
        while (row.size() < headers.size()) {
            row.push_back("");
        }
        
        // 填充动态日志的值
        auto frame_logs_it = frame_logs.find(frame_index);
        if (frame_logs_it != frame_logs.end()) {
            for (const auto& var : frame_logs_it->second) {
                // 找到变量名对应的列索引
                auto header_it = std::find(headers.begin(), headers.end(), var.name);
                if (header_it != headers.end()) {
                    size_t col_idx = std::distance(headers.begin(), header_it);
                    if (col_idx < row.size()) {
                        row[col_idx] = var.value_str;
                    }
                }
            }
        }
    }
    
    // 5. 写回CSV文件
#ifdef _WIN32
    fp = _wfopen(wpath.c_str(), L"wb");
    if (!fp) {
        fprintf(stderr, "[动态日志错误] 无法写入文件: %s\n", csv_path.c_str());
        return;
    }
    
    // 写入表头
    std::ostringstream oss;
    for (size_t i = 0; i < headers.size(); i++) {
        if (i > 0) oss << ",";
        oss << escape_csv_field(headers[i]);
    }
    oss << "\n";
    
    // 写入数据行
    for (const auto& row : existing_data) {
        for (size_t i = 0; i < row.size(); i++) {
            if (i > 0) oss << ",";
            oss << escape_csv_field(row[i]);
        }
        oss << "\n";
    }
    
    std::string output = oss.str();
    fwrite(output.c_str(), 1, output.size(), fp);
    fclose(fp);
#else
    std::ofstream outfile(csv_path, std::ios::trunc);
    if (!outfile.is_open()) {
        fprintf(stderr, "[动态日志错误] 无法写入文件: %s\n", csv_path.c_str());
        return;
    }
    
    // 写入表头
    for (size_t i = 0; i < headers.size(); i++) {
        if (i > 0) outfile << ",";
        outfile << escape_csv_field(headers[i]);
    }
    outfile << "\n";
    
    // 写入数据行
    for (const auto& row : existing_data) {
        for (size_t i = 0; i < row.size(); i++) {
            if (i > 0) outfile << ",";
            outfile << escape_csv_field(row[i]);
        }
        outfile << "\n";
    }
    
    outfile.close();
#endif
    
    size_t new_vars_count = headers.size() - original_col_count;
    fprintf(stderr, "[动态日志] 已更新CSV文件，添加 %zu 个新变量\n", new_vars_count);
}

// 注意：这两个函数已废弃，请使用 utils.h 中的公共函数
// 保留用于向后兼容
std::vector<std::string> DynamicLogManager::parseLine(const std::string& line) {
    return parse_csv_line(line);
}

std::string DynamicLogManager::escapeCSV(const std::string& str) {
    return escape_csv_field(str);
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
