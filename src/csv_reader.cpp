#include "csv_reader.h"
#include <fstream>
#include <sstream>
#include <algorithm>
#include <cctype>

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
    int timestampCol = -1;
    int hexCol = -1;
    int utf8Col = -1;
    
    while (std::getline(file, line)) {
        if (line.empty()) continue;
        
        std::vector<std::string> fields = parseLine(line);
        
        if (isFirstLine) {
            // 解析表头
            headers = fields;
            isFirstLine = false;
            
            // 自动检测关键列的位置（不区分大小写）
            for (size_t i = 0; i < headers.size(); i++) {
                std::string colName = trim(headers[i]);
                std::string lowerColName = colName;
                std::transform(lowerColName.begin(), lowerColName.end(), lowerColName.begin(), ::tolower);
                
                // 检测时间戳列
                if (lowerColName.find("time") != std::string::npos || 
                    lowerColName.find("iso") != std::string::npos ||
                    lowerColName.find("timestamp") != std::string::npos) {
                    if (timestampCol == -1) timestampCol = i;
                }
                // 检测hex列
                else if (lowerColName.find("hex") != std::string::npos) {
                    if (hexCol == -1) hexCol = i;
                }
                // 检测utf8/text列
                else if (lowerColName.find("utf") != std::string::npos || 
                         lowerColName.find("text") != std::string::npos) {
                    if (utf8Col == -1) utf8Col = i;
                }
            }
            
            // 如果没有检测到关键列，使用默认位置（前3列）
            if (timestampCol == -1 && headers.size() > 0) timestampCol = 0;
            if (hexCol == -1 && headers.size() > 1) hexCol = 1;
            if (utf8Col == -1 && headers.size() > 2) utf8Col = 2;
            
            // 提取自定义变量名（所有非关键列）
            for (size_t i = 0; i < headers.size(); i++) {
                if ((int)i != timestampCol && (int)i != hexCol && (int)i != utf8Col) {
                    variableNames.push_back(trim(headers[i]));
                }
            }
            continue;
        }
        
        // 解析数据行（不再要求至少3列）
        if (fields.empty()) continue;
        
        LogRecord record;
        
        // 从检测到的列位置读取数据，如果列不存在则为空
        if (timestampCol >= 0 && timestampCol < (int)fields.size()) {
            record.timestamp = trim(fields[timestampCol]);
        }
        if (hexCol >= 0 && hexCol < (int)fields.size()) {
            record.log_hex = trim(fields[hexCol]);
        }
        if (utf8Col >= 0 && utf8Col < (int)fields.size()) {
            record.log_utf8 = trim(fields[utf8Col]);
        }
        
        // 解析自定义变量值
        for (size_t i = 0; i < fields.size() && i < headers.size(); i++) {
            if ((int)i != timestampCol && (int)i != hexCol && (int)i != utf8Col) {
                std::string varName = trim(headers[i]);
                std::string varValue = trim(fields[i]);
                record.variables[varName] = varValue;
            }
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
