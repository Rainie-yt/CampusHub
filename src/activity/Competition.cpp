#include "Competition.h"
#include <iostream>

// 构造函数。参数与基类 Activity 相同，另加 prizeInfo（奖项设置）。
// 说明：先用初始化列表调用基类构造函数完成共有字段，再清洗并保存自己特有的 prize。
Competition::Competition(const std::string& aid, const std::string& cid, const std::string& name,
                         const std::vector<std::string>& tagList,
                         const std::string& st, const std::string& et,
                         const std::string& loc, int maxp,
                         const std::string& prizeInfo, const std::string& stat)
    : Activity(aid, cid, name, tagList, st, et, loc, maxp, stat), prize(clean(prizeInfo)) {}

// 重写基类的纯虚函数 showInfo()：先显示共有信息（基类 showBaseInfo），再补充比赛特有的奖项设置。
void Competition::showInfo() const {
    showBaseInfo();
    std::cout << "奖项设置：" << prize << "\n";
}
