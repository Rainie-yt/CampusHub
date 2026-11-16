#include "Activity.h"
#include <algorithm>
#include <cctype>
#include <iostream>

using std::string;
using std::vector;

namespace {

// 去掉字符串首尾的空白字符。
// 参数：s 原字符串；返回：去掉首尾空白后的新字符串（s 本身不变）。
// 原理：两个下标 b、e 分别从头、从尾向中间收缩，遇到非空白字符停止，再截取 [b, e)。
string trimStr(const string& s) {
    size_t b = 0, e = s.size();
    // 转成 unsigned char 再传给 isspace：中文字节的值大于 127，直接传 char 会产生负数，属于未定义行为
    while (b < e && std::isspace(static_cast<unsigned char>(s[b]))) ++b;
    while (e > b && std::isspace(static_cast<unsigned char>(s[e - 1]))) --e;
    return s.substr(b, e - b);
}

// 判断活动状态字符串是否合法。
// 参数：s 待判断的状态；返回：只有 pending / approved / rejected 三种取值合法。
bool validStatus(const string& s) {
    return s == "pending" || s == "approved" || s == "rejected";
}

}  // namespace

// ============================================================
//  Activity 的实现。各成员函数的功能、参数、步骤见各函数前的注释。
// ============================================================

// 清洗要写入文件的文本字段。
// 功能：数据文件用 '|' 分隔字段、用换行分隔记录，所以文本里出现这三种字符会破坏文件结构，
//       这里统一把它们替换成空格。
// 参数：s 用户输入的原始文本；返回：清洗后的文本。
string Activity::clean(const string& s) {
    string r = s;
    for (size_t i = 0; i < r.size(); ++i) {
        if (r[i] == '|' || r[i] == '\n' || r[i] == '\r') r[i] = ' ';
    }
    return r;
}

// 规范化标签列表。
// 功能：标签在文件里用英文逗号拼接，所以标签本身不能含逗号和 '|'；同时去掉首尾空白、空标签和重复标签。
// 参数：in 用户输入的标签列表；返回：规范化后的标签列表（保持原有先后顺序）。
// 步骤：逐个标签 -> 把非法字符换成空格 -> 去首尾空白 -> 非空且尚未出现过才加入结果。
vector<string> Activity::normalizeTags(const vector<string>& in) {
    vector<string> out;
    for (size_t i = 0; i < in.size(); ++i) {
        string t = in[i];
        for (size_t j = 0; j < t.size(); ++j) {
            if (t[j] == '|' || t[j] == ',' || t[j] == '\n' || t[j] == '\r') t[j] = ' ';
        }
        t = trimStr(t);
        // std::find 在已有结果里查重，保证标签不重复
        if (!t.empty() && std::find(out.begin(), out.end(), t) == out.end()) out.push_back(t);
    }
    return out;
}

// 构造函数。
// 参数：aid 活动编号；cid 主办社团编号；name 名称；tagList 兴趣标签；st/et 开始/结束时间；
//       loc 地点；maxp 人数上限；stat 初始状态。
// 说明：构造函数只负责清洗文本、规范化标签，不检查时间和人数是否合法，
//       合法性由 isValid() 单独判断（构造函数没有返回值，不方便报告失败）。
//       status 传入非法值时回退为 pending，避免出现无法识别的状态。
Activity::Activity(const string& aid, const string& cid, const string& name,
                   const vector<string>& tagList, const string& st, const string& et,
                   const string& loc, int maxp, const string& stat)
    : activityId(aid), clubId(cid), activityName(clean(name)), tags(normalizeTags(tagList)),
      startTime(st), endTime(et), location(clean(loc)), maxPeople(maxp),
      status(validStatus(stat) ? stat : "pending") {}

// ---------------- 查询 ----------------

// 判断 uid 是否已在正式报名名单中。参数：uid 用户编号；返回：在名单中为 true。
// 原理：在 participants 里线性查找，名单规模很小，O(n) 足够。
bool Activity::hasParticipant(const string& uid) const {
    return std::find(participants.begin(), participants.end(), uid) != participants.end();
}

// 判断 uid 是否在候补名单中。参数：uid 用户编号；返回：在候补名单中为 true。
bool Activity::inWaitList(const string& uid) const {
    return std::find(waitList.begin(), waitList.end(), uid) != waitList.end();
}

