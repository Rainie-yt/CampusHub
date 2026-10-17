#include"club.h"
#include<iostream>
Club::Club(const string& id, const string& name, const string& intro, const string& leader) 
{
	clubId = id;
	clubName = name;
	description = intro;
	leaderId = leader;
	status = "pending";
}
string Club::getClubName()const
{
	return clubName;
}
string Club::getClubId()const
{
	return clubId;
}
string Club::getDescription()const
{
	return description;
}
string Club::getLeaderId()const
{
	return leaderId;
}
string Club::getStatus()const
{
	return status;
}
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
int Club::getMemberCount()const
{
	return memberIds.size();
}
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
vector<string>Club::getMemberIds()const
{
	return memberIds;
}
void Club::clearMembers()
{
	memberIds.clear();
}
string Club::getType()const
{
	return "普通社团";
}
Club* Club::clone()const
{
	return new Club(*this);
}
Club::~Club()
{

}
string Club::getExtraInfo()const
{
	return"";
}