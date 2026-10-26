#include "Competition.h"
#include <iostream>

Competition::Competition(const std::string& aid, const std::string& cid, const std::string& name,
                         const std::vector<std::string>& tagList,
                         const std::string& st, const std::string& et,
                         const std::string& loc, int maxp,
                         const std::string& prizeInfo, const std::string& stat)
    : Activity(aid, cid, name, tagList, st, et, loc, maxp, stat), prize(clean(prizeInfo)) {}

void Competition::showInfo() const {
    showBaseInfo();
    std::cout << "奖项设置：" << prize << "\n";
}
