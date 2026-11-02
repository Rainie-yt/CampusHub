#include "ActivityManager.h"
#include "Lecture.h"
#include "Competition.h"
#include "Performance.h"
#include <algorithm>
#include <cctype>
#include <ctime>
#include <fstream>
#include <iomanip>
#include <iostream>
#include <set>
#include <sstream>

using std::cerr;
using std::cin;
using std::cout;
using std::string;
using std::vector;

namespace {

// 按分隔符切分，保留空字段（"a||b" -> {"a","","b"}）
vector<string> splitKeep(const string& s, char delim) {
    vector<string> r;
    string cur;
    for (size_t i = 0; i < s.size(); ++i) {
        if (s[i] == delim) {
            r.push_back(cur);
            cur.clear();
        } else {
            cur += s[i];
        }
    }
    r.push_back(cur);
    return r;
}

string trim(const string& s) {
    size_t b = 0, e = s.size();
    while (b < e && std::isspace(static_cast<unsigned char>(s[b]))) ++b;
    while (e > b && std::isspace(static_cast<unsigned char>(s[e - 1]))) --e;
    return s.substr(b, e - b);
}

void replaceAll(string& s, const string& from, const string& to) {
    size_t pos = 0;
    while ((pos = s.find(from, pos)) != string::npos) {
        s.replace(pos, from.size(), to);
        pos += to.size();
    }
}

// 解析标签输入：支持英文逗号、中文逗号、顿号
vector<string> parseTags(const string& input) {
    string t = input;
    replaceAll(t, "，", ",");
    replaceAll(t, "、", ",");
    vector<string> out;
    vector<string> parts = splitKeep(t, ',');
    for (size_t i = 0; i < parts.size(); ++i) {
        string x = trim(parts[i]);
        if (!x.empty() && std::find(out.begin(), out.end(), x) == out.end()) out.push_back(x);
    }
    return out;
}

bool parseInt(const string& s, int& out) {
    try {
        size_t pos = 0;
        int v = std::stoi(s, &pos);
        if (pos != s.size()) return false;
        out = v;
        return true;
    } catch (...) {
        return false;
    }
}

string readLine(const string& prompt) {
    cout << prompt;
    string line;
    if (!std::getline(cin, line)) return "";
    return trim(line);
}

// 反复询问直到输入合法的时间；直接回车返回 ""（表示放弃）
string askTime(const string& prompt) {
    while (true) {
        string s = readLine(prompt);
        if (s.empty() || Activity::isValidTime(s)) return s;
        cout << "时间格式应为 YYYY-MM-DD HH:MM（例如 2026-09-25 14:00），请重新输入，直接回车放弃。\n";
    }
}

string percent(double score) {
    std::ostringstream oss;
    oss << std::fixed << std::setprecision(1) << score * 100 << "%";
    return oss.str();
}

// 活动时间显示：同一天写成 "2026-12-01 14:00~16:00"，跨天则完整显示结束时间
string timeRange(const Activity& a) {
    const string& s = a.getStartTime();
    const string& e = a.getEndTime();
    if (s.substr(0, 10) == e.substr(0, 10)) return s + "~" + e.substr(11);
    return s + "~" + e;
}

// 列表中的一行：编号  名称（类型）  时间  已报名/上限人  [状态]
void printBrief(const Activity& a, const string& suffix = "") {
    cout << a.getActivityId() << "  " << a.getName() << "（" << a.getTypeName() << "）  "
         << timeRange(a) << "  " << a.getCur() << "/" << a.getMax() << "人";
    if (!suffix.empty()) cout << "  " << suffix;
    cout << "\n";
}

// 空位出现后，按候补顺序依次递补，返回被递补的 userId
vector<string> promoteWaiting(Activity& a) {
    vector<string> promoted;
    while (!a.isFull() && !a.getWaitList().empty()) {
        string u = a.popWait();
        if (a.addParticipant(u)) promoted.push_back(u);
    }
    return promoted;
}

}  // namespace

