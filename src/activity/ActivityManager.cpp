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
// 参数：s 原字符串；delim 分隔符；返回：切分后的各段。
// 说明：必须保留空字段，因为数据文件里 "a||b" 的中间空字段（例如没有奖项）也占一个位置，
//       如果丢掉空字段，后面的字段位置就全错了。
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

// 去掉首尾空白。参数：s 原字符串；返回：处理后的新字符串。
string trim(const string& s) {
    size_t b = 0, e = s.size();
    while (b < e && std::isspace(static_cast<unsigned char>(s[b]))) ++b;
    while (e > b && std::isspace(static_cast<unsigned char>(s[e - 1]))) --e;
    return s.substr(b, e - b);
}

// 把 s 中所有的 from 替换成 to（直接修改 s）。
// 说明：每替换一次，下一次查找的起点跳到替换内容之后（pos += to.size()），防止 to 里又包含 from 造成死循环。
void replaceAll(string& s, const string& from, const string& to) {
    size_t pos = 0;
    while ((pos = s.find(from, pos)) != string::npos) {
        s.replace(pos, from.size(), to);
        pos += to.size();
    }
}

// 解析标签输入：支持英文逗号、中文逗号、顿号
// 参数：input 用户输入的一行标签；返回：去空白、去重后的标签列表。
// 步骤：先把中文逗号、顿号统一换成英文逗号，再按逗号切分，逐项去首尾空白，非空且不重复才保留。
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

// 把字符串解析成整数。参数：s 输入；out 解析结果（输出）；返回：整个字符串都是合法整数才返回 true。
// 说明：stoi 遇到 "12abc" 只会解析前面的 12，所以用 pos 检查是否消耗了全部字符；
//       遇到完全不是数字或数值过大时 stoi 会抛异常，用 try-catch 接住并返回 false。
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

// 显示提示语并读取一整行输入（可含空格）。参数：prompt 提示语；返回：去首尾空白后的输入，输入流结束时返回空串。
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

// 把 0~1 的小数格式化成百分数字符串，保留 1 位小数，例如 0.667 -> "66.7%"。
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
// 参数：a 要显示的活动；suffix 行末附加的说明（如状态），为空则不显示。
void printBrief(const Activity& a, const string& suffix = "") {
    cout << a.getActivityId() << "  " << a.getName() << "（" << a.getTypeName() << "）  "
         << timeRange(a) << "  " << a.getCur() << "/" << a.getMax() << "人";
    if (!suffix.empty()) cout << "  " << suffix;
    cout << "\n";
}

// 空位出现后，按候补顺序依次递补，返回被递补的 userId
// 参数：a 要处理的活动；返回：本次被递补的用户编号列表（按递补先后）。
// 步骤：只要还有空位并且候补名单非空，就取候补队首，把他加入正式名单。循环条件保证不会超员。
vector<string> promoteWaiting(Activity& a) {
    vector<string> promoted;
    while (!a.isFull() && !a.getWaitList().empty()) {
        string u = a.popWait();
        if (a.addParticipant(u)) promoted.push_back(u);
    }
    return promoted;
}

}  // namespace

// 把报名结果枚举转成给用户看的提示文字。参数：r 报名结果；返回：对应的中文提示。
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

// 构造函数。参数：actFile 活动数据文件路径；regFileName 报名数据文件路径。
// 说明：编号从 3001 开始；默认开启自动保存，每次修改数据后自动写文件。
ActivityManager::ActivityManager(const string& actFile, const string& regFileName)
    : nextId(3001), activityFile(actFile), regFile(regFileName), autoSave(true) {}

// 返回当前时间，格式 "YYYY-MM-DD HH:MM"，与活动时间的格式一致，可直接比较。
// 说明：fixedNow 不为空时返回它，用于单元测试"固定当前时间"，避免测试结果随真实时间变化。
//       localtime_s 与 localtime_r 是 Windows 与 Linux 下各自的线程安全版本，用条件编译区分。
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

// 生成新的活动编号：返回 nextId 并加 1。编号是纯数字字符串，从 3001 起。
string ActivityManager::generateId() { return std::to_string(nextId++); }

// 开启自动保存时写文件，保存失败给出警告。每个修改数据的函数结束前调用，保证文件与内存一致。
void ActivityManager::autoSaveIfNeeded() const {
    if (autoSave && !save()) cerr << "警告：活动数据保存失败，请确认 data 目录存在。\n";
}

