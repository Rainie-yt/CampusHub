#include "Activity.h"
#include <algorithm>
#include <cctype>
#include <iostream>

using std::string;
using std::vector;

namespace {

string trimStr(const string& s) {
    size_t b = 0, e = s.size();
    while (b < e && std::isspace(static_cast<unsigned char>(s[b]))) ++b;
    while (e > b && std::isspace(static_cast<unsigned char>(s[e - 1]))) --e;
    return s.substr(b, e - b);
}

bool validStatus(const string& s) {
    return s == "pending" || s == "approved" || s == "rejected";
}

}  // namespace

string Activity::clean(const string& s) {
    string r = s;
    for (size_t i = 0; i < r.size(); ++i) {
        if (r[i] == '|' || r[i] == '\n' || r[i] == '\r') r[i] = ' ';
    }
    return r;
}

vector<string> Activity::normalizeTags(const vector<string>& in) {
    vector<string> out;
    for (size_t i = 0; i < in.size(); ++i) {
        string t = in[i];
        for (size_t j = 0; j < t.size(); ++j) {
            if (t[j] == '|' || t[j] == ',' || t[j] == '\n' || t[j] == '\r') t[j] = ' ';
        }
        t = trimStr(t);
        if (!t.empty() && std::find(out.begin(), out.end(), t) == out.end()) out.push_back(t);
    }
    return out;
}

Activity::Activity(const string& aid, const string& cid, const string& name,
                   const vector<string>& tagList, const string& st, const string& et,
                   const string& loc, int maxp, const string& stat)
    : activityId(aid), clubId(cid), activityName(clean(name)), tags(normalizeTags(tagList)),
      startTime(st), endTime(et), location(clean(loc)), maxPeople(maxp),
      status(validStatus(stat) ? stat : "pending") {}

// ---------------- 查询 ----------------

bool Activity::hasParticipant(const string& uid) const {
    return std::find(participants.begin(), participants.end(), uid) != participants.end();
}

bool Activity::inWaitList(const string& uid) const {
    return std::find(waitList.begin(), waitList.end(), uid) != waitList.end();
}

// ---------------- 修改 ----------------

void Activity::setName(const string& name) { activityName = clean(name); }
void Activity::setLocation(const string& loc) { location = clean(loc); }
void Activity::setTags(const vector<string>& tagList) { tags = normalizeTags(tagList); }

bool Activity::setMax(int maxp) {
    if (maxp <= 0 || maxp < getCur()) return false;
    maxPeople = maxp;
    return true;
}

bool Activity::setTime(const string& st, const string& et) {
    if (!isValidTime(st) || !isValidTime(et) || !(st < et)) return false;
    startTime = st;
    endTime = et;
    return true;
}

bool Activity::setStatus(const string& stat) {
    if (!validStatus(stat)) return false;
    status = stat;
    return true;
}

// ---------------- 名单操作 ----------------

bool Activity::addParticipant(const string& uid) {
    if (uid.empty() || isFull() || hasParticipant(uid)) return false;
    // 如果此人原本在候补名单里，转正时顺带移出候补
    removeWait(uid);
    participants.push_back(uid);
    return true;
}

bool Activity::removeParticipant(const string& uid) {
    vector<string>::iterator it = std::find(participants.begin(), participants.end(), uid);
    if (it == participants.end()) return false;
    participants.erase(it);
    return true;
}

bool Activity::addWait(const string& uid) {
    if (uid.empty() || hasParticipant(uid) || inWaitList(uid)) return false;
    waitList.push_back(uid);
    return true;
}

bool Activity::removeWait(const string& uid) {
    vector<string>::iterator it = std::find(waitList.begin(), waitList.end(), uid);
    if (it == waitList.end()) return false;
    waitList.erase(it);
    return true;
}

string Activity::popWait() {
    if (waitList.empty()) return "";
    string uid = waitList.front();
    waitList.erase(waitList.begin());
    return uid;
}

// ---------------- 时间 ----------------

bool Activity::overlaps(const Activity& other) const {
    return startTime < other.endTime && other.startTime < endTime;
}

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
    int y = std::stoi(t.substr(0, 4));
    int mo = std::stoi(t.substr(5, 2));
    int d = std::stoi(t.substr(8, 2));
    int h = std::stoi(t.substr(11, 2));
    int mi = std::stoi(t.substr(14, 2));
    if (mo < 1 || mo > 12 || d < 1 || h > 23 || mi > 59) return false;
    static const int days[] = {31, 28, 31, 30, 31, 30, 31, 31, 30, 31, 30, 31};
    int dim = days[mo - 1];
    bool leap = (y % 4 == 0 && y % 100 != 0) || (y % 400 == 0);
    if (mo == 2 && leap) dim = 29;
    return d <= dim;
}

bool Activity::isValid() const {
    return isValidTime(startTime) && isValidTime(endTime) && startTime < endTime && maxPeople > 0;
}

// ---------------- 序列化 / 显示 ----------------

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

string Activity::statusText(const string& st) {
    if (st == "approved") return "已通过";
    if (st == "rejected") return "已拒绝";
    return "待审核";
}

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
