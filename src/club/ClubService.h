#pragma once
#include<vector>
#include"club.h"
using namespace std;
class ActivityManager;
//提供给其他模块的社团简要资料
struct ClubBrief
{
	string clubId;
	string clubName;
	string description;
	string type;
	string extraInfo;
	string leaderId;
	int memberCount;
};
struct ClubActivityBrief
{
	string clubId;
	string clubName;
	int memberCount;
	int activityCount;
	int registrationCount;
	int score;
};
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
	//旧的直接加入方法，仅限类内部使用
	//外部统一使用applyToJoin和approveApplication
	bool joinClub(const string& userId, const string& clubId);
public:
	//构造、析构函数
	ClubService();
    void copyStateTo(ClubService& target)const;
    void swapState(ClubService& other);
    // 解散并归档：清理成员和申请，保留社团资料；关联活动及角色由管理员协调处理。
    bool dissolveClub(const string& clubId);
    string getClubStatus(const string& clubId)const;
	~ClubService();

	//-------学生菜单-------

	//显示已审核通过的社团，供学生浏览，直接输出结果
	void showApprovedClubs()const;
    //按名称关键词搜索已审核通过的社团
	//空关键词或没有匹配结果时输出提示
	void searchApprovedClubs(const string& keyword)const;
	//按编号显示社团资料，找不到时输出提示，不限制社团的审核状态
	void showClubInfo(const string& clubId)const;

	//按编号退出社团，社团不存在或用户不是成员时返回false
	bool leaveClub(const string& userId, const string& clubId);
    //显示指定用户加入的社团，没有加入的社团时输出提示
	void showMyClubs(const string& userId)const;
	//学生申请加入社团，userId来自当前登录用户
	//社团不存在、未审核通过、编号非法、已是成员、已申请或申请人是负责人本人时返回false
	bool applyToJoin(const string& userId, const string& clubId);
	//显示该用户尚未审核的加入申请
	//userId来自当前登录用户，没有申请时输出提示
	void showMyApplications(const string& userId)const;
	//按类型显示已审核通过的社团
	//类型为空或没有匹配结果时输出提示
	void searchApprovedClubsByType(const string& type)const;
	//按负责人编号显示已审核通过的社团
	//编号为空或没有找到时输出提示
	void searchApprovedClubByLeader(const string& leaderId)const;

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
	//负责人通过加入申请；reason 可选，用于接收失败原因，成功时清空
	//社团不存在、非负责人或没有该申请时返回false
	//成功后将申请人加入成员名单，并移除待审核申请
	bool approveApplication(const string& operatorId,
		const string& clubId, const string& userId, string* reason=nullptr);
	//负责人拒绝加入申请；reason 可选，用于接收失败原因，成功时清空
	//社团不存在、非负责人或申请不存在时返回false
	//成功后移除申请，不增加成员
	bool rejectApplication(const string& operatorId,
		const string& clubId, const string& userId, string* reason=nullptr);
	//负责人查看待审核的加入申请
	//社团不存在或非负责人时返回false
	//有权限时显示名单并返回true，名单为空时也返回true
	bool showApplications(const string& operatorId,
		const string& clubId)const;

	//-------创建与管理员操作-------

	//添加社团：编号重复或资料无效时返回false，否则添加并返回true
	bool addClub(const Club& club);
	//显示所有社团的基本资料
	void showAllClubs()const;
	//审核待审核的社团；reason 可选，用于接收失败原因，成功时清空
	//passed:true表示通过，false表示拒绝
	//返回值表示操作是否成功，拒绝申请也可以返回true
	//社团不存在或已审核时返回false
	//调用前需要检查当前用户是否为管理员
	bool auditClub(const string& clubId, bool passed, string* reason=nullptr);
    // 更换已通过社团的负责人，新负责人必须是本社团成员且没有其他待审核或已通过的社团。
    // 只修改内存，账号身份检查、角色修改和保存由管理员协调完成。
    bool changeClubLeader(const string& clubId, const string& newLeaderId);
	// 旧的删除方法，目前菜单里没有用。这里只删社团，不处理活动和负责人。
	// 管理员解散社团时用 dissolveClub，才能保留历史记录。
	// 社团不存在或仍有成员时返回false
	bool deleteClub(const string& clubId);
	//申请成立社团，成功返回新编号，失败返回空字符串
	//一个用户不能同时负责多个待审核或已通过的社团
	//只修改内存，不自动保存文件
	string applyClub(const string& applicantId, const string& type,
		const string& name, const string& intro, const string& extraInfo);

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
	//按社团编号获取负责人编号
	//社团不存在时返回空字符串
	string getClubLeaderId(const string& clubId)const;
	//查找用户负责的、已审核通过的社团
	//没有找到时返回空字符串
	string findClubByLeader(const string& userId)const;
	//查询用户负责的社团，包括待审核和已通过的社团，不包括被拒绝的社团
	// 没有找到时返回空字符串
	string findAnyClubByLeader(const string& userId)const;
	//返回未解散的社团数量，包括待审核、已通过和被拒绝的社团，不计已解散归档
	int getClubCount()const;
	//返回所以审核通过社团的资料副本
	//没有符合条件的社团时返回空列表
	vector<ClubBrief>getApprovedClubBriefs()const;
	//返回成员编号列表副本，社团不存在时返回空列表
	//不检查权限，负责人菜单调用前需要验证身份
	vector<string>getClubMemberIds(const string& clubId)const;
	//返回待审核申请人编号列表副本，社团不存在时返回空列表
	vector<string>getClubApplicantIds(const string& clubId)const;

	//-------文件保存与读取-------

	//保存社团基本资料，成功返回true，失败返回false
	//每个社团固定七行：类型、分类资料、编号、名称、简介、负责人编号、审核状态
	bool saveClubs(const string& fileName)const;
	//从文件读取社团基本资料
	//文件不存在时按空列表；存在但读取/解析失败时返回false并报告位置，保留原数据
	//成功时替换社团列表，不恢复成员；忽略记录之间的空行，记录内部固定七行
	bool loadClubs(const string& fileName);
	//保存社团成员关系，成功返回true，失败返回false
	bool saveMembers(const string& fileName)const;
	//必须先读取社团
	// 每行两个编号，空行可忽略；文件不存在按空名单，失败报告行号并保留原名单
	bool loadMembers(const string& fileName);
	//保存待审核申请，覆盖同名文件
	//每行：社团编号 用户编号
	//成功返回true，失败返回false
	bool saveApplications(const string& fileName)const;
	//恢复待审核申请，必须先读取社团和成员
	//文件不存在按空名单；成功后替换，读取/解析失败报告行号并保留原数据
	//不存在的社团、负责人本人、已是社团成员或重复申请均视为错误
	bool loadApplications(const string& fileName);

	//------社团活跃度-------
	int calculateActivityScore(int memberCount,
		int activityCount,int registrationCount)const;
	void sortActivityRank(vector<ClubActivityBrief>& rankList)const;
	bool addActivityRankRecord(vector<ClubActivityBrief>&rankList,const string& clubId,
		int activityCount, int registrationCount)const;
	void showActivityRank(const vector<ClubActivityBrief>& rankList)const;
	//使用活动模块的真实数据生成已通过社团的活跃度排行榜
	vector<ClubActivityBrief> getActivityRank(
		const ActivityManager& activityManager)const;
};