// 活动工厂：根据类型名创建对应的子类对象。
// 参数：type 为 Lecture / Competition / Performance；extra 为该类型的特有字段（主讲人 / 奖项 / 节目类型）；
//       其余参数是各活动共有的字段。
// 返回：基类智能指针；类型无法识别返回 nullptr。
// 说明：返回 unique_ptr<Activity>，调用方用基类接口使用对象，新增活动类型时只需要改这一个函数（多态）。
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

// 从文件读取全部数据。返回：所有行都读取成功为 true；有坏行时仍加载其余数据，返回 false。
// 步骤：1) 读 activities.txt，每行拆成 11 个字段，校验字段个数、人数、状态、活动合法性和编号不重复，
//          通过才用工厂创建活动；坏行输出警告并跳过，不会让整个程序读取失败；
//       2) 记录最大编号，nextId = 最大编号 + 1，保证以后新编号不与已有编号重复；
//       3) 读 registrations.txt，按 "registered" / "waitlist" 把用户放进对应活动的名单，
//          文件里的先后顺序就是候补顺序。
// 说明：兼容 Windows 编辑器带来的 UTF-8 BOM 和行末 '\r'；以 '#' 开头的行视为注释。
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
                // 0xEF 0xBB 0xBF 是 UTF-8 BOM 的三个字节，不去掉会让第一行的编号前多出看不见的字符
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

// 把全部数据写回文件。返回：两个文件都写成功为 true。
// 说明：activities.txt 每个活动一行；registrations.txt 每条报名一行，
//       同一个活动里先写正式名单、再写候补名单，候补按队列顺序写，所以重新读取后顺序保持不变。
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

// ======================= 活动管理 =======================

// 添加一个活动。参数：act 构造好的活动（所有权转移给管理器）；返回：新活动编号，活动不合法时返回空串。
// 说明：编号由这里统一分配（Activity 把管理器设为友元，只有管理器能写编号），避免编号重复。
string ActivityManager::addActivity(std::unique_ptr<Activity> act) {
    if (!act || !act->isValid()) return "";
    act->activityId = generateId();
    string id = act->getActivityId();
    activities.push_back(std::move(act));
    autoSaveIfNeeded();
    return id;
}

// 交互式发布活动（社团负责人用）。参数：clubId 当前社团编号；返回：发布成功为 true。
// 步骤：依次读取类型、名称、标签、起止时间、地点、人数上限和特有字段 -> 创建活动 -> 校验 -> 加入管理器。
//       新活动的状态固定为 pending，必须由管理员审核通过后学生才能报名。
// 说明：时间输入用 askTime 反复提示，直到格式正确；任一必填项非法就取消发布。
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

// 交互式修改活动信息。
// 参数：activityId 活动编号；operatorClubId 操作者所属社团，非空时只允许修改本社团的活动，空串表示不限制（管理员）。
// 返回：有任何一项被修改为 true。
// 规则：每项直接回车表示不修改；人数上限不能小于当前报名人数；
//       已有人报名或候补的活动不允许改时间，否则已经通过的冲突检测会失效。
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
            // 上限调大后出现空位，候补自动递补
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

// 删除活动（社团负责人用，带权限检查）。
// 参数：activityId 活动编号；operatorClubId 操作者所属社团，非空时必须与活动所属社团一致。
// 返回：删除成功为 true；活动不存在或不属于该社团返回 false。
bool ActivityManager::deleteActivity(const string& activityId, const string& operatorClubId) {
    const Activity* a = findActivity(activityId);
    if (!a || (!operatorClubId.empty() && a->getClubId() != operatorClubId)) return false;
    return removeActivity(activityId);
}

// 无条件删除活动（管理员用，不检查社团）。返回：删除成功为 true。
// 说明：报名名单存在活动对象里，删除活动时一并消失，保存后 registrations.txt 里不会残留该活动的记录。
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

// 审核活动（管理员用）。参数：activityId 活动编号；approve 为 true 通过、false 驳回。返回：活动存在为 true。
bool ActivityManager::auditActivity(const string& activityId, bool approve) {
    Activity* a = findActivity(activityId);
    if (!a) return false;
    a->setStatus(approve ? "approved" : "rejected");
    autoSaveIfNeeded();
    return true;
}

// ======================= 查询 =======================

// ======================= 查询 =======================

// 按编号查找活动。参数：activityId 活动编号；返回：找到返回指针，没有返回 nullptr。
// 说明：有普通和 const 两个版本，前者给能修改活动的函数用，后者给只读函数用。
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

