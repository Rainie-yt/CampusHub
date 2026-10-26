#include "ActivityManager.h"

ActivityManager::ActivityManager(QObject *parent)
    : QObject(parent), nextId_(1)
{

}

int ActivityManager::addActivity(const Activity &act)
{
    Activity newAct = act;
    newAct.id = nextId_++;
    activityList_.push_back(newAct);
    emit activityDataChanged();
    return newAct.id;
}

bool ActivityManager::removeActivity(int actId)
{
    for (auto it = activityList_.begin(); it != activityList_.end(); ++it)
    {
        if (it->id == actId)
        {
            activityList_.erase(it);
            emit activityDataChanged();
            return true;
        }
    }
    return false;
}

bool ActivityManager::updateActivity(int actId, const Activity &newInfo)
{
    for (auto& act : activityList_)
    {
        if (act.id == actId)
        {
            act = newInfo;
            act.id = actId; // id不允许外部修改，强制覆盖
            emit activityDataChanged();
            return true;
        }
    }
    return false;
}

Activity ActivityManager::getActivityById(int actId) const
{
    for (const auto& act : activityList_)
    {
        if (act.id == actId)
            return act;
    }
    return Activity();
}

QVector<Activity> ActivityManager::getAllActivities() const
{
    return activityList_;
}

QVector<Activity> ActivityManager::getPublishedActivities() const
{
    QVector<Activity> res;
    for (const auto& act : activityList_)
    {
        if (act.isPublished)
            res.push_back(act);
    }
    return res;
}

bool ActivityManager::signUp(int actId, int memberId)
{
    for (auto& act : activityList_)
    {
        if (act.id == actId)
        {
            if (hasSignedUp(actId, memberId))
                return false;
            if (act.signUpList.size() >= act.maxMember)
                return false;

            act.signUpList.push_back(memberId);
            emit activityDataChanged();
            return true;
        }
    }
    return false;
}

bool ActivityManager::cancelSignUp(int actId, int memberId)
{
    for (auto& act : activityList_)
    {
        if (act.id == actId)
        {
            for (auto it = act.signUpList.begin(); it != act.signUpList.end(); ++it)
            {
                if (*it == memberId)
                {
                    act.signUpList.erase(it);
                    emit activityDataChanged();
                    return true;
                }
            }
            // 找到活动，但是该成员没有报名，直接退出
            return false;
        }
    }
    return false;
}

bool ActivityManager::hasSignedUp(int actId, int memberId) const
{
    Activity act = getActivityById(actId);
    for (int id : act.signUpList)
    {
        if (id == memberId)
            return true;
    }
    return false;
}

void ActivityManager::clearAll()
{
    activityList_.clear();
    nextId_ = 1;
    emit activityDataChanged();
}

