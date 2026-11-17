#include"ClubService.h"
#include"../activity/ActivityManager.h"
#include"../common/TextUtils.h"
#include"../common/FileStore.h"
#include"../common/DataFile.h"
#include<sstream>
#include<iostream>
#include<fstream>
#include"SportsClub.h"
#include"AcademicClub.h"
#include"ArtClub.h"
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
//遍历社团列表，找到指定编号就返回下标，找不到返回-1
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
		cout << "========社团列表========" << endl;
		cout << "暂无社团。" << endl;
		return;
	}
	cout << "========社团列表========" << endl;
	//名称列宽度取最长的"名称（类型）"再加2格，让各列对齐
	int nameWidth = 20;
	for (int i = 0; i < clubs.size(); i++)
	{
		int w = displayWidth(clubs[i]->getClubName() + "（" + clubs[i]->getType() + "）") + 2;
		if (w > nameWidth)
		{
			nameWidth = w;
		}
	}
	for (int i = 0; i < clubs.size(); i++)
	{
		string statusText = clubs[i]->getStatus();
		if(statusText=="pending")
		{
			statusText = "待审核";
		}
		else if (statusText == "approved")
		{
			statusText = "已通过";
		}
		else if (statusText == "dissolved")
        {
            statusText = "已解散（历史归档）";
        }
        else if (statusText == "rejected")
		{
			statusText = "已拒绝";
		}
		cout << padText(clubs[i]->getClubId(), 6)
			<< padText(clubs[i]->getClubName() + "（" + clubs[i]->getType() + "）", nameWidth)
			<< padText(clubs[i]->getLeaderId(), 6)
			<< statusText << endl;
	}
	cout << endl;
	cout << "共" << clubs.size() << "条社团记录（含已解散归档）" << endl;
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
	string statusText = clubs[index]->getStatus();
	if(statusText=="pending")
	{
		statusText = "待审核";
	}
	else if (statusText == "approved")
	{
		statusText = "已通过";
	}
	else if (statusText == "dissolved")
        {
            statusText = "已解散（历史归档）";
        }
        else if (statusText == "rejected")
	{
		statusText = "已拒绝";
	}
	cout << "审核状态：" <<statusText << endl;
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
/*
函数名：auditClub
功能：审核社团成立申请，检查失败时通过 reason 说明原因
说明：只修改内存；账号角色及文件保存由管理员协调，保存失败的原因由菜单补充。
*/
bool ClubService::auditClub(const string& clubId, bool passed, string* reason)
{
    if(reason) reason->clear();
    int index=findClubIndex(clubId);
    if(index==-1){
        if(reason) *reason="社团不存在。";
        return false;
    }
    if(clubs[index]->getStatus()!="pending"){
        if(reason) *reason="该社团不是待审核状态。";
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
	//负责人不能通过退出社团来解除负责人身份
	if (clubs[index]->getLeaderId() == userId)
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
	ostringstream file;
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

	//检测文件操作有没有报告错误，无则返回true，有错误则返回false
	return FileStore::write(fileName,file.str());
}
/*
函数名：loadClubs
功能：从文件恢复社团资料和审核状态
参数：fileName 社团资料文件的路径
返回值：全部读取成功返回 true；记录缺行、类型或状态错误、资料不合法等返回 false。
实现步骤：
    1. 完整读入文件，再按每条七行的格式读取社团资料；
    2. 根据社团类型创建对象，恢复审核状态，添加到临时管理器；
    3. 全部记录通过检查后，再用临时列表替换原来的社团列表。
关键点：只跳过记录之间的空行，普通社团第二行的空分类资料不能跳过。
        读取失败时原列表不变，不能拿半份数据继续保存。
*/
bool ClubService::loadClubs(const string& fileName)
{
	string text;
	if (!DataFile::read(fileName, text)) return false;
	istringstream file(text);
	size_t lineNo = 0;
	//暂时读取结果，读取成功后才替换原列表
	ClubService loadedService;//专门检查读取的资料
	string type;
	string extraInfo;
	string id;
	string name;
	string intro;
	string leader;
	string savedStatus;
	while (DataFile::nextLine(file, type, lineNo))
	{
		// 只在记录之间忽略空行；记录内部的七行结构仍必须完整。
		if (DataFile::blank(type)) continue;
		const size_t recordLine = lineNo;
		if (!DataFile::nextLine(file, extraInfo, lineNo)
			|| !DataFile::nextLine(file, id, lineNo)
			|| !DataFile::nextLine(file, name, lineNo)
			|| !DataFile::nextLine(file, intro, lineNo)
			|| !DataFile::nextLine(file, leader, lineNo)
			|| !DataFile::nextLine(file, savedStatus, lineNo))
			return DataFile::error(fileName, recordLine, "社团记录不完整，每条记录需要七行");
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
				return DataFile::error(fileName, recordLine + 1, "普通社团的分类资料应为空");
			}
			club = new Club(id, name, intro, leader);
		}
		else
		{
			return DataFile::error(fileName, recordLine, "未知社团类型");
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
		else if (savedStatus == "dissolved")
        {
            club->dissolve();
        }
        else if (savedStatus != "pending")
		{
			delete club;
			return DataFile::error(fileName, recordLine + 6, "未知审核状态");
		}
		//addedClub会检查资料，并保存一个独立副本
		bool added = loadedService.addClub(*club);
		//临时对象已被用完，无论添加是否成功，都需要释放
		delete club;
		club = nullptr;
		if (added == false)
		{
			return DataFile::error(fileName, recordLine, "社团编号重复或基本资料不合法");
		}
	}
	clubs.swap( loadedService.clubs);
	return true;
}
bool ClubService::saveMembers(const string& fileName)const
{
	ostringstream file;
	for (int i = 0; i < clubs.size(); i++)
	{
		vector<string>ids = clubs[i]->getMemberIds();
		for (int j = 0; j < ids.size(); j++)
		{
			file << clubs[i]->getClubId()
				<< " " << ids[j] << '\n';
		}
	}

	if (file.fail())
	{
		return false;
	}
	return FileStore::write(fileName,file.str());
}
/*
函数名：loadMembers
功能：恢复各社团的成员名单，调用前必须先加载社团资料
参数：fileName 成员文件的路径，每行格式为“社团编号 用户编号”
返回值：全部记录有效返回 true；字段错误、社团不存在或成员不能加入时返回 false。
实现步骤：
    1. 复制每个社团到临时管理器，只清空副本中的成员名单；
    2. 逐行检查两个编号，找到社团后调用 addMember 恢复成员；
    3. 全部通过后换入新列表，出错则直接返回，原名单保留。
关键点：这里要 clone 出独立对象，不能复制指针后直接清空，否则原名单也会被清空。
*/
bool ClubService::loadMembers(const string& fileName)
{
	string text;
	if (!DataFile::read(fileName, text)) return false;
	istringstream file(text);
	ClubService loadedService;
	//每个社团都创建独立副本，不能直接复制指针列表
	for (int i = 0; i < clubs.size(); i++)
	{
		loadedService.clubs.push_back(clubs[i]->clone());
		loadedService.clubs[i]->clearMembers();
	}
	string line;
	size_t lineNo = 0;
	while (DataFile::nextLine(file, line, lineNo))
	{
		if (DataFile::blank(line)) continue;
		istringstream row(line);
		string clubId, userId, extra;
		if (!(row >> clubId >> userId) || (row >> extra))
			return DataFile::error(fileName, lineNo, "成员记录应为社团编号和用户编号两项");
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
			return DataFile::error(fileName, lineNo, "成员记录指向不存在的社团");
		}
		if (loadedService.clubs[index]->addMember(userId) == false)
		{
			return DataFile::error(fileName, lineNo, "成员重复、用户编号不合法或社团未通过审核");
		}
	}
	clubs.swap(loadedService.clubs);
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
		cout << "你尚未加入任何社团。" << endl;
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
string ClubService::applyClub(const string& applicantId, const string& type,
	const string& name, const string& intro, const string& extraInfo)
{
	if (applicantId.empty())
	{
		return"";
	}
	for (int i = 0; i < clubs.size(); i++)
	{
		if (clubs[i]->getLeaderId() == applicantId)
		{
			string status = clubs[i]->getStatus();
			if (status == "pending" || status == "approved")
			{
				return "";
			}
		}
	}
	string maxNumber = "0";
	for (int i = 0; i < clubs.size(); i++)
	{
		string id = clubs[i]->getClubId();
		//只统计C开头，且后面全部是数字编号
		if (id.length() < 2 || id[0] != 'C')
		{
			continue;
		}
		bool allDigits = true;
		string number = "";
		for (int j = 1; j < id.length(); j++)
		{
			if (id[j] < '0' || id[j]>'9')
			{
				allDigits = false;
				break;
			}
			//去掉开头的零，方便比较数字大小
			if (id[j] != '0' || !number.empty())
			{
				number += id[j];
			}
		}
		if (allDigits == false)
		{
			continue;
		}
		if (number.empty())
		{
			number = "0";
		}
		if (number.length() > maxNumber.length())
		{
			maxNumber = number;
		}
		else if (number.length() == maxNumber.length() && number > maxNumber)
		{
			maxNumber = number;
		}
	}
	string nextNumber = maxNumber;
		int position = nextNumber.length() - 1;
		//末尾是9，就改为0，继续向前进位
		while (position >= 0 && nextNumber[position] == '9')
		{
			nextNumber[position] = '0';
			position--;
		}
		if (position >= 0)
		{
			nextNumber[position]++;
		}
		else
		{
			nextNumber = "1" + nextNumber;
		}
		//至少补足三位
		while (nextNumber.length() < 3)
		{
			nextNumber = "0" + nextNumber;
		}
		string newId = "C" + nextNumber;
		Club* club = nullptr;
		if (type == "体育社团")
		{
			club = new SportsClub(newId, name, intro, applicantId, extraInfo);
		}
		else if (type == "学术社团")
		{
			club = new AcademicClub(newId, name, intro, applicantId, extraInfo);
		}
		else if (type == "艺术社团")
		{
			club = new ArtClub(newId, name, intro, applicantId, extraInfo);
		}
		else if (type == "普通社团")
		{
			if (!extraInfo.empty())
			{
				return"";
			}
			club = new Club(newId, name, intro, applicantId);
		}
		else
		{
			return"";
		}
		//addClub检查资料，通过clone保存独立副本
		bool added = addClub(*club);
		//临时对象使用完后释放
		delete club;
		club = nullptr;
		if (added == false)
		{
			return"";
		}
		return newId;
}
string ClubService::getClubLeaderId(const string& clubId)const
{
	int index = findClubIndex(clubId);
	if (index == -1)
	{
		return"";
	}
	return clubs[index]->getLeaderId();
}
string ClubService::findClubByLeader(const string& userId)const
{
	for (int i = 0; i < clubs.size(); i++)
	{
		if (clubs[i]->getLeaderId() == userId && clubs[i]->getStatus() == "approved")
		{
			return clubs[i]->getClubId();
		}
	}
	return "";
}
string ClubService::findAnyClubByLeader(const string& userId)const
{
	for(int i = 0; i < clubs.size(); i++)
	{
		if (clubs[i]->getLeaderId() == userId
			&& (clubs[i]->getStatus() == "pending"
			||clubs[i]->getStatus()=="approved"))
		{
			return clubs[i]->getClubId();
		}
	}
	return "";
}
bool ClubService::applyToJoin(const string& userId, const string& clubId)
{
	int index = findClubIndex(clubId);
	if (index == -1)
	{
		return false;
	}
	//负责人不需要申请加入自己的社团
	if (clubs[index]->getLeaderId() == userId)
	{
		return false;
	}
	//其余检查由Club:addApplicant完成
	return clubs[index]->addApplicant(userId);
}
/*
函数名：approveApplication
功能：通过入社申请，把申请人加入成员名单，再移除申请
说明：reason 接收失败原因；成为成员失败时保留申请，成功后由调用者保存文件。
*/
bool ClubService::approveApplication(const string& operatorId,
    const string& clubId, const string& userId, string* reason)
{
    if(reason) reason->clear();
    int index=findClubIndex(clubId);
    if(index==-1){
        if(reason) *reason="社团不存在。";
        return false;
    }
    if(clubs[index]->getLeaderId()!=operatorId){
        if(reason) *reason="无权限：只有本社团负责人可以审核加入申请。";
        return false;
    }
    if(clubs[index]->getStatus()!="approved"){
        if(reason) *reason="社团未通过审核或已解散，不能通过加入申请。";
        return false;
    }
    if(!clubs[index]->hasApplicant(userId)){
        if(reason) *reason="该用户不在待审核申请名单中。";
        return false;
    }
    if(clubs[index]->hasMember(userId)){
        if(reason) *reason="该用户已经是社团成员。";
        return false;
    }
    if(!clubs[index]->addMember(userId)){
        if(reason) *reason="申请人编号无效，无法加入成员名单。";
        return false;
    }
    clubs[index]->removeApplicant(userId);
    return true;
}
/*
函数名：rejectApplication
功能：拒绝入社申请并从待审核名单中移除
说明：reason 接收社团不存在、无权限或申请不存在等失败原因；不在这里保存文件。
*/
bool ClubService::rejectApplication(const string& operatorId,
    const string& clubId, const string& userId, string* reason)
{
    if(reason) reason->clear();
    int index=findClubIndex(clubId);
    if(index==-1){
        if(reason) *reason="社团不存在。";
        return false;
    }
    if(clubs[index]->getLeaderId()!=operatorId){
        if(reason) *reason="无权限：只有本社团负责人可以审核加入申请。";
        return false;
    }
    if(!clubs[index]->hasApplicant(userId)){
        if(reason) *reason="该用户不在待审核申请名单中。";
        return false;
    }
    return clubs[index]->removeApplicant(userId);
}
bool ClubService::showApplications(const string& operatorId,
	const string& clubId)const
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
	vector<string>applicants = clubs[index]->getApplicantIds();
	if (applicants.empty())
	{
		cout << "暂无待审核申请。" << endl;
		return true;
	}
	cout << "待审核申请人：" << endl;
	for (int i = 0; i < applicants.size(); i++)
	{
		cout << i + 1 << "." << applicants[i] << endl;
	}
	return true;
}
void ClubService::showMyApplications(const string& userId)const
{
	bool found = false;
	for (int i = 0; i < clubs.size(); i++)
	{
			if (clubs[i]->hasApplicant(userId))
			{
				cout << "编号：" << clubs[i]->getClubId()<<endl
					<< "名称：" << clubs[i]->getClubName()<<endl
					<< "状态：待审核" << endl;
				found = true;
			}
	}
	if (found == false)
	{
		cout << "暂无待审核的加入申请。" << endl;
	}
}
bool ClubService::saveApplications(const string& fileName)const
{
	ostringstream file;
	for (int i = 0; i < clubs.size(); i++)
	{
		vector<string>applicants = clubs[i]->getApplicantIds();
		for (int j = 0; j < applicants.size(); j++)
		{
			file << clubs[i]->getClubId() << " " << applicants[j ]<< '\n';
		}
	}

	if (file.fail())
	{
		return false;
	}
	return FileStore::write(fileName,file.str());
}
/*
函数名：loadApplications
功能：恢复尚未审核的入社申请，调用前必须先加载社团和成员
参数：fileName 申请文件的路径，每行格式为“社团编号 用户编号”
返回值：全部恢复成功返回 true；字段错误或申请不符合规则时返回 false。
实现步骤：
    1. 复制社团到临时管理器，只清空副本中的申请名单；
    2. 每行读取两个编号，调用 applyToJoin 检查并恢复申请；
    3. 全部成功才替换正式列表，有一条错误就保留原数据并报告行号。
说明：使用已有申请入口，可以一并检查社团状态、重复申请、已是成员等情况。
*/
bool ClubService::loadApplications(const string& fileName)
{
	string text;
	if (!DataFile::read(fileName, text)) return false;
	istringstream file(text);
	ClubService loadedService;
	//创建独立副本，只清空副本中的申请名单
	for (int i = 0; i < clubs.size(); i++)
	{
		loadedService.clubs.push_back(clubs[i]->clone());
		loadedService.clubs[i]->clearApplicants();
	}
	string line;
	size_t lineNo = 0;
	while (DataFile::nextLine(file, line, lineNo))
	{
		if (DataFile::blank(line)) continue;
		istringstream row(line);
		string clubId, userId, extra;
		if (!(row >> clubId >> userId) || (row >> extra))
			return DataFile::error(fileName, lineNo, "申请记录应为社团编号和用户编号两项");
		//复用申请入口，检查社团、成员和负责人等规则
		if (loadedService.applyToJoin(userId, clubId) == false)
		{
			return DataFile::error(fileName, lineNo, "社团不存在、未通过审核，或申请人重复、为负责人或已有成员");
		}
	}
	clubs.swap(loadedService.clubs);
	return true;
}
// 已解散社团保留在文件中作为历史，但不计入当前社团数量。
int ClubService::getClubCount()const
{
    int count=0;
    for(size_t i=0;i<clubs.size();i++){
        if(clubs[i]->getStatus()!="dissolved") ++count;
    }
    return count;
}
vector<ClubBrief>ClubService::getApprovedClubBriefs()const
{
	vector<ClubBrief>result;
	for (int i = 0; i < clubs.size(); i++)
	{
		if (clubs[i]->getStatus() == "approved")
		{
			ClubBrief brief;
			brief.clubId = clubs[i]->getClubId();
			brief.clubName = clubs[i]->getClubName();
			brief.description = clubs[i]->getDescription();
			brief.type = clubs[i]->getType();
			brief.extraInfo = clubs[i]->getExtraInfo();
			brief.leaderId = clubs[i]->getLeaderId();
			brief.memberCount = clubs[i]->getMemberCount();
			result.push_back(brief);
		}
	}
	return result;
}
vector<string>ClubService::getClubMemberIds(const string& clubId)const
{
	int index = findClubIndex(clubId);
	if (index == -1)
	{
		vector<string>emptyList;
		return emptyList;
	}
	return clubs[index]->getMemberIds();
}
vector<string>ClubService::getClubApplicantIds(const string& clubId)const
{
	int index = findClubIndex(clubId);
	if (index == -1)
	{
		vector<string>emptyList;
		return emptyList;
	}
	return clubs[index]->getApplicantIds();
}
void ClubService::searchApprovedClubsByType(const string& type)const
{
	if (type.empty())
	{
		cout << "请输入社团类型。" << endl;
		return;
	}
	bool found = false;
	for (int i = 0; i < clubs.size(); i++)
		{
			if (clubs[i]->getStatus() == "approved"
				&& clubs[i]->getType() == type)
			{
				showClubInfo(clubs[i]->getClubId());
				found = true;
			}
		}
	if (found == false)
	{
		cout << "没有找到该类型的已审核通过社团。" << endl;
	}
}
void ClubService::searchApprovedClubByLeader(const string& leaderId)const
{
	if (leaderId.empty())
	{
		cout << "请输入负责人编号。" << endl;
		return;
	}
	string clubId = findClubByLeader(leaderId);
	if (clubId.empty())
	{
		cout << "没有找到该负责人负责的已审核通过社团。" << endl;
		return;
	}
	showClubInfo(clubId);
}
int ClubService::calculateActivityScore(int memberCount,
	int activityCount, int registrationCount)const
{
	if (memberCount < 0 || activityCount < 0 || registrationCount < 0)
	{
		return -1;
	}
	int score = memberCount
		+ activityCount * 10
		+ registrationCount * 2;
	return score;
}
void ClubService::sortActivityRank(vector<ClubActivityBrief>& rankList)const
{
	//先记录数量，避免后面的减法涉及无符号数
	int count = 0;
	for (int i = 0; i < rankList.size(); i++)
	{
		count++;
	}
	for (int i = 0; i < count - 1; i++)
	{
		for (int j = 0; j < count - 1 - i; j++)
		{
			if (rankList[j].score < rankList[j + 1].score)
			{
				ClubActivityBrief temp = rankList[j];
				rankList[j] = rankList[j + 1];
				rankList[j + 1] = temp;
			}
		}
	}
}
bool ClubService::addActivityRankRecord(vector<ClubActivityBrief>& rankList,
	const string& clubId, int activityCount,
	int registrationCount)const
{
	int index = findClubIndex(clubId);
	if (index == -1)
	{
		return false;
	}
	if (clubs[index]->getStatus() != "approved")
	{
		return false;
	}
	if (activityCount < 0 || registrationCount < 0)
	{
		return false;
	}
	//同一个列表中不能重复添加同一个社团
	for (int i = 0; i < rankList.size(); i++)
	{
		if (rankList[i].clubId == clubId)
		{
			return false;
		}
	}
	ClubActivityBrief record;
	record.clubId = clubs[index]->getClubId();
	record.clubName = clubs[index]->getClubName();
	record.memberCount = clubs[index]->getMemberCount();
	record.activityCount = activityCount;
	record.registrationCount = registrationCount;
	record.score = calculateActivityScore(record.memberCount,
		record.activityCount, record.registrationCount);
	rankList.push_back(record);
	return true;
}
void ClubService::showActivityRank
(const vector<ClubActivityBrief>& rankList)const
{
	cout << "========社团活跃度排行========" << endl;
	if (rankList.empty())
	{
		cout << "暂无已通过的社团。" << endl;
		return;
	}
	//名称列宽度取最长的社团名称再加2格
	int nameWidth = 12;
	for (int i = 0; i < rankList.size(); i++)
	{
		int w = displayWidth(rankList[i].clubName) + 2;
		if (w > nameWidth)
		{
			nameWidth = w;
		}
	}
	//第一行是表头，下面每个社团一行
	cout << padText("名次", 6) << padText("编号", 6) << padText("名称", nameWidth)
		<< padText("成员数", 8) << padText("活动数", 8) << padText("报名人次", 10) << "活跃度" << endl;
	for (int i = 0; i < rankList.size(); i++)
	{
		cout << padText(to_string(i + 1), 6)
			<< padText(rankList[i].clubId, 6)
			<< padText(rankList[i].clubName, nameWidth)
			<< padText(to_string(rankList[i].memberCount), 8)
			<< padText(to_string(rankList[i].activityCount), 8)
			<< padText(to_string(rankList[i].registrationCount), 10)
			<< rankList[i].score << endl;
	}
	cout << endl;
	cout << "活跃度 = 成员数 + 活动数×10 + 报名人次×2" << endl;
}
vector<ClubActivityBrief> ClubService::getActivityRank(const ActivityManager&activityManager)const
{
	vector<ClubActivityBrief>rankList;
	for (int i = 0; i < clubs.size(); i++)
	{
		//只统计已审核通过的社团
		if (clubs[i]->getStatus() == "approved")
		{
			string clubId = clubs[i]->getClubId();
			//调用C模块提供的真实统计接口
			int activityCount = activityManager.getActivityCount(clubId);
			int registrationCount = activityManager.getParticipantCount(clubId);
			addActivityRankRecord(rankList, clubId,
				activityCount, registrationCount);
		}
	}
	sortActivityRank(rankList);
	return rankList;
}