string joinResultText(JoinResult r) {
    switch (r) {
        case JoinResult::SUCCESS: return "报名成功！";
        case JoinResult::WAITLISTED: return "活动已满，已加入候补名单！";
        case JoinResult::CONFLICT: return "与已报名的活动时间冲突！";
        case JoinResult::ALREADY_JOINED: return "你已报名该活动！";
        case JoinResult::ALREADY_WAITING: return "你已在该活动的候补名单中！";
        case JoinResult::NOT_FOUND: return "未找到该活动！";
        case JoinResult::NOT_APPROVED: return "活动尚未通过审核！";
        case JoinResult::ENDED: return "活动已结束！";
    }
    return "未知结果";
}

// ======================= 构造 / 文件 =======================

ActivityManager::ActivityManager(const string& actFile, const string& regFileName)
    : nextId(3001), activityFile(actFile), regFile(regFileName), autoSave(true) {}

string ActivityManager::now() const {
    if (!fixedNow.empty()) return fixedNow;
    std::time_t t = std::time(nullptr);
    std::tm tmv;
#ifdef _WIN32
    localtime_s(&tmv, &t);
#else
    localtime_r(&t, &tmv);
#endif
    char buf[32];
    std::strftime(buf, sizeof(buf), "%Y-%m-%d %H:%M", &tmv);
    return buf;
}

string ActivityManager::generateId() { return std::to_string(nextId++); }

void ActivityManager::autoSaveIfNeeded() const {
    if (autoSave && !save()) cerr << "警告：活动数据保存失败，请确认 data 目录存在。\n";
}

std::unique_ptr<Activity> ActivityManager::makeActivity(
    const string& type, const string& aid, const string& cid, const string& name,
    const vector<string>& tags, const string& st, const string& et, const string& loc,
    int maxp, const string& status, const string& extra) {
    if (type == "Lecture")
        return std::unique_ptr<Activity>(new Lecture(aid, cid, name, tags, st, et, loc, maxp, extra, status));
    if (type == "Competition")
        return std::unique_ptr<Activity>(new Competition(aid, cid, name, tags, st, et, loc, maxp, extra, status));
    if (type == "Performance")
        return std::unique_ptr<Activity>(new Performance(aid, cid, name, tags, st, et, loc, maxp, extra, status));
    return nullptr;
}

bool ActivityManager::load() {
    activities.clear();
    bool ok = true;
    int maxNum = 3000;

    // ---- activities.txt ----
    {
        std::ifstream fin(activityFile.c_str());
        string line;
        int lineNo = 0;
        while (std::getline(fin, line)) {
            ++lineNo;
            if (lineNo == 1 && line.size() >= 3 && static_cast<unsigned char>(line[0]) == 0xEF &&
                static_cast<unsigned char>(line[1]) == 0xBB && static_cast<unsigned char>(line[2]) == 0xBF)
                line.erase(0, 3);   // 去掉 UTF-8 BOM
            if (!line.empty() && line[line.size() - 1] == '\r') line.erase(line.size() - 1);
            if (trim(line).empty() || line[0] == '#') continue;

            vector<string> f = splitKeep(line, '|');
            int maxp = 0;
            std::unique_ptr<Activity> act;
            bool good = f.size() == 11 && !f[0].empty() && parseInt(f[8], maxp) &&
                        (f[9] == "pending" || f[9] == "approved" || f[9] == "rejected");
            if (good) {
                act = makeActivity(f[2], f[0], f[1], f[3], parseTags(f[4]), f[5], f[6], f[7], maxp, f[9], f[10]);
                good = act && act->isValid() && findActivity(f[0]) == nullptr;
            }
            if (!good) {
                cerr << "警告：" << activityFile << " 第 " << lineNo << " 行格式有误，已跳过。\n";
                ok = false;
                continue;
            }
            int num = 0;
            if (parseInt(f[0], num) && num > maxNum) maxNum = num;
            activities.push_back(std::move(act));
        }
    }
    nextId = maxNum + 1;

    // ---- registrations.txt ----
    {
        std::ifstream rin(regFile.c_str());
        string line;
        int lineNo = 0;
        while (std::getline(rin, line)) {
            ++lineNo;
            if (lineNo == 1 && line.size() >= 3 && static_cast<unsigned char>(line[0]) == 0xEF &&
                static_cast<unsigned char>(line[1]) == 0xBB && static_cast<unsigned char>(line[2]) == 0xBF)
                line.erase(0, 3);
            if (!line.empty() && line[line.size() - 1] == '\r') line.erase(line.size() - 1);
            if (trim(line).empty() || line[0] == '#') continue;

            vector<string> f = splitKeep(line, '|');
            Activity* a = f.size() == 3 ? findActivity(f[1]) : nullptr;
            bool good = a != nullptr && !f[0].empty();
            if (good) {
                if (f[2] == "registered") good = a->addParticipant(f[0]);
                else if (f[2] == "waitlist") good = a->addWait(f[0]);
                else good = false;
            }
            if (!good) {
                cerr << "警告：" << regFile << " 第 " << lineNo << " 行无效，已跳过。\n";
                ok = false;
            }
        }
    }
    return ok;
}

