#pragma once
#include "FileStore.h"
#include <cctype>
#include <sstream>

// 加载失败必须由调用方中止启动；只有文件确实不存在时才按空数据处理。
namespace DataFile {
inline bool error(const std::string& path, size_t line, const std::string& reason) {
    std::cerr << "数据错误：" << path;
    if (line) std::cerr << " 第 " << line << " 行";
    std::cerr << "：" << reason << "。\n";
    return false;
}
inline bool read(const std::string& path, std::string& text) {
    text.clear();
    bool exists = false;
    if (!FileStore::rawRead(path, text, exists))
        return error(path, 0, "无法完整读取，请检查文件类型、权限或占用情况");
    const size_t nul = text.find('\0');
    if (nul != std::string::npos) {
        size_t line = 1;
        for (size_t i = 0; i < nul; ++i) if (text[i] == '\n') ++line;
        return error(path, line, "包含空字节，请检查文件编码或损坏情况");
    }
    return true;
}
inline bool blank(const std::string& line) {
    for (unsigned char ch : line) if (!std::isspace(ch)) return false;
    return true;
}
inline bool nextLine(std::istream& in, std::string& line, size_t& number) {
    if (!std::getline(in, line)) return false;
    ++number;
    if (!line.empty() && line.back() == '\r') line.pop_back();
    if (number == 1 && line.compare(0, 3, "\xEF\xBB\xBF") == 0) line.erase(0, 3);
    return true;
}
}
