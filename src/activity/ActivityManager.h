#ifndef ACTIVITYMANAGER_H
#define ACTIVITYMANAGER_H

#include <QObject>
#include <QString>
#include <QDateTime>
#include <QVector>

// 社团活动实体
struct Activity
{
    int id;                 // 活动编号，唯一标识
    QString name;           // 活动名称
    QString description;    // 活动简介
    QString location;       // 活动地点
    QDateTime startTime;    // 开始时间
    QDateTime endTime;      // 结束时间
    QString sponsor;        // 主办方/负责人
    int maxMember;          // 最大报名人数
    QVector<int> signUpList;// 报名成员id列表
    bool isPublished;       // 是否发布

    Activity() : id(-1), maxMember(0), isPublished(false) {}
};

class ActivityManager : public QObject
{
    Q_OBJECT
public:
    explicit ActivityManager(QObject *parent = nullptr);

    /**
     * @brief 新增活动，id由管理器自动分配
     * @param act 活动信息，入参的id会被忽略
     * @return 返回分配好的活动id
     */
    int addActivity(const Activity& act);

    /**
     * @brief 根据id删除活动
     * @param actId 目标活动id
     * @return true删除成功，false不存在该id
     */
    bool removeActivity(int actId);

    /**
     * @brief 更新活动信息
     * @param actId 要修改的活动id
     * @param newInfo 新信息，newInfo内的id字段会被强制覆盖为actId
     * @return true修改成功，false不存在该id
     */
    bool updateActivity(int actId, const Activity& newInfo);

    /**
     * @brief 根据id查询单个活动
     * @param actId 活动id
     * @return 找不到返回id=-1的空Activity，调用方需要判断id是否有效
     */
    Activity getActivityById(int actId) const;

    /**
     * @brief 获取全部活动（包含未发布）
     */
    QVector<Activity> getAllActivities() const;

    /**
     * @brief 只获取已发布的活动
     */
    QVector<Activity> getPublishedActivities() const;

    /**
     * @brief 成员报名活动
     * @param actId 活动id
     * @param memberId 成员id
     * @return 成功true；已报名 / 人数已满 / 活动不存在 返回false
     */
    bool signUp(int actId, int memberId);

    /**
     * @brief 取消报名
     * @return true取消成功；未报名 / 活动不存在 返回false
     */
    bool cancelSignUp(int actId, int memberId);

    /**
     * @brief 查询成员是否已经报名该活动
     */
    bool hasSignedUp(int actId, int memberId) const;

    /**
     * @brief 清空所有活动（仅测试使用）
     */
    void clearAll();

signals:
    // 活动数据发生变更，通知UI刷新界面
    void activityDataChanged();

private:
    QVector<Activity> activityList_;
    int nextId_;
};

#endif // ACTIVITYMANAGER_H

