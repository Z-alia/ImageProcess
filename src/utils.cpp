#include "utils.h"
#include <algorithm>
#include <cctype>

#ifdef _WIN32
// Windows下将UTF-8路径转换为宽字符
std::wstring utf8_to_wstring(const std::string& utf8str) {
    if (utf8str.empty()) return std::wstring();
    
    int size_needed = MultiByteToWideChar(CP_UTF8, 0, utf8str.c_str(), 
                                         static_cast<int>(utf8str.size()), NULL, 0);
    if (size_needed <= 0) {
        return std::wstring();
    }
    
    std::wstring wstr(size_needed, 0);
    MultiByteToWideChar(CP_UTF8, 0, utf8str.c_str(), 
                       static_cast<int>(utf8str.size()), &wstr[0], size_needed);
    return wstr;
}
#endif

// CSV行解析（正确处理引号内的逗号和转义引号）
std::vector<std::string> parse_csv_line(const std::string& line) {
    std::vector<std::string> fields;
    std::string field;
    bool in_quotes = false;
    
    for (size_t i = 0; i < line.length(); i++) {
        char c = line[i];
        
        if (c == '"') {
            // 处理双引号转义 ""
            if (in_quotes && i + 1 < line.length() && line[i + 1] == '"') {
                field += '"';
                i++; // 跳过下一个引号
            } else {
                // 切换引号状态
                in_quotes = !in_quotes;
            }
        } else if (c == ',' && !in_quotes) {
            // 字段分隔符
            fields.push_back(field);
            field.clear();
        } else {
            field += c;
        }
    }
    
    // 添加最后一个字段
    fields.push_back(field);
    return fields;
}

// CSV字段转义（处理逗号、引号、换行符）
std::string escape_csv_field(const std::string& str) {
    // 检查是否需要转义
    bool needs_quotes = str.find(',') != std::string::npos ||
                       str.find('"') != std::string::npos ||
                       str.find('\n') != std::string::npos ||
                       str.find('\r') != std::string::npos;
    
    if (!needs_quotes) {
        return str;
    }
    
    // 转义引号并用引号包围
    std::string escaped = "\"";
    for (char c : str) {
        if (c == '"') {
            escaped += "\"\""; // 引号转义为两个引号
        } else {
            escaped += c;
        }
    }
    escaped += "\"";
    return escaped;
}

// 字符串trim（移除首尾空白和引号）
std::string trim_string(const std::string& str) {
    if (str.empty()) return "";
    
    size_t first = str.find_first_not_of(" \t\r\n\"");
    if (first == std::string::npos) {
        return "";
    }
    
    size_t last = str.find_last_not_of(" \t\r\n\"");
    return str.substr(first, last - first + 1);
}
