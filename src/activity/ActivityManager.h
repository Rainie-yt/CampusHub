#pragma once
// ============================================================
//  ActivityManager.h  —— 活动管理器（成员 C）
//
//  职责：活动增删改查、审核、报名/取消/候补、时间冲突检测、
//        兴趣推荐、积分统计、文件读写。
//
//  数据文件（UTF-8，字段用 '|' 分隔，data 目录需要提前存在）：
//    activities.txt   id|clubId|type|name|tags|start|end|location|max|status|extra
//    registrations.txt userId|activityId|registered 或 waitlist
//  报名名单以 registrations.txt 为准持久化，加载时恢复到各活动对象中；
//  删除活动时对应的报名记录随之消失，不会留下脏数据。
//
//  多态：容器里存的是 unique_ptr<Activity>（基类指针），
//        所以 Lecture / Competition / Performance 不会被“切片”。
// ============================================================
#include <memory>
#include <string>
#include <utility>
#include <vector>
#include "Activity.h"

// 报名结果。比 bool 多几种情况，菜单层可以按结果给出不同提示。
enum class JoinResult {
    SUCCESS,          // 报名成功
    WAITLISTED,       // 活动已满，已加入候补
    CONFLICT,         // 与已报名/候补的活动时间冲突
    ALREADY_JOINED,   // 已报名
    ALREADY_WAITING,  // 已在候补名单
    NOT_FOUND,        // 活动不存在
    NOT_APPROVED,     // 活动未通过审核
    ENDED             // 活动已结束
};

std::string joinResultText(JoinResult r);   // 结果的中文说明

// ============================================================
//  ActivityManager：活动管理器（活动子系统的对外入口）
//
//  职责：保存并管理所有活动对象，提供发布、修改、删除、审核、报名、取消、候补递补、
//        冲突检测、兴趣推荐、积分统计和文件读写。
//  与其他类的关系：
//    - 聚合 Activity：用 vector<unique_ptr<Activity>> 持有活动。存基类指针才能放入
//      Lecture / Competition / Performance 三种子类对象，避免按值存放造成"对象切片"，
//      调用虚函数时才能得到子类的行为（多态）。
//    - 友元：Activity 将本类设为友元，只有本类可以分配活动编号。
//    - 被调用：用户模块（Student / ClubManager / Admin）和社团模块通过本类的公有接口访问活动数据。
//  主要属性：activities 全部活动；nextId 下一个活动编号；activityFile / regFile 两个数据文件路径；
//            autoSave 是否自动保存；fixedNow 固定的"当前时间"（仅测试使用）。
// ============================================================
class ActivityManager {
public:
    explicit ActivityManager(const std::string& actFile = "data/activities.txt",
                             const std::string& regFile = "data/registrations.txt");

    // ---------- 文件 ----------
    // 读取两个文件。文件不存在视为空；格式有误的行会被跳过并在 cerr 提示，
    // 此时返回 false（其余正常的数据仍会被加载）。
    bool load();
    bool save() const;
    void setAutoSave(bool on) { autoSave = on; }   // 默认开启：每次修改后自动保存

    // 测试用：固定“当前时间”（格式同活动时间）。传空串恢复为系统时间。
    void setClock(const std::string& now) { fixedNow = now; }
    std::string now() const;

    // ---------- 活动管理（社团负责人 / 管理员）----------
    // 添加活动，成功返回新分配的活动编号，对象不合法返回 ""。
    std::string addActivity(std::unique_ptr<Activity> act);
    // 交互式发布活动（社团负责人调用），新活动状态为 pending，等待管理员审核。
    bool publishActivity(const std::string& clubId);
    // 交互式修改活动。接口表只传 activityId；operatorClubId 可选，非空时必须与
    // 活动所属社团一致（额外的权限保护），留空则不检查。
    // 已有人报名/候补时不允许修改时间，避免破坏已通过的冲突检测。
    bool modifyActivity(const std::string& activityId, const std::string& operatorClubId = "");
    // 社团负责人删除活动。operatorClubId 的规则同上。
    bool deleteActivity(const std::string& activityId, const std::string& operatorClubId = "");
    // 管理员删除违规活动（不检查社团）。
    bool removeActivity(const std::string& activityId);
    // 管理员审核：approve=true 通过，false 驳回。
    bool auditActivity(const std::string& activityId, bool approve = true);

