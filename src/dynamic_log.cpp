#include "dynamic_log.h"
#include <sstream>
#include <iomanip>
#include <cstring>

// ============================================================================
// C++ 实现
// ============================================================================

DynamicLogManager::DynamicLogManager() : current_frame(0) {
}

DynamicLogManager::~DynamicLogManager() {
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

} // extern "C"
