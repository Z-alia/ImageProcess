#include "csv_reader.h"
#include <fstream>
#include <sstream>
#include <algorithm>

CSVReader::CSVReader() {
}

CSVReader::~CSVReader() {
    clear();
}

bool CSVReader::loadCSV(const std::string& filename) {
    clear();
    
    std::ifstream file(filename);
    if (!file.is_open()) {
        return false;
    }
    
    std::string line;
    bool isFirstLine = true;
    std::vector<std::string> headers;
    
    while (std::getline(file, line)) {
        if (line.empty()) continue;
        
        std::vector<std::string> fields = parseLine(line);
        
        if (isFirstLine) {
            // 解析表头
            headers = fields;
            isFirstLine = false;
            
            // 提取自定义变量名（跳过前3个固定列）
            if (headers.size() > 3) {
                for (size_t i = 3; i < headers.size(); i++) {
                    variableNames.push_back(trim(headers[i]));
                }
            }
            continue;
        }
        
        // 解析数据行
        if (fields.size() < 3) continue; // 至少需要3个基本字段
        
        LogRecord record;
        record.timestamp = trim(fields[0]);
        record.log_hex = trim(fields[1]);
        record.log_utf8 = trim(fields[2]);
        
        // 解析自定义变量值
        for (size_t i = 3; i < fields.size() && i < headers.size(); i++) {
            std::string varName = trim(headers[i]);
            std::string varValue = trim(fields[i]);
            record.variables[varName] = varValue;
        }
        
        records.push_back(record);
    }
    
    file.close();
    return !records.empty();
}

LogRecord CSVReader::getLogByIndex(int index) const {
    if (index >= 0 && index < (int)records.size()) {
        return records[index];
    }
    return LogRecord(); // 返回空记录
}

void CSVReader::clear() {
    records.clear();
    variableNames.clear();
}

std::vector<std::string> CSVReader::parseLine(const std::string& line) {
    std::vector<std::string> fields;
    std::string field;
    bool inQuotes = false;
    
    for (size_t i = 0; i < line.length(); i++) {
        char c = line[i];
        
        if (c == '"') {
            inQuotes = !inQuotes;
        } else if (c == ',' && !inQuotes) {
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

std::string CSVReader::trim(const std::string& str) {
    size_t first = str.find_first_not_of(" \t\r\n\"");
    if (first == std::string::npos) return "";
    
    size_t last = str.find_last_not_of(" \t\r\n\"");
    return str.substr(first, last - first + 1);
}
