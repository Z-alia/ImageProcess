#include "csv_reader.h"
#include "utils.h"
#include <fstream>
#include <sstream>
#include <algorithm>
#include <cctype>
#include <cstdio>

CSVReader::CSVReader() {
}

CSVReader::~CSVReader() {
    clear();
}

bool CSVReader::loadCSV(const std::string& filename) {
    clear();
    
    // Windows下使用宽字符路径打开文件，支持中文路径
    std::ifstream file;
#ifdef _WIN32
    std::wstring wfilename = utf8_to_wstring(filename);
    file.open(wfilename.c_str(), std::ios::binary);
#else
    file.open(filename, std::ios::binary);
#endif
    
    if (!file.is_open()) {
        fprintf(stderr, "[CSV错误] 无法打开文件: %s\n", filename.c_str());
        return false;
    }
    
    std::string line;
    bool isFirstLine = true;
    std::vector<std::string> headers;
    int timestampCol = -1, hexCol = -1, utf8Col = -1;
    int totalLines = 0, skippedLines = 0, errorLines = 0;
    
    while (std::getline(file, line)) {
        totalLines++;
        
        // 移除行尾的\r (Windows换行符)
        if (!line.empty() && line.back() == '\r') {
            line.pop_back();
        }
        
        // 移除NULL字符和EOF字符(0x00和0x1A),这些会导致读取中断
        line.erase(std::remove(line.begin(), line.end(), '\0'), line.end());
        line.erase(std::remove(line.begin(), line.end(), '\x1A'), line.end());
        
        if (line.empty()) {
            skippedLines++;
            continue;
        }
        
        try {
            std::vector<std::string> fields = parse_csv_line(line);
            
            if (isFirstLine) {
                // 解析表头
                headers = fields;
                isFirstLine = false;
            
            // 自动检测关键列的位置（不区分大小写）
            for (size_t i = 0; i < headers.size(); i++) {
                std::string lowerColName = trim(headers[i]);
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
        if (fields.empty()) {
            skippedLines++;
            continue;
        }
        
        LogRecord record;
        
        // 从检测到的列位置读取数据，如果列不存在则为空
        if (timestampCol >= 0 && timestampCol < (int)fields.size()) {
            record.timestamp = trim_string(fields[timestampCol]);
        }
        if (hexCol >= 0 && hexCol < (int)fields.size()) {
            record.log_hex = trim_string(fields[hexCol]);
        }
        if (utf8Col >= 0 && utf8Col < (int)fields.size()) {
            record.log_utf8 = trim_string(fields[utf8Col]);
        }
        
        // 解析自定义变量值
        for (size_t i = 0; i < fields.size() && i < headers.size(); i++) {
            if ((int)i != timestampCol && (int)i != hexCol && (int)i != utf8Col) {
                std::string varName = trim_string(headers[i]);
                std::string varValue = trim_string(fields[i]);
                record.variables[varName] = varValue;
            }
        }
        
        records.push_back(record);
        
        } catch (const std::exception& e) {
            // 捕获解析错误,继续处理下一行
            errorLines++;
            fprintf(stderr, "[CSV解析错误] 行%d: %s\n", totalLines, e.what());
            continue;
        }
    }
    
    file.close();
    
    // 打印加载统计信息到stderr（方便调试）
    fprintf(stderr, "[CSV加载] 总行数: %d, 跳过: %d, 错误: %d, 加载记录: %zu\n", 
            totalLines, skippedLines, errorLines, records.size());
    
    if (totalLines < 100 && records.size() < 50) {
        fprintf(stderr, "[CSV警告] 读取的行数异常少,可能存在编码或格式问题\n");
    }
    
    // 如果至少读取了表头，允许加载（即使没有数据行）
    return totalLines > 0 || !records.empty();
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

// 注意：此函数已废弃，请使用 utils.h 中的 parse_csv_line
// 保留用于向后兼容
std::vector<std::string> CSVReader::parseLine(const std::string& line) {
    return parse_csv_line(line);
}

// 注意：此函数已废弃，请使用 utils.h 中的 trim_string
// 保留用于向后兼容
std::string CSVReader::trim(const std::string& str) {
    return trim_string(str);
}