// ---------------- 修改 ----------------

void Activity::setName(const string& name) { activityName = clean(name); }
void Activity::setLocation(const string& loc) { location = clean(loc); }
void Activity::setTags(const vector<string>& tagList) { tags = normalizeTags(tagList); }

// 修改人数上限。
// 参数：maxp 新的上限；返回：修改成功为 true。
// 规则：上限必须大于 0，且不能小于当前已报名人数，否则已报名的人会"超员"，这种修改直接拒绝。
bool Activity::setMax(int maxp) {
    if (maxp <= 0 || maxp < getCur()) return false;
    maxPeople = maxp;
    return true;
}

// 修改活动时间。
// 参数：st 新开始时间；et 新结束时间（格式均为 "YYYY-MM-DD HH:MM"）；返回：修改成功为 true。
// 规则：两个时间都必须合法，且开始时间严格早于结束时间；任何一条不满足都不修改原值。
// 说明：时间是补零的固定格式，所以 st < et 直接用字符串比较就等价于时间先后比较。
bool Activity::setTime(const string& st, const string& et) {
    if (!isValidTime(st) || !isValidTime(et) || !(st < et)) return false;
    startTime = st;
    endTime = et;
    return true;
}

// 修改审核状态。参数：stat 新状态；返回：状态合法并修改成功为 true。
bool Activity::setStatus(const string& stat) {
    if (!validStatus(stat)) return false;
    status = stat;
    return true;
}

// ---------------- 名单操作 ----------------

// 加入正式报名名单。
// 参数：uid 用户编号；返回：加入成功为 true。
// 失败情况：uid 为空、名额已满、该用户已在正式名单中。
// 说明：该函数不判断时间冲突和审核状态，这些由 ActivityManager::tryJoin 负责，这里只管名单本身。
bool Activity::addParticipant(const string& uid) {
    if (uid.empty() || isFull() || hasParticipant(uid)) return false;
    // 如果此人原本在候补名单里，转正时顺带移出候补
    // removeWait 会在不在候补时返回 false，这里忽略返回值是有意的
    removeWait(uid);
    participants.push_back(uid);
    return true;
}

// 从正式名单中移除。参数：uid 用户编号；返回：名单里有此人并移除成功为 true。
// 注意：移除后空出的名额由 ActivityManager 调用 popWait() 递补，本函数不负责递补。
bool Activity::removeParticipant(const string& uid) {
    vector<string>::iterator it = std::find(participants.begin(), participants.end(), uid);
    if (it == participants.end()) return false;
    participants.erase(it);
    return true;
}

// 加入候补名单（队尾）。参数：uid 用户编号；返回：加入成功为 true。
// 失败情况：uid 为空、已在正式名单、已在候补名单。
// 说明：候补按 push_back 的先后排队，先进先出，所以 waitList 的顺序就是递补顺序。
bool Activity::addWait(const string& uid) {
    if (uid.empty() || hasParticipant(uid) || inWaitList(uid)) return false;
    waitList.push_back(uid);
    return true;
}

// 从候补名单中移除。参数：uid 用户编号；返回：候补里有此人并移除成功为 true。
bool Activity::removeWait(const string& uid) {
    vector<string>::iterator it = std::find(waitList.begin(), waitList.end(), uid);
    if (it == waitList.end()) return false;
    waitList.erase(it);
    return true;
}

// 取出并删除候补名单的队首。
// 返回：队首用户编号；候补名单为空时返回空字符串。
// 用途：名额空出后，按"先候补先递补"的顺序补人。
string Activity::popWait() {
    if (waitList.empty()) return "";
    string uid = waitList.front();
    waitList.erase(waitList.begin());
    return uid;
}

// ---------------- 时间 ----------------

// 判断两个活动的时间段是否重叠。
// 参数：other 另一个活动；返回：重叠为 true。
// 原理：区间 [s1, e1) 与 [s2, e2) 重叠，当且仅当 s1 < e2 并且 s2 < e1。
//       用 "<" 而不是 "<="，所以首尾相接（如 14:00-16:00 与 16:00-17:00）不算冲突。
//       时间是固定格式字符串，直接比较字符串即可。
bool Activity::overlaps(const Activity& other) const {
    return startTime < other.endTime && other.startTime < endTime;
}