bool ActivityManager::save() const {
    std::ofstream fout(activityFile.c_str());
    std::ofstream rout(regFile.c_str());
    if (!fout || !rout) return false;
    for (size_t i = 0; i < activities.size(); ++i) {
        const Activity& a = *activities[i];
        fout << a.toFileString() << "\n";
        for (size_t j = 0; j < a.getParticipants().size(); ++j)
            rout << a.getParticipants()[j] << "|" << a.getActivityId() << "|registered\n";
        for (size_t j = 0; j < a.getWaitList().size(); ++j)
            rout << a.getWaitList()[j] << "|" << a.getActivityId() << "|waitlist\n";
    }
    fout.close();
    rout.close();
    return !fout.fail() && !rout.fail();
}

// ======================= 活动管理 =======================

string ActivityManager::addActivity(std::unique_ptr<Activity> act) {
    if (!act || !act->isValid()) return "";
    act->activityId = generateId();
    string id = act->getActivityId();
    activities.push_back(std::move(act));
    autoSaveIfNeeded();
    return id;
}

bool ActivityManager::publishActivity(const string& clubId) {
    cout << "========发布活动========\n";
    string t = readLine("活动类型（1 讲座  2 比赛  3 演出）：");
    string type, extraPrompt;
    if (t == "1") { type = "Lecture"; extraPrompt = "主讲人："; }
    else if (t == "2") { type = "Competition"; extraPrompt = "奖项设置："; }
    else if (t == "3") { type = "Performance"; extraPrompt = "节目类型："; }
    else { cout << "类型无效，已取消。\n"; return false; }

    string name = readLine("活动名称：");
    if (name.empty()) { cout << "名称不能为空，已取消。\n"; return false; }
    vector<string> tags = parseTags(readLine("兴趣标签（逗号分隔，如 科技,竞赛）："));
    string st = askTime("开始时间（YYYY-MM-DD HH:MM）：");
    string et = askTime("结束时间（YYYY-MM-DD HH:MM）：");
    string loc = readLine("活动地点：");
    int maxp = 0;
    if (!parseInt(readLine("人数上限："), maxp) || maxp <= 0) {
        cout << "人数上限必须是正整数，已取消。\n";
        return false;
    }
    string extra = readLine(extraPrompt);

    std::unique_ptr<Activity> act = makeActivity(type, "", clubId, name, tags, st, et, loc, maxp, "pending", extra);
    if (!act || !act->isValid()) {
        cout << "活动信息不合法（请检查时间格式，且结束时间必须晚于开始时间），已取消。\n";
        return false;
    }
    string id = addActivity(std::move(act));
    if (id.empty()) return false;
    cout << "活动已提交，编号为：" << id << "，请等待管理员审核！\n";
    return true;
}

