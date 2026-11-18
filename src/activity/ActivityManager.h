/*
文件名：ActivityManager.h
功能：声明活动管理、报名候补、时间冲突检测、兴趣推荐和积分统计的公共接口。
说明：各角色使用同一个管理器。函数参数和主要处理步骤写在对应 .cpp 的函数前。
*/
#pragma once
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
    SAVE_FAILED,      // 保存失败，已恢复原状态
    ENDED             // 活动已结束
};

std::string joinResultText(JoinResult r);   // 结果的中文说明

/*
类名：ActivityManager
功能：保存全部活动，向学生、负责人、管理员和社团模块提供统一接口
类之间的关系：
    1. 用 vector<unique_ptr<Activity>> 保存三种活动子类，调用基类接口时体现多态；
    2. Activity 把本类声明为友元，管理器可分配活动编号，并在回滚时直接恢复名单；
    3. 用户角色共用同一个管理器，不分别保存活动副本。
主要数据：活动列表、下一编号、两份数据文件路径、自动保存开关和测试时钟。
*/
class ActivityManager {
public:
    explicit ActivityManager(const std::string& actFile = "data/activities.txt",
                             const std::string& regFile = "data/registrations.txt");

    // ---------- 文件 ----------
    // 两个文件完整校验后才替换状态。文件不存在视为空；存在但无法读取、
    // 解析失败时报告文件和行号，返回false并保留原有活动、报名、候补及编号。
    bool load();
    bool save() const;
    void copyStateTo(ActivityManager& target) const;
    void swapState(ActivityManager& other);
    int cancelUnendedByClub(const std::string& clubId);
    void setAutoSave(bool on) { autoSave = on; }   // 默认开启：每次修改后自动保存

    // 测试用：固定“当前时间”（格式同活动时间）。传空串恢复为系统时间。
    void setClock(const std::string& now) { fixedNow = now; }
    std::string now() const;

    // ---------- 活动管理（社团负责人 / 管理员）----------
    // 添加活动，成功返回新分配的活动编号，对象不合法或结束时间不晚于当前时间时返回 ""。
    std::string addActivity(std::unique_ptr<Activity> act);
    // 交互式发布活动（社团负责人调用），新活动状态为 pending，等待管理员审核。
    bool publishActivity(const std::string& clubId);
    // 交互式修改活动。接口表只传 activityId；operatorClubId 可选，非空时必须与
    // 活动所属社团一致（额外的权限保护），留空则不检查。
    // 已有报名或候补时不允许修改任何资料；修改后的结束时间必须晚于当前时间。
    bool modifyActivity(const std::string& activityId, const std::string& operatorClubId = "");
    // 社团负责人删除未结束活动。operatorClubId 的规则同上，已结束活动保留为历史。
    bool deleteActivity(const std::string& activityId, const std::string& operatorClubId = "");
    // 管理员删除未结束的违规活动（不检查社团）；已结束活动及名单不能删除。
    bool removeActivity(const std::string& activityId);
    // approve 为 true 表示通过，false 表示拒绝；已结束的活动只能拒绝。
    // reason 接收失败原因，成功时清空；不需要原因时可省略，传入时写 &reason。
    bool auditActivity(const std::string& activityId, bool approve = true,
                       std::string* reason = nullptr);

    // 本社团被拒绝且未结束的活动可重新提交；保存失败恢复原状态。
    bool resubmitActivity(const std::string& activityId, const std::string& clubId);

    // ---------- 查询 ----------
    Activity* findActivity(const std::string& activityId);
    const Activity* findActivity(const std::string& activityId) const;
    std::vector<const Activity*> getAllActivities() const;
    std::vector<const Activity*> getApprovedActivities() const;
    std::vector<const Activity*> getPendingActivities() const;
    std::vector<const Activity*> getActivitiesByClub(const std::string& clubId, bool approvedOnly = false) const;

    void showApprovedActivities() const;               // 学生浏览
    void showPendingActivities() const;                // 管理员审核列表
    // 本社团活动显示完整起止时间和审核状态；approvedOnly 为 true 时只显示已通过的活动。
    void showActivitiesByClub(const std::string& clubId, bool approvedOnly = false) const;
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
    std::vector<std::unique_ptr<Activity> > activities;  // 管理器拥有活动对象
    int nextId;                         // 下一次分配的活动编号
    std::string activityFile;           // 活动资料文件路径
    std::string regFile;                // 正式报名、候补文件路径
    bool autoSave;                      // 是否在业务操作后自动保存
    std::string fixedNow;               // 测试用固定时间，空串表示系统时间

    std::string generateId();
    // 返回保存结果，让发布、修改、报名等操作能够在失败时恢复旧状态。
    bool autoSaveIfNeeded() const;
    bool persist() const;    // 重新提交审核时使用，未开启自动保存视为成功
};
