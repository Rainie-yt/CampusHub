#pragma once
#include <string>
#include <vector>

class ActivityManager;

/*
讲座、比赛和演出的抽象基类。
保存所属社团、起止时间、人数上限、正式名单和按先后排列的候补名单。
派生类实现 calcPoints、showInfo 等虚函数，管理器可通过 Activity 指针统一调用。
*/
class Activity {
    friend class ActivityManager;   // 只有管理器可以分配活动编号
	protected:
	    std::string activityId;                 // 活动编号（由 ActivityManager 分配）
	    std::string clubId;                     // 主办社团编号（来自社团模块）
	    std::string activityName;               // 活动名称
	    std::vector<std::string> tags;          // 兴趣标签，如 {"科技","竞赛"}
	    std::string startTime;                  // "2026-09-25 14:00"
	    std::string endTime;
	    std::string location;                   // 活动地点
	    int maxPeople;                          // 人数上限
	    std::string status;                     // pending 待审核 / approved 已通过 / rejected 未通过
	    std::vector<std::string> participants;  // 已报名的 userId（按报名先后）
	    std::vector<std::string> waitList;      // 候补的 userId（队首优先递补）

    // 去掉会破坏文件格式的字符（'|' 和换行）
    static std::string clean(const std::string& s);
    // 标签规范化：去空白、去重、去掉逗号等分隔符
    static std::vector<std::string> normalizeTags(const std::vector<std::string>& in);

	public:
    // 注意：构造函数不做合法性检查，创建后请调用 isValid()
    // （ActivityManager::addActivity 和文件读取时会自动检查）。
	    Activity(const std::string& aid, const std::string& cid, const std::string& name,
	             const std::vector<std::string>& tagList,
	             const std::string& st, const std::string& et,
	             const std::string& loc, int maxp,
	             const std::string& stat = "pending");
	    virtual ~Activity() = default;

    // ---------- 只读访问 ----------
	    const std::string& getActivityId() const { return activityId; }
	    const std::string& getClubId() const { return clubId; }
	    const std::string& getName() const { return activityName; }
	    const std::vector<std::string>& getTags() const { return tags; }
	    const std::string& getStartTime() const { return startTime; }
	    const std::string& getEndTime() const { return endTime; }
	    const std::string& getLocation() const { return location; }
	    int getMax() const { return maxPeople; }
	    int getCur() const { return static_cast<int>(participants.size()); }
	    const std::string& getStatus() const { return status; }
	    const std::vector<std::string>& getParticipants() const { return participants; }
	    const std::vector<std::string>& getWaitList() const { return waitList; }
	
	    bool isFull() const { return getCur() >= maxPeople; }
	    bool isApproved() const { return status == "approved"; }
	    bool hasParticipant(const std::string& uid) const;
	    bool inWaitList(const std::string& uid) const;

    // ---------- 修改（均带合法性检查，失败返回 false 且不改动对象）----------
	    void setName(const std::string& name);
	    void setLocation(const std::string& loc);
	    void setTags(const std::vector<std::string>& tagList);
	    bool setMax(int maxp);                                   // 必须 >0 且不小于当前报名人数
	    bool setTime(const std::string& st, const std::string& et);  // 格式合法且 st < et
	    bool setStatus(const std::string& stat);                 // 只接受三种状态
	
	    // ---------- 名单操作 ----------
	    bool addParticipant(const std::string& uid);   // 已满 / 已在名单中 返回 false
	    bool removeParticipant(const std::string& uid);
	    bool addWait(const std::string& uid);          // 已在报名或候补名单中返回 false
	    bool removeWait(const std::string& uid);
	    std::string popWait();                         // 取出并移除队首候补，空则返回 ""

	    // ---------- 时间相关 ----------
	    // 两个活动时间区间是否重叠：a.start < b.end && b.start < a.end
	    // 首尾相接（16:00 结束 / 16:00 开始）不算冲突。
	    bool overlaps(const Activity& other) const;
	    bool hasEnded(const std::string& now) const { return endTime <= now; }
	    // 校验 "YYYY-MM-DD HH:MM"，包括月份天数和闰年
	    static bool isValidTime(const std::string& t);
	    // 对象整体是否合法：时间格式、先后顺序、人数上限
	    bool isValid() const;
	
	    // ---------- 多态接口（子类必须实现）----------
	    virtual std::string getType() const = 0;      // 写入文件用的类型名："Lecture" 等
	    virtual std::string getTypeName() const = 0;  // 显示用的中文类型名："讲座" 等
	    virtual std::string getExtra() const = 0;     // 子类特有字段，写入文件最后一列
	    virtual int calcPoints() const = 0;           // 参加该类活动可获得的积分
	    virtual void showInfo() const = 0;            // 打印详情（子类追加自己的特有信息）
	
	    // 序列化为一行：
	    // id|clubId|type|name|tags(逗号分隔)|start|end|location|max|status|extra
	    std::string toFileString() const;
	
	    static std::string statusText(const std::string& st);  // 状态的中文显示

	protected:
	    void showBaseInfo() const;   // 打印所有活动共有的信息，供子类 showInfo 调用
};
