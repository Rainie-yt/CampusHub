#include "ActivityManager.h"
#include "Lecture.h"
#include "Competition.h"
#include "Performance.h"
#include "../common/TextUtils.h"
#include "../common/FileStore.h"
#include "../common/DataFile.h"
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

// 名称列的宽度：取这一组活动里最长的"名称（类型）"再加2格，名称再长也能对齐
int nameWidth(const vector<const Activity*>& v) {
    int w = 20;
    for (size_t i = 0; i < v.size(); ++i) {
        int cur = displayWidth(v[i]->getName() + "（" + v[i]->getTypeName() + "）") + 2;
        if (cur > w) w = cur;
    }
    return w;
}

// 列表中的一行：编号  名称（类型）  时间  已报名/上限人  [状态]
void printBrief(const Activity& a, int nameW, const string& suffix = "") {
    // 按显示宽度补空格，让各列对齐（中文占2格）
    string count = std::to_string(a.getCur()) + "/" + std::to_string(a.getMax()) + "人";
    cout << padText(a.getActivityId(), 6) << padText(a.getName() + "（" + a.getTypeName() + "）", nameW)
         << padText(timeRange(a), 24) << padText(count, 10);
    if (!suffix.empty()) cout << suffix;
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
        case JoinResult::SAVE_FAILED: return "保存失败，报名未生效，请稍后重试！";
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

bool ActivityManager::autoSaveIfNeeded() const {
    if(!autoSave || save()) return true;
    cerr<<"活动数据保存失败，本次修改未保存。\n";
    return false;
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
    // 两个文件均完整读取、校验后才替换，失败保留之前的活动和报名。
    string activityText, registrationText;
    if (!DataFile::read(activityFile, activityText) || !DataFile::read(regFile, registrationText)) return false;
    ActivityManager loaded(activityFile, regFile);
    int maxNum = 3000;

    // ---- activities.txt ----
    {
        std::istringstream fin(activityText);
        string line;
        size_t lineNo = 0;
        while (DataFile::nextLine(fin, line, lineNo)) {
            if (trim(line).empty() || line[0] == '#') continue;

            vector<string> f = splitKeep(line, '|');
            int maxp = 0;
            std::unique_ptr<Activity> act;
            bool good = f.size() == 11 && !f[0].empty() && parseInt(f[8], maxp) &&
                        (f[9] == "pending" || f[9] == "approved" || f[9] == "rejected");
            if (good) {
                act = makeActivity(f[2], f[0], f[1], f[3], parseTags(f[4]), f[5], f[6], f[7], maxp, f[9], f[10]);
                good = act && act->isValid() && loaded.findActivity(f[0]) == nullptr;
            }
            if (!good) {
                return DataFile::error(activityFile, lineNo, "活动字段不完整、类型/时间/人数/状态不合法或编号重复");
            }
            int num = 0;
            if (parseInt(f[0], num) && num > maxNum) maxNum = num;
            loaded.activities.push_back(std::move(act));
        }
    }

    // ---- registrations.txt ----
    {
        std::istringstream rin(registrationText);
        string line;
        size_t lineNo = 0;
        while (DataFile::nextLine(rin, line, lineNo)) {
            if (trim(line).empty() || line[0] == '#') continue;

            vector<string> f = splitKeep(line, '|');
            Activity* a = f.size() == 3 ? loaded.findActivity(f[1]) : nullptr;
            bool good = a != nullptr && !f[0].empty();
            if (good) {
                if (f[2] == "registered") good = a->addParticipant(f[0]);
                else if (f[2] == "waitlist") good = a->addWait(f[0]);
                else good = false;
            }
            if (!good) {
                return DataFile::error(regFile, lineNo, "报名记录字段、活动编号或状态不合法，或报名/候补重复、超出名额");
            }
        }
    }
    activities.swap(loaded.activities);
    nextId = maxNum + 1;
    return true;
}

bool ActivityManager::save() const {
    std::ostringstream fout,rout;
    for(const auto& entry:activities){
        const Activity& a=*entry;
        fout<<a.toFileString()<<'\n';
        for(const string& uid:a.getParticipants()) rout<<uid<<'|'<<a.getActivityId()<<"|registered\n";
        for(const string& uid:a.getWaitList()) rout<<uid<<'|'<<a.getActivityId()<<"|waitlist\n";
    }
    FileStore::Writes writes;
    writes.push_back(std::make_pair(activityFile,fout.str()));
    writes.push_back(std::make_pair(regFile,rout.str()));
    return FileStore::write(writes);
}

// ======================= 活动管理 =======================

string ActivityManager::addActivity(std::unique_ptr<Activity> act) {
    if(!act || !act->isValid() || act->hasEnded(now())) return "";
    int previousId=nextId;
    act->activityId=generateId();
    string id=act->getActivityId();
    activities.push_back(std::move(act));
    if(!autoSaveIfNeeded()){
        activities.pop_back();
        nextId=previousId;
        return "";
    }
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
    if (act->hasEnded(now())) {
        cout << "结束时间必须晚于当前时间，活动未提交。\n";
        return false;
    }
    string id = addActivity(std::move(act));
    if (id.empty()) { cout << "提交失败：结束时间已到或文件保存失败，活动未提交。\n"; return false; }
    cout << "活动已提交，编号为：" << id << "，请等待管理员审核！\n";
    return true;
}

bool ActivityManager::modifyActivity(const string& activityId, const string& operatorClubId) {
    Activity* a = findActivity(activityId);
    if (!a) { cout << "未找到该活动！\n"; return false; }
    if (!operatorClubId.empty() && a->getClubId() != operatorClubId) { cout << "无权限：只能修改本社团的活动。\n"; return false; }

    if(a->hasEnded(now())) { cout<<"已结束活动保留为历史，不能修改。\n"; return false; }
    ActivityManager previous;
    copyStateTo(previous);
    a->showInfo();
    cout << "\n以下直接回车表示保持不变；修改成功后需重新审核，已有报名和候补保留。\n";
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
            // 改动将重新送审，候补在审核通过后再递补。
        }
    }

    if (a->getParticipants().empty() && a->getWaitList().empty()) {
        string ns = readLine("新开始时间（YYYY-MM-DD HH:MM）：");
        string ne = readLine("新结束时间（YYYY-MM-DD HH:MM）：");
        if (!ns.empty() || !ne.empty()) {
            if (ns.empty()) ns = a->getStartTime();
            if (ne.empty()) ne = a->getEndTime();
            if (!Activity::isValidTime(ns) || !Activity::isValidTime(ne) || ne <= ns)
                cout << "时间无效（格式错误或结束不晚于开始），该项未修改。\n";
            else if (ne <= now())
                cout << "结束时间必须晚于当前时间，该项未修改。\n";
            else if (a->setTime(ns, ne)) changed = true;
        }
    } else {
        cout << "该活动已有人报名或候补，时间不可修改。\n";
    }

    if (changed) {
        // 输入期间也可能到达结束时间，提交前再次检查并撤销本次全部修改。
        if (a->hasEnded(now())) {
            swapState(previous);
            cout << "活动已结束，本次修改未保存。\n";
            return false;
        }
        a->setStatus("pending");
        if(!autoSaveIfNeeded()) { swapState(previous); return false; }
        cout << "活动信息已修改并重新提交审核，已有报名和候补保留，审核通过前暂停新报名。\n";
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
    for(size_t i=0;i<activities.size();++i){
        if(activities[i]->getActivityId()!=activityId) continue;
        std::unique_ptr<Activity> old=std::move(activities[i]);
        activities.erase(activities.begin()+i);
        if(!autoSaveIfNeeded()){
            activities.insert(activities.begin()+i,std::move(old));
            return false;
        }
        return true;
    }
    return false;
}