bool ActivityManager::modifyActivity(const string& activityId, const string& operatorClubId) {
    Activity* a = findActivity(activityId);
    if (!a) { cout << "未找到该活动！\n"; return false; }
    if (!operatorClubId.empty() && a->getClubId() != operatorClubId) { cout << "无权限：只能修改本社团的活动。\n"; return false; }

    a->showInfo();
    cout << "以下直接回车表示保持不变。\n";
    bool changed = false;

    string s = readLine("新名称：");
    if (!s.empty()) { a->setName(s); changed = true; }
    s = readLine("新标签（逗号分隔）：");
    if (!s.empty()) { a->setTags(parseTags(s)); changed = true; }
    s = readLine("新地点：");
    if (!s.empty()) { a->setLocation(s); changed = true; }

    s = readLine("新人数上限：");
    if (!s.empty()) {
        int m = 0;
        if (!parseInt(s, m) || !a->setMax(m)) {
            cout << "人数上限无效（必须大于 0 且不小于当前报名人数），该项未修改。\n";
        } else {
            changed = true;
            vector<string> promoted = promoteWaiting(*a);   // 名额增加后候补自动递补
            for (size_t i = 0; i < promoted.size(); ++i) cout << "候补用户 " << promoted[i] << " 已递补入选。\n";
        }
    }

    if (a->getParticipants().empty() && a->getWaitList().empty()) {
        string ns = readLine("新开始时间（YYYY-MM-DD HH:MM）：");
        string ne = readLine("新结束时间（YYYY-MM-DD HH:MM）：");
        if (!ns.empty() || !ne.empty()) {
            if (ns.empty()) ns = a->getStartTime();
            if (ne.empty()) ne = a->getEndTime();
            if (a->setTime(ns, ne)) changed = true;
            else cout << "时间无效（格式错误或结束不晚于开始），该项未修改。\n";
        }
    } else {
        cout << "该活动已有人报名或候补，时间不可修改。\n";
    }

    if (changed) {
        autoSaveIfNeeded();
        cout << "活动信息修改成功！\n";
    } else {
        cout << "没有任何修改。\n";
    }
    return changed;
}

bool ActivityManager::deleteActivity(const string& activityId, const string& operatorClubId) {
    const Activity* a = findActivity(activityId);
    if (!a || (!operatorClubId.empty() && a->getClubId() != operatorClubId)) return false;
    return removeActivity(activityId);
}

bool ActivityManager::removeActivity(const string& activityId) {
    for (size_t i = 0; i < activities.size(); ++i) {
        if (activities[i]->getActivityId() == activityId) {
            activities.erase(activities.begin() + i);
            autoSaveIfNeeded();
            return true;
        }
    }
    return false;
}

bool ActivityManager::auditActivity(const string& activityId, bool approve) {
    Activity* a = findActivity(activityId);
    if (!a) return false;
    a->setStatus(approve ? "approved" : "rejected");
    autoSaveIfNeeded();
    return true;
}

// ======================= 查询 =======================

Activity* ActivityManager::findActivity(const string& activityId) {
    for (size_t i = 0; i < activities.size(); ++i)
        if (activities[i]->getActivityId() == activityId) return activities[i].get();
    return nullptr;
}

const Activity* ActivityManager::findActivity(const string& activityId) const {
    for (size_t i = 0; i < activities.size(); ++i)
        if (activities[i]->getActivityId() == activityId) return activities[i].get();
    return nullptr;
}

vector<const Activity*> ActivityManager::getAllActivities() const {
    vector<const Activity*> r;
    for (size_t i = 0; i < activities.size(); ++i) r.push_back(activities[i].get());
    return r;
}

vector<const Activity*> ActivityManager::getApprovedActivities() const {
    vector<const Activity*> r;
    for (size_t i = 0; i < activities.size(); ++i)
        if (activities[i]->isApproved()) r.push_back(activities[i].get());
    return r;
}

vector<const Activity*> ActivityManager::getPendingActivities() const {
    vector<const Activity*> r;
    for (size_t i = 0; i < activities.size(); ++i)
        if (activities[i]->getStatus() == "pending") r.push_back(activities[i].get());
    return r;
}

vector<const Activity*> ActivityManager::getActivitiesByClub(const string& clubId) const {
    vector<const Activity*> r;
    for (size_t i = 0; i < activities.size(); ++i)
        if (activities[i]->getClubId() == clubId) r.push_back(activities[i].get());
    return r;
}

void ActivityManager::showApprovedActivities() const {
    vector<const Activity*> v = getApprovedActivities();
    cout << "========活动列表========\n";
    if (v.empty()) cout << "暂无活动。\n";
    for (size_t i = 0; i < v.size(); ++i) printBrief(*v[i]);
}

void ActivityManager::showPendingActivities() const {
    vector<const Activity*> v = getPendingActivities();
    cout << "========待审核活动========\n";
    if (v.empty()) cout << "暂无待审核的活动。\n";
    for (size_t i = 0; i < v.size(); ++i) printBrief(*v[i]);
}

