#pragma once
#include <chrono>
#include <cerrno>
#include <cstdio>
#include <fstream>
#include <functional>
#include <iostream>
#include <iterator>
#include <string>
#include <sys/stat.h>
#include <utility>
#include <vector>
#ifdef _WIN32
#ifndef NOMINMAX
#define NOMINMAX
#endif
#include <windows.h>
#ifdef STATUS_PENDING
#undef STATUS_PENDING
#endif
#endif

// 单进程文件保存：先准备所有临时文件，再替换；普通 I/O 失败时恢复已替换文件。
// 这不是断电恢复日志，也不支持多个程序同时写同一组数据。
namespace FileStore {
typedef std::vector<std::pair<std::string, std::string> > Writes;
inline bool rawRead(const std::string& path, std::string& text, bool& exists) {
    struct stat info;
    if (stat(path.c_str(), &info) != 0) {
        exists = false;
        return errno == ENOENT;
    }
    exists = true;
    if (!(info.st_mode & S_IFREG)) return false;
    std::ifstream in(path.c_str(), std::ios::binary);
    if (!in) return false;
    text.assign(std::istreambuf_iterator<char>(in), std::istreambuf_iterator<char>());
    return !in.bad();
}
inline bool writeTemp(const std::string& path, const std::string& text) {
    std::ofstream out(path.c_str(), std::ios::binary | std::ios::trunc);
    if (!out) return false;
    out.write(text.data(), static_cast<std::streamsize>(text.size()));
    out.close();
    return !out.fail();
}
inline bool replace(const std::string& from, const std::string& to) {
#ifdef _WIN32
    return MoveFileExA(from.c_str(), to.c_str(), MOVEFILE_REPLACE_EXISTING | MOVEFILE_WRITE_THROUGH) != 0;
#else
    return std::rename(from.c_str(), to.c_str()) == 0;
#endif
}
inline bool commit(const Writes& writes) {
    static unsigned long sequence = 0;
    const std::string suffix = ".campushub-" + std::to_string(
        std::chrono::high_resolution_clock::now().time_since_epoch().count()) + "-" + std::to_string(++sequence);
    std::vector<std::string> old(writes.size()), temps;
    std::vector<bool> existed(writes.size());
    bool ok = true;
    for (size_t i = 0; i < writes.size(); ++i) {
        bool found = false;
        if (!rawRead(writes[i].first, old[i], found)) { ok = false; break; }
        existed[i] = found;
        if (found) {
            // 不截断原文件；只读或锁定时提前失败，避免其他文件已写了一半。
            std::fstream probe(writes[i].first.c_str(), std::ios::in | std::ios::out | std::ios::binary);
            if (!probe) { ok = false; break; }
        }
        temps.push_back(writes[i].first + suffix);
        if (!writeTemp(temps.back(), writes[i].second)) { ok = false; break; }
    }
    size_t done = 0;
    if (ok) {
        for (; done < writes.size(); ++done) {
            if (!replace(temps[done], writes[done].first)) { ok = false; break; }
        }
    }
    if (!ok) {
        while (done > 0) {
            --done;
            bool restored;
            if (existed[done]) {
                restored = writeTemp(temps[done], old[done]) && replace(temps[done], writes[done].first);
            } else {
                restored = std::remove(writes[done].first.c_str()) == 0;
            }
            if (!restored) std::cerr << "严重错误：文件回滚失败，请从备份恢复 " << writes[done].first << "\n";
        }
    }
    for (size_t i = 0; i < temps.size(); ++i) std::remove(temps[i].c_str());
    return ok;
}

class Batch {
    Writes pending;
public:
    static Batch*& current() { static Batch* value = nullptr; return value; }
    Batch() { current() = this; }
    ~Batch() { current() = nullptr; }
    void put(const std::string& path, const std::string& text) {
        for (size_t i = 0; i < pending.size(); ++i) {
            if (pending[i].first == path) { pending[i].second = text; return; }
        }
        pending.push_back(std::make_pair(path, text));
    }
    bool get(const std::string& path, std::string& text) const {
        for (size_t i = 0; i < pending.size(); ++i) {
            if (pending[i].first == path) { text = pending[i].second; return true; }
        }
        return false;
    }
    bool finish() { return commit(pending); }
};
inline bool read(const std::string& path, std::string& text, bool& exists) {
    if (Batch::current() && Batch::current()->get(path, text)) { exists = true; return true; }
    return rawRead(path, text, exists);
}
inline bool write(const Writes& writes) {
    if (Batch::current()) {
        for (size_t i = 0; i < writes.size(); ++i) Batch::current()->put(writes[i].first, writes[i].second);
        return true;
    }
    return commit(writes);
}
inline bool write(const std::string& path, const std::string& text) {
    return write(Writes(1, std::make_pair(path, text)));
}
}