// 以下四个 get 函数都返回只读指针列表，不会让外部模块（用户模块、社团模块）修改活动。
// 返回全部活动。
vector<const Activity*> ActivityManager::getAllActivities() const {
    vector<const Activity*> r;
    for (size_t i = 0; i < activities.size(); ++i) r.push_back(activities[i].get());
    return r;
}

// 返回已通过审核的活动（包含已结束的，管理员统计用）。
vector<const Activity*> ActivityManager::getApprovedActivities() const {
    vector<const Activity*> r;
    for (size_t i = 0; i < activities.size(); ++i)
        if (activities[i]->isApproved()) r.push_back(activities[i].get());
    return r;
}

// 返回待审核的活动。
vector<const Activity*> ActivityManager::getPendingActivities() const {
    vector<const Activity*> r;
    for (size_t i = 0; i < activities.size(); ++i)
        if (activities[i]->getStatus() == "pending") r.push_back(activities[i].get());
    return r;
}

// 返回某个社团的全部活动（不论审核状态）。参数：clubId 社团编号。
vector<const Activity*> ActivityManager::getActivitiesByClub(const string& clubId) const {
    vector<const Activity*> r;
    for (size_t i = 0; i < activities.size(); ++i)
        if (activities[i]->getClubId() == clubId) r.push_back(activities[i].get());
    return r;
}

// 显示学生可见的活动列表：已通过审核且未结束。
void ActivityManager::showApprovedActivities() const {
    // 学生浏览时只显示已通过且未结束的活动（已结束的活动不能报名，没必要列出来）。
    // 注意：getApprovedActivities() 仍然包含已结束的活动，管理员统计要用。
    string current = now();
    vector<const Activity*> v;
    for (size_t i = 0; i < activities.size(); ++i)
        if (activities[i]->isApproved() && !activities[i]->hasEnded(current)) v.push_back(activities[i].get());
    cout << "========活动列表========\n";
    if (v.empty()) cout << "暂无活动。\n";
    for (size_t i = 0; i < v.size(); ++i) printBrief(*v[i]);
}

// 显示待审核活动列表（管理员审核前看）。
void ActivityManager::showPendingActivities() const {
    vector<const Activity*> v = getPendingActivities();
    cout << "========待审核活动========\n";
    if (v.empty()) cout << "暂无待审核的活动。\n";
    for (size_t i = 0; i < v.size(); ++i) printBrief(*v[i]);
}

// 显示某社团的活动列表，行末标注审核状态。参数：clubId 社团编号。
void ActivityManager::showActivitiesByClub(const string& clubId) const {
    vector<const Activity*> v = getActivitiesByClub(clubId);
    cout << "========本社团活动========\n";
    if (v.empty()) cout << "暂无活动。\n";
    for (size_t i = 0; i < v.size(); ++i) {
        printBrief(*v[i], Activity::statusText(v[i]->getStatus()));
    }
}

// 显示活动详情。参数：activityId 活动编号；活动不存在时提示"未找到该活动！"。
// 说明：这里调用的 showInfo() 是虚函数，运行时根据活动实际类型显示讲座 / 比赛 / 演出各自的信息（多态）。
void ActivityManager::showActivityDetail(const string& activityId) const {
    const Activity* a = findActivity(activityId);
    if (!a) { cout << "未找到该活动！\n"; return; }
    a->showInfo();
}

// ======================= 报名 =======================

// ======================= 报名 =======================

// 报名核心逻辑，只返回结果，不输出文字（输出由 joinActivity 负责，便于单元测试）。
// 参数：userId 用户编号；activityId 活动编号；返回：JoinResult 枚举。
// 检查顺序（一旦某条不满足就立即返回对应结果）：
//   活动存在 -> 已通过审核 -> 未结束 -> 没有已报名 -> 没有已候补 -> 与已报名/候补的活动不冲突
//   -> 有空位则正式报名，否则进入候补。
// 说明：顺序是有意安排的，越基础的问题越先提示；成功后自动保存。
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

// 报名接口（按接口总表约定返回 bool），先调用 tryJoin，再把结果输出给用户。
// 返回：正式报名成功或进入候补都算成功，返回 true；其余情况返回 false。
// 说明：遇到时间冲突时，额外输出冲突的是哪个活动、什么时间，方便用户判断。
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

// 取消报名或候补。
// 参数：userId 用户编号；activityId 活动编号；promotedUser 可选输出，返回被递补的用户编号（没有则为空）。
// 返回：取消成功为 true；活动不存在、已结束、用户不在名单中都返回 false。
// 步骤：在正式名单中 -> 移除，并由候补队首递补；否则在候补名单中 -> 直接移除；两者都不在 -> 失败。
// 说明：已结束的活动不允许取消，否则可以通过取消来逃避积分记录。
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

