#pragma once
#include <string>
#include <vector>
using namespace std;

//活动基类
class Activity {
	protected:
		string activityId;
		string clubId;
		string activityName;
		string tags;  //兴趣标签，逗号分隔“科技，学术”
		string startTime;  //"2026-09-25 14:00"
		string endTime;
		int maxPeople;
		int currentPeople;
		vector<string> waitList;
public:
	Activity(string aid, string cid, string name, string tag, string st, string et, int maxp);
	virtual ~Activity();
	string getActivityId() const;
	string getClubId() const;
	string getName() const;
	string getTags() const;
	string getStartTime() const;
	string getEndTime() const;
	int getMax() const;
	int getCur() const;
	vector<string> getWaitList() const;
	
	//修改当前报名人数
	void addCur();
	void subCur();

	//候补操作
	void addWait(const string& uid);

	//纯虚函数，子类重写打印详情
	virtual void showInfo() const = 0;
};


