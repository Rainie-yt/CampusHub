#pragma once
#include<vector>
#include"club.h"
using namespace std;
//社团管理模块
//各菜单公用一个clubService，通过引用传递，不允许复制
//增删改操作只修改内容，不会自动保存文件
class ClubService
{
private:
	vector<Club*>clubs;

	//释放列表中由本类管理的对象
	void clearClubs();

	//暂时禁止复制管理对象，避免两个列表重复释放同一指针
	ClubService(const ClubService&);
	ClubService& operator=(const ClubService&);
public:
	//构造、析构函数
	ClubService();
	~ClubService();

	//-------学生菜单-------
	
	//显示已审核通过的社团，供学生浏览，直接输出结果
	void showApprovedClubs()const;

    //按名称关键词搜索已审核通过的社团
	//空关键词或没有匹配结果时输出提示
	void searchApprovedClubs(const string& keyword)const;

	//按编号显示社团资料，找不到时输出提示，不限制社团的审核状态
	void showClubInfo(const string& clubId)const;

	//按社团编号加入社团，本方法尚未检查用户是否真实存在
	//userId必须来自当前登录用户
	//社团不存在、未审核通过、编号非法或重复加入时返回false
	bool joinClub(const string& userId, const string& clubId);
	//按编号退出社团，社团不存在或用户不是成员时返回false
	bool leaveClub(const string& userId, const string& clubId);
    //显示指定用户加入的社团，没有加入的社团时输出提示
	void showMyClubs(const string& userId)const;

	//-------负责人菜单-------
	
	//修改名称和简介
	//只有社团负责人可以修改资料
	//社团不存在或名称无效时返回false，原资料保持不变
	bool modifyClubInfo(const string&operatorId,const string& clubId, 
		const string& newName, const string& newDescription);
	//负责人查看成员
	//查询成功返回true，社团不存在或无权限时返回false
	bool showManagedMembers(const string& operatorId, const string& clubId)const;
	//负责人移除成员
	//社团不存在、操作者不是负责人或成员不存在时返回false
	bool removeClubMember(const string& operatorId, const string& clubId,
		const string& userId);

	//-------创建与管理员操作-------
	
	//添加社团：编号重复或资料无效时返回false，否则添加并返回true
	bool addClub(const Club& club);
	//显示所有社团的基本资料
	void showAllClubs()const;
	//审核待审核的社团
	//passed:true表示通过，false表示拒绝
	//返回值表示操作是否成功，拒绝申请也可以返回true
	//社团不存在或已审核时返回false
	//调用前需要检查当前用户是否为管理员
	bool auditClub(const string& clubId, bool passed);
	//调用前需检查管理员身份
	//当前未检查关联活动，需与活动模块整合
	//删除无成员的社团
	//社团不存在或仍有成员时返回false
	bool deleteClub(const string& clubId);
	//-------公共查询-------
	//查询社团是否存在
	bool clubExists(const string& clubId)const;	
	//查询社团是否审核通过，不存在时也返回false
	bool isClubApproved(const string& clubId)const;
    //判断用户是否为该社团负责人
	//社团不存在时返回false
	bool isClubLeader(const string& userId, const string& clubId)const;
	//按编号查找社团
	//找到返回下标，找不到返回-1
	int findClubIndex(const string& clubId)const;
	//按社团编号查看成员
	// 本方法不检查操作者权限
	// 负责人菜单应调用showManagedMembers	
	void showClubMembers(const string& clubId)const;

	//-------文件保存与读取-------
	
	//保存社团基本资料，成功返回true，失败返回false
	//每个社团固定七行：类型、分类资料、编号、名称、简介、负责人编号、审核状态
	bool saveClubs(const string& fileName)const;
	//从文件读取社团基本资料
	//成功返回true，替换社团列表，不恢复成员；文件无法打开或资料有错误时返回false，保留原数据
	bool loadClubs(const string& fileName);
	//保存社团成员关系，成功返回true，失败返回false
	bool saveMembers(const string& fileName)const;
	//必须先读取社团
	// 从文件恢复成员名单，成功返回true，替换成员名单，失败保留原名单
	bool loadMembers(const string& fileName);	
};
