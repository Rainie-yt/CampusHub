#include "Performance.h"
#include <iostream>

Performance::Performance(const std::string& aid, const std::string& cid, const std::string& name,
                         const std::vector<std::string>& tagList,
                         const std::string& st, const std::string& et,
                         const std::string& loc, int maxp,
                         const std::string& program, const std::string& stat)
    : Activity(aid, cid, name, tagList, st, et, loc, maxp, stat), programType(clean(program)) {}

void Performance::showInfo() const {
    showBaseInfo();
    std::cout << "节目类型：" << programType << "\n";
}
