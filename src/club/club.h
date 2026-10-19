#pragma once
#include  <string>
#include<vector>
using namespace std;
class Club
{
private:
	string clubId;
	string clubName;
	string description;
	string leaderId;
	string status;//审核状态：pending、approved、rejected
	vector<string>memberIds;//社团成员的用户编号
	vector<string>applicantIds;//待审核的申请人编号
public:
	//构造函数
	Club(const string& id, const string& name, const string& intro, const string& leader);
	//获取基本资料，不修改对象
	string getClubId()const;
	string getClubName()const;
	string getDescription()const;
	string getLeaderId()const;
	string getStatus()const;
	//修改名称,拒绝空名称或全为空白的名称
	// 成功返回true，失败返回false，并保留原名称
	bool rename(const string& newName);
	//简介修改：允许为空，拒绝回车或换行
	// 成功返回true，失败返回false，并保留原简介
	bool changeDescription(const string& newDescription);
	//审核社团：只允许审核待审核的社团
	//passed为true表示通过，false表示拒绝
	//操作成功返回true，状态不是pending时返回false
	bool audit(bool passed);
	//查询用户是否已在社团中
	bool hasMember(const string& userId)const;
	//加入社团：若未审核通过、编号为空或重复加入时返回false
	bool addMember(const string& userId);
	//获取成员人数
	int getMemberCount()const;
	//删除成员：成功返回true，成员不存在时返回false
	bool removeMember(const string& userId);
	//显示成员人数和成员编号
	void showMembers()const;
	//返回成员列表的副本
	vector<string>getMemberIds()const;
	//清空成员列表，供读取成员文件时使用
	void clearMembers();
	//获取社团类型，派生类可以提供自己的实现
	virtual string getType()const;
	//创建当前对象的独立副本
	// 返回的对象由调用者delete
	virtual Club* clone()const;
	//虚析构函数，为以后通过基类指针管理派生对象做准备
	virtual~Club();
	//获取分类特有资料，普通社团返回空字符串
	virtual string getExtraInfo()const;
	//查询用户是否已在待审核名单中
	bool hasApplicant(const string& userId)const;
	//添加加入申请
	//社团未通过、编号非法、已是成员或重复申请时返回false
	bool addApplicant(const string& userId);
	//移除待审核申请，不存在时返回false
	bool removeApplicant(const string& userId);
	//返回待审核名单的副本
	vector<string>getApplicantIds()const;
	//清空待审核名单，供文件读取使用
	void clearApplicants();
};