bool ActivityManager::auditActivity(const string& activityId, bool approve) {
    return tryAuditActivity(activityId, approve) == AuditResult::SUCCESS;
}

AuditResult ActivityManager::tryAuditActivity(const string& activityId, bool approve) {
    Activity* a=findActivity(activityId);
    if(!a) return AuditResult::NOT_FOUND;
    if(a->getStatus()!="pending") return AuditResult::NOT_PENDING;
    // 过期不能通过，但仍可拒绝以移出待审核列表；不删除历史报名。
    if(approve && a->hasEnded(now())) return AuditResult::ENDED;
    ActivityManager previous;
    copyStateTo(previous);
    a->setStatus(approve?"approved":"rejected");
    if(approve) promoteWaiting(*a);
    if(!autoSaveIfNeeded()){ swapState(previous); return AuditResult::SAVE_FAILED; }
    return AuditResult::SUCCESS;
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
    // 学生浏览时只显示已通过且未结束的活动（已结束的活动不能报名，没必要列出来）。
    // 注意：getApprovedActivities() 仍然包含已结束的活动，管理员统计要用。
    string current = now();
    vector<const Activity*> v;
    for (size_t i = 0; i < activities.size(); ++i)
        if (activities[i]->isApproved() && !activities[i]->hasEnded(current)) v.push_back(activities[i].get());
    cout << "========活动列表========\n";
    if (v.empty()) cout << "暂无活动。\n";
    int w = nameWidth(v);
    for (size_t i = 0; i < v.size(); ++i) printBrief(*v[i], w);
}

void ActivityManager::showPendingActivities() const {
    vector<const Activity*> v = getPendingActivities();
    cout << "========待审核活动========\n";
    if (v.empty()) cout << "暂无待审核的活动。\n";
    int w = nameWidth(v);
    string current = now();
    for (size_t i = 0; i < v.size(); ++i)
        printBrief(*v[i], w, v[i]->hasEnded(current) ? "已结束（仅可拒绝）" : "");
}