    // ---------- 查询 ----------
    Activity* findActivity(const std::string& activityId);
    const Activity* findActivity(const std::string& activityId) const;
    std::vector<const Activity*> getAllActivities() const;
    std::vector<const Activity*> getApprovedActivities() const;
    std::vector<const Activity*> getPendingActivities() const;
    std::vector<const Activity*> getActivitiesByClub(const std::string& clubId) const;

    void showApprovedActivities() const;               // 学生浏览
    void showPendingActivities() const;                // 管理员审核列表
    void showActivitiesByClub(const std::string& clubId) const;
    void showActivityDetail(const std::string& activityId) const;

    // ---------- 报名（学生）----------
    // 接口表约定：bool joinActivity(userId, activityId)。
    // 函数内部会自己打印结果提示（成功 / 已加入候补 / 时间冲突 / 已报名 ...），
    // 调用方不需要再重复提示。返回 true 表示已进入报名名单或候补名单。
    bool joinActivity(const std::string& userId, const std::string& activityId);
    // 不打印的底层版本，返回详细结果（测试或界面层想自己处理时使用）。
    JoinResult tryJoin(const std::string& userId, const std::string& activityId);
    // 取消报名或退出候补。若取消的是正式名额，队首候补自动递补，
    // 递补者的 userId 通过 promotedUser 带回（没有则为空串）。已结束的活动不能取消。
    bool cancelActivity(const std::string& userId, const std::string& activityId,
                        std::string* promotedUser = nullptr);
    void showMyActivities(const std::string& userId) const;

    // ---------- 报名管理（社团负责人）----------
    void showRegistration(const std::string& activityId) const;
    // 交互式移除报名者（会触发候补递补）。
    void manageRegistration(const std::string& activityId, const std::string& operatorClubId = "");

    // ---------- 智能服务 ----------
    // 时间冲突检测：返回与 target 时间重叠的、该用户已报名/候补的活动；没有则返回 nullptr。
    const Activity* findConflict(const std::string& userId, const Activity& target) const;

    // 兴趣匹配度 = |交集| / |并集|（Jaccard），范围 0~1。
    static double matchScore(const std::vector<std::string>& interests,
                             const std::vector<std::string>& tags);
    // 推荐：只考虑已通过、未结束、该用户尚未报名/候补、匹配度 > 0 的活动，
    // 按匹配度降序（相同则开始时间早的在前）。userId 可为空。
    std::vector<std::pair<const Activity*, double> >
    recommend(const std::vector<std::string>& interests, const std::string& userId = "") const;
    void recommendActivity(const std::vector<std::string>& interests,
                           const std::string& userId = "") const;

    // 积分：该用户已报名、已通过审核、且已结束的活动的 calcPoints 之和。
    // （目前以“报名成功且活动已结束”视为参加，没有签到环节。）
    int getUserPoints(const std::string& userId) const;

    // ---------- 提供给社团模块（B）的统计接口 ----------
    int getActivityCount(const std::string& clubId) const;      // 已通过的活动数
    int getParticipantCount(const std::string& clubId) const;   // 这些活动的报名总人次

    // 工厂：根据类型名创建对应子类，类型未知返回 nullptr。
    static std::unique_ptr<Activity> makeActivity(
        const std::string& type, const std::string& aid, const std::string& cid,
        const std::string& name, const std::vector<std::string>& tags,
        const std::string& st, const std::string& et, const std::string& loc,
        int maxp, const std::string& status, const std::string& extra);

private:
    std::vector<std::unique_ptr<Activity> > activities;
    int nextId;
    std::string activityFile;
    std::string regFile;
    bool autoSave;
    std::string fixedNow;

    std::string generateId();
    void autoSaveIfNeeded() const;
};