void ActivityManager::showActivitiesByClub(const string& clubId) const {
    vector<const Activity*> v = getActivitiesByClub(clubId);
    cout << "========本社团活动========\n";
    if (v.empty()) cout << "暂无活动。\n";
    for (size_t i = 0; i < v.size(); ++i) {
        printBrief(*v[i], Activity::statusText(v[i]->getStatus()));
    }
}

void ActivityManager::showActivityDetail(const string& activityId) const {
    const Activity* a = findActivity(activityId);
    if (!a) { cout << "未找到该活动！\n"; return; }
    a->showInfo();
}

// ======================= 报名 =======================

JoinResult ActivityManager::tryJoin(const string& userId, const string& activityId) {
    Activity* a = findActivity(activityId);
    if (!a) return JoinResult::NOT_FOUND;
    if (!a->isApproved()) return JoinResult::NOT_APPROVED;
    if (a->hasEnded(now())) return JoinResult::ENDED;
    if (a->hasParticipant(userId)) return JoinResult::ALREADY_JOINED;
    if (a->inWaitList(userId)) return JoinResult::ALREADY_WAITING;
    if (findConflict(userId, *a) != nullptr) return JoinResult::CONFLICT;

    JoinResult result;
    if (a->addParticipant(userId)) {
        result = JoinResult::SUCCESS;
    } else if (a->addWait(userId)) {   // 只会因为已满而走到这里
        result = JoinResult::WAITLISTED;
    } else {
        return JoinResult::NOT_FOUND;  // userId 为空等非法情况
    }
    autoSaveIfNeeded();
    return result;
}

bool ActivityManager::joinActivity(const string& userId, const string& activityId) {
    JoinResult r = tryJoin(userId, activityId);
    cout << joinResultText(r);
    if (r == JoinResult::CONFLICT) {
        const Activity* target = findActivity(activityId);
        const Activity* other = target ? findConflict(userId, *target) : nullptr;
        cout << "\n";
        if (other) cout << "冲突活动：" << other->getName() << "（" << timeRange(*other) << "）。";
    }
    cout << "\n";
    return r == JoinResult::SUCCESS || r == JoinResult::WAITLISTED;
}

bool ActivityManager::cancelActivity(const string& userId, const string& activityId, string* promotedUser) {
    if (promotedUser) promotedUser->clear();
    Activity* a = findActivity(activityId);
    if (!a || a->hasEnded(now())) return false;

    if (a->removeParticipant(userId)) {
        vector<string> promoted = promoteWaiting(*a);
        if (promotedUser && !promoted.empty()) *promotedUser = promoted.front();
        autoSaveIfNeeded();
        return true;
    }
    if (a->removeWait(userId)) {
        autoSaveIfNeeded();
        return true;
    }
    return false;
}

void ActivityManager::showMyActivities(const string& userId) const {
    cout << "========我的活动========\n";
    bool any = false;
    for (size_t i = 0; i < activities.size(); ++i) {
        const Activity& a = *activities[i];
        if (a.hasParticipant(userId)) {
            printBrief(a, a.hasEnded(now()) ? "已报名（已结束）" : "已报名");
            any = true;
        } else if (a.inWaitList(userId)) {
            size_t pos = std::find(a.getWaitList().begin(), a.getWaitList().end(), userId) - a.getWaitList().begin();
            printBrief(a, "候补第" + std::to_string(pos + 1) + "位");
            any = true;
        }
    }
    if (!any) cout << "你还没有报名任何活动。\n";
}

// ======================= 报名管理 =======================

void ActivityManager::showRegistration(const string& activityId) const {
    const Activity* a = findActivity(activityId);
    if (!a) { cout << "未找到该活动！\n"; return; }
    cout << "========报名情况========\n"
         << a->getName() << "（" << a->getCur() << "/" << a->getMax() << "）\n";
    for (size_t i = 0; i < a->getParticipants().size(); ++i)
        cout << i + 1 << "." << a->getParticipants()[i] << "\n";
    if (a->getParticipants().empty()) cout << "暂无报名。\n";
    if (!a->getWaitList().empty()) {
        cout << "候补名单：\n";
        for (size_t i = 0; i < a->getWaitList().size(); ++i)
            cout << i + 1 << "." << a->getWaitList()[i] << "\n";
    }
}

