#ifndef CSV_READER_H
#define CSV_READER_H

#include <string>
#include <vector>
#include <map>

// 日志记录结构
struct LogRecord {
    std::string timestamp;      // host_recv_iso
    std::string log_hex;        // log_text_hex
    std::string log_utf8;       // log_text_utf8
    std::map<std::string, std::string> variables; // 自定义变量 {变量名: 值}
};

// CSV读取器类
class CSVReader {
public:
    CSVReader();
    ~CSVReader();
    
    // 加载CSV文件
    bool loadCSV(const std::string& filename);
    
    // 根据索引获取日志记录
    LogRecord getLogByIndex(int index) const;
    
    // 获取总记录数
    int getRecordCount() const { return records.size(); }
    
    // 获取所有变量名（CSV列标题中的自定义变量）
    std::vector<std::string> getVariableNames() const { return variableNames; }
    
    // 清空数据
    void clear();
    
private:
    std::vector<LogRecord> records;
    std::vector<std::string> variableNames; // 自定义变量名列表
    
    // 辅助函数：解析CSV行
    std::vector<std::string> parseLine(const std::string& line);
    
    // 辅助函数：去除字符串首尾空格
    std::string trim(const std::string& str);
};

#endif // CSV_READER_H