/*
函数名：copyStateTo
功能：把社团、成员和申请复制到另一个管理器，供保存失败时恢复
参数：target 接收副本的管理器；与当前对象相同时直接返回
实现步骤：先释放 target 原有的社团，再逐个调用 clone 深复制，避免两个列表共用指针。
*/
void ClubService::copyStateTo(ClubService& target)const
{
	if (&target == this)
	{
		return;
	}
	target.clearClubs();
	for (const Club* club : clubs)
	{
		target.clubs.push_back(club->clone());
	}
}
/*
函数名：swapState
功能：交换两个管理器的社团列表，包含成员和加入申请
参数：other 保存备份的管理器
实现说明：交换列表而不是逐项赋值；原状态交给 other，由其析构时释放。
*/
void ClubService::swapState(ClubService& other)
{
	clubs.swap(other.clubs);
}
/*
函数名：getClubStatus
功能：按编号查询社团审核或归档状态
参数：clubId 社团编号
返回值：社团不存在时返回空字符串，否则返回该社团的状态。
实现说明：先查下标，再读取状态，避免用无效下标访问列表。
*/
string ClubService::getClubStatus(const string& clubId)const
{
	int i = findClubIndex(clubId);
	return i < 0 ? "" : clubs[i]->getStatus();
}
/*
函数名：dissolveClub
功能：解散并归档社团；关联活动、用户角色和文件保存由调用者处理
参数：clubId 社团编号
返回值：社团不存在或已解散时返回 false，归档成功返回 true。
实现说明：先检查状态，再调用社团对象的 dissolve 清理成员、申请并设置归档状态。
*/
bool ClubService::dissolveClub(const string& clubId)
{
	int i = findClubIndex(clubId);
	if (i < 0 || clubs[i]->getStatus() == "dissolved")
	{
		return false;
	}
	clubs[i]->dissolve();
	return true;
}

/*
函数名：changeClubLeader
功能：更换社团负责人，防止新负责人同时负责或申请其他社团
参数：clubId 社团编号；newLeaderId 新负责人编号
返回值：社团不存在、未通过、新负责人不符合条件时返回 false
*/
bool ClubService::changeClubLeader(const string& clubId, const string& newLeaderId){
    int index=findClubIndex(clubId);
    if(index==-1 || !findAnyClubByLeader(newLeaderId).empty()) return false;
    return clubs[index]->changeLeader(newLeaderId);
}