void ActivityManager::showActivitiesByClub(const string& clubId) const {
    vector<const Activity*> v = getActivitiesByClub(clubId);
    cout << "========本社团活动========\n";
    if (v.empty()) cout << "暂无活动。\n";
    int w = nameWidth(v);
    for (size_t i = 0; i < v.size(); ++i) {
        printBrief(*v[i], w, Activity::statusText(v[i]->getStatus()));
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

    ActivityManager previous;
    copyStateTo(previous);
    JoinResult result;
    if (a->addParticipant(userId)) {
        result = JoinResult::SUCCESS;
    } else if (a->addWait(userId)) {   // 只会因为已满而走到这里
        result = JoinResult::WAITLISTED;
    } else {
        return JoinResult::NOT_FOUND;  // userId 为空等非法情况
    }
    if(!autoSaveIfNeeded()){ swapState(previous); return JoinResult::SAVE_FAILED; }
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

bool ActivityManager::cancelActivity(const string& userId,const string& activityId,string* promotedUser){
    if(promotedUser) promotedUser->clear();
    Activity* a=findActivity(activityId);
    if(!a || a->hasEnded(now()) || (!a->hasParticipant(userId)&&!a->inWaitList(userId))) return false;
    ActivityManager previous;
    copyStateTo(previous);
    vector<string> promoted;
    if(a->removeParticipant(userId)){
        if(a->isApproved()) promoted=promoteWaiting(*a);
    } else a->removeWait(userId);
    if(!autoSaveIfNeeded()){ swapState(previous); return false; }
    if(promotedUser&&!promoted.empty()) *promotedUser=promoted.front();
    return true;
}

void ActivityManager::showMyActivities(const string& userId) const {
    cout << "========我的报名活动========\n";
    // 先找出报名或候补的活动，算好名称列宽度再输出
    vector<const Activity*> mine;
    for (size_t i = 0; i < activities.size(); ++i) {
        const Activity& a = *activities[i];
        if (a.hasParticipant(userId) || a.inWaitList(userId)) mine.push_back(&a);
    }
    int w = nameWidth(mine);
    for (size_t i = 0; i < mine.size(); ++i) {
        const Activity& a = *mine[i];
        string auditNote=a.getStatus()=="pending"?"（待审核，暂停新报名）":
            (a.getStatus()=="rejected"?"（未通过审核，可取消报名）":"");
        if (a.hasParticipant(userId)) {
            printBrief(a, w, (a.hasEnded(now()) ? "已报名（已结束）" : "已报名")+auditNote);
        } else {
            size_t pos = std::find(a.getWaitList().begin(), a.getWaitList().end(), userId) - a.getWaitList().begin();
            printBrief(a, w, "候补第" + std::to_string(pos + 1) + "位"+auditNote);
        }
    }
    if (mine.empty()) cout << "你还没有报名任何活动。\n";
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
        cout << "\n候补名单：\n";
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
        cout << "\n";
        string uid = readLine("请输入要移除的用户Id（直接回车返回）：");
        if (uid.empty()) return;
        string promoted;
        if (cancelActivity(uid, activityId, &promoted)) {
            cout << "已移除 " << uid << "！\n";
            if (!promoted.empty()) cout << "候补用户 " << promoted << " 已递补入选。\n";
        } else {
            cout << "移除失败：该用户不在名单中、活动已结束或保存失败。\n";
        }
        cout << "\n";
    }
}

// ======================= 智能服务 =======================

const Activity* ActivityManager::findConflict(const string& userId, const Activity& target) const {
    for (size_t i = 0; i < activities.size(); ++i) {
        const Activity& other = *activities[i];
        if (other.getActivityId() == target.getActivityId()) continue;
        // 待重新审核或驳回期间保留的报名也占用时间；用户取消后释放。
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


void ActivityManager::copyStateTo(ActivityManager& target) const {
    if(&target==this) return;
    target.activities.clear();
    target.nextId=nextId;
    for(const auto& entry:activities){
        const Activity& a=*entry;
        auto copy=makeActivity(a.getType(),a.getActivityId(),a.getClubId(),a.getName(),a.getTags(),
            a.getStartTime(),a.getEndTime(),a.getLocation(),a.getMax(),a.getStatus(),a.getExtra());
        copy->participants=a.participants;
        copy->waitList=a.waitList;
        target.activities.push_back(std::move(copy));
    }
}
void ActivityManager::swapState(ActivityManager& other){
    activities.swap(other.activities);
    std::swap(nextId,other.nextId);
}
bool ActivityManager::resubmitActivity(const string& activityId,const string& operatorClubId){
    Activity* a=findActivity(activityId);
    if(!a || a->getStatus()!="rejected" || a->hasEnded(now()) ||
       (!operatorClubId.empty()&&a->getClubId()!=operatorClubId)) return false;
    a->setStatus("pending");
    if(!autoSaveIfNeeded()){ a->setStatus("rejected"); return false; }
    return true;
}
int ActivityManager::cancelUnendedByClub(const string& clubId){
    ActivityManager previous;
    copyStateTo(previous);
    int count=0;
    string current=now();
    for(auto it=activities.begin();it!=activities.end();){
        if((*it)->getClubId()==clubId&&!(*it)->hasEnded(current)){
            it=activities.erase(it);
            ++count;
        } else ++it;
    }
    if(count>0&&!autoSaveIfNeeded()){ swapState(previous); return -1; }
    return count;
}
