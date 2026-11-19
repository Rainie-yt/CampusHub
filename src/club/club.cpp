#include"club.h"
#include<iostream>
// 按传入资料建立社团，初始状态为 pending，等管理员审核。
Club::Club(const string& id, const string& name, const string& intro, const string& leader)
{
	clubId = id;
	clubName = name;
	description = intro;
	leaderId = leader;
	status = "pending";
}
// 返回当前显示名称；跨模块关联使用 getClubId()。
string Club::getClubName()const
{
	return clubName;
}
// 返回社团编号，供成员关系和活动记录关联。
string Club::getClubId()const
{
	return clubId;
}
// 返回当前社团简介。
string Club::getDescription()const
{
	return description;
}
// 返回负责人的用户编号。
string Club::getLeaderId()const
{
	return leaderId;
}
// 返回审核或归档状态，供菜单判断可执行的操作。
string Club::getStatus()const
{
	return status;
}
// 检查新名称中有可见内容且不含换行，再替换原名称。无效时返回 false。
bool Club::rename(const string& newName)
{
	bool hasValidChar = false;
	for (int i = 0; i < newName.length(); i++)
	{
		//名称不能包含回车或换行
		if (newName[i] == '\r' || newName[i] == '\n')
		{
			return false;
		}
		//检查是否存在非空字符
		if (newName[i] != ' ' && newName[i] != '\t' && newName[i] != '\v' && newName[i] != '\f')
		{
			hasValidChar = true;
		}
	}
	if (hasValidChar == true)
	{
		clubName = newName;
	}
	return hasValidChar;//有非空字符返回true,无则返回false
}
// 简介可留空；检查到换行就返回 false，避免破坏逐行保存的格式。
bool Club::changeDescription(const string& newDescription)
{
	for (int i = 0; i < newDescription.length(); i++)
	{
		if (newDescription[i] == '\r' || newDescription[i] == '\n')
		{
			return false;
		}
	}
	description = newDescription;
	return true;
}
// 仅处理 pending 状态。passed 决定通过还是拒绝，返回值表示是否完成审核。
bool Club::audit(bool passed)
{
	if (status != "pending")
	{
		return false;
	}
	if (passed)
	{
		status = "approved";
	}
	else
	{
		status = "rejected";
	}
	return true;
}
// 按 userId 查成员名单，找到同一编号就返回 true。
bool Club::hasMember(const string& userId)const
{
	for (int i = 0; i < memberIds.size(); i++)
	{
		if (memberIds[i] == userId)
		{
			return true;
		}
	}
	return false;
}
// 已通过的社团才接收成员。检查编号和重复记录后追加到名单。
bool Club::addMember(const string& userId)
{
	if (status != "approved" || userId.empty())
	{
		return false;
	}
	//用户编号不能包含空白字符
	for (int i = 0; i < userId.length(); i++)
	{
		if (userId[i] == ' ' || userId[i] == '\t' || userId[i] == '\n'||userId[i]=='\r'||userId[i]=='\v'||userId[i] == '\f')
		{
			return false;
		}
	}
	if (hasMember(userId))
	{
		return false;
	}
	memberIds.push_back(userId);
	return true;
}
// 返回成员名单长度，供社团详情与活跃度计算使用。
int Club::getMemberCount()const
{
	return memberIds.size();
}
// 找到 userId 就移除对应成员，查无此人时返回 false。
bool Club::removeMember(const string& userId)
{
	for (int i = 0; i < memberIds.size(); i++)
	{
		if (memberIds[i] == userId)
		{
			memberIds.erase(memberIds.begin() + i);
			return true;
		}
	}
	return false;
}
// 打印人数与成员编号；空名单单独显示提示。
void Club::showMembers()const
{
	cout << "成员人数：" << getMemberCount() << endl;
	if (memberIds.empty())
	{
		cout << "暂无成员。" << endl;
		return;
	}
	for (int i = 0; i < memberIds.size(); i++)
	{
		cout << i + 1 << "." << memberIds[i] << endl;
	}
}
// 返回成员编号的副本，调用者可以遍历而不会改动原名单。
vector<string>Club::getMemberIds()const
{
	return memberIds;
}
// 重新装载成员记录前清空旧成员，保留社团基本资料。
void Club::clearMembers()
{
	memberIds.clear();
}
// 普通社团返回默认类型；派生类重写此方法。
string Club::getType()const
{
	return "普通社团";
}
// 创建独立副本，交给持有它的管理器释放。
Club* Club::clone()const
{
	return new Club(*this);
}
// 基类使用虚析构，允许管理器通过 Club 指针释放派生对象。
Club::~Club()
{

}
// 普通社团没有分类专属字段，保存时该行留空。
string Club::getExtraInfo()const
{
	return"";
}
// 查询 userId 是否已有一条待审入团申请。
bool Club::hasApplicant(const string& userId)const
{
	for (int i = 0; i < applicantIds.size(); i++)
	{
		if (applicantIds[i] == userId)
		{
			return true;
		}
	}
	return false;
}
// 检查社团状态和编号，排除成员及重复申请后加入待审名单。
bool Club::addApplicant(const string& userId)
{
	if (status != "approved" || userId.empty())
	{
		return false;
	}
	//用户编号不能包含空白字符
	for (int i = 0; i < userId.length(); i++)
	{
		if (userId[i] == ' ' || userId[i] == '\t' || userId[i] == '\r' ||
			userId[i] == '\n' || userId[i] == '\v' || userId[i] == '\f')
		{
			return false;
		}
	}
	if (hasMember(userId) || hasApplicant(userId))
		{
			return false;
		}
		applicantIds.push_back(userId);
		return true;
}
// 按用户编号删除待审申请，未找到时返回 false。
bool Club::removeApplicant(const string& userId)
{
	for (int i = 0; i < applicantIds.size(); i++)
	{
		if (applicantIds[i] == userId)
		{
			applicantIds.erase(applicantIds.begin() + i);
			return true;
		}
	}
	return false;
}
// 返回申请人编号的副本，供负责人显示待审名单。
vector<string>Club::getApplicantIds()const
{
	return applicantIds;
}
// 装载申请记录前清空旧申请，保留已加入的成员。
void Club::clearApplicants()
{
	applicantIds.clear();
}
// 标记为 dissolved 并清空成员和申请，社团编号仍能关联历史记录。
void Club::dissolve(){
    status="dissolved";
    memberIds.clear();
    applicantIds.clear();
}

/*
函数名：changeLeader
功能：把社团交给现有成员，原负责人继续保留为成员
参数：newLeaderId 新负责人的用户编号
返回值：社团未通过、不是成员或与原负责人相同时返回 false，否则返回 true
*/
bool Club::changeLeader(const string& newLeaderId){
    if(status!="approved" || newLeaderId==leaderId || !hasMember(newLeaderId)) return false;
    // 旧数据里的负责人可能没有单独记入成员名单，交接时补上，避免失去成员身份。
    if(!hasMember(leaderId) && !addMember(leaderId)) return false;
    leaderId=newLeaderId;
    return true;
}