// 显示某用户的全部报名情况（正式报名、已结束、候补及排位）。参数：userId 用户编号。
void ActivityManager::showMyActivities(const string& userId) const {
    cout << "========我的活动========\n";
    bool any = false;
    for (size_t i = 0; i < activities.size(); ++i) {
        const Activity& a = *activities[i];
        if (a.hasParticipant(userId)) {
            printBrief(a, a.hasEnded(now()) ? "已报名（已结束）" : "已报名");
            any = true;
        } else if (a.inWaitList(userId)) {
            // 迭代器相减得到下标；下标从 0 开始，显示时加 1 变成"第几位"
            size_t pos = std::find(a.getWaitList().begin(), a.getWaitList().end(), userId) - a.getWaitList().begin();
            printBrief(a, "候补第" + std::to_string(pos + 1) + "位");
            any = true;
        }
    }
    if (!any) cout << "你还没有报名任何活动。\n";
}

// ======================= 报名管理 =======================

// ======================= 报名管理 =======================

// 显示某活动的报名名单和候补名单。参数：activityId 活动编号。
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

// 交互式管理报名成员（移除成员）。
// 参数：activityId 活动编号；operatorClubId 操作者所属社团，非空时只能管理本社团活动。
// 步骤：循环 显示名单 -> 输入要移除的用户编号 -> 调用 cancelActivity -> 提示结果；直接回车退出循环。
// 说明：移除正式成员时，候补会自动递补，并提示递补的是谁。
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

// ======================= 智能服务 =======================

// 查找与目标活动时间冲突的活动。
// 参数：userId 用户编号；target 想报名的目标活动；返回：冲突的那个活动，没有冲突返回 nullptr。
// 步骤：遍历所有活动，跳过目标自己、未通过审核的活动、用户既没报名也没候补的活动，
//       剩下的是"用户已占用时间"的活动，逐个调用 overlaps 判断时间是否重叠。
// 说明：候补也算占用时间，防止用户同时候补两个时间重叠的活动，最后都递补成功而无法同时参加。
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

// 计算兴趣与活动标签的匹配度（Jaccard 相似度）。
// 参数：interests 用户兴趣；tags 活动标签；返回：0~1 之间的小数，越大越匹配。
// 公式：匹配度 = |交集| / |并集|。例：兴趣 {科技,竞赛,志愿}，标签 {科技,竞赛}，交集 2，并集 3，匹配度 2/3。
// 说明：先放进 set 自动去重；并集大小 = |A| + |B| - |交集|，不用真的构造并集；任一方为空返回 0，避免除以 0。
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

// 推荐活动，返回"活动 + 匹配度"的列表（已排好序）。
// 参数：interests 用户兴趣；userId 当前用户，非空时过滤掉其已报名或已候补的活动。
// 步骤：1) 过滤：未通过审核、已结束、用户已报名的活动都不推荐；
//       2) 打分：用 matchScore 计算匹配度，匹配度为 0 的不推荐；
//       3) 排序：匹配度高的在前，匹配度相同则开始时间早的在前。
// 说明：用 stable_sort（稳定排序），匹配度和开始时间都相同的活动保持原有顺序，让输出结果每次一致。
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

// 把推荐结果输出给用户，最多显示前 5 项，已满员的活动标注"可候补"。
// 参数：interests 用户兴趣；userId 当前用户。
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

// 计算用户的活动积分。参数：userId 用户编号；返回：积分总和。
// 规则：只统计"已通过审核、已结束、用户正式报名"的活动，每个活动的积分由 calcPoints() 决定。
// 说明：calcPoints() 是虚函数，讲座 5 分、比赛 10 分、演出 8 分，这里无需判断活动类型（多态）。
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

// ======================= 给社团模块的统计 =======================

// 统计某社团已通过审核的活动数量。参数：clubId 社团编号。
int ActivityManager::getActivityCount(const string& clubId) const {
    int n = 0;
    for (size_t i = 0; i < activities.size(); ++i)
        if (activities[i]->isApproved() && activities[i]->getClubId() == clubId) ++n;
    return n;
}

// 统计某社团已通过审核的活动的报名总人次（每个活动的当前人数相加）。参数：clubId 社团编号。
int ActivityManager::getParticipantCount(const string& clubId) const {
    int n = 0;
    for (size_t i = 0; i < activities.size(); ++i)
        if (activities[i]->isApproved() && activities[i]->getClubId() == clubId) n += activities[i]->getCur();
    return n;
}
