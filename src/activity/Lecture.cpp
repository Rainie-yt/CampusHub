#include "Lecture.h"
#include <iostream>

Lecture::Lecture(const std::string& aid, const std::string& cid, const std::string& name,
                 const std::vector<std::string>& tagList,
                 const std::string& st, const std::string& et,
                 const std::string& loc, int maxp,
                 const std::string& speakerName, const std::string& stat)
    : Activity(aid, cid, name, tagList, st, et, loc, maxp, stat), speaker(clean(speakerName)) {}

void Lecture::showInfo() const {
    showBaseInfo();
    std::cout << "主讲人：" << speaker << "\n";
}
