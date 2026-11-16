/*
文件名：DataFile.h（数据文件读取工具）
功能：统一读取文本、计算行号和输出数据错误，供账号、社团和活动模块共用。
说明：这里只负责读取和报告错误，不修改文件。读取失败时，调用方必须停止加载，
      不能拿已经读到的一部分内容继续运行，否则下次保存可能覆盖原来的完整数据。
      函数放在头文件中，使用 inline 避免多个文件包含时出现重复定义。
*/
#pragma once
#include "FileStore.h"
#include <cctype>
#include <sstream>

// 放进命名空间，调用时写 DataFile::read 等，避免和其他模块的函数重名。
namespace DataFile {
/*
函数名：error
功能：输出出错文件、行号和具体原因
参数：path 文件路径；line 出错行号，为 0 时表示无法确定行号；reason 错误原因
返回值：始终返回 false，调用处可以直接写 return DataFile::error(...)。
说明：使用 cerr 输出错误，不会修改数据，也不会在这里直接退出程序。
*/
inline bool error(const std::string& path, size_t line, const std::string& reason) {
    std::cerr << "数据错误：" << path;
    if (line) std::cerr << " 第 " << line << " 行";
    std::cerr << "：" << reason << "。\n";
    return false;
}
/*
函数名：read
功能：完整读取一份数据文件，并检查是否含有空字节
参数：path 文件路径；text 用来接收文件内容，进入函数时先清空
返回值：读取和检查成功返回 true；无法读取或含有空字节时返回 false。
实现步骤：
    1. 调用 FileStore::rawRead 读取；文件不存在时按空内容处理，允许首次使用；
    2. 如果文件存在但没有读成功，输出文件名和读取失败原因；
    3. 查找空字节，找到后计算它所在的行号，再输出错误位置。
关键点：文件不存在和文件损坏是两回事，损坏文件不能当作空数据继续处理。
        返回 false 时，调用方不要继续使用 text 中的内容。
*/
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
/*
函数名：blank
功能：判断一行是否为空行，或只含有空格、Tab 等空白字符
参数：line 当前行的内容
返回值：全是空白字符时返回 true，发现非空白字符时返回 false。
关键点：用 unsigned char 接收字节再交给 isspace，避免中文的负值字节引起未定义行为。
*/
inline bool blank(const std::string& line) {
    for (unsigned char ch : line) if (!std::isspace(ch)) return false;
    return true;
}
/*
函数名：nextLine
功能：读取下一行，同时更新行号，兼容 Windows 换行和 UTF-8 BOM
参数：in 输入流；line 接收本行内容；number 当前已读行数，由调用方从 0 开始计数
返回值：读到一行返回 true；读到文件末尾或读取失败返回 false。
实现步骤：
    1. getline 读入一行，成功后将行号加 1；
    2. 去掉行尾可能残留的回车符 \r；
    3. 第一行如果以 UTF-8 BOM 开头，就去掉这三个字节。
说明：只处理换行和文件头，不跳过空行，是否允许空行由各模块自己判断。
*/
inline bool nextLine(std::istream& in, std::string& line, size_t& number) {
    if (!std::getline(in, line)) return false;
    ++number;
    if (!line.empty() && line.back() == '\r') line.pop_back();
    if (number == 1 && line.compare(0, 3, "\xEF\xBB\xBF") == 0) line.erase(0, 3);
    return true;
}
}
