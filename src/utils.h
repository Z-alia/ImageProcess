#ifndef UTILS_H
#define UTILS_H

#include <string>
#include <vector>

// Windows UTF-8路径转换工具
#ifdef _WIN32
#include <windows.h>
std::wstring utf8_to_wstring(const std::string& utf8str);
#endif

// CSV解析工具
std::vector<std::string> parse_csv_line(const std::string& line);
std::string escape_csv_field(const std::string& str);
std::string trim_string(const std::string& str);

#endif // UTILS_H