// 校验时间字符串是否是真实存在的时间（静态函数，不依赖具体活动）。
// 参数：t 时间字符串；返回：合法为 true。
// 步骤：1) 长度必须恰好为 16；2) 按位检查格式 YYYY-MM-DD HH:MM，分隔符位置固定，其余必须是数字；
//       3) 取出年月日时分，检查月 1~12、日 >= 1、时 <= 23、分 <= 59；
//       4) 查该月天数表，闰年的二月按 29 天，检查日不超过当月天数。
bool Activity::isValidTime(const string& t) {
    if (t.size() != 16) return false;
    for (size_t i = 0; i < t.size(); ++i) {
        if (i == 4 || i == 7) {
            if (t[i] != '-') return false;
        } else if (i == 10) {
            if (t[i] != ' ') return false;
        } else if (i == 13) {
            if (t[i] != ':') return false;
        } else if (!std::isdigit(static_cast<unsigned char>(t[i]))) {
            return false;
        }
    }
    // 走到这里说明格式已经确认全是数字，所以 stoi 不会抛异常
    int y = std::stoi(t.substr(0, 4));
    int mo = std::stoi(t.substr(5, 2));
    int d = std::stoi(t.substr(8, 2));
    int h = std::stoi(t.substr(11, 2));
    int mi = std::stoi(t.substr(14, 2));
    if (mo < 1 || mo > 12 || d < 1 || h > 23 || mi > 59) return false;
    static const int days[] = {31, 28, 31, 30, 31, 30, 31, 31, 30, 31, 30, 31};
    int dim = days[mo - 1];
    // 闰年规则：能被 4 整除且不能被 100 整除，或者能被 400 整除
    bool leap = (y % 4 == 0 && y % 100 != 0) || (y % 400 == 0);
    if (mo == 2 && leap) dim = 29;
    return d <= dim;
}

// 判断活动对象本身是否合法：开始、结束时间都合法，开始早于结束，人数上限大于 0。
// 用途：构造完活动后、真正加入管理器之前调用；读文件时用它过滤掉脏数据行。
bool Activity::isValid() const {
    return isValidTime(startTime) && isValidTime(endTime) && startTime < endTime && maxPeople > 0;
}

// ---------------- 序列化 / 显示 ----------------

// 把活动序列化成 activities.txt 中的一行（不含报名名单，名单存在 registrations.txt 里）。
// 格式：id|clubId|type|name|tags|start|end|location|max|status|extra
// 说明：标签用英文逗号拼接；type 和 extra 通过虚函数 getType()/getExtra() 取得，
//       所以三个子类不需要各写一遍序列化代码（多态的应用）。
string Activity::toFileString() const {
    string tagStr;
    for (size_t i = 0; i < tags.size(); ++i) {
        if (i > 0) tagStr += ",";
        tagStr += tags[i];
    }
    return activityId + "|" + clubId + "|" + getType() + "|" + activityName + "|" + tagStr + "|" +
           startTime + "|" + endTime + "|" + location + "|" + std::to_string(maxPeople) + "|" +
           status + "|" + getExtra();
}

// 把内部的英文状态转成界面显示的中文。参数：st 状态；返回：已通过 / 已拒绝 / 待审核。
// 说明：文件里存英文、界面显示中文，避免文件里的中文编码问题。
string Activity::statusText(const string& st) {
    if (st == "approved") return "已通过";
    if (st == "rejected") return "已拒绝";
    return "待审核";
}

// 显示所有类型共有的基本信息。子类的 showInfo() 先调用它，再补充自己特有的一行（主讲人、奖项等）。
void Activity::showBaseInfo() const {
    std::cout << "活动编号：" << activityId << "\n"
              << "名称：" << activityName << "\n"
              << "类型：" << getTypeName() << "    所属社团：" << clubId << "\n"
              << "时间：" << startTime << " ~ " << endTime << "\n"
              << "地点：" << location << "\n"
              << "标签：";
    for (size_t i = 0; i < tags.size(); ++i) {
        if (i > 0) std::cout << "、";
        std::cout << tags[i];
    }
    std::cout << "\n"
              << "人数：" << getCur() << "/" << maxPeople;
    if (isFull()) std::cout << "（已满）";
    std::cout << "    候补：" << waitList.size() << "人\n"
              << "积分：" << calcPoints() << "\n"
              << "状态：" << statusText(status) << "\n";
}