void ActivityManager::manageRegistration(const string& activityId, const string& operatorClubId) {
    Activity* a = findActivity(activityId);
    if (!a) { cout << "未找到该活动！\n"; return; }
    if (!operatorClubId.empty() && a->getClubId() != operatorClubId) { cout << "无权限：只能管理本社团的活动。\n"; return; }
    while (true) {
        showRegistration(activityId);
        string uid = readLine("请输入要移除的用户Id（直接回车返回）：");
        if (uid.empty()) return;
        string promoted;
        if (cancelActivity(uid, activityId, &promoted)) {
            cout << "已移除 " << uid << "！\n";
            if (!promoted.empty()) cout << "候补用户 " << promoted << " 已递补入选。\n";
        } else {
            cout << "移除失败：该用户不在名单中，或活动已结束。\n";
        }
    }
}

// ======================= 智能服务 =======================

const Activity* ActivityManager::findConflict(const string& userId, const Activity& target) const {
    for (size_t i = 0; i < activities.size(); ++i) {
        const Activity& other = *activities[i];
        if (other.getActivityId() == target.getActivityId()) continue;
        if (!other.isApproved()) continue;
        if (!other.hasParticipant(userId) && !other.inWaitList(userId)) continue;
        if (other.overlaps(target)) return &other;
    }
    return nullptr;
}

double ActivityManager::matchScore(const vector<string>& interests, const vector<string>& tags) {
    std::set<string> a(interests.begin(), interests.end());
    std::set<string> b(tags.begin(), tags.end());
    if (a.empty() || b.empty()) return 0.0;
    size_t inter = 0;
    for (std::set<string>::const_iterator it = a.begin(); it != a.end(); ++it)
        if (b.count(*it)) ++inter;
    size_t uni = a.size() + b.size() - inter;
    return static_cast<double>(inter) / static_cast<double>(uni);
}

vector<std::pair<const Activity*, double> >
ActivityManager::recommend(const vector<string>& interests, const string& userId) const {
    vector<std::pair<const Activity*, double> > result;
    string current = now();
    for (size_t i = 0; i < activities.size(); ++i) {
        const Activity& a = *activities[i];
        if (!a.isApproved() || a.hasEnded(current)) continue;
        if (!userId.empty() && (a.hasParticipant(userId) || a.inWaitList(userId))) continue;
        double score = matchScore(interests, a.getTags());
        if (score > 0.0) result.push_back(std::make_pair(&a, score));
    }
    std::stable_sort(result.begin(), result.end(),
                     [](const std::pair<const Activity*, double>& x, const std::pair<const Activity*, double>& y) {
                         if (x.second != y.second) return x.second > y.second;
                         return x.first->getStartTime() < y.first->getStartTime();
                     });
    return result;
}

void ActivityManager::recommendActivity(const vector<string>& interests, const string& userId) const {
    vector<std::pair<const Activity*, double> > r = recommend(interests, userId);
    cout << "========== 为你推荐 ==========\n";
    if (r.empty()) {
        cout << "暂时没有与你的兴趣匹配的活动。\n";
        return;
    }
    size_t n = std::min<size_t>(r.size(), 5);
    for (size_t i = 0; i < n; ++i) {
        cout << i + 1 << ". [" << r[i].first->getActivityId() << "] " << r[i].first->getName()
             << "  匹配度 " << percent(r[i].second);
        if (r[i].first->isFull()) cout << "（已满，可候补）";
        cout << "\n";
    }
}

int ActivityManager::getUserPoints(const string& userId) const {
    int total = 0;
    string current = now();
    for (size_t i = 0; i < activities.size(); ++i) {
        const Activity& a = *activities[i];
        if (a.isApproved() && a.hasEnded(current) && a.hasParticipant(userId)) total += a.calcPoints();
    }
    return total;
}

// ======================= 给社团模块的统计 =======================

int ActivityManager::getActivityCount(const string& clubId) const {
    int n = 0;
    for (size_t i = 0; i < activities.size(); ++i)
        if (activities[i]->isApproved() && activities[i]->getClubId() == clubId) ++n;
    return n;
}

int ActivityManager::getParticipantCount(const string& clubId) const {
    int n = 0;
    for (size_t i = 0; i < activities.size(); ++i)
        if (activities[i]->isApproved() && activities[i]->getClubId() == clubId) n += activities[i]->getCur();
    return n;
}
