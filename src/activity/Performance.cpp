#include "Performance.h"
#include <iostream>

// 构造函数。参数与基类 Activity 相同，另加 program（节目类型）。
// 说明：先用初始化列表调用基类构造函数完成共有字段，再清洗并保存自己特有的 programType。
Performance::Performance(const std::string& aid, const std::string& cid, const std::string& name,
                         const std::vector<std::string>& tagList,
                         const std::string& st, const std::string& et,
                         const std::string& loc, int maxp,
                         const std::string& program, const std::string& stat)
    : Activity(aid, cid, name, tagList, st, et, loc, maxp, stat), programType(clean(program)) {}

// 重写基类的纯虚函数 showInfo()：先显示共有信息（基类 showBaseInfo），再补充演出特有的节目类型。
void Performance::showInfo() const {
    showBaseInfo();
    std::cout << "节目类型：" << programType << "\n";
}
