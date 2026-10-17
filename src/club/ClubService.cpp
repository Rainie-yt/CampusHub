#include"ClubService.h"
#include<iostream>
#include<fstream>
#include"SportsClub.h"
#include"AcademicClub.h"
#include"ArtClub.h"
//遍历社团列表，找到指定编号就返回下标
ClubService::ClubService()
{

}
void ClubService::clearClubs()
{
	for (int i = 0; i < clubs.size(); i++)
	{
		delete clubs[i];
	}
	clubs.clear();
}
ClubService::~ClubService()
{
	clearClubs();
}
int ClubService::findClubIndex(const string& clubId)const
{
	for (int i = 0; i < clubs.size(); i++)
	{
		if (clubs[i]->getClubId() == clubId)
		{
			return i;
		}
	}
	return -1;
}
bool ClubService::addClub(const Club& club)
{
	string id = club.getClubId();
	string leader = club.getLeaderId();
	string name = club.getClubName();
	string intro = club.getDescription();
	string extraInfo = club.getExtraInfo();
	//社团编号和负责人编号不能为空
	if (id.empty() || leader.empty())
	{
		return false;
	}
	//社团编号不能包含空白字符
	for (int i = 0; i < id.length(); i++)
	{
		if (id[i] == ' ' || id[i] == '\t' || id[i] == '\r' || id[i] == '\n' || id[i] == '\v' || id[i] == '\f')
		{
			return false;
		}
	}	
	//负责人编号不能包含空白字符
		for (int i = 0; i < leader.length(); i++)
		{
			if (leader[i] == ' ' || leader[i] == '\t' || leader[i] == '\r' || leader[i] == '\n' || leader[i] == '\v' || leader[i] == '\f')
			{
				return false;
			}
		}
		//名称不能全为空白，也不能包含换行
		bool hasValidChar = false;
		for (int i = 0; i < name.length(); i++)
		{
			if (name[i] == '\r' || name[i] == '\n')
			{
				return false;
			}
			if (name[i] != ' ' && name[i] != '\t'&&name[i]!='\v'&&name[i]!='\f')
			{
				hasValidChar = true;
			}
		}
		if (hasValidChar == false)
		{
			return false;
		}
		//社团编号不能重复
	if (findClubIndex(club.getClubId()) != -1)
	{
		return false;
	}
	//简介不能换行，避免破坏文件格式
	for (int i = 0; i < intro.length(); i++)
	{
		if (intro[i] == '\r' || intro[i] == '\n')
		{
			return false;
		}
	}
	//分类资料不能包含换行或回车
	for (int i = 0; i < extraInfo.length(); i++)
	{
		if (extraInfo[i] == '\r' || extraInfo[i] == '\n')
		{
			return false;
		}
	}
	clubs.push_back(club.clone());
	return true;
}
void ClubService::showAllClubs()const
{
	if (clubs.empty())
	{
		cout << "暂无社团。" << endl;
		return;
	}
	for (int i = 0; i < clubs.size(); i++)
	{
		cout << "------------" << endl;
		cout << "编号：" << clubs[i]->getClubId() << endl;
		cout << "名称：" << clubs[i]->getClubName() << endl;
		cout << "简介：" << clubs[i]->getDescription() << endl;
		cout << "负责人：" << clubs[i]->getLeaderId() << endl;
		cout << "审核状态：" << clubs[i]->getStatus() << endl;
	}
}
void ClubService::showClubInfo(const string& clubId)const
{
	int index = findClubIndex(clubId);
	if (index == -1)
	{
		cout << "未找到该社团。" << endl;
		return;
	}
	cout << "------------" << endl;
	cout << "类型：" << clubs[index]->getType() << endl;
	string extraInfo = clubs[index]->getExtraInfo();
	if (!extraInfo.empty())
	{
		cout << "分类资料：" << extraInfo << endl;
	}
	cout << "编号：" << clubs[index]->getClubId() << endl;
	cout << "名称：" << clubs[index]->getClubName() << endl;
	cout << "简介：" << clubs[index]->getDescription() << endl;
	cout << "负责人：" << clubs[index]->getLeaderId() << endl;
	cout << "审核状态：" << clubs[index]->getStatus() << endl;
}
bool ClubService::modifyClubInfo(const string& operatorId, const string& clubId, const string& newName, const string& newDescription)
{
	int index = findClubIndex(clubId);
	if (index == -1)
	{
		return false;
	}
	//操作人必须是该社团负责人
	if (clubs[index]->getLeaderId() != operatorId)
	{
		return false;
	}
	//名称不能包含回车或换行
	for (int i = 0; i < newName.length(); i++)
	{
		if (newName[i] == '\r' || newName[i] == '\n')
		{
			return false;
		}
	}
	//简介不能包含回车或换行
	for (int i = 0; i < newDescription.length(); i++)
	{
		if (newDescription[i] == '\r' || newDescription[i] == '\n')
		{
			return false;
		}
	}
	//先验证并修改名称，名称无效就结束
	if (clubs[index]->rename(newName) == false)
	{
		return false;
	}
	clubs[index]->changeDescription(newDescription);
	return true;
}
bool ClubService::auditClub(const string& clubId, bool passed)
{
	int index = findClubIndex(clubId);
	if (index == -1)
	{
		return false;
	}
	return clubs[index]->audit(passed);
}
bool ClubService::joinClub(const string& userId, const string& clubId)
{
	int index = findClubIndex(clubId);
	if (index == -1)
	{
		return false;
	}
	return clubs[index]->addMember(userId);
}
bool ClubService::leaveClub(const string& userId, const string& clubId)
{
	int index = findClubIndex(clubId);
	if (index == -1)
	{
		return false;
	}
	return clubs[index]->removeMember(userId);
}
void ClubService::showClubMembers(const string& clubId)const
{
	int index = findClubIndex(clubId);
	if (index == -1)
	{
		cout << "未找到该社团。" << endl;
		return;
	}
	clubs[index]->showMembers();
}
bool ClubService::saveClubs(const string& fileName)const
{
	//分类资料也必须保持单行
	//先检查，再打开文件，避免检查失败时覆盖原文件
	for (int i = 0; i < clubs.size(); i++)
	{
		string extraInfo = clubs[i]->getExtraInfo();
		for (int j = 0; j < extraInfo.length(); j++)
		{
			if (extraInfo[j] == '\r' || extraInfo[j] == '\n')
			{
				return false;
			}
		}
	}
	ofstream file(fileName);
	if (!file.is_open())
	{
		return false;
	}
	for (int i = 0; i < clubs.size(); i++)
	{
		file << clubs[i]->getType() << '\n';
		file << clubs[i]->getExtraInfo() << '\n';
		file << clubs[i]->getClubId() << '\n';
		file << clubs[i]->getClubName() << '\n';
		file << clubs[i]->getDescription() << '\n';
		file << clubs[i]->getLeaderId() << '\n';
		file << clubs[i]->getStatus() << '\n';
	}
	file.close();
	//检测文件操作有没有报告错误，无则返回true，有错误则返回false
	return !file.fail();
}
bool ClubService::loadClubs(const string& fileName)
{
	ifstream file(fileName);
	if (!file.is_open())
	{
		return false;
	}
	//暂时读取结果，读取成功后才替换原列表
	ClubService loadedService;//专门检查读取的资料
	string type;
	string extraInfo;
	string id;
	string name;
	string intro;
	string leader;
	string savedStatus;
	while (getline(file,type ))
	{
		//编号之后必须还有四行资料
		if (!getline(file,extraInfo)||!getline(file,id)||!getline(file, name) || !getline(file, intro) || !getline(file, leader) || !getline(file, savedStatus))
		{
			return false;
		}
		Club* club = nullptr;
		//根据类型创建对应对象
		if (type == "体育社团")
		{
			club = new SportsClub(id, name, intro, leader, extraInfo);
		}
		else if (type == "学术社团")
		{
			club = new AcademicClub(id, name, intro, leader, extraInfo);
		}
		else if (type == "艺术社团")
		{
			club = new ArtClub(id, name, intro, leader, extraInfo);
		}
		else if (type == "普通社团")
		{
			if (!extraInfo.empty())
			{
				return false;
			}
			club = new Club(id, name, intro, leader);
		}
		else
		{
			return false;
		}
		//构造时默认pending，根据文件恢复审核状态
		if (savedStatus == "approved")
		{
			club->audit(true);
		}
		else if (savedStatus == "rejected")
		{
			club->audit(false);
		}
		else if (savedStatus != "pending")
		{
			delete club;
			return false;
		}
		//addedClub会检查资料，并保存一个独立副本
		bool added = loadedService.addClub(*club);
		//临时对象已被用完，无论添加是否成功，都需要释放
		delete club;
		club = nullptr;
		if (added == false)
		{
			return false;
		}
	}
	//正常到达文件末尾可以结束，其他错误则失败
	if (file.bad() || !file.eof())
	{
		return false;
	}
	clubs.swap( loadedService.clubs);
	return true;
}
bool ClubService::saveMembers(const string& fileName)const
{
	ofstream file(fileName);
	if (!file.is_open())
	{
		return false;
	}
	for (int i = 0; i < clubs.size(); i++)
	{
		vector<string>ids = clubs[i]->getMemberIds();
		for (int j = 0; j < ids.size(); j++)
		{
			file << clubs[i]->getClubId()
				<< " " << ids[j] << '\n';
		}
	}
	file.close();
	if (file.fail())
	{
		return false;
	}
	return true;
}
bool ClubService::loadMembers(const string& fileName)
{
	ifstream file(fileName);
	if (!file.is_open())
	{
		return false;
	}
	ClubService loadedService;
	//每个社团都创建独立副本，不能直接复制指针列表
	for (int i = 0; i < clubs.size(); i++)
	{
		loadedService.clubs.push_back(clubs[i]->clone());
		loadedService.clubs[i]->clearMembers();
	}
	string clubId;
	string userId;
	while (file >> clubId)
	{
		//读到了社团编号，就必须还能读到用户编号
		if (!(file >> userId))
		{
			return false;
		}
		int index = -1;
		for (int i = 0; i < loadedService.clubs.size(); i++)
		{
			if (loadedService.clubs[i]->getClubId() == clubId)
			{
				index = i;
				break;
			}
		}
		if (index == -1)
		{
			return false;
		}
		if (loadedService.clubs[index]->addMember(userId) == false)
		{
			return false;
		}
	}
	if (file.bad() || !file.eof())
		{
			return false;
		}
		clubs.swap( loadedService.clubs);
		return true;
}
void ClubService::showMyClubs(const string& userId)const
{
	bool found = false;
	for (int i = 0; i < clubs.size(); i++)
	{
		if (clubs[i]->hasMember(userId))
		{
			cout << "编号：" << clubs[i]->getClubId() << " 名称：" << clubs[i]->getClubName() << endl;
			found = true;
		}
	}
	if (found == false)
	{
		cout << "尚未加入任何社团。" << endl;
	}
}
void ClubService::showApprovedClubs()const
{
	bool found = false;
	for (int i = 0; i < clubs.size(); i++)
	{
		if (clubs[i]->getStatus() == "approved")
		{
			showClubInfo(clubs[i]->getClubId());
			found = true;
		}
	}
	if (found == false)
	{
		cout << "暂无审核通过的社团。" << endl;
	}
}
bool ClubService::deleteClub(const string& clubId)
{
	int index = findClubIndex(clubId);
	if (index == -1)
	{
		return false;
	}
	if (clubs[index]->getMemberCount() > 0)
	{
		return false;
	}
	//先释放社团对象，再移除列表中的指针
	delete clubs[index];
	clubs.erase(clubs.begin() + index);
	return true;
}
bool ClubService::removeClubMember(const string& operatorId, const string& clubId, const string& userId)
{
	int index = findClubIndex(clubId);
	if (index == -1)
	{
		return false;
	}
	if (clubs[index]->getLeaderId() != operatorId)
	{
		return false;
	}
	return clubs[index]->removeMember(userId);
}
bool ClubService::showManagedMembers(const string& operatorId, const string& clubId)const
{
	int index = findClubIndex(clubId);
	if (index == -1)
	{
		return false;
	}
	if (clubs[index]->getLeaderId() != operatorId)
	{
		return false;
	}
	clubs[index]->showMembers();
	return true;
}
bool ClubService::clubExists(const string& clubId)const
{
	return findClubIndex(clubId) != -1;
}
bool ClubService::isClubApproved(const string& clubId)const
{
	int index = findClubIndex(clubId);
	if (index == -1)
	{
		return false;
	}
	return clubs[index]->getStatus() == "approved";
}
bool ClubService::isClubLeader(const string& userId, const string& clubId)const
{
	int index = findClubIndex(clubId);
	if (index == -1)
	{
		return false;
	}
	return clubs[index]->getLeaderId() == userId;
}
void ClubService::searchApprovedClubs(const string& keyword)const
{
	if (keyword.empty())
	{
		cout << "请输入搜索关键词。" << endl;
		return;
	}
	bool found = false;
	for (int i = 0; i < clubs.size(); i++)
	{
		if (clubs[i]->getStatus() != "approved")
		{
			continue;
		}
		string name = clubs[i]->getClubName();
		bool matched = false;
		//从名称不同的位置开始尝试匹配
		for (int start =0 ; start + keyword.length() <= name.length(); start++)
		{
			bool same = true;
			//逐个比较关键词中的字符
			for (int j = 0; j < keyword.length(); j++)
			{
				if (name[start + j] != keyword[j])
				{
					same = false;
					break;
				}
			}
			if (same)
			{
				matched = true;
				break;
			}
		}
		if (matched)
		{
			showClubInfo(clubs[i]->getClubId());
			found = true;
		}
	}
	if (found == false)
	{
		cout << "没有找到符合条件的社团。" << endl;
	}
}